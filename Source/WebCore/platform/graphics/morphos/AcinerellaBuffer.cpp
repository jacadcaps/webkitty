#include "AcinerellaBuffer.h"
#include "MediaPlayerMorphOS.h"

#if ENABLE(VIDEO)

#include <WebCore/CurlRequest.h>
#include <WebCore/NetworkingContext.h>
#include <WebCore/NetworkStorageSession.h>
#include <WebCore/CookieJar.h>
#include <WebCore/CookieJarCurl.h>
#include <WebCore/SameSiteInfo.h>
#include <WebCore/SharedBuffer.h>
#include <WebCore/SynchronousLoaderClient.h>
#include <WebCore/ResourceRequest.h>
#include <WebCore/ResourceResponse.h>
#include <WebCore/CurlRequestClient.h>
#include <WebCore/CurlRequest.h>
#include <WebCore/SharedBuffer.h>
#include <queue>
#include "AcinerellaDecoder.h"

#define D(x) x

namespace WebCore {
namespace Acinerella {

class AcinerellaNetworkBufferInternal : public AcinerellaNetworkBuffer, public CurlRequestClient
{
public:
	AcinerellaNetworkBufferInternal(const String &url, size_t readAhead)
		: AcinerellaNetworkBuffer(url, readAhead)
	{
	
	}
	virtual ~AcinerellaNetworkBufferInternal() = default;

	void ref() override { ThreadSafeRefCounted<AcinerellaNetworkBuffer>::ref(); }
	void deref() override { ThreadSafeRefCounted<AcinerellaNetworkBuffer>::deref(); }

	void start(uint64_t from = 0) override
	{
		D(dprintf("%s(%p) - from %llu\n", __func__, this, from));

		if (m_curlRequest)
			m_curlRequest->cancel();
		m_curlRequest = nullptr;

		{
			auto lock = holdLock(m_bufferLock);
			while (!m_buffer.empty())
				m_buffer.pop();
		}

		m_finishedLoading = false;
		m_didFailLoading = false;

		m_bufferRead = 0;
		m_bufferPositionAbs = from;
		m_bufferSize = 0;
		m_redirectCount = 0;
		m_isPaused = false;

		m_request = ResourceRequest(m_url);
		m_curlRequest = createCurlRequest(m_request);
		if (m_curlRequest)
		{
			m_curlRequest->setResumeOffset(static_cast<long long>(m_bufferPositionAbs));
			m_curlRequest->start();
		}
	}

	void stop() override
	{
		D(dprintf("%s(%p)\n", __func__, this));

		if (m_curlRequest)
			m_curlRequest->cancel();
		m_curlRequest = nullptr;
		m_finishedLoading = true;
		
		m_eventSemaphore.signal();
		D(dprintf("%s(%p) ..\n", __func__, this));
	}
	
	int read(uint8_t *outBuffer, int size) override
	{
		D(dprintf("%s(%p): requested %ld\n", __func__, this, size));
		int sizeWritten = 0;
		int sizeLeft = size;

		while (sizeWritten < size)
		{
			{
				auto lock = holdLock(m_bufferLock);
				if (!m_buffer.empty())
				{
					auto buffer = m_buffer.front();
					int write = std::min(int(buffer->size() - m_bufferRead), sizeLeft);

					D(dprintf("%s: %p write %ld br %ld sl %ld sw %ld\n", __func__, this, write, m_bufferRead, sizeLeft, sizeWritten));

					memcpy(outBuffer + sizeWritten, buffer->data() + m_bufferRead, write);
					m_bufferRead += write;
					m_bufferPositionAbs += write;
					sizeLeft -= write;
					sizeWritten += write;

					D(dprintf("%s: %p nbr %ld w %ld\n", __func__, this, m_bufferRead, m_bufferPositionAbs));

					if (m_bufferRead == int(buffer->size()))
					{
						m_buffer.pop();
						m_bufferSize -= m_bufferRead;
						m_bufferRead = 0;
						
						D(dprintf("%s: pop!\n", __func__));
						
						// Check if we don't need to resume reading...
						if (m_isPaused && m_bufferRead < (m_readAhead / 2))
						{
							WTF::callOnMainThread([this]() {
								continueBuffering();
							});
						}
					}
				}
			}
			
			if (sizeWritten < size && !m_finishedLoading)
				m_eventSemaphore.waitFor(10_s);
			else if (m_finishedLoading)
				break;
		}

		D(dprintf("%s(%p): written %ld\n", __func__, this, sizeWritten));
		return sizeWritten;
	}
	
	void continueBuffering()
	{
		D(dprintf("%s(%p)\n", __func__, this));
		if (m_isPaused && m_bufferRead < (m_readAhead / 2) && m_curlRequest && !m_finishedLoading)
		{
			D(dprintf("%s(%p): resuming...\n", __func__, this));
			m_isPaused = false;
			m_curlRequest->resume();
		}
	}
	
