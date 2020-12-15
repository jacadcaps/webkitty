#include "config.h"

#if ENABLE(VIDEO)

#include "AcinerellaContainer.h"
#include "AcinerellaAudioDecoder.h"
#include "AcinerellaBuffer.h"

#define D(x) x

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
	dispatch([this] {
		if (m_audioDecoder)
			m_audioDecoder->play();
		if (m_videoDecoder)
			m_videoDecoder->play();
	});
}

void Acinerella::pause()
{
	dispatch([this] {
		if (m_audioDecoder)
			m_audioDecoder->pause();
		if (m_videoDecoder)
			m_videoDecoder->pause();
	});
}

void Acinerella::terminate()
{
	D(dprintf("%s: %p\n", __func__, this));
	m_terminating = true;
	m_client = nullptr;
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
	
	if (m_networkBuffer)
		m_networkBuffer->stop();
	m_networkBuffer = nullptr;

	if (m_videoDecoder)
		m_videoDecoder->terminate();
	
	if (m_audioDecoder)
		m_audioDecoder->terminate();

	D(dprintf("%s: %p done\n", __func__, this));
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
		
		// make sure this is disposed on the thread!
		m_ac.reset();
	}
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
	if (m_muxer)
		m_muxer->terminate();
}

bool Acinerella::initialize()
{
	D(dprintf("%s: %p\n", __func__, this));
	m_ac = deleted_unique_ptr<ac_instance>(ac_init(), [](ac_instance*instance){ ac_free(instance); });
	if (m_ac)
	{
		if (-1 == ac_open(m_ac.get(), static_cast<void *>(this), &acOpenCallback, &acReadCallback, &acSeekCallback, &acCloseCallback, nullptr))
		{
			m_ac.reset();
		}
		else
		{
			D(dprintf("ac initialized, stream count %d\n", m_ac->stream_count));
			int audioIndex = -1;
			int videoIndex = -1;

			for (int i = m_ac->stream_count - 1; i >= 0; --i)
			{
				ac_stream_info info;
				ac_get_stream_info(m_ac.get(), i, &info);
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
				ac_get_stream_info(m_ac.get(), audioIndex, &info);

				m_muxer = AcinerellaMuxedBuffer::create([this, protectedThis = makeRef(*this)]() {
						// look ma, a lambda within a lambda
						protectedThis->dispatch([this]() {
							demuxNextPackage();
						});
					}, audioIndex, videoIndex);

				if (-1 != audioIndex)
					m_audioDecoder = WTF::adoptRef(*new AcinerellaAudioDecoder(*this, m_muxer, audioIndex, info));
				
				m_client->accSetNetworkState(WebCore::MediaPlayerEnums::NetworkState::Loading);
				m_client->accSetReadyState(WebCore::MediaPlayerEnums::ReadyState::HaveMetadata);

				if (m_audioDecoder)
					m_audioDecoder->warmUp();
				
				if (m_videoDecoder)
					m_videoDecoder->warmUp();
			}
		}
	}

	if (m_ac)
		return true;
	return false;
}

void Acinerella::demuxNextPackage()
{
	if (m_muxer)
	{
		AcinerellaPackage package(ac_read_package(m_ac.get()));
		m_muxer->push(WTFMove(package));
	}
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
	D(dprintf("%s: %p\n", __func__, this));
	RefPtr<AcinerellaNetworkBuffer> buffer(m_networkBuffer);
	if (buffer)
	{
		return buffer->read(buf, size);
	}
	return -1;
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
