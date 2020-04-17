#include "WebProcess.h"
#include "WebPage.h"
#include "WebFrame.h"

#include <WebCore/GCController.h>
#include <WebCore/FontCache.h>
#include <WebCore/MemoryCache.h>
#include <WebCore/MemoryRelease.h>
#include <WebCore/PageCache.h>
#include <WebCore/CommonVM.h>
#include <WebCore/CurlCacheManager.h>
#include <WebCore/ProcessWarming.h>
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

#include <proto/dos.h>

extern "C" {
	void dprintf(const char *, ...);
};

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
	dprintf("%s mask %u\n", __PRETTY_FUNCTION__, m_sigMask);

// makes shit fail quickly now
// 	ProcessWarming::prewarmGlobally();

	GCController::singleton().setJavaScriptGarbageCollectorTimerEnabled(true);
}

void WebProcess::terminate()
{
    GCController::singleton().garbageCollectNow();
    FontCache::singleton().invalidate();
    MemoryCache::singleton().setDisabled(true);
}

WebProcess::~WebProcess()
{
dprintf("WebProcess will wait for threads...\n");
	waitForThreads();
}

void WebProcess::waitForThreads()
{
	for (;;)
	{
		{
			LockHolder lock(Thread::allThreadsMutex());
			auto count = Thread::allThreads(lock).size();
			if (0 == count)
				return;
			dprintf("wait for %ld threads\n", count);
		}
		Delay(10);
		dispatchFunctionsFromMainThread();
		WTF::RunLoop::iterate();
	}
}

void WebProcess::handleSignals(const uint32_t sigmask)
{
	try {
		dispatchFunctionsFromMainThread();
		WTF::RunLoop::iterate();
	} catch (std::exception &ex) {
		dprintf("%s: webkit threw %s\n", ex.what());
	}
}

WebPage* WebProcess::webPage(WebCore::PageIdentifier pageID) const
{
    return m_pageMap.get(pageID);
}

void WebProcess::createWebPage(WebCore::PageIdentifier pageID, WebPageCreationParameters&& parameters)
{
dprintf("%s\n", __PRETTY_FUNCTION__);
    auto result = m_pageMap.add(pageID, nullptr);
//    auto oldPageID = parameters.oldPageID ? parameters.oldPageID.value() : pageID;
    if (result.isNewEntry) {
        ASSERT(!result.iterator->value);
        result.iterator->value = WebPage::create(pageID, WTFMove(parameters));
dprintf("%s >> %p\n", __PRETTY_FUNCTION__, result.iterator->value);

// Balanced by an enableTermination in removeWebPage.
//        disableTermination();
    }
    else
    {
//        result.iterator->value->reinitializeWebPage(WTFMove(parameters));
	}
}

void WebProcess::removeWebPage(PAL::SessionID, WebCore::PageIdentifier pageID)
{
    ASSERT(m_pageMap.contains(pageID));
dprintf("%s\n", __PRETTY_FUNCTION__);
 //   pageWillLeaveWindow(pageID);
    m_pageMap.remove(pageID);

//    enableTermination();
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

WebFrame* WebProcess::webFrame(uint64_t frameID) const
{
    return m_frameMap.get(frameID);
}

void WebProcess::addWebFrame(uint64_t frameID, WebFrame* frame)
{
dprintf("%s %p %llu\n", __PRETTY_FUNCTION__, frame, frameID);

    m_frameMap.set(frameID, frame);
}

void WebProcess::removeWebFrame(uint64_t frameID)
{
dprintf("%s %llu\n", __PRETTY_FUNCTION__, frameID);

    m_frameMap.remove(frameID);
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
    ss << "GlyphPageCount " << GlyphPage::count(); ss.nextLine();;
	
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


cacheMinDeadCapacity = cacheMaxDeadCapacity = cacheTotalCapacity = 512*1024*1024;
pageCacheSize = 256*1024*1024;

    auto& memoryCache = MemoryCache::singleton();
    memoryCache.setCapacities(cacheMinDeadCapacity, cacheMaxDeadCapacity, cacheTotalCapacity);
    memoryCache.setDeadDecodedDataDeletionInterval(deadDecodedDataDeletionInterval);
    PageCache::singleton().setMaxSize(pageCacheSize);
    CurlCacheManager::singleton().setCacheDirectory(String("PROGDIR:Cache/Curl"));
	
    dprintf("CACHES SETUP, total %d\n", cacheTotalCapacity);
}

void WebProcess::signalMainThread()
{
	Signal(m_sigTask, m_sigMask);
}

}
