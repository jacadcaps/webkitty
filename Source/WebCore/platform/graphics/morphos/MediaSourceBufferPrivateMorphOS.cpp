#include "config.h"
#include "MediaSourcePrivateMorphOS.h"
#include "ContentType.h"

#if ENABLE(VIDEO) && ENABLE(MEDIA_SOURCE)

#include "MediaSourceBufferPrivateMorphOS.h"
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

#define D(x)
#define DR(x) //do { if (m_audioDecoderMask != 0) x; } while (0);
#define DIO(x) //do { if (m_audioDecoderMask != 0) x; } while (0);
#define DM(x)
#define DI(x)
#define DN 0
#define DNVIDEOONLY 0
#define DNERR(x)
#define DAPPEND(x) // do { if (m_audioDecoderMask != 0) x; } while (0);
#define DBR(x)
#define DRMS(x)
#define DENABLED(x)
#define DLIFETIME(x) 
#define DSEEK(x)
#define DENQ(x) // do { if (m_audioDecoderMask != 0) x; } while (0);
#define DRECEIVED(x) // do { if (m_audioDecoderMask != 0) x; } while (0);
#define DABORT(x) 
#define DENQDEBUGSTEPS 20

// #pragma GCC optimize ("O0")
// #define DEBUG_FILE

