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
#define DN 1
#define DNVIDEOONLY 1
#define DNERR(x) x
#define DAPPEND(x) // do { if (m_audioDecoderMask != 0) x; } while (0);
#define DBR(x)
#define DRMS(x)
#define DENABLED(x) x
#define DLIFETIME(x)
#define DSEEK(x) x
#define DENQ(x) // do { if (m_audioDecoderMask != 0) x; } while (0);
#define DRECEIVED(x) // do { if (m_audioDecoderMask != 0) x; } while (0);
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

    m_reader->decodeAsync(std::move(buffer))->then(RunLoop::main(),[protectedThis = Ref { *this }, this](MediaSourceChunkReader::DecodeResult result) {
        DAPPEND(dprintf("[MS][%c]appendInternal result %d\n", m_audioDecoderMask == 0 ?'V':'A', int(result)));
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
        m_appendPromise->reject(PlatformMediaError::AppendError);
    });

    return *m_appendPromise;

#if 0
	if (m_initializationBuffer.size() == 0 || isInitializationSegment(buffer))
	{
        m_didReceiveFirstInitializationBuffer = true;
        m_initializationBuffer.resize(buffer->size());
        if (m_initializationBuffer.size() == buffer->size())
        {
            buffer->copyTo(m_initializationBuffer.mutableSpan());
        }
	}
	else
	{
		Vector<unsigned char> merged;
		merged.reserveCapacity(m_initializationBuffer.size() + buffer->size());
		merged.append(m_initializationBuffer.span());
        merged.resize(m_initializationBuffer.size() + buffer->size());
        unsigned char *dest = merged.mutableSpan().data();
        dest += m_initializationBuffer.size();
        buffer->copyTo(std::span(dest, buffer->size()));

        m_reader = MediaSourceChunkReader::create(this,
            [this](bool success, WebCore::SourceBufferPrivateClient::InitializationSegment& segment, MediaPlayerMorphOSInfo& info){
                initialize(success, segment, info);
            },
            [this](bool success){
                WTF::callOnMainThread([success, this, protect = Ref{*this}]() {
                    appendComplete(success);
                });
            }
        );

		m_reader->decode(WTFMove(merged));
		m_appendCount ++;
        return *m_appendPromise;
	}

    RefPtr mediaSource = m_mediaSource.get();
    if (!mediaSource)
        return *m_appendPromise;

    // youtube is a mess: we have to try and guess whether the stream is a live and only do ac_is_initialization_segment checks
    // if it is not... and we only need those because youtube will randomly feed us data from different streams in order to
    // change media quality (instead of doing this correctly...)
    if (m_appendCount > 2 && !m_durationAtAppend.isValid())
        m_durationAtAppend = mediaSource->duration();

    if (m_appendCount > 3 && !m_isLive && mediaSource)
    {
        unsigned char tmp[1024];
        buffer->copyTo(tmp, std::min(size_t(1024), buffer->size()));
        int is_initialization = ac_is_initialization_segment(tmp, std::min(size_t(1024), buffer->size()), nullptr, 1);
        DAPPEND(dprintf("[MS][%c]%s: %p appended chunk is initialization segment score %d; duration %f oldduration %f\n", m_audioDecoderMask == 0 ?'V':'A', __func__, this, is_initialization, mediaSource->duration().toFloat(), m_durationAtAppend.toFloat()));

        if (mediaSource->duration() > m_durationAtAppend)
        {
            DAPPEND(dprintf("[MS][%c]%s: %p determined this is a live!\n", m_audioDecoderMask == 0 ?'V':'A', __func__, this));
            m_isLive = true;
            is_initialization = 0;
        }

        if (is_initialization == 1)
        {
            m_appendCompleteCount = 0;
            m_appendCount = 0;
            m_mustAppendInitializationSegment = false;

            m_reader->signalEOF();
            m_reader->terminate();
            m_reader = MediaSourceChunkReader::create(this,
                [this](bool success, WebCore::SourceBufferPrivateClient::InitializationSegment& segment, MediaPlayerMorphOSInfo& info) {
                    reinitialize(success, segment, info);
                },
                [this](bool success) {
                    appendComplete(success);
                }
            );
            
            m_initializationBuffer.resize(buffer->size());
            if (m_initializationBuffer.size() == buffer->size())
            {
                buffer->copyTo(m_initializationBuffer.mutableSpan());
            }
        }
    }

    Vector<unsigned char> vector;
    vector.resize(buffer->size());
    if (vector.size() == buffer->size())
    {
        buffer->copyTo(vector.mutableSpan());
        m_reader->decode(WTFMove(vector));
    }
	m_appendCount ++;

    return *m_appendPromise;
