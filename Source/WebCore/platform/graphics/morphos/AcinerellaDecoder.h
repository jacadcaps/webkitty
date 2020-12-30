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
#include "AcinerellaMuxer.h"
#include "AcinerellaPointer.h"

template<typename T>
using deleted_unique_ptr = std::unique_ptr<T,std::function<void(T*)>>;

namespace WebCore {
namespace Acinerella {

class Acinerella;

class AcinerellaDecodedFrame
{
public:
	explicit AcinerellaDecodedFrame(RefPtr<AcinerellaPointer> pointer, ac_decoder *decoder) : m_pointer(pointer), m_frame(ac_alloc_decoder_frame(decoder)) { }
	explicit AcinerellaDecodedFrame() : m_frame(nullptr) { }
	explicit AcinerellaDecodedFrame(const AcinerellaDecodedFrame &) = delete;
	explicit AcinerellaDecodedFrame(AcinerellaDecodedFrame &) = delete;
	explicit AcinerellaDecodedFrame(AcinerellaDecodedFrame && otter) : m_pointer(otter.m_pointer), m_frame(otter.m_frame) { otter.m_pointer = nullptr; otter.m_frame = nullptr; }
	~AcinerellaDecodedFrame() { ac_free_decoder_frame(m_frame); }
	
	AcinerellaDecodedFrame & operator=(const AcinerellaDecodedFrame &) = delete;
	AcinerellaDecodedFrame & operator=(AcinerellaDecodedFrame &) = delete;
	AcinerellaDecodedFrame & operator=(AcinerellaDecodedFrame && otter) { std::swap(m_pointer, otter.m_pointer); std::swap(m_frame, otter.m_frame); return *this; }

	ac_decoder_frame *frame() { return m_frame; }
	const ac_decoder_frame *frame() const { return m_frame; }
	
	RefPtr<AcinerellaPointer> &pointer() { return m_pointer; }

protected:
	RefPtr<AcinerellaPointer> m_pointer;
	ac_decoder_frame         *m_frame;
};

class AcinerellaDecoder : public ThreadSafeRefCounted<AcinerellaDecoder>
{
public:
	AcinerellaDecoder(Acinerella* parent, RefPtr<AcinerellaMuxedBuffer> buffer, int index, const ac_stream_info &info, bool isLiveStream);
	virtual ~AcinerellaDecoder();

	// call from: Acinerella thread
	void terminate();

	void warmUp();

	void play();
	void pause();

	virtual bool isPlaying() const = 0;
	virtual bool isReadyToPlay() const = 0;

	virtual bool isAudio() const = 0;
	void setVolume(float volume);

	float duration() const { return m_duration; }
	float bitRate() const { return m_bitrate; }
	virtual float position() const = 0;
	virtual float bufferSize() const = 0;
	
	int index() const { return m_index; }

protected:
	// call from: Own thread
	void threadEntryPoint();
	// call from: Any thread
	void dispatch(Function<void ()>&& function);
	// call from: Acinerella thread
	void performTerminate();

	// call from: Own thread
	bool decodeNextFrame();
	void decodeUntilBufferFull();
	void onPositionChanged();
	void onDurationChanged();
	void onEnded();
	virtual void flush();

	// call from: Own thread
	virtual bool onThreadInitialize() { return true; }
	virtual void onThreadShutdown() { }

	// call from: Own thread, under m_lock!
	virtual void onFrameDecoded(const AcinerellaDecodedFrame &) { }

	// call from: Own thread
	virtual void startPlaying() = 0;
	virtual void stopPlaying() = 0;
	virtual void doSetVolume(float) { };

	// call from: Any thread
	virtual float readAheadTime() const = 0;

protected:
	Acinerella                        *m_parent; // valid until terminate is called
    RefPtr<Thread>                     m_thread;
    MessageQueue<Function<void ()>>    m_queue;

	RefPtr<AcinerellaMuxedBuffer>      m_muxer;
	float                              m_duration;
	int                                m_bitrate;
	int                                m_index;
	
	std::queue<AcinerellaDecodedFrame> m_decodedFrames;
	Lock                               m_lock;

	bool                               m_playing = false;
	bool                               m_terminating = false;
	bool                               m_isLive = false;
};

}
}

#endif