namespace WebCore {

Ref<MediaSourceBufferPrivateMorphOS> MediaSourceBufferPrivateMorphOS::create(MediaSourcePrivateMorphOS* parent)
{
	D(dprintf("[MS]%s\n", __func__));
	return adoptRef(*new MediaSourceBufferPrivateMorphOS(parent));
}

MediaSourceBufferPrivateMorphOS::MediaSourceBufferPrivateMorphOS(MediaSourcePrivateMorphOS* parent)
    : SourceBufferPrivate(*parent)
    , m_mediaSource(parent)
{
	for (int i = 0; i < Acinerella::AcinerellaMuxedBuffer::maxDecoders; i++)
	{
		m_decodersStarved[i] = false;
        m_enabled[i] = false;
        m_maxBuffer[i] = 0;
        m_maxPackets[i] = 0;
        m_decoderReadyForMore[i] = true;
        m_notifyRequested[i] = false;
	}

    m_reader = MediaSourceChunkReader::create(*this);
	m_thread = Thread::create("MediaSourceBuffer"_s, [this] {
		threadEntryPoint();
	});

	DLIFETIME(dprintf("[MS]%s: %p hello!\n", __func__, this));
}

MediaSourceBufferPrivateMorphOS::~MediaSourceBufferPrivateMorphOS()
{
	DLIFETIME(dprintf("[MS]%s: %p bye!\n", __func__, this));
	clearMediaSource();
}

Ref<MediaPromise> MediaSourceBufferPrivateMorphOS::appendInternal(Ref<SharedBuffer>&& buffer)
{
	EP_EVENT(append);
	DAPPEND(dprintf("[MS][%c]%s bytes %lu main %d appendcnt %d\n", m_audioDecoderMask == 0 ?'V':'A', __func__, buffer->size(), isMainThread(), m_appendCount));

    m_appendPromise.emplace();

    m_reader->decodeAsync(std::move(buffer))->then(RunLoop::mainSingleton(),[protectedThis = Ref { *this }, this](MediaSourceChunkReader::DecodeResult result) {
        DAPPEND(dprintf("[MS][%c]appendInternal result %d\n", m_audioDecoderMask == 0 ?'V':'A', int(result)));
        if (!m_appendPromise)
            return;

        switch (result)
        {
        case MediaSourceChunkReader::DecodeResult::InitialInitialize:
            if (initialize(InitializeMode::First))
                m_appendPromise->resolve();
            else
                m_appendPromise->reject(PlatformMediaError::ParsingError);
            break;

        case MediaSourceChunkReader::DecodeResult::Reinitialize:
            if (initialize(InitializeMode::Next))
                m_appendPromise->resolve();
            else
                m_appendPromise->reject(PlatformMediaError::DecoderCreationError);
            break;

        case MediaSourceChunkReader::DecodeResult::Samples:
            if (appendComplete())
                m_appendPromise->resolve();
            else
                m_appendPromise->reject(PlatformMediaError::AppendError);
            break;
        }
    }, [protectedThis = Ref { *this }, this](void) {
        DAPPEND(dprintf("[MS][%c]appendInternal error\n", m_audioDecoderMask == 0 ?'V':'A'));
        if (m_appendPromise)
            m_appendPromise->reject(PlatformMediaError::AppendError);
    });

    return *m_appendPromise;
}

bool MediaSourceBufferPrivateMorphOS::appendComplete()
{
	DAPPEND(dprintf("[MS][%c]%s: %p swf %d main %d pending %d\n", m_audioDecoderMask == 0 ?'V':'A', __func__, this, m_seeking, isMainThread(), !!m_appendCompletePending));

    if (m_terminating)
        return false;

    RefPtr mediaSource = m_mediaSource.get();
    if (!mediaSource)
        return false;

    if (!createDecoders())
        return false;

    MediaSourceChunkReader::MediaSamplesList samples;
    m_reader->getSamples(samples);

    m_appendCompleteCount ++;

    DAPPEND(dprintf("[MS][%c]%s: %p %d, queue %d samples from %f-%f\n",  m_audioDecoderMask == 0 ?'V':'A',__func__, this, samples.size(), samples.size()?(*(samples.begin()))->presentationTime().toFloat():-1,samples.size()?(*(samples.rbegin()))->presentationTime().toFloat():-1));

    float previousTime = -1;
    for (auto sample : samples)
    {
        DRECEIVED(dprintf("[MS][%c]%s: %p received sample @ %f\n",  m_audioDecoderMask == 0 ?'V':'A',__func__, this, sample->presentationTime().toFloat()));
        didReceiveSample(*sample.get());
    }

    if (samples.size())
    {
        auto lastSample = samples.rbegin();
        auto lastPTS = (*lastSample)->presentationTime().toFloat();
        if (m_info.m_duration < lastPTS)
            m_info.m_duration = lastPTS;
    }

    mediaSource->onSourceBufferLoadingProgressed();

    bool isReady = true;
    if (!m_seeking)
    {
        for (int i = 0; i < m_numDecoders; i++)
        {
            if (!!m_decoders[i] && m_decoders[i]->isWarmedUp())
            {
                if (m_muxer->bytesForDecoder(i) > (m_maxBuffer[i] / 2))
                {
                    isReady = false;
                    break;
                }
            }
        }
    }

    if (isReady)
    {
    }

    return true;
}

void MediaSourceBufferPrivateMorphOS::willSeek(double time)
{
	DSEEK(dprintf("[MS]%s %p to %f appendPending %d\n", __func__, this, float(time), m_appendCompleteDelayed));

	m_seeking = true;
	m_seekTime = time;
	m_postSeekingAppendDone = false;
	m_readerFailed = false;
    m_ended = false;

	for (int i = 0; i < m_numDecoders; i++)
	{
		if (!!m_decoders[i])
		{
			m_decoders[i]->pause(true);
		}
	}

	flush();

	m_appendCompleteCount = 0;
	m_appendCount = 0;
	m_mustAppendInitializationSegment = true;
}

void MediaSourceBufferPrivateMorphOS::seekToTime(const MediaTime&mt)
{
	DSEEK(dprintf("[MS]%s %p to %f\n", __func__, this, mt.toFloat()));

	if (m_seeking)
		m_seeking = false;

	SourceBufferPrivate::seekToTime(mt);
}

bool MediaSourceBufferPrivateMorphOS::isSeeking() const
{
    return m_seeking;
}

void MediaSourceBufferPrivateMorphOS::setVolume(double vol)
{
	for (int i = 0; i < m_numDecoders; i++)
	{
		if (!!m_decoders[i])
		{
			m_decoders[i]->setVolume(vol);
		}
	}
}

void MediaSourceBufferPrivateMorphOS::clearMediaSource()
{
	DAPPEND(dprintf("[MS]%s %p main %d\n", __func__, this, isMainThread()));

    if (isMainThread())
    {
        if (m_appendPromise)
        {
            m_appendPromise->reject(PlatformMediaError::BufferRemoved);
            m_appendPromise.reset();
        }

        terminate();
        m_mediaSource = nullptr;
    }
    else
    {
        WTF::callOnMainThread([this, protect = Ref{*this}]() {
            clearMediaSource();
        });
    }
}

void MediaSourceBufferPrivateMorphOS::abort()
{
	// TODO: implement this
	DABORT(dprintf("[MS]%s %p\n", __func__, this));
	SourceBufferPrivate::abort();
}

void MediaSourceBufferPrivateMorphOS::terminate()
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

