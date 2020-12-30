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
		
		D(dprintf("%s: type %s\n", __PRETTY_FUNCTION__, index == m_audioPackageIndex ? "audio" : (index == m_videoPackageIndex ? "video" : "drop")));
		
		if (index == m_audioPackageIndex || (!m_dropVideoPackages && (index == m_videoPackageIndex)) || package.isFlushPackage())
		{
			bool wantMore = false;

			{
				auto lock = holdLock(m_lock);

				if (package.isFlushPackage())
				{
					m_audioPackages.emplace(AcinerellaPackage(package.acinerella(), package.package()));
					if (!m_dropVideoPackages)
						m_videoPackages.emplace(AcinerellaPackage(package.acinerella(), package.package()));
				}
				else if (index == m_audioPackageIndex)
				{
					m_audioPackages.emplace(WTFMove(package));
				}
				else
				{
					m_videoPackages.emplace(WTFMove(package));
				}
				
				wantMore = ((m_audioPackages.size() < m_audioQueueAheadSize) || (!m_dropVideoPackages && (m_videoPackages.size() < m_videoQueueAheadSize)));
			}
			
			if (index == m_audioPackageIndex)
				m_audioEvent.signal();
			else
				m_videoEvent.signal();
			
			// no need to lock since it can only be cleared from within the same thread as this function is called on
			if (wantMore && m_sinkFunction)
				m_sinkFunction();
		}
		else if ((m_audioPackages.size() < m_audioQueueAheadSize) || (!m_dropVideoPackages && (m_videoPackages.size() < m_videoQueueAheadSize)))
		{
			if (m_sinkFunction)
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
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));

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
	D(dprintf("%s: isAudio %d\n", __PRETTY_FUNCTION__, decoder.isAudio()));

	if (decoder.isAudio())
	{
		for (;;)
		{
			bool requestMore = false;
			bool hasPackage = false;

			{
				auto lock = holdLock(m_lock);
				D(dprintf("%s: audiopackages %d complete %d\n", __func__, m_audioPackages.size(), m_queueCompleteOrError));
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

				if (requestMore && m_sinkFunction && !m_queueCompleteOrError)
					m_sinkFunction();
			}
			
			if (hasPackage)
				return true;
			
			m_audioEvent.waitFor(2_s);
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
					requestMore = m_videoPackages.size() < m_videoQueueAheadSize;
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
			
			m_videoEvent.waitFor(2_s);
		}
	}

	return false;
}

}
}
#endif
