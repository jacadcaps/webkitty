#pragma once

#include "config.h"

#if ENABLE(VIDEO)

#include <wtf/MainThread.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/text/WTFString.h>
#include <wtf/Function.h>
#include <wtf/ThreadSafeRefCounted.h>
#include <wtf/threads/BinarySemaphore.h>
#include <WebCore/SharedBuffer.h>
#include "acinerella.h"
#include "AcinerellaPointer.h"
#include <queue>

namespace WebCore {
namespace Acinerella {

class AcinerellaMuxedBuffer;
class AcinerellaDecoder;

class AcinerellaPackage
{
public:
	explicit AcinerellaPackage(RefPtr<AcinerellaPointer>&pointer, ac_package *package) : m_acinerella(pointer), m_package(package) { }
	explicit AcinerellaPackage() : m_package(nullptr) { }
	explicit AcinerellaPackage(const AcinerellaPackage &) = delete;
	explicit AcinerellaPackage(AcinerellaPackage &) = delete;
	explicit AcinerellaPackage(AcinerellaPackage && otter) : m_acinerella(otter.m_acinerella), m_package(otter.m_package) { otter.m_acinerella = nullptr; otter.m_package = nullptr; }
	~AcinerellaPackage() { if (m_package) ac_free_package(m_package); }

	AcinerellaPackage& operator=(AcinerellaPackage&& otter) {
		std::swap(otter.m_acinerella, m_acinerella);
		std::swap(otter.m_package, m_package);
		return *this;
	}
	AcinerellaPackage& operator=(AcinerellaPackage const & otter) = delete;

	ac_package *package() { return m_package; }
	int index() const { return m_package ? m_package->stream_index : -1; }
	bool isFlushPackage() { return m_package == ac_flush_packet(); }
	explicit operator bool() const { return nullptr != m_package; }

	RefPtr<AcinerellaPointer>& acinerella() { return m_acinerella; }

protected:
	RefPtr<AcinerellaPointer> m_acinerella;
	ac_package               *m_package;
};

class AcinerellaMuxedBuffer : public ThreadSafeRefCounted<AcinerellaMuxedBuffer>
{
public:
	AcinerellaMuxedBuffer() = default;
	~AcinerellaMuxedBuffer() = default;

	static RefPtr<AcinerellaMuxedBuffer> create() {
		return WTF::adoptRef(*new AcinerellaMuxedBuffer());
	}

	static constexpr int maxDecoders = 32;
	static constexpr int queueReadAheadSize = 128;

	void setSinkFunction(Function<void()>&& sinkFunction);
	void setDecoderMask(uint32_t mask);

	// To be called on Acinerella thread
	void push(AcinerellaPackage&&package);
	void flush();
	void terminate();

	// This is meant to be called from the decoder threads. Will block until a valid package can be returned
	// Returns false on error or EOS
	bool nextPackage(AcinerellaDecoder &decoder, AcinerellaPackage &outPackage);

protected:
	typedef std::queue<AcinerellaPackage> AcinerellaPackageQueue;

	inline bool isDecoderValid(int index) {
		return 0 != (m_decoderMask & (1UL << index));
	}

	inline void forValidDecoders(Function<void(AcinerellaPackageQueue& queue, BinarySemaphore& event)> &&function) {
		uint32_t mask = m_decoderMask;
		for (int i = 0; mask && (i < maxDecoders); i++) {
			if (isDecoderValid(i)) {
				function(m_packages[i], m_events[i]);
				mask &= ~(1L << i);
			}
		}
	}

	inline void whileValidDecoders(Function<bool(AcinerellaPackageQueue& queue, BinarySemaphore& event)> &&function) {
		uint32_t mask = m_decoderMask;
		for (int i = 0; mask && (i < maxDecoders); i++) {
			if (isDecoderValid(i)) {
				if (!function(m_packages[i], m_events[i]))
					return;
				mask &= ~(1L << i);
			}
		}
	}

	inline void forInvalidDecoders(Function<void(AcinerellaPackageQueue& queue, BinarySemaphore& event)> &&function) {
		for (int i = 0; i < maxDecoders; i++) {
			if (!isDecoderValid(i)) {
				function(m_packages[i], m_events[i]);
			}
		}
	}

	bool needsToCallSinkFunction() {
		bool hasNonFullQueues = false;
		whileValidDecoders([&](AcinerellaPackageQueue& queue, BinarySemaphore&) -> bool {
			if (queue.size() < queueReadAheadSize) {
				hasNonFullQueues = true;
				return false;
			}
			return true; // continue scanning
		});
		return hasNonFullQueues;
	}

protected:
	Function<void()>        m_sinkFunction;
	AcinerellaPackageQueue  m_packages[maxDecoders];
	BinarySemaphore         m_events[maxDecoders];
	Lock                    m_lock;

	uint32_t                m_decoderMask = 0;
	bool                    m_queueCompleteOrError = false;
};

}
}

#endif