	for (int i = 0; i < m_numDecoders; i++)
	{
		if (!!m_decoders[i])
		{
			m_decoders[i]->terminate();
			DI(dprintf("[MS]%s %p decoder %p shutdown\n", __func__, this, m_decoders[i].get()));
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

void MediaSourceBufferPrivateMorphOS::resetParserStateInternal()
{
	D(dprintf("[MS]%s\n", __func__));
	m_appendCompletePending = false;
}

void MediaSourceBufferPrivateMorphOS::removedFromMediaSource()
{
	D(dprintf("[MS]%s\n", __func__));
	terminate();
	RefPtr<MediaSourceBufferPrivateMorphOS> me = Ref{*this};
    RefPtr mediaSource = m_mediaSource.get();
	if (mediaSource)
		mediaSource->onSourceBufferRemoved(me);
	SourceBufferPrivate::removedFromMediaSource();
}

void MediaSourceBufferPrivateMorphOS::onTrackEnabled(int index, bool enabled)
{
	DENABLED(dprintf("[MS]%s: %d enabled %d\n", __func__, index, enabled));
	if (index >= Acinerella::AcinerellaMuxedBuffer::maxDecoders)
		return;

	RefPtr mediaSource = m_mediaSource.get();
	if (mediaSource)
	{
		m_enabled[index] = enabled;
		if (enabled)
		{
			if (!!m_decoders[index])
			{
				m_decoders[index]->setEnabled(true);
				if (mediaSource->paused())
				{
					m_decoders[index]->warmUp();
				}
				else
				{
					m_decoders[index]->prePlay();
				}
			}
		}
		else if (!enabled)
		{
			if (!!m_decoders[index])
			{
				m_decoders[index]->setEnabled(false);
				m_decoders[index]->coolDown();
			}
		}
	}
}

void MediaSourceBufferPrivateMorphOS::dumpStatus()
{
	int numDec = 0;
	for (int i = 0; i < m_numDecoders; i++)
	{
		if (!!m_decoders[i])
			numDec++;
	}
	dprintf("\033[36m[MSB%p]: DEC %d SEEK %d TERM %d RMS %d AC %d ACC %d ENQCNT %d PEND %d\033[0m\n", this, numDec, m_seeking, m_terminating, m_readyForMoreSamples, m_appendCount, m_appendCompleteCount, m_enqueueCount, m_appendCompleteDelayed);
	for (int i = 0; i < m_numDecoders; i++)
	{
		if (!!m_decoders[i])
		{
			dprintf("[%d][%s]", m_muxer->packagesForDecoder(i), m_decoders[i]->isEnabled() ? "EN" : "DI");
			m_decoders[i]->dumpStatus();
		}
	}
	dprintf("\033[36m[MSB%p]: -- \033[0m\n", this);
}

void MediaSourceBufferPrivateMorphOS::getFrameCounts(unsigned& decoded, unsigned &dropped) const
{
	Acinerella::AcinerellaVideoDecoder *decoder = static_cast<Acinerella::AcinerellaVideoDecoder *>(m_paintingDecoder.get());
	if (decoder)
	{
		decoded = decoder->decodedFrameCount();
		dropped = decoder->droppedFrameCount();
	}
	else
	{
		decoded = dropped = 0;
	}
}

void MediaSourceBufferPrivateMorphOS::flush(TrackID trackID)
{
	if (trackID < Acinerella::AcinerellaMuxedBuffer::maxDecoders)
	{
		m_muxer->flush(trackID);
		// Emptied the muxer queue for this track: re-open the backpressure gate.
		m_decoderReadyForMore[trackID] = true;
		m_notifyRequested[trackID] = false;
		auto acinerella = m_reader->acinerella();
		if (!!acinerella)
		{
			RefPtr<Acinerella::AcinerellaPackage> package = Acinerella::AcinerellaPackage::create(m_reader->acinerella(), ac_flush_packet());
			m_muxer->push(package, trackID);
		}
	}
}

void MediaSourceBufferPrivateMorphOS::becomeReadyForMoreSamples(int index)
{
	// Called from the muxer sink (decoder thread) once a decoder has drained its muxer queue below the
	// low-water mark. Re-open the backpressure gate and, if the WebCore MSE core asked to be told when
	// we are ready again (notifyClientWhenReadyForMoreSamples), pump more data on the dispatcher thread.
	if (index < 0 || index >= Acinerella::AcinerellaMuxedBuffer::maxDecoders)
		return;

	DRMS(dprintf("[MS]%s: %d seeking %d (%d)\n", __func__, index, m_seeking, isSeeking()));

	m_decoderReadyForMore[index] = true;

	if (m_notifyRequested[index].exchange(false))
	{
		WTF::callOnMainThread([this, protect = Ref{*this}, index]() {
			if (m_terminating)
				return;
			RefPtr mediaSource = m_mediaSource.get();
			if (!mediaSource)
				return;
			provideMediaData(TrackID(index));
		});
	}
}

void MediaSourceBufferPrivateMorphOS::flush()
{
	D(dprintf("[MS]%s\n", __func__));

	if (m_muxer && m_reader)
	{
		m_muxer->flush();
// TODO: how?
		RefPtr<Acinerella::AcinerellaPackage> package = Acinerella::AcinerellaPackage::create(m_reader->acinerella(), ac_flush_packet());
		m_muxer->push(package);

		for (int i = 0; i < m_numDecoders; i++)
		{
			m_decodersStarved[i] = false;
			// Emptied the muxer queues: re-open the backpressure gate for every track.
			m_decoderReadyForMore[i] = true;
			m_notifyRequested[i] = false;
		}
	}
}

void MediaSourceBufferPrivateMorphOS::enqueueSample(Ref<MediaSample>&&sample, TrackID)
{
	auto msample = static_cast<MediaSampleMorphOS *>(&sample.get());
	RefPtr<Acinerella::AcinerellaPackage> package = msample->package();
	int index = package->index();

    m_enqueuedSamples = true;
	m_enqueueCount ++;

	DENQ(if (0 == (m_enqueueCount % DENQDEBUGSTEPS) || (msample->isSync() && !(m_audioDecoderMask & (1uLL << package->index())))) dprintf("[MS][%s]%s PTS %f key %d seeking %f\n", __func__, (m_audioDecoderMask & (1uLL << package->index())) ? "A":"V", msample->presentationTime().toFloat(), msample->isSync(), m_seeking?float(m_seekTime):-1.0f));

	if (index >= Acinerella::AcinerellaMuxedBuffer::maxDecoders)
		return;

	m_requestedMoreFrames = false;
	m_eos = false;

	if (m_seeking)
		return;

	m_muxer->push(package);

	// Backpressure: once this decoder's muxer queue reaches a high-water mark, close its gate so the
	// WebCore MSE core stops pulling samples out of the TrackBuffer until the decoder drains it again.
	// We cap on BYTES (memory) and PACKETS (time ahead of currentTime); the packet cap is what keeps
	// us from enqueuing tens of seconds ahead on low-bitrate streams and getting flushed by re-appends.
	// (m_maxBuffer is only non-zero once createDecoders() has run; until then we keep accepting.)
	if ((m_maxBuffer[index] && m_muxer->bytesForDecoder(index) >= m_maxBuffer[index])
		|| (m_maxPackets[index] && uint32_t(m_muxer->packagesForDecoder(index)) >= m_maxPackets[index]))
		m_decoderReadyForMore[index] = false;

	if (!!m_decoders[index] && m_muxer->bytesForDecoder(index) >= m_maxBuffer[index] &&
		m_decodersStarved[index])
	{
		m_decodersStarved[index] = false;
		m_decoders[index]->warmUp();
	}
}

void MediaSourceBufferPrivateMorphOS::allSamplesInTrackEnqueued(TrackID)
{
	D(dprintf("[MS]%s\n", __func__));
	m_eos = true;
	RefPtr<Acinerella::AcinerellaPackage> nothing;
    if (m_muxer)
        m_muxer->push(nothing);
}

bool MediaSourceBufferPrivateMorphOS::isReadyForMoreSamples(TrackID trackID)
{
	int index = int(trackID);
	if (index < 0 || index >= Acinerella::AcinerellaMuxedBuffer::maxDecoders)
		return false;
	// Before the decoders/muxer exist (m_maxBuffer still 0) we have no queue to gauge, so accept samples;
	// otherwise report the per-track backpressure gate.
	if (!m_maxBuffer[index])
		return true;
	return m_decoderReadyForMore[index].load();
}

void MediaSourceBufferPrivateMorphOS::setActive(bool isActive)
{
	D(dprintf("[MS]%s\n", __func__));
	// Chain to the base so the WebCore MSE core's active-source-buffer set is populated. Without this the
	// base m_activeSourceBuffers stays empty for the player's lifetime, leaving base hasAudio()/hasVideo(),
	// updateTracksType() and notifyActiveSourceBuffersChanged() dead. The MorphOS-specific reactions below
	// (painting buffer, warm/cool, volume) are still driven through onSourceBufferDidChangeActiveState().
	SourceBufferPrivate::setActive(isActive);
    RefPtr mediaSource = m_mediaSource.get();
    if (mediaSource)
    {
		RefPtr<MediaSourceBufferPrivateMorphOS> me = Ref{*this};
        mediaSource->onSourceBufferDidChangeActiveState(me, isActive);
	}
}

void MediaSourceBufferPrivateMorphOS::notifyClientWhenReadyForMoreSamples(TrackID trackID)
{
	int index = int(trackID);
	if (index < 0 || index >= Acinerella::AcinerellaMuxedBuffer::maxDecoders)
		return;

	// The WebCore MSE core stopped pulling because isReadyForMoreSamples() returned false. Record that it
	// wants to be pumped again; becomeReadyForMoreSamples() will call provideMediaData() once the decoder
	// drains the muxer queue. If the gate is already open (drained in the meantime), pump immediately.
	m_notifyRequested[index] = true;

	if (m_decoderReadyForMore[index].load() && m_notifyRequested[index].exchange(false))
	{
		WTF::callOnMainThread([this, protect = Ref{*this}, index]() {
			if (m_terminating)
				return;
			RefPtr mediaSource = m_mediaSource.get();
			if (!mediaSource)
				return;
			provideMediaData(TrackID(index));
		});
	}
}

bool MediaSourceBufferPrivateMorphOS::canSetMinimumUpcomingPresentationTime(TrackID) const
{
	DBR(dprintf("[MS]%s\n", __func__));
	// WARNING: HACK
	// SourceBuffer calls this after a batch of enqueueSample calls
	auto *me = const_cast<MediaSourceBufferPrivateMorphOS *>(this);
    if (m_enqueuedSamples)
    {
        me->m_enqueuedSamples = false;
        me->warmUp();
    }
	return false;
}

bool MediaSourceBufferPrivateMorphOS::initialize(InitializeMode mode)
{
    DI(dprintf("[MS]%s: mode %d\n", __func__, int(mode)));
    RefPtr mediaSource = m_mediaSource.get();
    if (!mediaSource)
        return false;

    m_info = m_reader->getInfo();

    DI(dprintf("[MS]%s: mode %d info width %d channels %d\n", __func__, int(mode), m_info.m_width, m_info.m_channels));
    m_metaInitDone = true;

    if (InitializeMode::First == mode)
    {
        didReceiveInitializationSegment(m_reader->getInitializationSegment());
		RefPtr<MediaSourceBufferPrivateMorphOS> me = Ref{*this};
        mediaSource->onSourceBufferInitialized(me);
    }
    else
    {
		m_mustReinitializeDecoders = true;
		RefPtr<MediaSourceBufferPrivateMorphOS> me = Ref{*this};
		mediaSource->onSourceBufferInitialized(me);
    }

    return true;
}

static void toMorphOSInfo(MediaPlayerMorphOSInfo& mInfo, const ac_stream_info &info)
{
    auto& video = info.additional_info.video_info;
    auto& audio = info.additional_info.audio_info;

    switch (info.stream_type)
    {
    case AC_STREAM_TYPE_VIDEO:
        mInfo.m_fps = video.frames_per_second;
        mInfo.m_width = video.frame_width;
        mInfo.m_height = video.frame_height;
        break;
    case AC_STREAM_TYPE_AUDIO:
        mInfo.m_bits = audio.bit_depth;
        mInfo.m_channels = audio.channel_count;
        if (mInfo.m_channels == 0)
            mInfo.m_channels = 2; // meh
        mInfo.m_frequency = audio.samples_per_second;
        break;
    default: break;
    }
}

bool MediaSourceBufferPrivateMorphOS::createDecoders()
{
    RefPtr<Acinerella::AcinerellaPointer> acinerella = m_reader->acinerella();
    if (!acinerella)
		return false;

	if (m_muxer && m_mustReinitializeDecoders)
	{
		RefPtr<Acinerella::AcinerellaPointer> acinerella = m_reader->acinerella();
		uint32_t audioDecoderMask = 0;
		uint32_t decoderIndexMask = 0;

		DI(dprintf("[MS]%s: reinitializing decoders cnt %d\n", __func__, m_reader->numDecoders()));

		for (int i = 0; i < m_reader->numDecoders(); i++)
		{
			ac_stream_info info;
			ac_get_stream_info(acinerella->instance(), i, &info);

			switch (info.stream_type)
			{
			case AC_STREAM_TYPE_VIDEO:
				decoderIndexMask |= (1UL << i);
				break;
			case AC_STREAM_TYPE_AUDIO:
				decoderIndexMask |= (1UL << i);
				audioDecoderMask |= (1UL << i);
				break;
			}
		}

		if (m_audioDecoderMask != audioDecoderMask || m_numDecoders != m_reader->numDecoders())
		{
			dprintf("[MS]%s: failed re-initializing decoders due to media mask change. Please report this Wayfarer error!\n", __func__);
			return false;
		}

		for (int i = 0; i < m_reader->numDecoders(); i++)
		{
			ac_stream_info info;
			ac_get_stream_info(acinerella->instance(), i, &info);

			switch (info.stream_type)
			{
			case AC_STREAM_TYPE_VIDEO:
				acinerella->setDecoder(i, ac_create_decoder(acinerella->instance(), i));
				ac_decoder_fake_seek(acinerella->decoder(i));
				break;
			case AC_STREAM_TYPE_AUDIO:
				acinerella->setDecoder(i, ac_create_decoder(acinerella->instance(), i));
				ac_decoder_fake_seek(acinerella->decoder(i));
				break;
			}
		}

		m_audioDecoderMask = audioDecoderMask;
		m_mustReinitializeDecoders = false;
        warmUp();
        return true;
	}

    if (m_muxer)
        return true;

    double duration = 0.0;
    uint32_t decoderIndexMask = 0;

	if (!m_muxer)
		m_muxer = Acinerella::AcinerellaMuxedBuffer::create();
    DI(dprintf("[MS]%s: muxer created. decoders %d\n", __func__, m_reader->numDecoders()));

    m_audioDecoderMask = 0;
    m_numDecoders = m_reader->numDecoders();
    m_mustReinitializeDecoders = false;

    MediaPlayerMorphOSInfo mInfo;

    for (int i = 0; i < m_numDecoders; i++)
    {
        ac_stream_info info;
        ac_get_stream_info(acinerella->instance(), i, &info);

        switch (info.stream_type)
        {
        case AC_STREAM_TYPE_VIDEO:
            DI(dprintf("video stream: %dx%d\n", info.additional_info.video_info.frame_width, info.additional_info.video_info.frame_height));
            acinerella->setDecoder(i, ac_create_decoder(acinerella->instance(), i));
            m_decoders[i] = Acinerella::AcinerellaVideoDecoder::create(this, acinerella, m_muxer, i, info, false, false);
            if (!!m_decoders[i])
            {
                duration = std::max(duration, m_decoders[i]->duration());
                DM(dprintf("[MS] video decoder created, duration %f\n", duration));
                decoderIndexMask |= (1ULL << i);
                ac_decoder_fake_seek(acinerella->decoder(i));
                Acinerella::AcinerellaVideoDecoder *vdecoder = static_cast<Acinerella::AcinerellaVideoDecoder *>(m_decoders[i].get());
                vdecoder->setCanDropKeyFrames(true); // needed for Media Source to function better on seek/catchup, we don't want this for single-file non-MS playback
                if (m_enabled[i])
                    m_decoders[i]->setEnabled(true);
                toMorphOSInfo(mInfo, info);
                mInfo.m_videoCodec = m_decoders[i]->codec();
                mInfo.m_bitRate = m_decoders[i]->bitRate();
            }
            break;

        case AC_STREAM_TYPE_AUDIO:
            DI(dprintf("audio stream: %d %d %d\n", info.additional_info.audio_info.samples_per_second,
                info.additional_info.audio_info.channel_count, info.additional_info.audio_info.bit_depth));
            acinerella->setDecoder(i, ac_create_decoder(acinerella->instance(), i));
            m_decoders[i] = Acinerella::AcinerellaAudioDecoder::create(this, acinerella, m_muxer, i, info, false, false);
            if (!!m_decoders[i])
            {
                duration = std::max(duration, m_decoders[i]->duration());
                DM(dprintf("[MS] audio decoder created, duration %f\n", float(duration)));
                decoderIndexMask |= (1ULL << i);
                m_audioDecoderMask |= (1ULL << i);
                ac_decoder_fake_seek(acinerella->decoder(i));
                if (m_enabled[i])
                    m_decoders[i]->setEnabled(true);
                toMorphOSInfo(mInfo, info);
                mInfo.m_audioCodec = m_decoders[i]->codec();
            }
            break;
            
        case AC_STREAM_TYPE_UNKNOWN:
            break;
        }
    }

    if (decoderIndexMask != 0)
    {
        DI(dprintf("[MS] decoder mask %x %x\n", decoderIndexMask, m_audioDecoderMask));
        m_muxer->setDecoderMask(decoderIndexMask, m_audioDecoderMask);
        for (int i = 0; i < std::min(Acinerella::AcinerellaMuxedBuffer::maxDecoders, acinerella->instance()->stream_count); i++) {
            // Bound the compressed muxer backlog to a few read-ahead windows (decoder-derived) instead
            // of the fixed multi-MB cap. A smaller backlog means a fall-behind/seek has far less stale
            // data to grind through, avoiding the video-decoder CPU spike and frozen-picture stalls.
            if (!!m_decoders[i])
            {
                m_maxBuffer[i] = m_decoders[i]->maxCompressedBufferSize();
                m_maxPackets[i] = m_decoders[i]->maxCompressedPackets();
            }
            else
            {
                m_maxBuffer[i] = m_muxer->maxBufferSizeForMediaSourceDecoder(i);
                m_maxPackets[i] = 0;
            }
        }

        m_muxer->setSinkFunction([this, protectedThis = Ref{*this}](int decoderIndex, int left, uint32_t bytesInBuffer) {
            // Re-open the gate only when BOTH the byte and packet queues have drained below half their
            // high-water marks. The packet half-mark is what bounds how far ahead of currentTime we
            // re-enqueue (so re-appends stop overlapping our enqueued samples and flushing us).
            bool bytesOk = bytesInBuffer < m_maxBuffer[decoderIndex] / 2;
            bool packetsOk = !m_maxPackets[decoderIndex] || uint32_t(left) < std::max(1u, m_maxPackets[decoderIndex] / 2);
            if (bytesOk && packetsOk)
                becomeReadyForMoreSamples(decoderIndex);
            return false; // avoid blocking the pipeline!
        });

        // force mediasource to update the metadata as it might have changed/become more complete
        RefPtr mediaSource = m_mediaSource.get();
		RefPtr<MediaSourceBufferPrivateMorphOS> me = Ref{*this};
        m_info = mInfo;
        if (mediaSource)
            mediaSource->onSourceBufferInitialized(me);

        warmUp();
    }
    return true;
}

void MediaSourceBufferPrivateMorphOS::warmUp()
{
	D(dprintf("[MS] warmUp\n"));

	dispatch([this]() {
		for (int i = 0; i < m_numDecoders; i++)
		{
			if (!!m_decoders[i] && m_decoders[i]->isEnabled())
			{
				D(dprintf("[MS] warmup decoder index %d - %p\n", i, m_decoders[i].get()));
				m_decoders[i]->warmUp();
			}
		}
	});
}

void MediaSourceBufferPrivateMorphOS::coolDown()
{
	dispatch([this]() {
		for (int i = 0; i < m_numDecoders; i++)
		{
			if (!!m_decoders[i] && m_decoders[i]->isEnabled())
			{
				D(dprintf("[MS] coolDown decoder index %d - %p\n", i, m_decoders[i].get()));
				m_decoders[i]->coolDown();
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

RefPtr<VideoTrackPrivateMorphOS> MediaSourceBufferPrivateMorphOS::videoTrack(int index) const
{
    return VideoTrackPrivateMorphOSMS::create(const_cast<MediaSourceBufferPrivateMorphOS*>(this), index);
}

RefPtr<AudioTrackPrivateMorphOS> MediaSourceBufferPrivateMorphOS::audioTrack(int index) const
{
    return AudioTrackPrivateMorphOSMS::create(const_cast<MediaSourceBufferPrivateMorphOS*>(this), index);
}

void MediaSourceBufferPrivateMorphOS::play()
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
	EP_EVENT(play);
	dispatch([this] {
		D(dprintf("%s: ... numd %d \n", __PRETTY_FUNCTION__, m_numDecoders));
		for (int i = 0; i < m_numDecoders; i++)
		{
            D(dprintf("%s: play at index %d. enabled %d\n", __PRETTY_FUNCTION__, i, !!m_decoders[i] && m_decoders[i]->isEnabled()));
			if (!!m_decoders[i] && m_decoders[i]->isEnabled())
			{
				m_decoders[i]->play();
			}
		}
	});
}

void MediaSourceBufferPrivateMorphOS::prePlay()
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
	EP_EVENT(play);
	dispatch([this] {
		D(dprintf("%s: ... \n", __PRETTY_FUNCTION__));
		for (int i = 0; i < m_numDecoders; i++)
		{
			if (!!m_decoders[i] && m_decoders[i]->isEnabled())
			{
				D(dprintf("%s: play at index %d\n", __PRETTY_FUNCTION__, i));
				m_decoders[i]->prePlay();
			}
		}
	});
}

void MediaSourceBufferPrivateMorphOS::pause()
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
	EP_EVENT(pause);
	dispatch([this] {
		for (int i = 0; i < m_numDecoders; i++)
		{
			if (!!m_decoders[i] && m_decoders[i]->isEnabled())
			{
				m_decoders[i]->pause();
			}
		}
	});
}

const WebCore::MediaPlayerMorphOSStreamSettings& MediaSourceBufferPrivateMorphOS::streamSettings()
{
	static WebCore::MediaPlayerMorphOSStreamSettings defaults;
    RefPtr mediaSource = m_mediaSource.get();
    if (mediaSource)
	{
		return mediaSource->streamSettings();
	}
	return defaults;
}

void MediaSourceBufferPrivateMorphOS::onDecoderWarmedUp(RefPtr<Acinerella::AcinerellaDecoder>)
{
	D(dprintf("%s: allreadytoplay %d\n", __PRETTY_FUNCTION__, areDecodersReadyToPlay()));
}

void MediaSourceBufferPrivateMorphOS::onDecoderReadyToPlay(RefPtr<Acinerella::AcinerellaDecoder>)
{
	D(dprintf("%s: decoders ready: %d\n", __PRETTY_FUNCTION__, areDecodersReadyToPlay()));
	if (areDecodersReadyToPlay())
	{
        RefPtr mediaSource = m_mediaSource.get();
        if (mediaSource)
            mediaSource->onSourceBuffersReadyToPlay();
	}
}

void MediaSourceBufferPrivateMorphOS::onDecoderPlaying(RefPtr<Acinerella::AcinerellaDecoder>, bool)
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));

}

void MediaSourceBufferPrivateMorphOS::onDecoderUpdatedBufferLength(RefPtr<Acinerella::AcinerellaDecoder>, double)
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));

}

