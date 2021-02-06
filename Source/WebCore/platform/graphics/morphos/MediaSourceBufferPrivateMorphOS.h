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

namespace WebCore {

class MediaSourcePrivateMorphOS;

class MediaSourceBufferPrivateMorphOS final : public SourceBufferPrivate, public Acinerella::AcinerellaDecoderClient {
public:
    static Ref<MediaSourceBufferPrivateMorphOS> create(MediaSourcePrivateMorphOS*);
    virtual ~MediaSourceBufferPrivateMorphOS();

	void warmUp();
    void clearMediaSource();

	const MediaPlayerMorphOSInfo &info() { return m_info; }

private:
	explicit MediaSourceBufferPrivateMorphOS(MediaSourcePrivateMorphOS*);

    void setClient(SourceBufferPrivateClient*) override;

    void append(Vector<unsigned char>&&) override;
    void abort() override;
    void resetParserState() override;
    void removedFromMediaSource() override;

	void demuxNextPackage();

    MediaPlayer::ReadyState readyState() const override;
    void setReadyState(MediaPlayer::ReadyState) override;

	bool initialize();
	void threadEntryPoint();
	void dispatch(Function<void ()>&& function);
	void performTerminate();

	int read(uint8_t *buf, int size);
	int64_t seek(int64_t pos, int whence);

	static int acReadCallback(void *me, uint8_t *buf, int size);
	static int64_t acSeekCallback(void *me, int64_t pos, int whence);

	// AcinerellaDecoderClient
	RefPtr<Acinerella::AcinerellaPointer> &acinerellaPointer() override { return m_acinerella; }

	void onDecoderReadyToPlay(Acinerella::AcinerellaDecoder& decoder) override;
	void onDecoderPlaying(Acinerella::AcinerellaDecoder& decoder, bool playing) override;
	void onDecoderUpdatedBufferLength(Acinerella::AcinerellaDecoder& decoder, float buffer) override;
	void onDecoderUpdatedPosition(Acinerella::AcinerellaDecoder& decoder, float position) override;
	void onDecoderUpdatedDuration(Acinerella::AcinerellaDecoder& decoder, float duration) override;
	void onDecoderEnded(Acinerella::AcinerellaDecoder& decoder)  override;

private:
	MediaSourcePrivateMorphOS                    *m_mediaSource;
	SourceBufferPrivateClient                    *m_client;
	RefPtr<Acinerella::AcinerellaPointer>         m_acinerella;
	RefPtr<Acinerella::AcinerellaMuxedBuffer>     m_muxer;
	RefPtr<Acinerella::AcinerellaDecoder>         m_decoders[Acinerella::AcinerellaMuxedBuffer::maxDecoders];
	Vector<unsigned char>                         m_buffer;
	int                                           m_bufferPosition = 0;

    RefPtr<Thread>                                m_thread;
    MessageQueue<Function<void ()>>               m_queue;
	BinarySemaphore                               m_event;
	Lock                                          m_lock;
	
	bool                                          m_enableVideo = false;
	bool                                          m_enableAudio = true;
	bool                                          m_terminating = false;
	
	MediaPlayerMorphOSInfo                        m_info;
};

}

#endif
