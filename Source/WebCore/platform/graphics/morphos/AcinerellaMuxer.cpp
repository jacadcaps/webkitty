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
#include "AcinerellaHLS.h"

#define D(x) 

namespace WebCore {
namespace Acinerella {

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AcinerellaMuxedBuffer::setSinkFunction(Function<void()>&& sinkFunction)
{
	auto lock = holdLock(m_lock);
	m_sinkFunction = WTFMove(sinkFunction);
}

void AcinerellaMuxedBuffer::setDecoderMask(uint32_t mask)
{
	auto lock = holdLock(m_lock);
	m_decoderMask = mask;
}

void AcinerellaMuxedBuffer::push(AcinerellaPackage&&package)
{
	// is this a valid package?
	if (!!package)
	{
		const auto index = package.index();
		bool wantMore = false;
		
		D(dprintf("%s: package index %lu isFlush %d\n", __PRETTY_FUNCTION__, index, package.isFlushPackage()));

		{
			auto lock = holdLock(m_lock);

			// Flush goes into all valid queues!
			if (package.isFlushPackage())
			{
				forValidDecoders([&](AcinerellaPackageQueue& queue, BinarySemaphore&) {
					queue.emplace(AcinerellaPackage(package.acinerella(), package.package()));
				});
			}
			else if (isDecoderValid(index))
			{
				m_packages[index].emplace(WTFMove(package));
			}
			
			wantMore = needsToCallSinkFunction();
		}
		
		if (isDecoderValid(index))
			m_events[index].signal();

		// no need to lock since it can only be cleared from within the same thread as this function is called on
		if (wantMore && m_sinkFunction)
			m_sinkFunction();
	}
	else
	{
		m_queueCompleteOrError = true;
	
		forValidDecoders([](AcinerellaPackageQueue& queue, BinarySemaphore& event) {
			while (!queue.empty())
				event.signal();
		});
	}
}

void AcinerellaMuxedBuffer::flush()
{
	{
		auto lock = holdLock(m_lock);
		m_queueCompleteOrError = false;
		forValidDecoders([](AcinerellaPackageQueue& queue, BinarySemaphore&) {
			while (!queue.empty())
				queue.pop();
		});
	}
}

void AcinerellaMuxedBuffer::terminate()
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));

	{
		auto lock = holdLock(m_lock);
		m_sinkFunction = nullptr;
		m_queueCompleteOrError = true;
	}

	forValidDecoders([](AcinerellaPackageQueue& queue, BinarySemaphore& event) {
		event.signal();
	});
}

bool AcinerellaMuxedBuffer::nextPackage(AcinerellaDecoder &decoder, AcinerellaPackage &outPackage)
{
	D(dprintf("%s: isAudio %d index %lu\n", __PRETTY_FUNCTION__, decoder.isAudio(), decoder.index()));

	const auto index = decoder.index();

	for (;;)
	{
		bool requestMore = false;
		bool hasPackage = false;

		{
			auto lock = holdLock(m_lock);
			D(dprintf("%s: packages %d complete %d\n", __func__, m_packages[index].size(), m_queueCompleteOrError));
			if (!m_packages[index].empty())
			{
				outPackage = WTFMove(m_packages[index].front());
				m_packages[index].pop();
				hasPackage = true;
				requestMore = m_packages[index].size() < queueReadAheadSize;
			}
			else if (m_queueCompleteOrError)
			{
				return false;
			}
			else
			{
				requestMore = true;
			}

			if (requestMore && m_sinkFunction && !m_queueCompleteOrError)
				m_sinkFunction();
		}
		
		if (hasPackage)
			return true;
		
		m_events[index].waitFor(10_s);
	}

	return false;
}

}
}
#endif