void MediaSourceBufferPrivateMorphOS::onDecoderUpdatedPosition(RefPtr<Acinerella::AcinerellaDecoder> decoder, double position)
{
	D(dprintf("[MS]%s decoder %p isaudio %d mask %x position %f\n", __func__, decoder.get(), ((1ULL << decoder->index()) & m_audioDecoderMask) ? 1 : 0,
		m_audioDecoderMask, float(position)));

    RefPtr mediaSource = m_mediaSource.get();
    if (mediaSource)
	{
		RefPtr<MediaSourceBufferPrivateMorphOS> me = Ref{*this};
		if ((1ULL << decoder->index()) & m_audioDecoderMask)
			mediaSource->onAudioSourceBufferUpdatedPosition(me, position);
		else if (0 == m_audioDecoderMask)
			mediaSource->onVideoSourceBufferUpdatedPosition(me, position);
	}
}

void MediaSourceBufferPrivateMorphOS::onDecoderUpdatedDuration(RefPtr<Acinerella::AcinerellaDecoder>, double)
{
	// live streams
}

void MediaSourceBufferPrivateMorphOS::onDecoderEnded(RefPtr<Acinerella::AcinerellaDecoder> decoder)
{
	WTF::callOnMainThread([this, protect = Ref{*this}, decoder]() {
        RefPtr mediaSource = m_mediaSource.get();
		if (mediaSource && !m_terminating)
		{
			RefPtr<MediaSourceBufferPrivateMorphOS> me = Ref{*this};
            m_ended = true;
			mediaSource->onSourceBufferEnded(me);
		}
	});
}

