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
	AcinerellaMuxedBuffer(Function<void()>&& sinkFunction, int audioIndex = -1, int videoIndex = -1, unsigned int audioQueueSize = 128, unsigned int videoQueueSize = 128);
	~AcinerellaMuxedBuffer() { };

	static RefPtr<AcinerellaMuxedBuffer> create(Function<void()>&& sinkFunction, int audioIndex = -1, int videoIndex = -1, unsigned int audioQueueSize = 128, unsigned int videoQueueSize = 128) {
		return WTF::adoptRef(*new AcinerellaMuxedBuffer(WTFMove(sinkFunction), audioIndex, videoIndex, audioQueueSize, videoQueueSize));
	}

	// To be called on Acinerella thread
	void push(AcinerellaPackage&&package);
	void flush();
	void terminate();
	void setDropVideoPackages(bool drop) { m_dropVideoPackages = drop; }

	// This is meant to be called from the decoder threads. Will block until a valid package can be returned
	// Returns false on error or EOS
	bool nextPackage(AcinerellaDecoder &decoder, AcinerellaPackage &outPackage);
	
protected:
	Function<void()>                 m_sinkFunction;
	std::queue<AcinerellaPackage>    m_audioPackages;
	std::queue<AcinerellaPackage>    m_videoPackages;
	BinarySemaphore                  m_audioEvent;
	BinarySemaphore                  m_videoEvent;
	Lock                             m_lock;

	int m_audioPackageIndex;
	unsigned int m_audioQueueAheadSize;
	int m_videoPackageIndex;
	unsigned int m_videoQueueAheadSize;

	bool m_queueCompleteOrError = false;
	bool m_dropVideoPackages = false;
};

}
}

#endif