#endif
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

void MediaSourceBufferPrivateMorphOS::signalEOF()
{
// ?
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

	terminate();
	m_mediaSource = nullptr;

    if (m_appendPromise)
    {
        if (isMainThread())
        {
            m_appendPromise->reject(PlatformMediaError::BufferRemoved);
            m_appendPromise.reset();
        }
        else
        {
            WTF::callOnMainThread([this, protect = Ref{*this}]() {
                if (m_appendPromise) {
                    m_appendPromise->reject(PlatformMediaError::BufferRemoved);
                    m_appendPromise.reset();
                }
            });
        }
    }
}

void MediaSourceBufferPrivateMorphOS::abort()
{
	DI(dprintf("[MS]%s %p\n", __func__, this));

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
		RefPtr<Acinerella::AcinerellaPackage> package = Acinerella::AcinerellaPackage::create(m_reader->acinerella(), ac_flush_packet());
		m_muxer->push(package, trackID);
	}
}

void MediaSourceBufferPrivateMorphOS::becomeReadyForMoreSamples(int index)
{
	DRMS(dprintf("[MS]%s: %d apc %d starved %d seeking %d (%d)\n", __func__, index, m_appendCompleteDelayed, m_decodersStarved[index], m_seeking, isSeeking()));
	if (m_appendCompleteDelayed)
	{
		DRMS(dprintf("[MS]%s: issuing appendComplete...\n", __func__));
		m_appendCompleteDelayed = false;
  #if 0
		WTF::callOnMainThread([this, protect = Ref{*this}]() {
			if (m_mediaSource && !m_terminating)
				appendCompleted(true);
		});
#endif
	}

#if 0
	if (!m_decodersStarved[index] && !m_seeking)
	{
		m_decodersStarved[index] = true;

		if (!!m_decoders[index])
		{
			WTF::callOnMainThread([this, protect = Ref{*this}, isVideo = m_decoders[index]->isVideo(), index]() {

				DRMS(dprintf("[MS:%c]becomeReadyForMoreSamples %d\n", isVideo?'V':'A', index));

				m_requestedMoreFrames = true;
				m_readyForMoreSamples = true;
				provideMediaData(index);

			});
		}
	}
#endif
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
			m_decodersStarved[i] = false;
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

bool MediaSourceBufferPrivateMorphOS::isReadyForMoreSamples(TrackID)
{
// 	D(dprintf("[MS]%s %d\n", __func__, m_readyForMoreSamples));
	return m_readyForMoreSamples;
}

void MediaSourceBufferPrivateMorphOS::setActive(bool isActive)
{
	D(dprintf("[MS]%s\n", __func__));
    RefPtr mediaSource = m_mediaSource.get();
    if (mediaSource)
    {
		RefPtr<MediaSourceBufferPrivateMorphOS> me = Ref{*this};
        mediaSource->onSourceBufferDidChangeActiveState(me, isActive);
	}
}

void MediaSourceBufferPrivateMorphOS::notifyClientWhenReadyForMoreSamples(TrackID)
{
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

    if (InitializeMode::First == mode)
    {
        didReceiveInitializationSegment(m_reader->getInitializationSegment());
		RefPtr<MediaSourceBufferPrivateMorphOS> me = Ref{*this};
        mediaSource->onSourceBufferInitialized(me);
    }
    else
    {
    
    }

    return true;
}

bool MediaSourceBufferPrivateMorphOS::createDecoders()
{
    if (m_muxer)
        return true;

    double duration = 0.0;
    uint32_t decoderIndexMask = 0;

    m_muxer = Acinerella::AcinerellaMuxedBuffer::create();
    DI(dprintf("[MS]%s: muxer created. decoders %d\n", __func__, m_reader->numDecoders()));

    m_audioDecoderMask = 0;
    m_numDecoders = m_reader->numDecoders();

    RefPtr<Acinerella::AcinerellaPointer> acinerella = m_reader->acinerella();

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
            m_maxBuffer[i] = m_muxer->maxBufferSizeForMediaSourceDecoder(i);
        }

        m_muxer->setSinkFunction([this, protectedThis = Ref{*this}](int decoderIndex, int , uint32_t bytesInBuffer) {
            if (bytesInBuffer < m_maxBuffer[decoderIndex] / 2)
                becomeReadyForMoreSamples(decoderIndex);
            return false; // avoid blocking the pipeline!
        });

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
        if (mediaSource)
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
