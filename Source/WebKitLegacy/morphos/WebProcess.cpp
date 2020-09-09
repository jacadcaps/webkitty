#include "WebKit.h"
#include "WebProcess.h"
#include "WebPage.h"
#include "WebFrame.h"

// todo: check why it isn't enabled inside WebKitLegacy
#ifdef ENABLE_CONTENT_EXTENSIONS
#undef ENABLE_CONTENT_EXTENSIONS
#endif
#define ENABLE_CONTENT_EXTENSIONS 1

#include <WebCore/GCController.h>
#include <WebCore/FontCache.h>
#include <WebCore/MemoryCache.h>
#include <WebCore/MemoryRelease.h>
#include <WebCore/CacheStorageProvider.h>
#include <WebCore/BackForwardCache.h>
#include <WebCore/CommonVM.h>
#include <WebCore/CurlCacheManager.h>
#include <WebCore/ProcessWarming.h>
#include <WebCore/DocumentLoader.h>
#include <WebCore/DOMWindow.h>
#include <WebCore/FrameLoader.h>
#include <wtf/Algorithms.h>
#include <wtf/Language.h>
#include <wtf/ProcessPrivilege.h>
#include <wtf/RunLoop.h>
#include <wtf/SystemTracing.h>
#include <wtf/URLParser.h>
#include <wtf/text/StringHash.h>
#include <JavaScriptCore/VM.h>
#include <JavaScriptCore/Heap.h>
#include <JavaScriptCore/JSLock.h>
#include <JavaScriptCore/MemoryStatistics.h>
#include <WebCore/CrossOriginPreflightResultCache.h>
#include <WebCore/ResourceLoadInfo.h>
#include <WebCore/CurlContext.h>
#include "NetworkStorageSessionMap.h"
#include "WebDatabaseProvider.h"
#include "WebStorageNamespaceProvider.h"
#include "WebFrameNetworkingContext.h"
// bloody include shit
extern "C" {
LONG WaitSelect(LONG nfds, fd_set *readfds, fd_set *writefds, fd_set *exeptfds,
                struct timeval *timeout, ULONG *maskp);
}
typedef uint32_t socklen_t;
#include <pal/crypto/gcrypt/Initialization.h>
#include <proto/dos.h>

#define USE_ADFILTER 1

extern "C" {
	void dprintf(const char *, ...);
};

#define D(x) 

/// TODO
/// MemoryPressureHandler !

using namespace WebCore;
using namespace WTF;
using namespace JSC;

namespace WTF {
	void scheduleDispatchFunctionsOnMainThread()
	{
		static WebKit::WebProcess &singleton = WebKit::WebProcess::singleton();
		singleton.signalMainThread();
	}
}