void MediaSourceBufferPrivateMorphOS::paint(GraphicsContext& gc, const FloatRect& rect)
{
//	DI(dprintf("[MS]%s: %p decoder %p\n", __func__, this, m_paintingDecoder.get()));
	if (!!m_paintingDecoder)
		m_paintingDecoder->paint(gc, rect);
}

void MediaSourceBufferPrivateMorphOS::setOverlayWindowCoords(struct ::Window *w, int scrollx, int scrolly, int mleft, int mtop, int mright, int mbottom, int width, int height)
{
	DI(dprintf("[MS]%s: %p decoder %p\n", __func__, this, m_paintingDecoder.get()));
	if (!!m_paintingDecoder)
	{
		Acinerella::AcinerellaVideoDecoder *decoder = static_cast<Acinerella::AcinerellaVideoDecoder *>(m_paintingDecoder.get());
		decoder->setOverlayWindowCoords(w, scrollx, scrolly, mleft, mtop, mright, mbottom, width, height);
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

void MediaSourceBufferPrivateMorphOS::clearAudioPresentationTime()
{
	if (!!m_paintingDecoder)
	{
		Acinerella::AcinerellaVideoDecoder *decoder = static_cast<Acinerella::AcinerellaVideoDecoder *>(m_paintingDecoder.get());
		decoder->clearAudioPresentationTime();
	}
}

bool MediaSourceBufferPrivateMorphOS::areDecodersReadyToPlay()
{
	for (int i = 0; i < m_numDecoders; i++)
	{
		if (!!m_decoders[i] && m_decoders[i]->isEnabled())
		{
			if (!m_decoders[i]->isReadyToPlay())
            {
                D(dprintf("[MS]%s: not yet ready index %d type %s\n", __func__, i, m_decoders[i]->isAudio() ? "A" : "V"));
				return false;
            }
		}
	}
	return true;
}

bool MediaSourceBufferPrivateMorphOS::areDecodersPlaying()
{
	for (int i = 0; i < m_numDecoders; i++)
	{
		if (!!m_decoders[i] && m_decoders[i]->isEnabled() && !m_decoders[i]->isPlaying())
			return false;
	}

	return true;
}

float MediaSourceBufferPrivateMorphOS::decodersBufferedTime()
{
	float buffer = -1.f;

	for (int i = 0; i < m_numDecoders; i++)
	{
		if (!!m_decoders[i] && m_decoders[i]->isEnabled())
			buffer = std::max(buffer, float(m_decoders[i]->bufferSize()));
	}
	
	return buffer;
}

void MediaSourceBufferPrivateMorphOS::onDecoderWantsToRender(RefPtr<Acinerella::AcinerellaDecoder> decoder)
{
	DI(if (!m_paintingDecoder) dprintf("[MS]%s: %p decoder %p\n", __func__, this, m_paintingDecoder.get()));
	EP_EVENT(readyToPaint);
	m_paintingDecoder = decoder;

	WTF::callOnMainThread([this, protect = Ref{*this}, decoder]() {
        RefPtr mediaSource = m_mediaSource.get();
		if (mediaSource && !m_terminating)
		{
			RefPtr<MediaSourceBufferPrivateMorphOS> me = Ref{*this};
			EP_EVENT(readyToPaintMT);
			mediaSource->onSourceBufferReadyToPaint(me);
		}
	});
}

void MediaSourceBufferPrivateMorphOS::onDecoderNotReadyToRender(RefPtr<Acinerella::AcinerellaDecoder> decoder)
{
	if (decoder == m_paintingDecoder)
		m_paintingDecoder = nullptr;
}

void MediaSourceBufferPrivateMorphOS::onDecoderRenderUpdate(RefPtr<Acinerella::AcinerellaDecoder> decoder)
{
	if (decoder == m_paintingDecoder)
	{
		WTF::callOnMainThread([this, protect = Ref{*this}, decoder]() {
            RefPtr mediaSource = m_mediaSource.get();
			if (mediaSource && !m_terminating) {
				RefPtr<MediaSourceBufferPrivateMorphOS> me = Ref{*this};
				mediaSource->onSourceBufferFrameUpdate(me);
			}
		});
	}
}

} // namespace
#endif
