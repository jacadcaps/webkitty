#pragma once

#if ENABLE(VIDEO) && ENABLE(MEDIA_SOURCE)

#include "AcinerellaPointer.h"
#include "AcinerellaBuffer.h"
#include "AcinerellaMuxer.h"
#include "AcinerellaDecoder.h"
#include "MediaPlayerMorphOS.h"

#include "SourceBufferPrivate.h"
#include "SourceBufferPrivateClient.h"
#include "MediaSourceChunkReader.h"
#include "MediaSample.h"

#include <wtf/Function.h>
#include <wtf/MessageQueue.h>
#include <wtf/Threading.h>
#include <wtf/StdList.h>
#include <wtf/text/WTFString.h>
#include <wtf/ThreadSafeRefCounted.h>
#include <wtf/RunLoop.h>

#include <dos/dos.h>

struct Window;

namespace WebCore {

class GraphicsContext;
class FloatRect;
class MediaSourcePrivateMorphOS;
class MediaPlayerPrivateMorphOS;

class MediaSourceBufferPrivateMorphOS final : public SourceBufferPrivate, public Acinerella::AcinerellaDecoderClient, public MediaSourceChunkReaderTrackFactory {
public:
    static Ref<MediaSourceBufferPrivateMorphOS> create(MediaSourcePrivateMorphOS*);
    virtual ~MediaSourceBufferPrivateMorphOS();

    constexpr MediaPlatformType platformType() const { return MediaPlatformType::MorphOS; }

	void play();
	void prePlay();
	void pause();

	void warmUp();
	void coolDown();
    void clearMediaSource();
    void terminate();

	void willSeek(double seekTo);
	void seekToTime(const MediaTime&) override;
    bool isEnded() const { return m_ended; }

    void setVolume(double vol);

	const MediaPlayerMorphOSInfo &info() { return m_info; }
	bool isInitialized() { return m_metaInitDone; }

	void paint(GraphicsContext&, const FloatRect&);
	void setOverlayWindowCoords(struct ::Window *w, int scrollx, int scrolly, int mleft, int mtop, int mright, int mbottom, int width, int height);

	void setAudioPresentationTime(double apts);
    void clearAudioPresentationTime();
	bool areDecodersReadyToPlay();
	bool areDecodersPlaying();
	float decodersBufferedTime();

    void onTrackEnabled(int index, bool enabled);
    void dumpStatus();

	void getFrameCounts(unsigned& decoded, unsigned &dropped) const;
	
	bool didFailDecodingFrames() const { return m_readerFailed; }

private:
	explicit MediaSourceBufferPrivateMorphOS(MediaSourcePrivateMorphOS*);

    Ref<MediaPromise> appendInternal(Ref<SharedBuffer>&&) override;
    void abort() override;
    void resetParserStateInternal() override;
    void removedFromMediaSource() override;

    void flush(TrackID) override;
    void enqueueSample(Ref<MediaSample>&&, TrackID)  override;
    void allSamplesInTrackEnqueued(TrackID)  override;
    bool isReadyForMoreSamples(TrackID)  override;
    void setActive(bool) override;
    void notifyClientWhenReadyForMoreSamples(TrackID)  override;
    bool canSetMinimumUpcomingPresentationTime(TrackID) const override;

	bool isSeeking() const override;
	bool isSeekingInternal() const { return m_seeking; }

	void flush();
	void becomeReadyForMoreSamples(int decoderIndex);

    enum class InitializeMode { First, Next };
    bool initialize(InitializeMode mode);
    bool appendComplete();
    bool createDecoders();

	void threadEntryPoint();
	void dispatch(Function<void ()>&& function);
	void performTerminate();

    RefPtr<VideoTrackPrivateMorphOS> videoTrack(int index) const;
    RefPtr<AudioTrackPrivateMorphOS> audioTrack(int index) const;