namespace WebKit {

WebProcess::WebProcess()
	: m_sessionID(PAL::SessionID::defaultSessionID())
	, m_cacheStorageProvider(CacheStorageProvider::create())
{
}

WebProcess& WebProcess::singleton()
{
    static WebProcess process;
    return process;
}

void WebProcess::initialize(int sigbit)
{
	m_sigTask = FindTask(0);
	m_sigMask = 1UL << sigbit;
	setCacheModel(CacheModel::PrimaryWebBrowser);
	D(dprintf("%s mask %u\n", __PRETTY_FUNCTION__, m_sigMask));

	GCController::singleton().setJavaScriptGarbageCollectorTimerEnabled(true);
	PAL::GCrypt::initialize();

	m_dummyNetworkingContext = WebFrameNetworkingContext::create(nullptr);

#if USE_ADFILTER
	WTF::String easyListPath = "PROGDIR:Resources/easylist.txt";
	WTF::String easyListSerializedPath = "PROGDIR:Resources/easylist.dat";

	WTF::FileSystemImpl::PlatformFileHandle fh = WTF::FileSystemImpl::openFile(easyListSerializedPath, WTF::FileSystemImpl::FileOpenMode::Read);

	if (WTF::FileSystemImpl::invalidPlatformFileHandle != fh)
	{
		long long size;

		if (WTF::FileSystemImpl::getFileSize(fh, size) && size > 0ull)
		{
			m_urlFilterData.resize(size + 1);
			if (size == WTF::FileSystemImpl::readFromFile(fh, &m_urlFilterData[0], int(size)))
			{
				m_urlFilterData[size] = 0; // terminate just in case
				m_urlFilter.deserialize(&m_urlFilterData[0]);
			}
			else
			{
				m_urlFilterData.clear();
			}
		}

		WTF::FileSystemImpl::closeFile(fh);
	}
	else
	{
		WTF::FileSystemImpl::PlatformFileHandle fh = WTF::FileSystemImpl::openFile(easyListPath, WTF::FileSystemImpl::FileOpenMode::Read);

		if (WTF::FileSystemImpl::invalidPlatformFileHandle != fh)
		{
			long long size;
			if (WTF::FileSystemImpl::getFileSize(fh, size) && size > 0ull)
			{
				char *buffer = (char *)malloc(size + 1);
				if (buffer)
				{
					if (size == WTF::FileSystemImpl::readFromFile(fh, buffer, int(size)))
					{
						buffer[size] = 0; // terminate, parser expects this to be a null-term string
dprintf("Parsing easylist.txt; this will take a while... and will be faster on next launch!\n");
						m_urlFilter.parse(buffer);
						int ssize;
						char *sbuffer = m_urlFilter.serialize(&ssize, false);
						WTF::FileSystemImpl::PlatformFileHandle dfh = WTF::FileSystemImpl::openFile(easyListSerializedPath, WTF::FileSystemImpl::FileOpenMode::Write);
						if (WTF::FileSystemImpl::invalidPlatformFileHandle != dfh)
						{
							if (ssize != WTF::FileSystemImpl::writeToFile(dfh, sbuffer, ssize))
							{
								WTF::FileSystemImpl::closeFile(dfh);
								WTF::FileSystemImpl::deleteFile(easyListSerializedPath);
							}
							else
							{
								WTF::FileSystemImpl::closeFile(dfh);
							}
						}
						else
						{
							dprintf(">> failed opening fiel for write\n");
						}
						delete[] sbuffer;
					}
					
					free(buffer);
				}
			}
			WTF::FileSystemImpl::closeFile(fh);
		}
	}
#endif
}

void WebProcess::terminate()
{
	D(dprintf("%s\n", __PRETTY_FUNCTION__));
	WebCore::DOMWindow::dispatchAllPendingUnloadEvents();
	WebCore::CurlContext::singleton().stopThread();
	NetworkStorageSessionMap::destroyAllSessions();
	WebStorageNamespaceProvider::closeLocalStorage();

	waitForThreads();

    GCController::singleton().garbageCollectNow();
    FontCache::singleton().invalidate();
    MemoryCache::singleton().setDisabled(true);
	WTF::Thread::deleteTLSKey();
	D(dprintf("%s done\n", __PRETTY_FUNCTION__));
}

WebProcess::~WebProcess()
{
	D(dprintf("%s\n", __PRETTY_FUNCTION__));
}

void WebProcess::waitForThreads()
{
	int loops = 5 * 5; // 5s grace period, then sigint
	while (loops-- > 0)
	{
		{
			LockHolder lock(Thread::allThreadsMutex());
			auto& allThreads = Thread::allThreads(lock);
			auto count = allThreads.size();
			if (0 == count)
				return;
			if (2 * 5 == loops)
			{
				D(dprintf("sending SIGINT...\n"));
				for (auto thread : allThreads)
				{
					thread->signal(SIGINT);
				}
			}
		}
		Delay(10);
		dispatchFunctionsFromMainThread();
		WTF::RunLoop::iterate();
	}
	D(dprintf("..done waiting\n"));
}

void WebProcess::handleSignals(const uint32_t sigmask)
{
	dispatchFunctionsFromMainThread();
	WTF::RunLoop::iterate();
}

WebPage* WebProcess::webPage(WebCore::PageIdentifier pageID) const
{
    return m_pageMap.get(pageID);
}

void WebProcess::createWebPage(WebCore::PageIdentifier pageID, WebPageCreationParameters&& parameters)
{
	D(dprintf("%s\n", __PRETTY_FUNCTION__));
    auto result = m_pageMap.add(pageID, nullptr);
//    auto oldPageID = parameters.oldPageID ? parameters.oldPageID.value() : pageID;
    if (result.isNewEntry) {
        ASSERT(!result.iterator->value);
        result.iterator->value = WebPage::create(pageID, WTFMove(parameters));

		D(dprintf("%s >> %p\n", __PRETTY_FUNCTION__, result.iterator->value));

 		ProcessWarming::prewarmGlobally();

// Balanced by an enableTermination in removeWebPage.
//        disableTermination();
    }
    else
    {
//        result.iterator->value->reinitializeWebPage(WTFMove(parameters));
	}
}

void WebProcess::removeWebPage(WebCore::PageIdentifier pageID)
{
    ASSERT(m_pageMap.contains(pageID));
	D(dprintf("%s at %d\n", __PRETTY_FUNCTION__, m_pageMap.size()));
 //   pageWillLeaveWindow(pageID);
    m_pageMap.remove(pageID);

	if (0 == m_pageMap.size() && 0 == m_frameMap.size() && m_fLastPageClosed)
		m_fLastPageClosed();
}

WebPage* WebProcess::focusedWebPage() const
{
#if 0
    for (auto& page : m_pageMap.values()) {
        if (page->windowAndWebPageAreFocused())
            return page.get();
    }
#endif
    return 0;

}

void WebProcess::returnedFromConstrainedRunLoop()
{
	handleSignals(0);

	for (auto& object : m_pageMap.values())
	{
		object->invalidate();
	}
}

WebFrame* WebProcess::webFrame(WebCore::FrameIdentifier frameID) const
{
    return m_frameMap.get(frameID);
}

void WebProcess::addWebFrame(WebCore::FrameIdentifier frameID, WebFrame* frame)
{
	D(dprintf("%s %p %llu\n", __PRETTY_FUNCTION__, frame, frameID));

    m_frameMap.set(frameID, frame);
}

void WebProcess::removeWebFrame(WebCore::FrameIdentifier frameID)
{
	D(dprintf("%s %llu\n", __PRETTY_FUNCTION__, frameID));

    m_frameMap.remove(frameID);

	if (0 == m_pageMap.size() && 0 == m_frameMap.size() && m_fLastPageClosed)
		m_fLastPageClosed();
}

static void fromCountedSetToDebug(TypeCountSet* countedSet, WTF::TextStream& ss)
{
    TypeCountSet::const_iterator end = countedSet->end();
    for (TypeCountSet::const_iterator it = countedSet->begin(); it != end; ++it)
    {
		const char *key = it->key;
        ss << key << " :" << it->value; ss.nextLine();;
	}
}

static void getWebCoreMemoryCacheStatistics(WTF::TextStream& ss)
{
    String imagesString("Images"_s);
    String cssString("CSS"_s);
    String xslString("XSL"_s);
    String javaScriptString("JavaScript"_s);
	
    MemoryCache::Statistics memoryCacheStatistics = MemoryCache::singleton().getStatistics();
	
	ss << "Images count " << memoryCacheStatistics.images.count << " size " << memoryCacheStatistics.images.size << " liveSize " << memoryCacheStatistics.images.liveSize << " decodedSize " << memoryCacheStatistics.images.decodedSize; ss.nextLine();;

	ss << "CSS Sheets count " << memoryCacheStatistics.cssStyleSheets.count << " size " << memoryCacheStatistics.cssStyleSheets.size << " liveSize " << memoryCacheStatistics.cssStyleSheets.liveSize << " decodedSize " << memoryCacheStatistics.cssStyleSheets.decodedSize; ss.nextLine();;

	ss << "XSL Sheets count " << memoryCacheStatistics.xslStyleSheets.count << " size " << memoryCacheStatistics.xslStyleSheets.size << " liveSize " << memoryCacheStatistics.xslStyleSheets.liveSize << " decodedSize " << memoryCacheStatistics.xslStyleSheets.decodedSize; ss.nextLine();;

	ss << "Scripts count " << memoryCacheStatistics.scripts.count << " size " << memoryCacheStatistics.scripts.size << " liveSize " << memoryCacheStatistics.scripts.liveSize << " decodedSize " << memoryCacheStatistics.scripts.decodedSize; ss.nextLine();;
}

void WebProcess::dumpWebCoreStatistics()
{
    GCController::singleton().garbageCollectNow();

	WTF::TextStream ss;
	
    // Gather JavaScript statistics.
    {
        JSLockHolder lock(commonVM());
        ss << "JavaScriptObjectsCount " << commonVM().heap.objectCount(); ss.nextLine();;
        ss << "JavaScriptGlobalObjectsCount " << commonVM().heap.globalObjectCount(); ss.nextLine();;
        ss << "JavaScriptProtectedObjectsCount " << commonVM().heap.protectedObjectCount(); ss.nextLine();;
        ss << "JavaScriptProtectedGlobalObjectsCount " << commonVM().heap.protectedGlobalObjectCount(); ss.nextLine();;
		
        std::unique_ptr<TypeCountSet> protectedObjectTypeCounts(commonVM().heap.protectedObjectTypeCounts());
        fromCountedSetToDebug(protectedObjectTypeCounts.get(), ss);
		
        std::unique_ptr<TypeCountSet> objectTypeCounts(commonVM().heap.objectTypeCounts());
        fromCountedSetToDebug(objectTypeCounts.get(), ss);
		
        uint64_t javaScriptHeapSize = commonVM().heap.size();
        ss << "JavaScriptHeapSize " << javaScriptHeapSize; ss.nextLine();;
        ss << "JavaScriptFreeSize " << (commonVM().heap.capacity() - javaScriptHeapSize); ss.nextLine();;
    }

    WTF::FastMallocStatistics fastMallocStatistics = WTF::fastMallocStatistics();
    ss << "FastMallocReservedVMBytes " << fastMallocStatistics.reservedVMBytes; ss.nextLine();;
    ss << "FastMallocCommittedVMBytes " << fastMallocStatistics.committedVMBytes; ss.nextLine();;
    ss << "FastMallocFreeListBytes " << fastMallocStatistics.freeListBytes; ss.nextLine();;
	
    // Gather font statistics.
    auto& fontCache = FontCache::singleton();
    ss << "CachedFontDataCount " << fontCache.fontCount(); ss.nextLine();;
    ss << "CachedFontDataInactiveCount " << fontCache.inactiveFontCount(); ss.nextLine();;
	
    // Gather glyph page statistics.
//    ss << "GlyphPageCount " << GlyphPage::count(); ss.nextLine();;
	
    // Get WebCore memory cache statistics
    getWebCoreMemoryCacheStatistics(ss);
	
    dprintf(ss.release().utf8().data());
}

void WebProcess::garbageCollectJavaScriptObjects()
{
    GCController::singleton().garbageCollectNow();
}

void WebProcess::clearResourceCaches()
{
    // Toggling the cache model like this forces the cache to evict all its in-memory resources.
    // FIXME: We need a better way to do this.
    CacheModel cacheModel = m_cacheModel;
    setCacheModel(CacheModel::DocumentViewer);
    setCacheModel(cacheModel);

    MemoryCache::singleton().evictResources();

    // Empty the cross-origin preflight cache.
    CrossOriginPreflightResultCache::singleton().clear();
}

void WebProcess::setCacheModel(CacheModel cacheModel)
{
    if (m_hasSetCacheModel && (cacheModel == m_cacheModel))
        return;

    m_hasSetCacheModel = true;
    m_cacheModel = cacheModel;

    unsigned cacheTotalCapacity = 0;
    unsigned cacheMinDeadCapacity = 0;
    unsigned cacheMaxDeadCapacity = 0;
    Seconds deadDecodedDataDeletionInterval;
    unsigned pageCacheSize = 0;

    calculateMemoryCacheSizes(cacheModel, cacheTotalCapacity, cacheMinDeadCapacity, cacheMaxDeadCapacity, deadDecodedDataDeletionInterval, pageCacheSize);

    auto& memoryCache = MemoryCache::singleton();
    memoryCache.setCapacities(cacheMinDeadCapacity, cacheMaxDeadCapacity, cacheTotalCapacity);
    memoryCache.setDeadDecodedDataDeletionInterval(deadDecodedDataDeletionInterval);
    BackForwardCache::singleton().setMaxSize(pageCacheSize);
    CurlCacheManager::singleton().setCacheDirectory(String("PROGDIR:Cache/Curl"));

    D(dprintf("CACHES SETUP, total %d\n", cacheTotalCapacity));
}

void WebProcess::signalMainThread()
{
	Signal(m_sigTask, m_sigMask);
}

bool WebProcess::shouldAllowRequest(const char *url, const char *mainPageURL, WebCore::DocumentLoader& loader)
{
	WebFrame *frame = WebFrame::fromCoreFrame(*loader.frame());
	if (!frame)
		return false;
	WebPage *page = frame->page();
	if (!page)
		return false;

	if (!page->adBlockingEnabled())
		return true;

	if (m_urlFilter.matches(url, ABP::FONoFilterOption, mainPageURL))
	{
		return false;
	}

	return true;
}

}

