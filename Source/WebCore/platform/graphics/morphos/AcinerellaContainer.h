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
#include "AcinerellaDecoder.h"
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

	void play();
	void pause();

protected:
	ac_instance *ac() { return m_ac.get(); }

	void threadEntryPoint();
	void dispatch(Function<void ()>&& function);
	void performTerminate();

	bool initialize();

	int open();
	int close();
	int read(uint8_t *buf, int size);
	int64_t seek(int64_t pos, int whence);

	void demuxNextPackage();

protected:
	static int acOpenCallback(void *me);
	static int acCloseCallback(void *me);
	static int acReadCallback(void *me, uint8_t *buf, int size);
	static int64_t acSeekCallback(void *me, int64_t pos, int whence);

protected:
	AcinerellaClient                *m_client;
	String                           m_url;
    deleted_unique_ptr<ac_instance>  m_ac;
	RefPtr<AcinerellaNetworkBuffer>  m_networkBuffer;

	RefPtr<AcinerellaMuxedBuffer>    m_muxer;
	RefPtr<AcinerellaDecoder>        m_audioDecoder;
	RefPtr<AcinerellaDecoder>        m_videoDecoder;

    RefPtr<Thread>                   m_thread;
    MessageQueue<Function<void ()>>  m_queue;
    bool                             m_terminating = false;
    bool                             m_enableAudio = true;
    bool                             m_enableVideo = false;
};

}
}

#endif