	Ref<CurlRequest> createCurlRequest(ResourceRequest&request)
	{
		auto context = MediaPlayerMorphOSSettings::settings().m_networkingContextForRequests;
		if (context)
		{
			auto& storageSession = *context->storageSession();
			auto& cookieJar = storageSession.cookieStorage();
			auto includeSecureCookies = request.url().protocolIs("https") ? IncludeSecureCookies::Yes : IncludeSecureCookies::No;
			String cookieHeaderField = cookieJar.cookieRequestHeaderFieldValue(storageSession, request.firstPartyForCookies(), SameSiteInfo::create(request), request.url(), WTF::nullopt, WTF::nullopt, includeSecureCookies).first;
			if (!cookieHeaderField.isEmpty())
				request.addHTTPHeaderField(HTTPHeaderName::Cookie, cookieHeaderField);
		}

		auto curlRequest = CurlRequest::create(WTFMove(request), *this);
		return curlRequest;
	}

	void curlDidSendData(CurlRequest&, unsigned long long, unsigned long long) override
	{
		D(dprintf("%s(%p)\n", __func__, this));
	}

	inline bool shouldRedirectAsGET(const ResourceRequest& request, bool crossOrigin)
	{
		if ((request.httpMethod() == "GET") || (request.httpMethod() == "HEAD"))
			return false;

		if (!request.url().protocolIsInHTTPFamily())
			return true;

		if (m_response.isSeeOther())
			return true;

		if ((m_response.isMovedPermanently() || m_response.isFound()) && (request.httpMethod() == "POST"))
			return true;

		if (crossOrigin && (request.httpMethod() == "DELETE"))
			return true;

		return false;
	}
	
	void curlDidReceiveResponse(CurlRequest& request, CurlResponse&& response) override
	{
		D(dprintf("%s(%p)\n", __func__, this));
		if (m_curlRequest.get() == &request)
		{
			D(dprintf("%s(%p)..\n", __func__, this));
			m_response = ResourceResponse(response);

			if (m_response.shouldRedirect())
			{
				static const int maxRedirects = 20;

				if (m_redirectCount++ > maxRedirects)
				{
					m_didFailLoading = true;
					m_eventSemaphore.signal();
					return;
				}

				String location = m_response.httpHeaderField(HTTPHeaderName::Location);
				URL newURL = URL(m_response.url(), location);
				bool crossOrigin = !protocolHostAndPortAreEqual(m_request.url(), newURL);

				ResourceRequest newRequest = m_request;
				newRequest.setURL(newURL);

				if (shouldRedirectAsGET(newRequest, crossOrigin)) {
					newRequest.setHTTPMethod("GET");
					newRequest.setHTTPBody(nullptr);
					newRequest.clearHTTPContentType();
				}

				if (crossOrigin) {
					// If the network layer carries over authentication headers from the original request
					// in a cross-origin redirect, we want to clear those headers here.
					newRequest.clearHTTPAuthorization();
					newRequest.clearHTTPOrigin();
				}

				m_curlRequest->cancel();
				m_curlRequest = createCurlRequest(newRequest);
				if (m_curlRequest)
				{
					m_curlRequest->setResumeOffset(static_cast<long long>(m_bufferPositionAbs));
					m_curlRequest->start();
				}

				D(dprintf("%s(%p): redirected to %s\n", __func__, this, location.utf8().data()));
				return;
			}
			
			request.completeDidReceiveResponse();
		}
	}
	
	void curlDidReceiveBuffer(CurlRequest& request, Ref<SharedBuffer>&&buffer) override
	{
		D(dprintf("%s(%p): %d bytes, currently buffered size: %d\n", __func__, this, buffer->size(), m_bufferSize));
		if (m_curlRequest.get() == &request)
		{
			if (buffer->size())
			{
				{
					auto lock = holdLock(m_bufferLock);
					m_bufferSize += buffer->size();
					m_buffer.push(RefPtr<SharedBuffer>(WTFMove(buffer)));

					if (m_bufferSize > m_readAhead && !m_isPaused)
					{
						if (m_curlRequest)
						{
							D(dprintf("%s: suspending...\n", __func__));
							m_curlRequest->suspend();
							m_isPaused = true;
						}
					}
				}

				m_eventSemaphore.signal();
			}
		}
	}
	
	void curlDidComplete(CurlRequest& request, NetworkLoadMetrics&&) override
	{
		D(dprintf("%s(%p)\n", __func__, this));
		if (m_curlRequest.get() == &request)
		{
			m_finishedLoading = true;
			m_eventSemaphore.signal();
			m_curlRequest->cancel();
			m_curlRequest = nullptr;
		}
	}
	