RefPtr<WebCore::SharedBuffer> loadResourceIntoBuffer(const char* name);
RefPtr<WebCore::SharedBuffer> loadResourceIntoBuffer(const char* name)
{
	WTF::String path = "PROGDIR:Resources/";
	path.append(name);
	path.append(".png");

	WTF::FileSystemImpl::PlatformFileHandle fh = WTF::FileSystemImpl::openFile(path, WTF::FileSystemImpl::FileOpenMode::Read);

	if (WTF::FileSystemImpl::invalidPlatformFileHandle != fh)
	{
		long long size;
		if (WTF::FileSystemImpl::getFileSize(fh, size) && size > 0ull && size < (32ull * 1024ull))
		{
			char buffer[size];
			if (size == WTF::FileSystemImpl::readFromFile(fh, buffer, int(size)))
			{
				WTF::FileSystemImpl::closeFile(fh);
				return WebCore::SharedBuffer::create(reinterpret_cast<const char*>(buffer), size);
			}
		}
		WTF::FileSystemImpl::closeFile(fh);
	}

	return nullptr;
}

bool shouldLoadResource(const WebCore::ContentExtensions::ResourceLoadInfo& info, WebCore::DocumentLoader& loader)
{
#if USE_ADFILTER
	static WebKit::WebProcess &instance = WebKit::WebProcess::singleton();
	auto url = info.resourceURL.string().utf8();
	auto mainurl = info.mainDocumentURL.string().utf8();
	return instance.shouldAllowRequest(url.data(), mainurl.data(), loader);
#else
	return true;
#endif
}
