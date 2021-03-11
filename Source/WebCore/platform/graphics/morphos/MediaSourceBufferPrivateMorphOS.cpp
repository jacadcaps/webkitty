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

#define D(x) x
#define DM(x) x
#define DI(x) x
#define DN(x) x
#pragma GCC optimize ("O0")

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
	
	m_pumpThread = Thread::create("Acinerella Media Source Pump", [this] {
		pumpThread();
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

	{
		auto lock = holdLock(m_lock);
		m_bufferEOF = false;

		if (m_initializationBuffer.size() == 0)
		{
			m_initializationBuffer = WTFMove(vector);
			m_initializationBufferPosition = 0;

			WTF::callOnMainThread([this, protect = makeRef(*this)]() {
				EP_EVENT(initialAppendComplete);
				if (m_client)
					m_client->sourceBufferPrivateAppendComplete(SourceBufferPrivateClient::AppendSucceeded);
			});
		}
		else
		{
			m_buffer = WTFMove(vector);
			m_bufferPosition = 0;
		}
	}

	// wake read operation
	m_event.signal();
	m_pumpEvent.signal();
	
	dispatch([this]() {
		reinitialize();
	});
}

void MediaSourceBufferPrivateMorphOS::clearMediaSource()
{
	abort();
	m_mediaSource = nullptr;
}

void MediaSourceBufferPrivateMorphOS::seekToTime(float time)
{

}


void MediaSourceBufferPrivateMorphOS::abort()
{
	EP_SCOPE(abort);

	if (m_terminating)
		return;

	DI(dprintf("[MS]%s %p\n", __func__, this));
	
	m_terminating = true;
	m_bufferEOF = true;
	m_doPump = false;
	m_paintingDecoder = nullptr;

	m_event.signal();
	m_pumpEvent.signal();

	if (m_pumpThread)
	{
		DI(dprintf("[MS]%s %p pump shutdown\n", __func__, this));
		m_pumpThread->waitForCompletion();
		m_pumpThread = nullptr;
	}

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
	m_bufferEOF = true;
	m_event.signal();
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

void MediaSourceBufferPrivateMorphOS::flush()
{
	D(dprintf("[MS]%s\n", __func__));

	m_bufferEOF = true;
	m_event.signal();

	if (m_muxer)
		m_muxer->flush();
}

void MediaSourceBufferPrivateMorphOS::enqueueSample(Ref<MediaSample>&&sample, const AtomString&)
{
	RefPtr<Acinerella::AcinerellaPackage> package = static_cast<MediaSampleMorphOS *>(&sample.get())->package();
//	D(dprintf("[MS][%s]%s\n", __func__, (m_audioDecoderMask & (1uLL << package->index())) ? "A":"V"));
	m_muxer->push(package);
}

void MediaSourceBufferPrivateMorphOS::allSamplesInTrackEnqueued(const AtomString&)
{
	D(dprintf("[MS]%s\n", __func__));
}

bool MediaSourceBufferPrivateMorphOS::isReadyForMoreSamples(const AtomString&)
{
//	D(dprintf("[MS]%s\n", __func__));
	return true;
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

void MediaSourceBufferPrivateMorphOS::reinitialize()
{
	D(dprintf("[MS]%s\n", __func__));
	if (!m_acinerella)
		initialize();
}

bool MediaSourceBufferPrivateMorphOS::initialize()
{
	EP_SCOPE(initialize);

	m_acinerella = Acinerella::AcinerellaPointer::create();

	if (m_acinerella)
	{
		lp_ac_proberesult probe = nullptr;
		
		while (!m_terminating)
		{
			auto lock = holdLock(m_lock);
			if (m_initializationBuffer.size())
			{
				int score = 0;
				probe = ac_probe_input_buffer(m_initializationBuffer.data(), m_initializationBuffer.size(), NULL, &score);
				D(dprintf("[MS] ac_probe()-> %p\n", probe));
				break;
			}

			m_event.waitFor(10_s);
		}
		
		if (nullptr == probe)
			return false;
		
		D(dprintf("[MS] ac_open()... \n"));
		if (-1 == ac_open(m_acinerella->instance(), static_cast<void *>(this), nullptr, &acReadCallback, &acSeekCallback, nullptr, probe))
		{
			m_acinerella = nullptr;
			D(dprintf("[MS]---- ac failed to open :(\n"));
		}
		else
		{
			if (!initializeMetaData())
			{
				DM(dprintf("[MS]---- ac failed meta init :(\n"));
				return false;
			}

			DM(dprintf("[MS]ac initialized, stream count %d\n", m_acinerella->instance()->stream_count));
			double duration = 0.0;
			uint32_t decoderIndexMask = 0;

			m_muxer = Acinerella::AcinerellaMuxedBuffer::create();

			for (int i = 0; i < std::min(Acinerella::AcinerellaMuxedBuffer::maxDecoders, m_acinerella->instance()->stream_count); i++)
			{
				ac_stream_info info;
				ac_get_stream_info(m_acinerella->instance(), i, &info);

				switch (info.stream_type)
				{
				case AC_STREAM_TYPE_VIDEO:
					DM(dprintf("video stream: %dx%d\n", info.additional_info.video_info.frame_width, info.additional_info.video_info.frame_height));
					m_acinerella->setDecoder(i, ac_create_decoder(m_acinerella->instance(), i));
					m_decoders[i] = Acinerella::AcinerellaVideoDecoder::create(this, m_muxer, i, info, false);
					if (!!m_decoders[i])
					{
						duration = std::max(duration, m_decoders[i]->duration());
						DM(dprintf("[MS] video decoder created, duration %f\n", duration));
						decoderIndexMask |= (1ULL << i);
					}
					break;

				case AC_STREAM_TYPE_AUDIO:
					DM(dprintf("audio stream: %d %d %d\n", info.additional_info.audio_info.samples_per_second,
						info.additional_info.audio_info.channel_count, info.additional_info.audio_info.bit_depth));
					m_acinerella->setDecoder(i, ac_create_decoder(m_acinerella->instance(), i));
					m_decoders[i] = Acinerella::AcinerellaAudioDecoder::create(this, m_muxer, i, info, false);
					if (!!m_decoders[i])
					{
						duration = std::max(duration, m_decoders[i]->duration());
						DM(dprintf("[MS] audio decoder created, duration %f\n", float(duration)));
						decoderIndexMask |= (1ULL << i);
						m_audioDecoderMask |= (1ULL << i);
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
					if (!m_terminating)
						m_doPump = true;
					m_pumpEvent.signal();
				});

				warmUp();

				return true;
			}
		}
	}

	return false;
}

bool MediaSourceBufferPrivateMorphOS::initializeMetaData()
{
	if (m_metaInitDone)
		return true;

	auto metaCinerella = m_acinerella;//Acinerella::AcinerellaPointer::create();

	if (metaCinerella)
	{
		// Need to build and forward the InitializationSegment now...
		WebCore::SourceBufferPrivateClient::InitializationSegment initializationSegment;
		double duration = 0.0;
		m_info.m_width = 0;

		DM(dprintf("%s: streams %d\n", __func__, metaCinerella->instance()->stream_count));

		for (int i = 0; i < std::min(Acinerella::AcinerellaMuxedBuffer::maxDecoders, metaCinerella->instance()->stream_count); i++)
		{
			ac_stream_info info;
			ac_get_stream_info(metaCinerella->instance(), i, &info);

			switch (info.stream_type)
			{
			case AC_STREAM_TYPE_VIDEO:
				{
					WebCore::SourceBufferPrivateClient::InitializationSegment::VideoTrackInformation videoTrackInformation;
					videoTrackInformation.track = VideoTrackPrivateMorphOS::create(m_mediaSource->player(), i);
					videoTrackInformation.description = MediaDescriptionMorphOS::createVideoWithCodec(ac_codec_name(metaCinerella->instance(), i));
					initializationSegment.videoTracks.append(WTFMove(videoTrackInformation));
					duration = std::max(duration, std::max(double(ac_get_stream_duration(metaCinerella->instance(), i)), double(metaCinerella->instance()->info.duration)/1000.0));
					DM(dprintf("%s: video %d\n", __func__, i));
					m_info.m_width = info.additional_info.video_info.frame_width;
					m_info.m_height = info.additional_info.video_info.frame_height;
				}
				break;
				
			case AC_STREAM_TYPE_AUDIO:
				{
					WebCore::SourceBufferPrivateClient::InitializationSegment::AudioTrackInformation audioTrackInformation;
					audioTrackInformation.track = AudioTrackPrivateMorphOS::create(m_mediaSource->player(), i);
					audioTrackInformation.description = MediaDescriptionMorphOS::createAudioWithCodec(ac_codec_name(metaCinerella->instance(), i));
					initializationSegment.audioTracks.append(WTFMove(audioTrackInformation));
					duration = std::max(duration, std::max(double(ac_get_stream_duration(metaCinerella->instance(), i)), double(metaCinerella->instance()->info.duration)/1000.0));
					DM(dprintf("%s: audio %d\n", __func__, i));
				}
				break;
			}
		}

		initializationSegment.duration = MediaTime::createWithDouble(duration);
		m_info.m_duration = duration;
		m_info.m_isLive = false;
		m_info.m_channels = 0;

		WTF::callOnMainThread([initializationSegment, duration, this, protect = makeRef(*this)]() {
			DM(dprintf("[MS] calling sourceBufferPrivateDidReceiveInitializationSegment, duration %f size %dx%d\n", float(duration), m_info.m_width, m_info.m_height));
			if (m_client)
				m_client->sourceBufferPrivateDidReceiveInitializationSegment(initializationSegment);
			if (m_mediaSource) {
				m_mediaSource->player()->accSetDuration(duration);
				if (m_info.m_width > 0)
					m_mediaSource->player()->accSetVideoSize(m_info.m_width, m_info.m_height);
			}
		});
		
		m_metaInitDone = true;
		
		return true;
	}
	
	return false;
}

void MediaSourceBufferPrivateMorphOS::warmUp()
{
	D(dprintf("[MS] warmUp\n"));

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

bool MediaSourceBufferPrivateMorphOS::demuxNext()
{
	EP_SCOPE(demuxNext);

	RefPtr<Acinerella::AcinerellaPointer> acinerella;
	RefPtr<Acinerella::AcinerellaMuxedBuffer> muxer;

	{
		auto lock = holdLock(m_lock);
		acinerella = m_acinerella;
		muxer = m_muxer;
	}

	DN(dprintf("%s: %p %p %p\n", __func__ , this, muxer.get(), acinerella.get(), acinerella->instance()));

	if (muxer && acinerella && acinerella->instance() && !m_terminating)
	{
		RefPtr<Acinerella::AcinerellaPackage> package = Acinerella::AcinerellaPackage::create(acinerella, ac_read_package(acinerella->instance()));
		if (package.get() && package->package())
		{
			if (m_client)
			{
				String trackID;
				if (m_audioDecoderMask & (1uLL << package->index()))
				{
					trackID = "A" + String::number(package->index());
				}
				else
				{
					trackID = "V" + String::number(package->index());
				}
				Ref<MediaSample> mediaSample = MediaSampleMorphOS::create(package, FloatSize(320, 240), trackID);
				
				DN(dprintf("%s: %s media sample queued (%f)\n", __func__, (m_audioDecoderMask & (1uLL << package->index())) ? "audio" : "video",
					double(ac_get_stream_duration(m_acinerella->instance(), package->index()))));
				
				WTF::callOnMainThread([ms = makeRef(mediaSample.get()), this, protect = makeRef(*this)]() {
					if (m_client)
						m_client->sourceBufferPrivateDidReceiveSample(ms);
				});
				
				return true;
			}
		}
	}
	
	return false;
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

void MediaSourceBufferPrivateMorphOS::pumpThread()
{
	while (!m_terminating)
	{
		if (m_doPump)
		dprintf("%p pump wakes up...\n", this);

		while (m_doPump && !m_terminating)
		{
			if (!demuxNext() || m_bufferEOF)
			{
				dprintf("%p pump sleeps due to EOF \n", this);
				m_doPump = false;
			}
		}

		dprintf("%p pump sleeps\n", this);
		m_pumpEvent.waitFor(30_s);
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

int MediaSourceBufferPrivateMorphOS::read(uint8_t *buf, int size)
{
	EP_SCOPE(read);

	int sizeLeft = size;
	int pos = 0;

	D(dprintf("[MS]%s>> %p size %d\n", __func__, this, size));

	while (sizeLeft > 0 && !m_terminating)
	{
		bool bufferEaten = false;
		bool waitForBuffer = false;
		
		{
			auto lock = holdLock(m_lock);
			
			int initBufferSize = m_initializationBuffer.size();
			if (m_initializationBufferPosition < initBufferSize)
			{
				int toCopy = std::min(sizeLeft, initBufferSize - m_initializationBufferPosition);
			
				D(dprintf("[MS]%s: [I] left %d pos %d bs %d bp %d tc %d\n", __func__, sizeLeft, pos, initBufferSize, m_initializationBufferPosition, toCopy));
				
				if (toCopy > 0)
				{
					memcpy(buf + pos, m_initializationBuffer.data() + m_initializationBufferPosition, toCopy);
					sizeLeft -= toCopy;
					pos += toCopy;
					m_initializationBufferPosition += toCopy;
				}
				
				continue;
			}
			else if (0 == initBufferSize)
			{
				waitForBuffer = true;
			}
			else
			{
				int bufferSize = m_buffer.size();
				int toCopy = std::min(sizeLeft, bufferSize - m_bufferPosition);
				
				D(dprintf("[MS]%s: [B] left %d pos %d bs %d bp %d tc %d\n", __func__, sizeLeft, pos, bufferSize, m_bufferPosition, toCopy));
				
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
					m_acinerella = nullptr;
				}
				else if (m_buffer.size() == 0)
				{
					waitForBuffer = true;
				}
			}
		}

		if (bufferEaten)
		{
		// TODO: is this correct?

#if 0
			double duration = double(m_acinerella->instance()->info.duration)/1000.0;

			for (int i = 0; i < std::min(Acinerella::AcinerellaMuxedBuffer::maxDecoders, m_acinerella->instance()->stream_count); i++)
			{
				duration = std::max(duration, double(ac_get_stream_duration(m_acinerella->instance(), i)));
			}
			
			if (m_info.m_duration < duration)
			{
				m_info.m_duration = duration;

				WTF::callOnMainThread([duration, this, protect = makeRef(*this)]() {
					if (m_mediaSource)
						m_mediaSource->player()->accSetDuration(duration);
				});
			}
#endif

			m_doPump = false;
			
			#if 0
			WTF::callOnMainThread([this, protect = makeRef(*this)]() {
				D(dprintf("[MS]%s: calling AppendComplete...\n", __func__));
				EP_EVENT(appendComplete);
				if (m_client)
					m_client->sourceBufferPrivateAppendComplete(SourceBufferPrivateClient::AppendSucceeded);
			});
			#endif

			break;
		}
		
		if (m_bufferEOF)
		{
			break;
		}
		
		if (waitForBuffer)
		{
			EP_EVENT(wait);
			m_event.waitFor(10_s);
		}
	}
	
	D(dprintf("[MS]%s<< read %d pos %d\n", __func__, size - sizeLeft, pos));

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
	DI(dprintf("[MS]%s: %p decoder %p\n", __func__, this, m_paintingDecoder.get()));
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
