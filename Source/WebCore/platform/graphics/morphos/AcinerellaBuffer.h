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

class AcinerellaNetworkBuffer : public ThreadSafeRefCounted<AcinerellaNetworkBuffer>
{
protected:
	AcinerellaNetworkBuffer(const String &url, size_t readAhead);
public:
	virtual ~AcinerellaNetworkBuffer() = default;

	static RefPtr<AcinerellaNetworkBuffer> create(const String &url, size_t readAhead = 1 * 1024 * 1024);
	static RefPtr<AcinerellaNetworkBuffer> createDisregardingFileType(const String &url, size_t readAhead = 1 * 1024 * 1024);

	const String &url() const { return m_url; }

	// Main Thread Methods
	virtual void start(uint64_t from = 0) = 0;
	virtual void stop() = 0;
	virtual bool canSeek() { return true; }
	void die() { m_dead = true; stop(); }

	// Acinerella Thread Methods
	static const int eRead_Discontinuity = -2;
	static const int eRead_Error = -1;
	static const int eRead_EOF = 0;
	
	virtual int read(uint8_t *outBuffer, int size, int64_t position = -1) = 0;

	virtual int64_t length() { return m_length; }
	virtual int64_t position() { return 0; }

protected:
	String     m_url;
    int64_t    m_length;
    int        m_readAhead;
    bool       m_dead = false;
};

class AcinerellaNetworkFileRequest : public ThreadSafeRefCounted<AcinerellaNetworkFileRequest>
{
protected:
	AcinerellaNetworkFileRequest(const String &url, Function<void(bool)>&& onFinished)
		: m_url(url)
		, m_onFinished(WTFMove(onFinished))
	{
	}
public:
	static RefPtr<AcinerellaNetworkFileRequest> create(const String &url, Function<void(bool)>&& onFinished);
	virtual ~AcinerellaNetworkFileRequest() = default;
	
	virtual RefPtr<SharedBuffer> buffer() = 0;
	virtual void cancel() = 0;

protected:
	String               m_url;
	Function<void(bool)> m_onFinished;
};

}
}

#endif
