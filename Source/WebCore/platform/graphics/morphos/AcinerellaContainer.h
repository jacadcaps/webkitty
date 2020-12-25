#pragma once

#include "config.h"

#if ENABLE(VIDEO)

#include <wtf/Function.h>
#include <wtf/MessageQueue.h>
#include <wtf/Threading.h>
#include <wtf/text/WTFString.h>
#include <wtf/ThreadSafeRefCounted.h>
#include "AcinerellaClient.h"
#include "AcinerellaBuffer.h"
#include "AcinerellaMuxer.h"
#include "AcinerellaDecoder.h"
#include "AcinerellaPointer.h"
#include "acinerella.h"

template<typename T>
using deleted_unique_ptr = std::unique_ptr<T,std::function<void(T*)>>;

namespace WebCore {
namespace Acinerella {

class Acinerella : public ThreadSafeRefCounted<Acinerella>
{
friend class AcinerellaDecoder;
friend class AcinerellaMuxedBuffer;
public:
	Acinerella(AcinerellaClient *client, const String &url);
	~Acinerella() = default;

	static RefPtr<Acinerella> create(AcinerellaClient *client, const String &url) {
		return WTF::adoptRef(*new Acinerella(client, url));
	}

	void terminate();
	void warmUp();

	void play();

	void pause();
	bool paused();

	float duration();
	
	bool hasAudio() { return m_audioDecoder.get(); }
	bool hasVideo() { return m_videoDecoder.get(); }
	
	void setVolume(float volume);
	void setMuted(bool muted);
	float volume() const { return m_volume; }

	bool canSeek();
	bool isSeeking();
	void seek(float time);
	
	bool isLive();

	RefPtr<AcinerellaPointer> &acinerellaPointer() { return m_acinerella; }

protected:
	ac_instance *ac() { return m_acinerella ? m_acinerella->instance() : nullptr; }

	void threadEntryPoint();
	void dispatch(Function<void ()>&& function);
	void performTerminate();

	bool initialize();
	void initializeAfterDiscontinuity();
	
	void startSeeking(float pos);

	int open();
	int close();
	int read(uint8_t *buf, int size);
	int64_t seek(int64_t pos, int whence);
	
	void demuxNextPackage();
	
	void onDecoderReadyToPlay(AcinerellaDecoder& decoder);
	void onDecoderPlaying(AcinerellaDecoder& decoder, bool playing);
	void onDecoderUpdatedBufferLength(AcinerellaDecoder& decoder, float buffer);
	void onDecoderUpdatedPosition(AcinerellaDecoder& decoder, float buffer);

protected:
	static int acOpenCallback(void *me);
	static int acCloseCallback(void *me);
	static int acReadCallback(void *me, uint8_t *buf, int size);
	static int64_t acSeekCallback(void *me, int64_t pos, int whence);

protected:
	AcinerellaClient                *m_client;
	String                           m_url;
	RefPtr<AcinerellaPointer>        m_acinerella;
	Lock                             m_acinerellaLock;
	RefPtr<AcinerellaNetworkBuffer>  m_networkBuffer;

	RefPtr<AcinerellaMuxedBuffer>    m_muxer;
	RefPtr<AcinerellaDecoder>        m_audioDecoder;
	RefPtr<AcinerellaDecoder>        m_videoDecoder;

	float                            m_duration;
	float                            m_volume = 1.f;
	bool                             m_muted = false;
	bool                             m_canSeek = true;
	bool                             m_isSeeking = false;
	bool                             m_isLive = false;
	
	int64_t                          m_readPosition = -1;

    RefPtr<Thread>                   m_thread;
    MessageQueue<Function<void ()>>  m_queue;
    bool                             m_terminating = false;
    bool                             m_enableAudio = true;
    bool                             m_enableVideo = false;
    bool                             m_paused = false;
};

}
}

#endif
