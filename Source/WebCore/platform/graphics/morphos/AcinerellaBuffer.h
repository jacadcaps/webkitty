#pragma once

#include "config.h"

#if ENABLE(VIDEO)

#include <wtf/MainThread.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/text/WTFString.h>
#include <wtf/Function.h>
#include <wtf/ThreadSafeRefCounted.h>
#include <wtf/threads/BinarySemaphore.h>
#include "acinerella.h"
#include <queue>

namespace WebCore {
namespace Acinerella {

class AcinerellaMuxedBuffer;
class AcinerellaDecoder;

class AcinerellaNetworkBuffer : public ThreadSafeRefCounted<AcinerellaNetworkBuffer>
{
protected:
	AcinerellaNetworkBuffer(const String &url, size_t readAhead);
public:
	virtual ~AcinerellaNetworkBuffer() = default;

	static RefPtr<AcinerellaNetworkBuffer> create(const String &url, size_t readAhead = 1 * 1024 * 1024);
	
	const String &url() const { return m_url; }

	// Main Thread Methods
	virtual void start(uint64_t from = 0) = 0;
	virtual void stop() = 0;
	
	// Acinerella Thread Methods
	virtual int read(uint8_t *outBuffer, int size) = 0;

protected:
	String                           m_url;
    int                              m_readAhead;
};

class AcinerellaPackage
{
public:
	explicit AcinerellaPackage(ac_package *package) : m_package(package) { }
	explicit AcinerellaPackage() : m_package(nullptr) { }
	explicit AcinerellaPackage(const AcinerellaPackage &) = delete;
	explicit AcinerellaPackage(AcinerellaPackage &) = delete;
	explicit AcinerellaPackage(AcinerellaPackage && otter) : m_package(otter.m_package) { otter.m_package = nullptr; }
	~AcinerellaPackage() { if (m_package) ac_free_package(m_package); }

	AcinerellaPackage& operator=(AcinerellaPackage&& otter) {
		std::swap(otter.m_package, m_package);
		return *this;
	}
	AcinerellaPackage& operator=(AcinerellaPackage const & otter) = delete;

	ac_package *package() { return m_package; }
	int index() const { return m_package ? m_package->stream_index : -1; }
	explicit operator bool() const { return nullptr != m_package; }
	
protected:
	ac_package *m_package;
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
};

}
}

#endif