	void curlDidFailWithError(CurlRequest& request, ResourceError&&, CertificateInfo&&) override
	{
		D(dprintf("%s(%p)\n", __func__, this));
		if (m_curlRequest.get() == &request)
		{
			m_finishedLoading = true;
			m_didFailLoading = true;
			m_eventSemaphore.signal();
			m_curlRequest->cancel();
			m_curlRequest = nullptr;
		}
	}
protected:
	ResourceRequest                  m_request;
	ResourceResponse                 m_response;
	unsigned                         m_redirectCount = 0;
	BinarySemaphore                  m_eventSemaphore;
	RefPtr<CurlRequest>              m_curlRequest;
	Lock                             m_bufferLock;
	std::queue<RefPtr<SharedBuffer>> m_buffer;
	// pos within the front() chunk
	int                              m_bufferRead = 0;
	// abs position in the stream that we've read (not in the buffer anymore)
	uint64_t                         m_bufferPositionAbs = 0;
	// size of all chunks on the list (- m_bufferRead)
	int                              m_bufferSize = 0;
	bool                             m_finishedLoading = false;
	bool                             m_didFailLoading = false;
	bool                             m_isPaused = false;
};

AcinerellaNetworkBuffer::AcinerellaNetworkBuffer(const String &url, size_t readAhead)
	: m_url(url)
	, m_readAhead(readAhead)
{
	D(dprintf("%s(%p) - %s\n", __func__, this, url.utf8().data()));
}

RefPtr<AcinerellaNetworkBuffer> AcinerellaNetworkBuffer::create(const String &url, size_t readAhead)
{
	return RefPtr<AcinerellaNetworkBuffer>(WTF::adoptRef(*new AcinerellaNetworkBufferInternal(url, readAhead)));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AcinerellaMuxedBuffer::AcinerellaMuxedBuffer(Function<void()>&& sinkFunction, int audioIndex, int videoIndex, unsigned int audioQueueSize, unsigned int videoQueueSize)
	: m_sinkFunction(WTFMove(sinkFunction))
	, m_audioPackageIndex(audioIndex)
	, m_audioQueueAheadSize(audioQueueSize)
	, m_videoPackageIndex(videoIndex)
	, m_videoQueueAheadSize(videoQueueSize)
{
}

void AcinerellaMuxedBuffer::push(AcinerellaPackage&&package)
{
	// is this a valid package?
	if (!!package)
	{
		const auto index = package.index();
		
		D(dprintf("%s: type %s\n", __func__, index == m_audioPackageIndex ? "audio" : (index == m_videoPackageIndex ? "video" : "drop")));
		
		if (index == m_audioPackageIndex || index == m_videoPackageIndex)
		{
			bool wantMore = false;

			{
				auto lock = holdLock(m_lock);

				if (index == m_audioPackageIndex)
					m_audioPackages.emplace(WTFMove(package));
				else
					m_videoPackages.emplace(WTFMove(package));
				
				wantMore = (m_audioPackages.size() < m_audioQueueAheadSize) || (m_videoPackages.size() < m_videoQueueAheadSize);
			}
			
			if (index == m_audioPackageIndex)
				m_audioEvent.signal();
			else
				m_videoEvent.signal();
			
			// no need to lock since it can only be cleared from within the same thread as this function is called on
			if (wantMore && m_sinkFunction)
				m_sinkFunction();
		}
	}
	else
	{
		m_queueCompleteOrError = true;
		m_audioEvent.signal();
		m_videoEvent.signal();
	}
}

void AcinerellaMuxedBuffer::flush()
{
	{
		auto lock = holdLock(m_lock);
		m_queueCompleteOrError = false;
		while (!m_audioPackages.empty())
			m_audioPackages.pop();
		while (!m_videoPackages.empty())
			m_videoPackages.pop();
	}
}

void AcinerellaMuxedBuffer::terminate()
{
	{
		auto lock = holdLock(m_lock);
		m_sinkFunction = nullptr;
		m_queueCompleteOrError = true;
	}

	m_audioEvent.signal();
	m_videoEvent.signal();
}

bool AcinerellaMuxedBuffer::nextPackage(AcinerellaDecoder &decoder, AcinerellaPackage &outPackage)
{
	D(dprintf("%s: isAudio %d\n", __func__, decoder.isAudio()));

	if (decoder.isAudio())
	{
		for (;;)
		{
			bool requestMore = false;
			bool hasPackage = false;

			{
				auto lock = holdLock(m_lock);
				if (!m_audioPackages.empty())
				{
					outPackage = WTFMove(m_audioPackages.front());
					m_audioPackages.pop();
					hasPackage = true;
					requestMore = m_audioPackages.size() < m_audioQueueAheadSize;
				}
				else if (m_queueCompleteOrError)
				{
					return false;
				}
				else
				{
					requestMore = true;
				}

				if (requestMore && m_sinkFunction)
					m_sinkFunction();
			}
			
			if (hasPackage)
				return true;
			
			m_audioEvent.waitFor(15_s);
		}
	}
	else
	{
		for (;;)
		{
			bool requestMore = false;
			bool hasPackage = false;

			{
				auto lock = holdLock(m_lock);
				if (!m_videoPackages.empty())
				{
					outPackage = WTFMove(m_videoPackages.front());
					m_videoPackages.pop();
					hasPackage = true;
					requestMore = m_audioPackages.size() < m_audioQueueAheadSize;
				}
				else if (m_queueCompleteOrError)
				{
					return false;
				}
				else
				{
					requestMore = true;
				}
			
				if (requestMore && m_sinkFunction)
					m_sinkFunction();
			}
			
			if (hasPackage)
				return true;
			
			m_videoEvent.waitFor(15_s);
		}
	}

	return false;
}

}
}
#endif
