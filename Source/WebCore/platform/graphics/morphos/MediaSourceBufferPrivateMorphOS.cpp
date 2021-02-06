#include "MediaSourceBufferPrivateMorphOS.h"

#if ENABLE(VIDEO) && ENABLE(MEDIA_SOURCE)

#include "SourceBufferPrivateClient.h"
#include "MediaPlayerPrivateMorphOS.h"
#include "MediaSourcePrivateMorphOS.h"
#include "MediaDescriptionMorphOS.h"
#include "InbandTextTrackPrivate.h"
#include "AcinerellaAudioDecoder.h"
#include "AcinerellaMuxer.h"
#include "AudioTrackPrivateMorphOS.h"
#include "VideoTrackPrivateMorphOS.h"

#define D(x) x

namespace WebCore {

Ref<MediaSourceBufferPrivateMorphOS> MediaSourceBufferPrivateMorphOS::create(MediaSourcePrivateMorphOS* parent)
{
	D(dprintf("[MS]%s\n", __func__));
	return adoptRef(*new MediaSourceBufferPrivateMorphOS(parent));
}

MediaSourceBufferPrivateMorphOS::MediaSourceBufferPrivateMorphOS(MediaSourcePrivateMorphOS* parent)
    : m_mediaSource(parent)
    , m_client(0)
{
	m_thread = Thread::create("Acinerella Media Source Buffer", [this] {
		threadEntryPoint();
	});
}

MediaSourceBufferPrivateMorphOS::~MediaSourceBufferPrivateMorphOS()
{
	D(dprintf("[MS]%s: bye!\n", __func__));
}

void MediaSourceBufferPrivateMorphOS::setClient(SourceBufferPrivateClient* client)
{
	D(dprintf("[MS]%s client %p main %d\n", __func__, client, isMainThread()));
	m_client = client;
}

void MediaSourceBufferPrivateMorphOS::append(Vector<unsigned char>&&vector)
{
	D(dprintf("[MS]%s bytes %lu main %d\n", __func__, vector.size(), isMainThread()));

	{
		auto lock = holdLock(m_lock);
		m_buffer = WTFMove(vector);
		m_bufferPosition = 0;
	}

	// wake read operation
	m_event.signal();
}

void MediaSourceBufferPrivateMorphOS::clearMediaSource()
{
	abort();
	m_mediaSource = nullptr;
}

void MediaSourceBufferPrivateMorphOS::abort()
{
	D(dprintf("[MS]%s\n", __func__));
	m_terminating = true;
	m_event.signal();

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
}

void MediaSourceBufferPrivateMorphOS::resetParserState()
{
	D(dprintf("[MS]%s\n", __func__));

}

void MediaSourceBufferPrivateMorphOS::removedFromMediaSource()
{
	D(dprintf("[MS]%s\n", __func__));
	abort();
}

MediaPlayer::ReadyState MediaSourceBufferPrivateMorphOS::readyState() const
{
	D(dprintf("[MS]%s\n", __func__));
	if (m_mediaSource)
		return m_mediaSource->readyState();
	return MediaPlayer::ReadyState::HaveNothing;
}

void MediaSourceBufferPrivateMorphOS::setReadyState(MediaPlayer::ReadyState)
{
	D(dprintf("[MS]%s\n", __func__));

}

bool MediaSourceBufferPrivateMorphOS::initialize()
{
	m_acinerella = Acinerella::AcinerellaPointer::create();

	if (m_acinerella)
	{
		if (-1 == ac_open(m_acinerella->instance(), static_cast<void *>(this), nullptr, &acReadCallback, nullptr, nullptr, nullptr))
		{
			m_acinerella = nullptr;
			D(dprintf("[MS]---- ac failed to open :(\n"));
		}
		else
		{
			D(dprintf("[MS]ac initialized, stream count %d\n", m_acinerella->instance()->stream_count));
			float duration = 0.f;
			uint32_t decoderIndexMask = 0;
bool doFakeVideo = false;
			m_muxer = Acinerella::AcinerellaMuxedBuffer::create();

			for (int i = 0; i < std::min(Acinerella::AcinerellaMuxedBuffer::maxDecoders, m_acinerella->instance()->stream_count); i++)
			{
				ac_stream_info info;
				ac_get_stream_info(m_acinerella->instance(), i, &info);

				switch (info.stream_type)
				{
				case AC_STREAM_TYPE_VIDEO:
					D(dprintf("video stream: %dx%d\n", info.additional_info.video_info.frame_width, info.additional_info.video_info.frame_height));
					doFakeVideo = true;
					break;

				case AC_STREAM_TYPE_AUDIO:
					D(dprintf("audio stream: %d %d %d\n", info.additional_info.audio_info.samples_per_second,
						info.additional_info.audio_info.channel_count, info.additional_info.audio_info.bit_depth));
					if (m_enableAudio)
					{
						m_acinerella->setDecoder(i, ac_create_decoder(m_acinerella->instance(), i));
						m_decoders[i] = WTF::adoptRef(*new Acinerella::AcinerellaAudioDecoder(this, m_muxer, i, info, false));
						if (!!m_decoders[i])
						{
							duration = std::max(duration, m_decoders[i]->duration());
							D(dprintf("[MS] audio decoder created, duration %f\n", duration));
							decoderIndexMask |= (1ULL << i);
						}
					}
					break;
					
				case AC_STREAM_TYPE_UNKNOWN:
					break;
				}
			}
	
			if (decoderIndexMask != 0 || doFakeVideo)
			{
				// Need to build and forward the InitializationSegment now...
				WebCore::SourceBufferPrivateClient::InitializationSegment initializationSegment;
				
				m_muxer->setDecoderMask(decoderIndexMask);
				m_muxer->setSinkFunction([this, protectedThis = makeRef(*this)]() {
					// look ma, a lambda within a lambda
					protectedThis->dispatch([this]() {
						demuxNextPackage();
					});
				});

				initializationSegment.duration = MediaTime::createWithFloat(duration);

// TODO: this thing will need to reflect selected/enabled tracks
				m_info.m_duration = duration;
				m_info.m_isLive = false;
				m_info.m_channels = 0;
				m_info.m_width = 0;

				for (int i = 0; i < Acinerella::AcinerellaMuxedBuffer::maxDecoders; i++)
				{
					if (!!m_decoders[i])
					{
						auto &decoder = m_decoders[i];
						if (decoder->isAudio())
						{
							WebCore::SourceBufferPrivateClient::InitializationSegment::AudioTrackInformation audioTrackInformation;
							audioTrackInformation.track = AudioTrackPrivateMorphOS::create(m_mediaSource->player(), i);
							audioTrackInformation.description = MediaDescriptionMorphOS::createAudioWithCodec(ac_codec_name(m_acinerella->instance(), i));
							initializationSegment.audioTracks.append(WTFMove(audioTrackInformation));
						}
						else if (decoder->isVideo())
						{
							WebCore::SourceBufferPrivateClient::InitializationSegment::VideoTrackInformation videoTrackInformation;
							videoTrackInformation.track = VideoTrackPrivateMorphOS::create(m_mediaSource->player(), i);
							videoTrackInformation.description = MediaDescriptionMorphOS::createVideoWithCodec(ac_codec_name(m_acinerella->instance(), i));
							initializationSegment.videoTracks.append(WTFMove(videoTrackInformation));
						}
					}
				}
				
				// fake video
				if (doFakeVideo)
				{
					initializationSegment.duration = MediaTime::createWithFloat(std::max(ac_get_stream_duration(m_acinerella->instance(), 0), double(m_acinerella->instance()->info.duration)/1000.0));
dprintf("---- faking video track....\n");
					WebCore::SourceBufferPrivateClient::InitializationSegment::VideoTrackInformation videoTrackInformation;
					videoTrackInformation.track = VideoTrackPrivateMorphOS::create(m_mediaSource->player(), 1);
					videoTrackInformation.description = MediaDescriptionMorphOS::createVideoWithCodec("vp9");
					initializationSegment.videoTracks.append(WTFMove(videoTrackInformation));
				}

				WTF::callOnMainThread([initializationSegment, duration, this, protect = makeRef(*this)]() {
					D(dprintf("[MS] calling sourceBufferPrivateDidReceiveInitializationSegment...\n"));
					if (m_client)
						m_client->sourceBufferPrivateDidReceiveInitializationSegment(initializationSegment);
					if (m_mediaSource)
						m_mediaSource->player()->accSetDuration(duration);
				});

				return true;
			}
		}
	}

	return false;
}

void MediaSourceBufferPrivateMorphOS::warmUp()
{
	D(dprintf("[MS] warmUp calling notifyReadyForMoreSamples\n"));
	if (m_client)
		m_client->sourceBufferPrivateDidBecomeReadyForMoreSamples(AtomString::number(0));
	
	dispatch([this]() {
		for (int i = 0; i < Acinerella::AcinerellaMuxedBuffer::maxDecoders; i++)
		{
			if (!!m_decoders[i])
			{
				m_decoders[i]->warmUp();
			}
		}
	});
}

void MediaSourceBufferPrivateMorphOS::demuxNextPackage()
{
	RefPtr<Acinerella::AcinerellaPointer> acinerella;
	RefPtr<Acinerella::AcinerellaMuxedBuffer> muxer;

	{
		auto lock = holdLock(m_lock);
		acinerella = m_acinerella;
		muxer = m_muxer;
	}

	D(dprintf("%s: %p %p %p\n", __func__ , this, muxer.get(), acinerella.get(), acinerella->instance()));

	if (muxer && acinerella && acinerella->instance())
	{
		Acinerella::AcinerellaPackage package(acinerella, ac_read_package(acinerella->instance()));
		if (!!package)
		{
			muxer->push(WTFMove(package));
		}
	}
}

void MediaSourceBufferPrivateMorphOS::threadEntryPoint()
{
	if (initialize())
	{
		while (auto function = m_queue.waitForMessage())
		{
			(*function)();
		}
	}
}

void MediaSourceBufferPrivateMorphOS::dispatch(Function<void ()>&& function)
{
	ASSERT(isMainThread());
	ASSERT(!m_queue.killed() && m_thread);
	m_queue.append(makeUnique<Function<void ()>>(WTFMove(function)));
}

void MediaSourceBufferPrivateMorphOS::performTerminate()
{
	D(dprintf("[MS]%s: %p\n", __func__, this));
	ASSERT(!isMainThread());
	m_queue.kill();
}

int MediaSourceBufferPrivateMorphOS::read(uint8_t *buf, int size)
{
	int sizeLeft = size;
	int pos = 0;

	D(dprintf("[MS]%s>> %p size %d\n", __func__, this, size));

	while (sizeLeft > 0 && !m_terminating)
	{
		bool bufferEaten = false;
		bool waitForBuffer = false;
		
		{
			auto lock = holdLock(m_lock);
			int bufferSize = m_buffer.size();
			int toCopy = std::min(sizeLeft, bufferSize - m_bufferPosition);
			
			D(dprintf("[MS]%s: left %d pos %d bs %d bp %d tc %d\n", __func__, sizeLeft, pos, bufferSize, m_bufferPosition, toCopy));
			
			if (toCopy > 0)
			{
				memcpy(buf + pos, m_buffer.data() + m_bufferPosition, toCopy);
				sizeLeft -= toCopy;
				pos += toCopy;
				m_bufferPosition += toCopy;
			}

			if (m_bufferPosition >= m_buffer.size() && m_bufferPosition > 0)
			{
				bufferEaten = true;
				waitForBuffer = true;
				m_buffer.clear();
				m_bufferPosition = 0;
			}
			else if (m_buffer.size() == 0)
			{
				waitForBuffer = true;
			}
		}

		if (bufferEaten)
		{
			WTF::callOnMainThread([this, protect = makeRef(*this)]() {
				D(dprintf("[MS]%s: calling AppendComplete...\n", __func__));
				if (m_client)
					m_client->sourceBufferPrivateAppendComplete(SourceBufferPrivateClient::AppendSucceeded);
			});
		}
		
		if (waitForBuffer)
		{
			m_event.waitFor(10_s);
		}
	}
	
	D(dprintf("[MS]%s<< read %d\n", __func__, size - sizeLeft));
	return size - sizeLeft;
}

int64_t MediaSourceBufferPrivateMorphOS::seek(int64_t pos, int whence)
{
	D(dprintf("[MS]%s: %p %llu %d\n", __func__, this, pos, whence));
	return -1;
}

int MediaSourceBufferPrivateMorphOS::acReadCallback(void *me, uint8_t *buf, int size)
{
	return static_cast<MediaSourceBufferPrivateMorphOS *>(me)->read(buf, size);
}

int64_t MediaSourceBufferPrivateMorphOS::acSeekCallback(void *me, int64_t pos, int whence)
{
	return static_cast<MediaSourceBufferPrivateMorphOS *>(me)->seek(pos, whence);
}

void MediaSourceBufferPrivateMorphOS::onDecoderReadyToPlay(Acinerella::AcinerellaDecoder& decoder)
{

}

void MediaSourceBufferPrivateMorphOS::onDecoderPlaying(Acinerella::AcinerellaDecoder& decoder, bool playing)
{

}

void MediaSourceBufferPrivateMorphOS::onDecoderUpdatedBufferLength(Acinerella::AcinerellaDecoder& decoder, float buffer)
{

}

void MediaSourceBufferPrivateMorphOS::onDecoderUpdatedPosition(Acinerella::AcinerellaDecoder& decoder, float position)
{

}

void MediaSourceBufferPrivateMorphOS::onDecoderUpdatedDuration(Acinerella::AcinerellaDecoder& decoder, float duration)
{

}

void MediaSourceBufferPrivateMorphOS::onDecoderEnded(Acinerella::AcinerellaDecoder& decoder)
{

}

}

#endif
