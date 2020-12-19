#include "config.h"

#if ENABLE(VIDEO)

#include "AcinerellaContainer.h"
#include "AcinerellaAudioDecoder.h"
#include "AcinerellaBuffer.h"

#define D(x) 

namespace WebCore {
namespace Acinerella {

Acinerella::Acinerella(AcinerellaClient *client, const String &url)
	: m_client(client)
	, m_url(url)
{
	D(dprintf("%s: %p\n", __func__, this));
	m_networkBuffer = AcinerellaNetworkBuffer::create(m_url);
	m_enableAudio = client->accEnableAudio();
	m_enableVideo = client->accEnableVideo();
	m_thread = Thread::create("Acinerella", [this] {
		threadEntryPoint();
	});
}

void Acinerella::play()
{
	m_paused = false;
	dispatch([this] {
		if (m_audioDecoder)
			m_audioDecoder->play();
		if (m_videoDecoder)
			m_videoDecoder->play();
	});
}

void Acinerella::pause()
{
	m_paused = true;
	dispatch([this] {
		if (m_audioDecoder)
			m_audioDecoder->pause();
		if (m_videoDecoder)
			m_videoDecoder->pause();
	});
}

bool Acinerella::paused()
{
	return m_paused;
}

void Acinerella::setVolume(float volume)
{
	m_volume = volume;
	dispatch([this, volume] {
		if (m_audioDecoder)
			m_audioDecoder->setVolume(volume);
	});
}

void Acinerella::terminate()
{
	D(dprintf("ac%s: %p\n", __func__, this));
	m_terminating = true;
	m_client = nullptr;

	// Prevent muxer from obtaining more data
	if (m_networkBuffer)
		m_networkBuffer->stop();

	D(dprintf("ac%s: %p network done\n", __func__, this));

	if (!m_thread)
		return;
	ASSERT(isMainThread());
	ASSERT(!m_queue.killed() && m_thread);

	if (!m_thread)
		return;

	m_queue.append(makeUnique<Function<void ()>>([this] {
		performTerminate();
	}));
	m_thread->waitForCompletion();
	ASSERT(m_queue.killed());
	m_thread = nullptr;
	
	m_networkBuffer = nullptr;

	D(dprintf("ac%s: %p done\n", __func__, this));
}

void Acinerella::threadEntryPoint()
{
	D(dprintf("%s: %p\n", __func__, this));
	if (initialize())
	{
		while (auto function = m_queue.waitForMessage())
		{
			(*function)();
		}
	}

	if (m_videoDecoder)
		m_videoDecoder->terminate();

	D(dprintf("ac%s: %p video done\n", __func__, this));

	if (m_audioDecoder)
		m_audioDecoder->terminate();

	D(dprintf("ac%s: %p audio done\n", __func__, this));

	if (m_muxer)
		m_muxer->terminate();

	D(dprintf("ac%s: %p muxer done\n", __func__, this));

	{
		auto lock = holdLock(m_acinerellaLock);
		m_acinerella = nullptr;
		m_muxer = nullptr;
	}

	D(dprintf("%s: %p exits...\n", __func__, this));
}

void Acinerella::dispatch(Function<void ()>&& function)
{
	ASSERT(isMainThread());
	ASSERT(!m_queue.killed() && m_thread);
	m_queue.append(makeUnique<Function<void ()>>(WTFMove(function)));
}

void Acinerella::performTerminate()
{
	D(dprintf("%s: %p\n", __func__, this));
	ASSERT(!isMainThread());
	m_queue.kill();
}

bool Acinerella::initialize()
{
	D(dprintf("%s: %p\n", __func__, this));
	m_acinerella = AcinerellaPointer::create();
	if (m_acinerella)
	{
		if (-1 == ac_open(m_acinerella->instance(), static_cast<void *>(this), &acOpenCallback, &acReadCallback, &acSeekCallback, &acCloseCallback, nullptr))
		{
			m_acinerella = nullptr;
		}
		else
		{
			D(dprintf("ac initialized, stream count %d\n", m_acinerella->instance()->stream_count));
			int audioIndex = -1;
			int videoIndex = -1;

			for (int i = m_acinerella->instance()->stream_count - 1; i >= 0; --i)
			{
				ac_stream_info info;
				ac_get_stream_info(m_acinerella->instance(), i, &info);
				switch (info.stream_type)
				{
				case AC_STREAM_TYPE_VIDEO:
					D(dprintf("video stream: %dx%d\n", info.additional_info.video_info.frame_width, info.additional_info.video_info.frame_height));
					if (-1 == videoIndex && m_enableVideo)
						videoIndex = i;
					break;

				case AC_STREAM_TYPE_AUDIO:
					D(dprintf("audio stream: %d %d %d\n", info.additional_info.audio_info.samples_per_second,
						info.additional_info.audio_info.channel_count, info.additional_info.audio_info.bit_depth));
					if (-1 == audioIndex && m_enableAudio)
						audioIndex = i;
					break;
					
				case AC_STREAM_TYPE_UNKNOWN:
					break;
				}
			}

			if (-1 != audioIndex || -1 != videoIndex)
			{
				ac_stream_info info;

				m_muxer = AcinerellaMuxedBuffer::create([this, protectedThis = makeRef(*this)]() {
						// look ma, a lambda within a lambda
						protectedThis->dispatch([this]() {
							demuxNextPackage();
						});
					}, audioIndex, videoIndex);

				if (-1 != audioIndex)
				{
					ac_get_stream_info(m_acinerella->instance(), audioIndex, &info);
					m_acinerella->setAudioDecoder(ac_create_decoder(m_acinerella->instance(), audioIndex));
					m_audioDecoder = WTF::adoptRef(*new AcinerellaAudioDecoder(this, m_muxer, audioIndex, info));
				}
				
				m_client->accSetNetworkState(WebCore::MediaPlayerEnums::NetworkState::Loading);
				m_client->accSetReadyState(WebCore::MediaPlayerEnums::ReadyState::HaveMetadata);

				m_muxer->setDropVideoPackages(m_videoDecoder ? false : true);

				float audioDuration = 0;
				float videoDuration = 0;

				if (m_audioDecoder)
				{
					m_audioDecoder->warmUp();
					audioDuration = m_audioDecoder->duration();
					if (audioDuration <= 0.f && m_networkBuffer->length() > 0)
					{
						double total = m_networkBuffer->length();
						total *= 8;
						total /= m_audioDecoder->bitRate();
						audioDuration = total;
					}
				}
				
				if (m_videoDecoder)
				{
					m_videoDecoder->warmUp();
					videoDuration = m_videoDecoder->duration();
					if (videoDuration <= 0.f && m_networkBuffer->length() > 0)
					{
						double total = m_networkBuffer->length();
						total *= 8;
						total /= m_videoDecoder->bitRate();
						videoDuration = total;
					}
				}
				
				m_duration = std::max(audioDuration, videoDuration);
				WTF::callOnMainThread([this, protectedThis = makeRef(*this)]() {
					if (m_client)
						m_client->accSetDuration(m_duration);
				});
			}
		}
	}

	if (m_acinerella)
		return true;
	return false;
}

void Acinerella::initializeAfterDiscontinuity()
{
	auto acinerella = AcinerellaPointer::create();
	if (-1 == ac_open(acinerella->instance(), static_cast<void *>(this), &acOpenCallback, &acReadCallback, &acSeekCallback, &acCloseCallback, nullptr))
	{
		// TODO failure path - signal to client
	}
	else
	{
		D(dprintf("ac initialized, stream count %d\n", acinerella->instance()->stream_count));
		int audioIndex = -1;
		int videoIndex = -1;

		for (int i = acinerella->instance()->stream_count - 1; i >= 0; --i)
		{
			ac_stream_info info;
			ac_get_stream_info(acinerella->instance(), i, &info);
			switch (info.stream_type)
			{
			case AC_STREAM_TYPE_VIDEO:
				D(dprintf("video stream: %dx%d\n", info.additional_info.video_info.frame_width, info.additional_info.video_info.frame_height));
				if (-1 == videoIndex && m_enableVideo)
					videoIndex = i;
				break;

			case AC_STREAM_TYPE_AUDIO:
				D(dprintf("audio stream: %d %d %d\n", info.additional_info.audio_info.samples_per_second,
					info.additional_info.audio_info.channel_count, info.additional_info.audio_info.bit_depth));
				if (-1 == audioIndex && m_enableAudio)
					audioIndex = i;
				break;
				
			case AC_STREAM_TYPE_UNKNOWN:
				break;
			}
		}

		if (-1 != audioIndex || -1 != videoIndex)
		{
			if (-1 != audioIndex)
				acinerella->setAudioDecoder(ac_create_decoder(m_acinerella->instance(), audioIndex));
			
			{
				auto lock = holdLock(m_acinerellaLock);
				m_acinerella = acinerella;
			}

			// Flush packet!
			AcinerellaPackage package(acinerella, ac_flush_packet());
			m_muxer->push(WTFMove(package));
			D(dprintf("muxer sent an ac_flush_packet!\n"));

			if (m_audioDecoder)
				m_audioDecoder->warmUp();
			if (m_videoDecoder)
				m_videoDecoder->warmUp();
		}
	}
}

void Acinerella::demuxNextPackage()
{
	RefPtr<AcinerellaPointer> acinerella;
	RefPtr<AcinerellaMuxedBuffer> muxer;

	{
		auto lock = holdLock(m_acinerellaLock);
		acinerella = m_acinerella;
		muxer = m_muxer;
	}

	if (muxer && acinerella && acinerella->instance())
	{
		AcinerellaPackage package(acinerella, ac_read_package(acinerella->instance()));
		muxer->push(WTFMove(package));
	}
}

void Acinerella::onDecoderReadyToPlay(AcinerellaDecoder& decoder)
{
	WTF::callOnMainThread([this, protectedThis = makeRef(*this)]() {
		if (m_client)
		{
			m_client->accSetReadyState(WebCore::MediaPlayerEnums::ReadyState::HaveEnoughData);
			m_client->accSetReadyState(WebCore::MediaPlayerEnums::ReadyState::HaveFutureData);
		}
	});
}

void Acinerella::onDecoderPlaying(AcinerellaDecoder& decoder, bool playing)
{
	WTF::callOnMainThread([this, protectedThis = makeRef(*this)]() {
	});
}

void Acinerella::onDecoderUpdatedBufferLength(AcinerellaDecoder& decoder, float buffer)
{
	WTF::callOnMainThread([this, buffer, protectedThis = makeRef(*this)]() {
		if (m_client)
			m_client->accSetBufferLength(buffer);
	});
}

void Acinerella::onDecoderUpdatedPosition(AcinerellaDecoder& decoder, float buffer)
{
	WTF::callOnMainThread([this, buffer, protectedThis = makeRef(*this)]() {
		if (m_client)
			m_client->accSetPosition(buffer);
	});
}

// callback from acinerella on acinerella's main thread!
int Acinerella::open()
{
	D(dprintf("%s: %p\n", __func__, this));
	WTF::callOnMainThread([this, protectedThis = makeRef(*this)]() {
		if (m_networkBuffer)
			m_networkBuffer->start();
	});
	return 0;
}

// callback from acinerella on acinerella's main thread!
int Acinerella::close()
{
	D(dprintf("%s: %p\n", __func__, this));
	WTF::callOnMainThread([this, protectedThis = makeRef(*this)]() {
		D(dprintf("%s: %p .. \n", __func__, this));
		if (m_networkBuffer)
			m_networkBuffer->stop();
		m_networkBuffer = nullptr;
	});
	return 0;
}

// callback from acinerella on acinerella's main thread!
int Acinerella::read(uint8_t *buf, int size)
{
// 	D(dprintf("%s: %p\n", __func__, this));
	RefPtr<AcinerellaNetworkBuffer> buffer(m_networkBuffer);
	if (buffer)
	{
		int rc = buffer->read(buf, size);
		if (rc == AcinerellaNetworkBuffer::eRead_Discontinuity)
		{
			rc = AcinerellaNetworkBuffer::eRead_EOF;
			dispatch([this, protectedThis = makeRef(*this)]() {
				initializeAfterDiscontinuity();
			});
		}
		return rc;
	}

	return AcinerellaNetworkBuffer::eRead_EOF;
}

// callback from acinerella on acinerella's main thread!
int64_t Acinerella::seek(int64_t pos, int whence)
{
	(void)pos;
	(void)whence;
	return -1;
}

int Acinerella::acOpenCallback(void *me)
{
	return static_cast<Acinerella *>(me)->open();
}

int Acinerella::acCloseCallback(void *me)
{
	return static_cast<Acinerella *>(me)->close();
}

int Acinerella::acReadCallback(void *me, uint8_t *buf, int size)
{
	return static_cast<Acinerella *>(me)->read(buf, size);
}

int64_t Acinerella::acSeekCallback(void *me, int64_t pos, int whence)
{
	return static_cast<Acinerella *>(me)->seek(pos, whence);
}

}
}

#undef D
#endif
