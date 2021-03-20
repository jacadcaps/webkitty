#include "MediaSourceBufferPrivateMorphOS.h"

#if ENABLE(VIDEO) && ENABLE(MEDIA_SOURCE)

#include "SourceBufferPrivateClient.h"
#include "MediaPlayerPrivateMorphOS.h"
#include "MediaSourcePrivateMorphOS.h"
#include "MediaDescriptionMorphOS.h"
#include "InbandTextTrackPrivate.h"
#include "AcinerellaAudioDecoder.h"
#include "AcinerellaVideoDecoder.h"
#include "AcinerellaMuxer.h"
#include "AudioTrackPrivateMorphOS.h"
#include "VideoTrackPrivateMorphOS.h"
#include "MediaSampleMorphOS.h"

#include <proto/dos.h>
#include <proto/exec.h>

#define D(x)  x
#define DM(x) x
#define DI(x) x
#define DN(x) x
#define DIO(x)

// #pragma GCC optimize ("O0")

namespace WebCore {

MediaSourceChunkReader::MediaSourceChunkReader(WeakPtr<MediaPlayerPrivateMorphOS> player, InitializationCallback &&icb, ChunkDecodedCallback &&ccb)
	: m_initializationCallback(WTFMove(icb))
	, m_chunkDecodedCallback(WTFMove(ccb))
	, m_player(player)
{
	m_thread = Thread::create("Acinerella Media Source Chunk Reader", [this] {
		while (auto function = m_queue.waitForMessage())
		{
			(*function)();
		}
	});
}

void MediaSourceChunkReader::terminate()
{
	m_terminating = true;
	m_event.signal();

	m_queue.append(makeUnique<Function<void ()>>([this] {
		m_queue.kill();
	}));

	m_thread->waitForCompletion();
	m_thread = nullptr;
}

void MediaSourceChunkReader::decode(Vector<unsigned char>&& data)
{
	D(dprintf("[MS]%s\n", __func__));

	{
		auto lock = holdLock(m_lock);
		m_buffer = WTFMove(data);
	}

	dispatch([this] {
		decodeAllMediaSamples();
	});

	m_event.signal();
}

void MediaSourceChunkReader::signalEOF()
{
	D(dprintf("[MS]%s\n", __func__));
	m_bufferEOF = true;

	dispatch([this] {
		decodeAllMediaSamples();
	});

	m_event.signal();
}

void MediaSourceChunkReader::getSamples(MediaSamplesList& outSamples)
{
	D(dprintf("[MS]%s\n", __func__));
	auto lock = holdLock(m_lock);
	std::swap(outSamples, m_samples);
}

void MediaSourceChunkReader::dispatch(Function<void ()>&& function)
{
	D(dprintf("[MS]%s\n", __func__));
	ASSERT(isMainThread());
	ASSERT(!m_queue.killed() && m_thread);
	m_queue.append(makeUnique<Function<void ()>>(WTFMove(function)));
}

bool MediaSourceChunkReader::initialize()
{
	D(dprintf("[MS]%s\n", __func__));
	EP_SCOPE(initialize);

	m_acinerella = Acinerella::AcinerellaPointer::create();

	if (m_acinerella)
	{
		D(dprintf("[MS] ac_open()... \n"));
		int score;
		auto probe = ac_probe_input_buffer(m_buffer.data(), m_buffer.size(), nullptr, &score);

		if (!probe)
			return false;

		if (-1 == ac_open(m_acinerella->instance(), static_cast<void *>(this), nullptr, &acReadCallback, nullptr, nullptr, probe))
		{
			return false;
		}
		
		D(dprintf("[MS] ac_open() success!\n"));
		m_audioDecoderMask = 0;
		m_videoDecoderMask = 0;
		
		for (int i = 0; i < std::min(Acinerella::AcinerellaMuxedBuffer::maxDecoders, m_acinerella->instance()->stream_count); i++)
		{
			ac_stream_info info;
			ac_get_stream_info(m_acinerella->instance(), i, &info);

			switch (info.stream_type)
			{
			case AC_STREAM_TYPE_VIDEO:
				m_videoDecoderMask |= (1UL << i);
				break;

			case AC_STREAM_TYPE_AUDIO:
				m_audioDecoderMask |= (1UL << i);
				break;
				
			default:
				break;
			}
		}
		
		return true;
	}
		
	return false;
}

void MediaSourceChunkReader::getMeta(WebCore::SourceBufferPrivateClient::InitializationSegment& initializationSegment, MediaPlayerMorphOSInfo& minfo)
{
	D(dprintf("[MS]%s\n", __func__));
	auto metaCinerella = m_acinerella;

	if (metaCinerella)
	{
		// Need to build and forward the InitializationSegment now...
		double duration = 0.0;
		minfo.m_width = 0;

		DM(dprintf("%s: streams %d\n", __func__, metaCinerella->instance()->stream_count));

		for (int i = 0; i < std::min(Acinerella::AcinerellaMuxedBuffer::maxDecoders, metaCinerella->instance()->stream_count); i++)
		{
			ac_stream_info info;
			ac_get_stream_info(metaCinerella->instance(), i, &info);

			DM(dprintf("%s: index %d st %d\n", __func__, i, info.stream_type));

			switch (info.stream_type)
			{
			case AC_STREAM_TYPE_VIDEO:
				{
					WebCore::SourceBufferPrivateClient::InitializationSegment::VideoTrackInformation videoTrackInformation;
					videoTrackInformation.track = VideoTrackPrivateMorphOS::create(m_player, i);
					videoTrackInformation.description = MediaDescriptionMorphOS::createVideoWithCodec(ac_codec_name(metaCinerella->instance(), i));
					initializationSegment.videoTracks.append(WTFMove(videoTrackInformation));
					duration = std::max(duration, std::max(double(ac_get_stream_duration(metaCinerella->instance(), i)), double(metaCinerella->instance()->info.duration)/1000.0));
					DM(dprintf("%s: video %d %f\n", __func__, i, float(duration)));
					minfo.m_width = info.additional_info.video_info.frame_width;
					minfo.m_height = info.additional_info.video_info.frame_height;
				}
				break;
				
			case AC_STREAM_TYPE_AUDIO:
				{
					WebCore::SourceBufferPrivateClient::InitializationSegment::AudioTrackInformation audioTrackInformation;
					audioTrackInformation.track = AudioTrackPrivateMorphOS::create(m_player, i);
					audioTrackInformation.description = MediaDescriptionMorphOS::createAudioWithCodec(ac_codec_name(metaCinerella->instance(), i));
					initializationSegment.audioTracks.append(WTFMove(audioTrackInformation));
					duration = std::max(duration, std::max(double(ac_get_stream_duration(metaCinerella->instance(), i)), double(metaCinerella->instance()->info.duration)/1000.0));
					DM(dprintf("%s: audio %d %f\n", __func__, i, float(duration)));
				}
				break;
			}
		}

		initializationSegment.duration = MediaTime::createWithDouble(duration);
		minfo.m_duration = duration;
		minfo.m_isLive = false;
		minfo.m_channels = 0;
	}
}

bool MediaSourceChunkReader::keepDecoding()
{
	auto lock = holdLock(m_lock);

//	D(dprintf("[MS]%s: term %d bs %d bp %d\n", __func__, m_terminating, m_buffer.size(), m_bufferPosition));

	if (m_terminating)
		return false;

	if (m_buffer.size() - m_bufferPosition < AC_BUFSIZE)
		return false;
		
	return true;
}

void MediaSourceChunkReader::decodeAllMediaSamples()
{
	D(dprintf("[MS]%s\n", __func__));

	if (!m_acinerella)
	{
		WebCore::SourceBufferPrivateClient::InitializationSegment segment;
		MediaPlayerMorphOSInfo info;
		
		if (initialize())
		{
			getMeta(segment, info);
			m_initializationCallback(true, segment, info);
		}
		else
		{
			m_initializationCallback(false, segment, info);
			return;
		}
	}

	
	DN(dprintf("%s: audiomask %lx videomask %lx\n", __func__, m_audioDecoderMask, m_videoDecoderMask));
	DN(int total = 0);
	
	while (keepDecoding())
	{
		RefPtr<Acinerella::AcinerellaPackage> package = Acinerella::AcinerellaPackage::create(m_acinerella, ac_read_package(m_acinerella->instance()));

		if (package.get() && package->package())
		{
			String trackID;
			if (m_audioDecoderMask & (1uL << package->index()))
			{
				trackID = "A" + String::number(package->index());
			}
			else if (m_videoDecoderMask & (1uL << package->index()))
			{
				trackID = "V" + String::number(package->index());
			}
			else
			{
				// reject unknown packets completely
				continue;
			}

			RefPtr<MediaSample> mediaSample = MediaSampleMorphOS::create(package, FloatSize(320, 240), trackID);
			
			DN(m_decodeCount++);
			DN(if (0 == (m_decodeCount % 15)) dprintf("%s: %s sample created (PTS %f)\n", __func__, (m_audioDecoderMask & (1uLL << package->index())) ? "audio" : "video",
				mediaSample->presentationTime().toFloat()));
			DN(total++);

			auto lock = holdLock(m_lock);
			m_samples.emplace_back(mediaSample);
		}
	}

	DN(dprintf("%s: total decoded packets %lu\n", __func__, total));
}

int MediaSourceChunkReader::read(uint8_t *buf, int size)
{
	EP_SCOPE(read);

	int sizeLeft = size;
	int pos = 0;

	DIO(dprintf("[MS]%s>> %p size %d\n", __func__, this, size));

	while (sizeLeft > 0)
	{
		bool wait = false;
		bool chunkDecoded = false;

		{
			auto lock = holdLock(m_lock);
			int leftBufferSize = m_leftOver.size();
			
			// Acinerella will call us once with size=1024 on startup (probing), then keep calling with AC_BUFSIZE
			ASSERT(leftBufferSize < size);
			if (leftBufferSize > size)
				return -1;
			
			if (leftBufferSize > 0)
			{
				int toCopy = leftBufferSize;
			
				DIO(dprintf("[MS]%s: [O] left %d pos %d bs %d tc %d\n", __func__, sizeLeft, pos, leftBufferSize, toCopy));
				
				if (toCopy > 0)
				{
					memcpy(buf + pos, m_leftOver.data(), toCopy);
					sizeLeft -= toCopy;
					pos += toCopy;
					m_leftOver.clear();
				}
				
				continue;
			}
			else
			{
				int bufferSize = m_buffer.size();
				int toCopy = std::min(sizeLeft, bufferSize - m_bufferPosition);
				
				DIO(dprintf("[MS]%s: [B] left %d pos %d bs %d bp %d tc %d\n", __func__, sizeLeft, pos, bufferSize, m_bufferPosition, toCopy));
				
				if (toCopy > 0)
				{
					memcpy(buf + pos, m_buffer.data() + m_bufferPosition, toCopy);
					sizeLeft -= toCopy;
					pos += toCopy;
					m_bufferPosition += toCopy;
				}

				if (m_bufferPosition >= int(m_buffer.size() - AC_BUFSIZE) && m_bufferPosition > 0)
				{
					int leftOver = m_buffer.size() - m_bufferPosition;

					if (leftOver)
					{
						m_leftOver.resize(leftOver);
						memcpy(m_leftOver.data(), m_buffer.data() + m_bufferPosition, leftOver);
					}

					m_buffer.clear();
					m_bufferPosition = 0;

					DIO(dprintf("[MS]%s -- chunk consumed!\n", __func__));

					chunkDecoded = true;
					wait = true;
				}
				else if (m_terminating || m_bufferEOF)
				{
					return -1;
				}
				else if (0 == bufferSize)
				{
					wait = true;
				}
			}
		} // lock

		if (chunkDecoded)
			m_chunkDecodedCallback(true);

		if (wait)
			m_event.waitFor(10_s);
	}
	
	DIO(dprintf("[MS]%s<< read %d pos %d\n", __func__, size - sizeLeft, pos));

	return size - sizeLeft;
}

int MediaSourceChunkReader::acReadCallback(void *me, uint8_t *buf, int size)
{
	return static_cast<MediaSourceChunkReader *>(me)->read(buf, size);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Ref<MediaSourceBufferPrivateMorphOS> MediaSourceBufferPrivateMorphOS::create(MediaSourcePrivateMorphOS* parent)
{
	D(dprintf("[MS]%s\n", __func__));
	return adoptRef(*new MediaSourceBufferPrivateMorphOS(parent));
}

MediaSourceBufferPrivateMorphOS::MediaSourceBufferPrivateMorphOS(MediaSourcePrivateMorphOS* parent)
    : m_mediaSource(parent)
    , m_client(0)
{
	m_reader = MediaSourceChunkReader::create(parent->player(),
		[this](bool success, WebCore::SourceBufferPrivateClient::InitializationSegment& segment, MediaPlayerMorphOSInfo& info){
			initialize(success, segment, info);
		},
		[this](bool success){
			appendComplete(success);
		}
	);

	m_thread = Thread::create("Acinerella Media Source Buffer", [this] {
		threadEntryPoint();
	});

	DI(dprintf("[MS]%s: %p hello!\n", __func__, this));
}

MediaSourceBufferPrivateMorphOS::~MediaSourceBufferPrivateMorphOS()
{
	DI(dprintf("[MS]%s: %p bye!\n", __func__, this));
}

void MediaSourceBufferPrivateMorphOS::setClient(SourceBufferPrivateClient* client)
{
	D(dprintf("[MS]%s client %p main %d\n", __func__, client, isMainThread()));
	m_client = client;
}

void MediaSourceBufferPrivateMorphOS::append(Vector<unsigned char>&&vector)
{
	EP_EVENT(append);
	D(dprintf("[MS]%s bytes %lu main %d\n", __func__, vector.size(), isMainThread()));
	m_reader->decode(WTFMove(vector));
}

void MediaSourceBufferPrivateMorphOS::appendComplete(bool success)
{
	DI(dprintf("[MS]%s: %p %d\n", __func__, this, success));
	WTF::callOnMainThread([success, this, protect = makeRef(*this)]() {
		EP_EVENT(appendComplete);
		if (m_client && !m_terminating)
		{
			MediaSourceChunkReader::MediaSamplesList samples;
			m_reader->getSamples(samples);

			DI(dprintf("[MS]%s: %p %d, queue %d samples\n", __func__, this, success, samples.size()));

			for (auto sample : samples)
			{
				m_client->sourceBufferPrivateDidReceiveSample(*sample.get());
			}
			
			if (m_mediaSource && success)
			{
				m_mediaSource->player()->setLoadingProgresssed(true);
			}

			m_client->sourceBufferPrivateAppendComplete(success ? SourceBufferPrivateClient::AppendSucceeded : SourceBufferPrivateClient::ParsingFailed);
		}
	});
}

void MediaSourceBufferPrivateMorphOS::willSeek(double time)
{
	DI(dprintf("[MS]%s %p\n", __func__, this));

	if (m_seeking)
		return;
		
	m_seeking = true;
	m_seekTime = time;

	for (int i = 0; i < Acinerella::AcinerellaMuxedBuffer::maxDecoders; i++)
	{
		if (!!m_decoders[i])
		{
			m_decoders[i]->pause(true);
		}
	}
	
	flush();
}

void MediaSourceBufferPrivateMorphOS::clearMediaSource()
{
	abort();
	m_mediaSource = nullptr;
}

void MediaSourceBufferPrivateMorphOS::abort()
{
	EP_SCOPE(abort);

	if (m_terminating)
		return;

	DI(dprintf("[MS]%s %p\n", __func__, this));
	
	m_terminating = true;
	m_paintingDecoder = nullptr;

	m_event.signal();

	DI(dprintf("[MS]%s %p muxer shutdown\n", __func__, this));
	if (m_muxer)
		m_muxer->terminate();

	for (int i = 0; i < Acinerella::AcinerellaMuxedBuffer::maxDecoders; i++)
	{
		if (!!m_decoders[i])
		{
			DI(dprintf("[MS]%s %p decoder %p shutdown\n", __func__, this, m_decoders[i].get()));
			m_decoders[i]->terminate();
			dprintf("erasingptr..\n");
			m_decoders[i] = nullptr;
		}
	}

	if (m_reader)
		m_reader->terminate();
	m_reader = nullptr;

	DI(dprintf("[MS]%s %p thread shutdown\n", __func__, this));
	if (!m_thread)
	{
		DI(dprintf("[MS]%s %p already done\n", __func__, this));
		return;
	}
	ASSERT(isMainThread());
	ASSERT(!m_queue.killed() && m_thread);

	if (!m_thread)
		return;

	m_queue.append(makeUnique<Function<void ()>>([this] {
		performTerminate();
	}));

	m_thread->waitForCompletion();
	m_thread = nullptr;
	ASSERT(m_queue.killed());
	DI(dprintf("[MS]%s %p done\n", __func__, this));

}

void MediaSourceBufferPrivateMorphOS::resetParserState()
{
	D(dprintf("[MS]%s\n", __func__));
}

void MediaSourceBufferPrivateMorphOS::removedFromMediaSource()
{
	D(dprintf("[MS]%s\n", __func__));
	abort();
	RefPtr<MediaSourceBufferPrivateMorphOS> me = makeRef(*this);
	if (m_mediaSource)
		m_mediaSource->onSourceBufferRemoved(me);
}

void MediaSourceBufferPrivateMorphOS::flush(const AtomString&)
{
	D(dprintf("[MS]%s\n", __func__));
}

void MediaSourceBufferPrivateMorphOS::becomeReadyForMoreSamples(void)
{
	if (!m_readyForMoreSamples)
	{
		m_readyForMoreSamples = true;

		D(dprintf("[MS]%s\n", __func__));

#if 0
		if (m_client)
		{
			for (int i = 0; i < Acinerella::AcinerellaMuxedBuffer::maxDecoders; i++)
			{
				if (!!m_decoders[i])
				{
				
				}
			}
		
			m_client->sourceBufferPrivateDidBecomeReadyForMoreSamples(
		}
#endif
	}
}

void MediaSourceBufferPrivateMorphOS::flush()
{
	D(dprintf("[MS]%s\n", __func__));

	if (m_muxer && m_reader)
	{
		m_muxer->flush();
		RefPtr<Acinerella::AcinerellaPackage> package = Acinerella::AcinerellaPackage::create(m_reader->acinerella(), ac_flush_packet());
		m_muxer->push(package);
	}
}

void MediaSourceBufferPrivateMorphOS::enqueueSample(Ref<MediaSample>&&sample, const AtomString&)
{
	auto msample = static_cast<MediaSampleMorphOS *>(&sample.get());
	RefPtr<Acinerella::AcinerellaPackage> package = msample->package();

	D(m_enqueueCount ++);
	D(if (0 == (m_enqueueCount % 50)) dprintf("[MS][%s]%s PTS %f key %d\n", __func__, (m_audioDecoderMask & (1uLL << package->index())) ? "A":"V", msample->presentationTime().toFloat(), msample->isSync()));

	if (m_seeking)
	{
		double pts = msample->presentationTime().toDouble();
		if (msample->isSync() && pts > m_seekTime - 2.5 && pts < m_seekTime + 15.0)
		{
			m_seeking = false;
			D(dprintf("[MS]Keyframe found!\n"));
		}
		else
		{
			return;
		}
	}

	m_muxer->push(package);
}

void MediaSourceBufferPrivateMorphOS::allSamplesInTrackEnqueued(const AtomString&)
{
	D(dprintf("[MS]%s\n", __func__));
}

bool MediaSourceBufferPrivateMorphOS::isReadyForMoreSamples(const AtomString&)
{
// 	D(dprintf("[MS]%s %d\n", __func__, m_readyForMoreSamples));
	return m_readyForMoreSamples;
}

void MediaSourceBufferPrivateMorphOS::setActive(bool)
{
	D(dprintf("[MS]%s\n", __func__));
}

void MediaSourceBufferPrivateMorphOS::notifyClientWhenReadyForMoreSamples(const AtomString&)
{
	D(dprintf("[MS]%s\n", __func__));
}

MediaPlayer::ReadyState MediaSourceBufferPrivateMorphOS::readyState() const
{
	D(dprintf("[MS]%s\n", __func__));
	if (m_mediaSource)
		return m_mediaSource->readyState();
	return MediaPlayer::ReadyState::HaveNothing;
}

void MediaSourceBufferPrivateMorphOS::setReadyState(MediaPlayer::ReadyState rs)
{
	D(dprintf("[MS]%s %d\n", __func__, int(rs)));
	if (m_mediaSource)
		m_mediaSource->setReadyState(rs);
}

void MediaSourceBufferPrivateMorphOS::initialize(bool success,
	WebCore::SourceBufferPrivateClient::InitializationSegment& segment,
	MediaPlayerMorphOSInfo& minfo)
{
	RefPtr<Acinerella::AcinerellaPointer> acinerella = m_reader->acinerella();

	EP_SCOPE(initialize);
	DM(dprintf("[MS]ac initialized, stream count %d\n", acinerella->instance()->stream_count));
	double duration = 0.0;
	uint32_t decoderIndexMask = 0;

	m_muxer = Acinerella::AcinerellaMuxedBuffer::create();
	m_info = minfo;

	for (int i = 0; i < std::min(Acinerella::AcinerellaMuxedBuffer::maxDecoders, acinerella->instance()->stream_count); i++)
	{
		ac_stream_info info;
		ac_get_stream_info(acinerella->instance(), i, &info);

		switch (info.stream_type)
		{
		case AC_STREAM_TYPE_VIDEO:
			DM(dprintf("video stream: %dx%d\n", info.additional_info.video_info.frame_width, info.additional_info.video_info.frame_height));
			acinerella->setDecoder(i, ac_create_decoder(acinerella->instance(), i));
			m_decoders[i] = Acinerella::AcinerellaVideoDecoder::create(this, acinerella, m_muxer, i, info, false);
			if (!!m_decoders[i])
			{
				duration = std::max(duration, m_decoders[i]->duration());
				DM(dprintf("[MS] video decoder created, duration %f\n", duration));
				decoderIndexMask |= (1ULL << i);
				ac_decoder_fake_seek(acinerella->decoder(i));
			}
			break;

		case AC_STREAM_TYPE_AUDIO:
			DM(dprintf("audio stream: %d %d %d\n", info.additional_info.audio_info.samples_per_second,
				info.additional_info.audio_info.channel_count, info.additional_info.audio_info.bit_depth));
			acinerella->setDecoder(i, ac_create_decoder(acinerella->instance(), i));
			m_decoders[i] = Acinerella::AcinerellaAudioDecoder::create(this, acinerella, m_muxer, i, info, false);
			if (!!m_decoders[i])
			{
				duration = std::max(duration, m_decoders[i]->duration());
				DM(dprintf("[MS] audio decoder created, duration %f\n", float(duration)));
				decoderIndexMask |= (1ULL << i);
				m_audioDecoderMask |= (1ULL << i);
				ac_decoder_fake_seek(acinerella->decoder(i));
			}
			break;
			
		case AC_STREAM_TYPE_UNKNOWN:
			break;
		}
	}

	if (decoderIndexMask != 0)
	{
		m_muxer->setDecoderMask(decoderIndexMask, m_audioDecoderMask);
		m_muxer->setSinkFunction([this, protectedThis = makeRef(*this)]() {
			becomeReadyForMoreSamples();
		});

		warmUp();

		DM(dprintf("[MS] duration %f size %dx%d\n", float(duration), m_info.m_width, m_info.m_height));
		WTF::callOnMainThread([segment, duration, this, protect = makeRef(*this)]() {
			DM(dprintf("[MS] calling sourceBufferPrivateDidReceiveInitializationSegment, duration %f size %dx%d\n", float(duration), m_info.m_width, m_info.m_height));
			if (m_client)
				m_client->sourceBufferPrivateDidReceiveInitializationSegment(segment);
			if (m_mediaSource)
			{
				m_mediaSource->player()->accSetDuration(duration);

				if (m_info.m_width > 0)
					m_mediaSource->player()->accSetVideoSize(m_info.m_width, m_info.m_height);
			}
		});

		m_metaInitDone = true;
	}
}

void MediaSourceBufferPrivateMorphOS::warmUp()
{
	D(dprintf("[MS] warmUp\n"));

	becomeReadyForMoreSamples();

	dispatch([this]() {
		for (int i = 0; i < Acinerella::AcinerellaMuxedBuffer::maxDecoders; i++)
		{
			if (!!m_decoders[i])
			{
				D(dprintf("[MS] warmup decoder index %d - %p\n", i, m_decoders[i].get()));
				m_decoders[i]->warmUp();
			}
		}
	});
}

void MediaSourceBufferPrivateMorphOS::threadEntryPoint()
{
	while (auto function = m_queue.waitForMessage())
	{
		(*function)();
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

void MediaSourceBufferPrivateMorphOS::play()
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
	EP_EVENT(play);
	dispatch([this] {
		D(dprintf("%s: ... \n", __PRETTY_FUNCTION__));
		for (int i = 0; i < Acinerella::AcinerellaMuxedBuffer::maxDecoders; i++)
		{
			if (!!m_decoders[i])
			{
				D(dprintf("%s: play at index %d\n", __PRETTY_FUNCTION__, i));
				m_decoders[i]->play();
			}
		}
	});
}

void MediaSourceBufferPrivateMorphOS::pause()
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
	EP_EVENT(pause);
	dispatch([this] {
		for (int i = 0; i < Acinerella::AcinerellaMuxedBuffer::maxDecoders; i++)
		{
			if (!!m_decoders[i])
			{
				m_decoders[i]->pause();
			}
		}
	});
}

void MediaSourceBufferPrivateMorphOS::onDecoderReadyToPlay(RefPtr<Acinerella::AcinerellaDecoder> decoder)
{

}

void MediaSourceBufferPrivateMorphOS::onDecoderPlaying(RefPtr<Acinerella::AcinerellaDecoder> decoder, bool playing)
{

}

void MediaSourceBufferPrivateMorphOS::onDecoderUpdatedBufferLength(RefPtr<Acinerella::AcinerellaDecoder> decoder, double buffer)
{

}

void MediaSourceBufferPrivateMorphOS::onDecoderUpdatedPosition(RefPtr<Acinerella::AcinerellaDecoder> decoder, double position)
{
	D(dprintf("[MS]%s decoder audio %d position %f\n", __func__, (1ULL << decoder->index()) & m_audioDecoderMask, float(position)));

	if ((1ULL << decoder->index()) & m_audioDecoderMask)
	{
		RefPtr<MediaSourceBufferPrivateMorphOS> me = makeRef(*this);
		if (m_mediaSource)
			m_mediaSource->onAudioSourceBufferUpdatedPosition(me, position);
	
		WTF::callOnMainThread([position, this, protect = makeRef(*this)]() {
			if (m_mediaSource)
				m_mediaSource->player()->accSetPosition(position);
		});
	}
}

void MediaSourceBufferPrivateMorphOS::onDecoderUpdatedDuration(RefPtr<Acinerella::AcinerellaDecoder> decoder, double duration)
{

}

void MediaSourceBufferPrivateMorphOS::onDecoderEnded(RefPtr<Acinerella::AcinerellaDecoder> decoder)
{

}

void MediaSourceBufferPrivateMorphOS::paint(GraphicsContext& gc, const FloatRect& rect)
{
//	DI(dprintf("[MS]%s: %p decoder %p\n", __func__, this, m_paintingDecoder.get()));
	if (!!m_paintingDecoder)
		m_paintingDecoder->paint(gc, rect);
}

void MediaSourceBufferPrivateMorphOS::setOverlayWindowCoords(struct ::Window *w, int scrollx, int scrolly, int mleft, int mtop, int mright, int mbottom)
{
	DI(dprintf("[MS]%s: %p decoder %p\n", __func__, this, m_paintingDecoder.get()));
	if (!!m_paintingDecoder)
	{
		Acinerella::AcinerellaVideoDecoder *decoder = static_cast<Acinerella::AcinerellaVideoDecoder *>(m_paintingDecoder.get());
		decoder->setOverlayWindowCoords(w, scrollx, scrolly, mleft, mtop, mright, mbottom);
	}
}

void MediaSourceBufferPrivateMorphOS::setAudioPresentationTime(double apts)
{
	if (!!m_paintingDecoder)
	{
		Acinerella::AcinerellaVideoDecoder *decoder = static_cast<Acinerella::AcinerellaVideoDecoder *>(m_paintingDecoder.get());
		decoder->setAudioPresentationTime(apts);
	}
}

void MediaSourceBufferPrivateMorphOS::onDecoderReadyToPaint(RefPtr<Acinerella::AcinerellaDecoder> decoder)
{
	DI(dprintf("[MS]%s: %p decoder %p\n", __func__, this, m_paintingDecoder.get()));
	EP_EVENT(readyToPaint);
	m_paintingDecoder = decoder;

	WTF::callOnMainThread([this, protect = makeRef(*this), decoder]() {
		if (m_mediaSource && !m_terminating)
		{
			RefPtr<MediaSourceBufferPrivateMorphOS> me = makeRef(*this);
			EP_EVENT(readyToPaintMT);
			m_mediaSource->onSourceBufferReadyToPaint(me);
		}
	});
}

void MediaSourceBufferPrivateMorphOS::onDecoderNotReadyToPaint(RefPtr<Acinerella::AcinerellaDecoder> decoder)
{
	if (decoder == m_paintingDecoder)
		m_paintingDecoder = nullptr;
}

}

#endif
