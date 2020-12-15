#pragma once

#include "config.h"

#if ENABLE(VIDEO)

#include "acinerella.h"
#include <wtf/Seconds.h>
#include <wtf/Function.h>
#include <wtf/MessageQueue.h>
#include <wtf/Threading.h>
#include <wtf/text/WTFString.h>
#include <wtf/ThreadSafeRefCounted.h>
#include <memory>
#include "AcinerellaBuffer.h"

template<typename T>
using deleted_unique_ptr = std::unique_ptr<T,std::function<void(T*)>>;

namespace WebCore {
namespace Acinerella {

class Acinerella;

class AcinerellaDecoder : public ThreadSafeRefCounted<AcinerellaDecoder>
{
public:
	AcinerellaDecoder(Acinerella &parent, RefPtr<AcinerellaMuxedBuffer> buffer, int index, const ac_stream_info &info);
	virtual ~AcinerellaDecoder() { }

	void terminate();
	bool isValid() const { return !!m_decoder; }

	void warmUp();
	void play();
	void pause();

	virtual bool isPlaying() const = 0;
	virtual bool isAudio() const = 0;

	Seconds const &duration() const { return m_duration; }
	virtual Seconds position() const = 0;

protected:
	void threadEntryPoint();
	void dispatch(Function<void ()>&& function);
	void performTerminate();
	virtual void startPlaying() = 0;
	virtual void stopPlaying() = 0;
	virtual int warmUpQueueSize() const = 0;

protected:
    RefPtr<Thread>                   m_thread;
    MessageQueue<Function<void ()>>  m_queue;

	RefPtr<AcinerellaMuxedBuffer>    m_muxer;
    deleted_unique_ptr<ac_decoder>   m_decoder;
	Seconds                          m_duration;
	
	bool                             m_terminating = false;
};

}
}

#endif