	// AcinerellaDecoderClient
	const WebCore::MediaPlayerMorphOSStreamSettings& streamSettings() override;
	void onDecoderWarmedUp(RefPtr<Acinerella::AcinerellaDecoder> decoder) override;
	void onDecoderReadyToPlay(RefPtr<Acinerella::AcinerellaDecoder> decoder) override;
	void onDecoderPlaying(RefPtr<Acinerella::AcinerellaDecoder> decoder, bool playing) override;
	void onDecoderUpdatedBufferLength(RefPtr<Acinerella::AcinerellaDecoder> decoder, double buffer) override;
	void onDecoderUpdatedPosition(RefPtr<Acinerella::AcinerellaDecoder> decoder, double position) override;
	void onDecoderUpdatedDuration(RefPtr<Acinerella::AcinerellaDecoder> decoder, double duration) override;
	void onDecoderEnded(RefPtr<Acinerella::AcinerellaDecoder> decoder)  override;
	void onDecoderWantsToRender(RefPtr<Acinerella::AcinerellaDecoder> decoder) override;
	void onDecoderNotReadyToRender(RefPtr<Acinerella::AcinerellaDecoder> decoder) override;
	void onDecoderRenderUpdate(RefPtr<Acinerella::AcinerellaDecoder> decoder) override;

	void seekTimerFired();

private:
	ThreadSafeWeakPtr<MediaSourcePrivateMorphOS>  m_mediaSource;
	RefPtr<MediaSourceChunkReader>                m_reader;
	RefPtr<Acinerella::AcinerellaMuxedBuffer>     m_muxer;
	RefPtr<Acinerella::AcinerellaDecoder>         m_decoders[Acinerella::AcinerellaMuxedBuffer::maxDecoders];
	RefPtr<Acinerella::AcinerellaDecoder>         m_paintingDecoder;
	bool                                          m_decodersStarved[Acinerella::AcinerellaMuxedBuffer::maxDecoders];
    bool                                          m_enabled[Acinerella::AcinerellaMuxedBuffer::maxDecoders];
	uint32_t                                      m_maxBuffer[Acinerella::AcinerellaMuxedBuffer::maxDecoders];
	int                                           m_numDecoders = 0;
    std::optional<MediaPromise::Producer>         m_appendPromise;

    RefPtr<Thread>                                m_thread;
    MessageQueue<Function<void ()>>               m_queue;
	BinarySemaphore                               m_event;
	Lock                                          m_lock;

    Vector<unsigned char>                         m_initializationBuffer;
    bool                                          m_didReceiveFirstInitializationBuffer = false;

	uint32_t                                      m_audioDecoderMask = 0;
	bool                                          m_enableVideo = false;
	bool                                          m_enableAudio = true;
	bool                                          m_terminating = false;
	bool                                          m_eos = false;
    bool                                          m_isLive = false;
    bool                                          m_ended = false;
	std::atomic<bool>                             m_appendCompletePending = false;
	bool                                          m_appendCompleteDelayed = false;
    MediaTime                                     m_durationAtAppend = MediaTime::invalidTime();

	MediaPlayerMorphOSInfo                        m_info;
	bool                                          m_metaInitDone = false;

	bool                                          m_readyForMoreSamples = true;
	bool                                          m_requestedMoreFrames = false;
    bool                                          m_enqueuedSamples = false;
    bool                                          m_isActive = false;
	
	bool                                          m_seeking = false;
	bool                                          m_seekingWaitsForFrames = false;
	bool                                          m_postSeekingAppendDone = false;
	double                                        m_seekTime;
	
	int                                           m_enqueueCount = 0;
	int                                           m_appendCount = 0;
	int                                           m_appendCompleteCount = 0;
	bool                                          m_readerFailed = false;
	bool                                          m_mustAppendInitializationSegment = false;
    bool                                          m_mustReinitializeDecoders = false;
};

}

SPECIALIZE_TYPE_TRAITS_BEGIN(WebCore::MediaSourceBufferPrivateMorphOS)
static bool isType(const WebCore::SourceBufferPrivate& sourceBuffer) { return sourceBuffer.platformType() == WebCore::MediaPlatformType::MorphOS; }
SPECIALIZE_TYPE_TRAITS_END()

#endif
