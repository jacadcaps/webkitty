#pragma once

#include "config.h"

#if ENABLE(VIDEO) && ENABLE(MEDIA_SOURCE)

#include "SourceBufferPrivate.h"
#include "AcinerellaPointer.h"
#include "AcinerellaBuffer.h"
#include "AcinerellaMuxer.h"
#include "AcinerellaDecoder.h"
#include "MediaPlayerMorphOS.h"

#include <wtf/Function.h>
#include <wtf/MessageQueue.h>
#include <wtf/Threading.h>
#include <wtf/text/WTFString.h>
#include <wtf/ThreadSafeRefCounted.h>

struct Window;

namespace WebCore {

class GraphicsContext;
class FloatRect;
class MediaSourcePrivateMorphOS;

class MediaSourceBufferPrivateMorphOS final : public SourceBufferPrivate, public Acinerella::AcinerellaDecoderClient {
public:
    static Ref<MediaSourceBufferPrivateMorphOS> create(MediaSourcePrivateMorphOS*);
    virtual ~MediaSourceBufferPrivateMorphOS();

	void play();
	void pause();

	void warmUp();
    void clearMediaSource();

	void seekToTime(float time);

	const MediaPlayerMorphOSInfo &info() { return m_info; }

	void paint(GraphicsContext&, const FloatRect&);
	void setOverlayWindowCoords(struct ::Window *w, int scrollx, int scrolly, int mleft, int mtop, int mright, int mbottom);

	void setAudioPresentationTime(double apts);

private:
	explicit MediaSourceBufferPrivateMorphOS(MediaSourcePrivateMorphOS*);

    void setClient(SourceBufferPrivateClient*) override;

    void append(Vector<unsigned char>&&) override;
    void abort() override;
    void resetParserState() override;
    void removedFromMediaSource() override;

    void flush(const AtomString&) override;
    void enqueueSample(Ref<MediaSample>&&, const AtomString&)  override;
    void allSamplesInTrackEnqueued(const AtomString&)  override;
    bool isReadyForMoreSamples(const AtomString&)  override;
    void setActive(bool) override;
    void notifyClientWhenReadyForMoreSamples(const AtomString&)  override;

	void flush();

	bool demuxNext();

    MediaPlayer::ReadyState readyState() const override;
    void setReadyState(MediaPlayer::ReadyState) override;

	bool initialize();
	bool initializeMetaData();
	void reinitialize();
	void threadEntryPoint();
	void dispatch(Function<void ()>&& function);
	void performTerminate();

	void pumpThread();

	int read(uint8_t *buf, int size);
	int64_t seek(int64_t pos, int whence);

	static int acReadCallback(void *me, uint8_t *buf, int size);
	static int64_t acSeekCallback(void *me, int64_t pos, int whence);

	// AcinerellaDecoderClient
	RefPtr<Acinerella::AcinerellaPointer> &acinerellaPointer() override { return m_acinerella; }

	void onDecoderReadyToPlay(RefPtr<Acinerella::AcinerellaDecoder> decoder) override;
	void onDecoderPlaying(RefPtr<Acinerella::AcinerellaDecoder> decoder, bool playing) override;
	void onDecoderUpdatedBufferLength(RefPtr<Acinerella::AcinerellaDecoder> decoder, double buffer) override;
	void onDecoderUpdatedPosition(RefPtr<Acinerella::AcinerellaDecoder> decoder, double position) override;
	void onDecoderUpdatedDuration(RefPtr<Acinerella::AcinerellaDecoder> decoder, double duration) override;
	void onDecoderEnded(RefPtr<Acinerella::AcinerellaDecoder> decoder)  override;
	void onDecoderReadyToPaint(RefPtr<Acinerella::AcinerellaDecoder> decoder) override;
	void onDecoderNotReadyToPaint(RefPtr<Acinerella::AcinerellaDecoder> decoder) override;

private:
	MediaSourcePrivateMorphOS                    *m_mediaSource;
	SourceBufferPrivateClient                    *m_client;
	RefPtr<Acinerella::AcinerellaPointer>         m_acinerella;
	RefPtr<Acinerella::AcinerellaMuxedBuffer>     m_muxer;
	RefPtr<Acinerella::AcinerellaDecoder>         m_decoders[Acinerella::AcinerellaMuxedBuffer::maxDecoders];
	RefPtr<Acinerella::AcinerellaDecoder>         m_paintingDecoder;
	Vector<unsigned char>                         m_buffer;
	Vector<unsigned char>                         m_initializationBuffer;
	int                                           m_bufferPosition = 0;
	int                                           m_initializationBufferPosition = 0;
	bool                                          m_bufferEOF = false;

    RefPtr<Thread>                                m_thread;
    MessageQueue<Function<void ()>>               m_queue;
	BinarySemaphore                               m_event;
	Lock                                          m_lock;
	
	uint32_t                                      m_audioDecoderMask = 0;
	bool                                          m_enableVideo = false;
	bool                                          m_enableAudio = true;
	bool                                          m_terminating = false;

    RefPtr<Thread>                                m_pumpThread;
	BinarySemaphore                               m_pumpEvent;
	bool                                          m_doPump = false;

	MediaPlayerMorphOSInfo                        m_info;
	bool                                          m_metaInitDone = false;
};

}

#endif
