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
#include <WebCore/MediaPlayerMorphOS.h>
#include <WebCore/FontCascade.h>
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
#include <WebCore/HTMLMediaElement.h>
#include <WebCore/FrameDestructionObserverInlines.h>
#include <WebCore/Page.h>
#include <WebCore/WebLockRegistry.h>
#include "NetworkStorageSessionMap.h"
#include "WebDatabaseProvider.h"
#include "WebStorageNamespaceProvider.h"
#include "WebPlatformStrategies.h"
#include "WebFrameNetworkingContext.h"
#include "WebServiceWorkerProvider.h"
#include "WebSWOriginStore.h"
#include "WebSWServerToContextConnection.h"
#include "WebSWServerConnection.h"
#include "WebSWContextManagerConnection.h"
#include "storage/WebStorageTrackerClient.h"
#include "cache/WebCacheStorageProvider.h"
#include "cache/CacheStorageEngineConnection.h"
#include "StorageTracker.h"
#include <WebCore/SWContextManager.h>
#include "ServiceWorkerSoftUpdateLoader.h"
#include <WebCore/PageConsoleClient.h>
#include <WebCore/DeprecatedGlobalSettings.h>
#include "Gamepad.h"
#if !MORPHOS_MINIMAL
#include "WebDatabaseManager.h"
#endif

// bloody include shit
extern "C" {
LONG WaitSelect(LONG nfds, fd_set *readfds, fd_set *writefds, fd_set *exeptfds,
                struct timeval *timeout, ULONG *maskp);
}
typedef uint32_t socklen_t;
#if (!MORPHOS_MINIMAL)
#include <pal/crypto/gcrypt/Initialization.h>
#endif
#include <proto/dos.h>

#if (MORPHOS_MINIMAL)
#define USE_ADFILTER 0
#else
#define USE_ADFILTER 1
#endif

extern "C" {
	void dprintf(const char *, ...);
};

#define D(x) 

/// TODO
/// MemoryPressureHandler !

using namespace WebCore;
using namespace WTF;
using namespace JSC;

namespace WebCore {
    void setCookieJarPath(const String& path);
}

namespace WTF {
	void scheduleDispatchFunctionsOnMainThread()
	{
		static WebKit::WebProcess &singleton = WebKit::WebProcess::singleton();
		singleton.signalMainThread();
	}
}

#if 0
#include <wtf/threads/BinarySemaphore.h>
#include <proto/dos.h>
class blocktest
{
	BinarySemaphore bLock;
	BinarySemaphore bLock2;
public:
	blocktest() {
		Thread::create("blockthread", [this] {
			dprintf("calling waitFor\n");
//			bLock2.signal();
			bLock.waitFor(100_s);
			dprintf("block unlocked!\n");
		});
//		bLock2.waitFor(2_s);
		Delay(200);
		dprintf("signalling..\n");
		bLock.signal();
	}
};
static blocktest bt;
#endif

namespace WebKit {

QUAD calculateMaxCacheSize(const char *path)
{
	BPTR lock = Lock(path, ACCESS_READ);

	if (0 == lock)
	{
		WTF::FileSystemImpl::makeAllDirectories(WTF::String::fromUTF8(path));
		lock = Lock(path, ACCESS_READ);
	}

	if (lock)
	{
		struct InfoData idata = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		Info(lock, &idata);
		QUAD total = idata.id_BytesPerBlock;
		QUAD used = idata.id_BytesPerBlock;
		total *= idata.id_NumBlocks;
		used *= idata.id_NumBlocksUsed;
		UnLock(lock);
		if (total > 0)
		{
			QUAD free = total - used;
			free /= 2;
			return std::min(512ll * 1024ll * 1024ll, free);
		}
	}
	
	return 0;
}

WebProcess::WebProcess()
	: m_sessionID(PAL::SessionID::defaultSessionID())
	, m_cacheStorageProvider(WebCacheStorageProvider::create())
    , m_cacheStorageEngineConnection(CacheStorageEngineConnection::create())
    , m_networkSession(NetworkSession::create())
{
}

WebProcess& WebProcess::singleton()
{
    static WebProcess process;
    return process;
}

class DownloadsNetworkingContext : public NetworkingContext {
public:
    static Ref<DownloadsNetworkingContext> create()
    {
        return adoptRef(*new DownloadsNetworkingContext());
    }

    bool shouldClearReferrerOnHTTPSToHTTPRedirect() const override
    {
		return true;
    }
	NetworkStorageSession* storageSession() const
	{
		return &NetworkStorageSessionMap::defaultStorageSession();
	}
protected:
    DownloadsNetworkingContext()
    {
    }
};


void WebProcess::initialize(int sigbit)
{
	m_sigTask = FindTask(0);
	m_sigMask = 1UL << sigbit;

	D(dprintf("%s mask %u\n", __PRETTY_FUNCTION__, m_sigMask));

	GCController::singleton().setJavaScriptGarbageCollectorTimerEnabled(true);

#if (!MORPHOS_MINIMAL)
	PAL::GCrypt::initialize();
#endif

#if 0 // debug
	{
		JSLockHolder lock(commonVM());
		PageConsoleClient::setShouldPrintExceptions(true);
	}
#endif

	WebPlatformStrategies::initialize();
#if !MORPHOS_MINIMAL
	WebKitInitializeWebDatabasesIfNecessary();
#endif

	DeprecatedGlobalSettings::setAccessibilityObjectModelEnabled(false);
//	DeprecatedGlobalSettings::setKeygenElementEnabled(true);
	
    DeprecatedGlobalSettings::setOffscreenCanvasEnabled(true);
    DeprecatedGlobalSettings::setOffscreenCanvasInWorkersEnabled(true);
    
// This doesn't work yet in WebKitLegacy so it will potentially break pages if enabled
    DeprecatedGlobalSettings::setCacheAPIEnabled(true);
    
    // WebKitGTK overrides this - fixes ligatures by enforcing harfbuzz runs
    // so replacements like 'home' -> home icon from a font work with this enabled
    WebCore::FontCascade::setCodePath(WebCore::FontCascade::CodePath::Complex);
 
	m_dummyNetworkingContext = DownloadsNetworkingContext::create();

	WTF::FileSystemImpl::makeAllDirectories("PROGDIR:Cache/FavIcons"_s);

    WebKit::StorageTracker::initializeTracker("PROGDIR:Cache/WebStorage"_s, WebStorageTrackerClient::sharedWebStorageTrackerClient());

#if ENABLE(VIDEO)
	MediaPlayerMorphOSSettings::settings().m_networkingContextForRequests = WebKit::WebProcess::singleton().networkingContext().get();
//	DeprecatedGlobalSettings::setModernMediaControlsEnabled(false);

	MediaPlayerMorphOSSettings::settings().m_supportMediaForHost = [this](WebCore::Page *page, const String &host) -> bool {
		WebPage *wpage = WebPage::fromCorePage(page);
		if (!wpage)
			wpage = webPageForHost(host);
		if (wpage && wpage->_fMediaSupportCheck)
			return wpage->_fMediaSupportCheck(WebViewDelegate::mediaType::Media);
		return false;
	};

	MediaPlayerMorphOSSettings::settings().m_supportMediaSourceForHost = [this](WebCore::Page *page, const String &host) -> bool {
		WebPage *wpage = WebPage::fromCorePage(page);
		if (!wpage)
			wpage = webPageForHost(host);
		if (wpage && wpage->_fMediaSupportCheck)
			return wpage->_fMediaSupportCheck(WebViewDelegate::mediaType::MediaSource);
		return false;
	};

	MediaPlayerMorphOSSettings::settings().m_supportHLSForHost = [this](WebCore::Page *page, const String &host) -> bool {
		WebPage *wpage = WebPage::fromCorePage(page);
		if (!wpage)
			wpage = webPageForHost(host);
		if (wpage && wpage->_fMediaSupportCheck)
			return wpage->_fMediaSupportCheck(WebViewDelegate::mediaType::HLS);
		return false;
	};
	
	MediaPlayerMorphOSSettings::settings().m_supportVP9ForHost = [this](WebCore::Page *page, const String &host) -> bool {
		WebPage *wpage = WebPage::fromCorePage(page);
		if (!wpage)
			wpage = webPageForHost(host);
		if (wpage && wpage->_fMediaSupportCheck)
			return wpage->_fMediaSupportCheck(WebViewDelegate::mediaType::VP9);
		return false;
	};
	
	MediaPlayerMorphOSSettings::settings().m_supportHVCForHost = [this](WebCore::Page *page, const String &host) -> bool {
		WebPage *wpage = WebPage::fromCorePage(page);
		if (!wpage)
			wpage = webPageForHost(host);
		if (wpage && wpage->_fMediaSupportCheck)
			return wpage->_fMediaSupportCheck(WebViewDelegate::mediaType::HVC1);
		return false;
	};

	MediaPlayerMorphOSSettings::settings().m_load = [this](WebCore::MediaPlayer *player, const String &url, WebCore::MediaPlayerMorphOSInfo& info,
		MediaPlayerMorphOSStreamSettings &settings, Function<void()> &&yieldFunc) {
		for (auto& webpage : m_pageMap.values())
		{
			bool found = false;
			webpage->corePage()->forEachMediaElement([player, &found](WebCore::HTMLMediaElement&e){
				if (player == e.player().get())
				{
					found = true;
				}
			});

			if (found)
			{
				if (webpage->_fMediaAdded)
				{
					webpage->_fMediaAdded(player, url, info, settings, WTFMove(yieldFunc));
				}

				return;
			}
		}
	};

	MediaPlayerMorphOSSettings::settings().m_update = [this](WebCore::MediaPlayer *player, WebCore::MediaPlayerMorphOSInfo& info) {
		for (auto& webpage : m_pageMap.values())
		{
			bool found = false;
			webpage->corePage()->forEachMediaElement([player, &found](WebCore::HTMLMediaElement&e){
				if (player == e.player().get())
				{
					found = true;
				}
			});

			if (found)
			{
				if (webpage->_fMediaUpdated)
				{
					webpage->_fMediaUpdated(player, info);
				}

				return;
			}
		}
	};

	MediaPlayerMorphOSSettings::settings().m_loadCancelled = [this](WebCore::MediaPlayer *player) {
		for (auto& webpage : m_pageMap.values())
		{
			if (webpage->_fMediaRemoved)
				webpage->_fMediaRemoved(player);
		}
	};

	MediaPlayerMorphOSSettings::settings().m_willPlay = [this](WebCore::MediaPlayer *player) {
		for (auto& webpage : m_pageMap.values())
		{
			if (webpage->_fMediaWillPlay)
				webpage->_fMediaWillPlay(player);
		}
	};

	MediaPlayerMorphOSSettings::settings().m_pausedOrFinished = [this](WebCore::MediaPlayer *player) {
		for (auto& webpage : m_pageMap.values())
		{
			if (webpage->_fMediaPausedOrFinished)
				webpage->_fMediaPausedOrFinished(player);
		}
	};

	MediaPlayerMorphOSSettings::settings().m_overlayRequest = [this](WebCore::MediaPlayer *player,
		Function<void(void *windowPtr, int scrollX, int scrollY, int left, int top, int right, int bottom, int width, int height)>&& overlaycallback) {
		for (auto& webpage : m_pageMap.values())
		{
			WebCore::Element* pElement;
			bool found = false;
			webpage->corePage()->forEachMediaElement([player, &found, &pElement](WebCore::HTMLMediaElement&e){
				if (player == e.player().get())
				{
					pElement = &e;
					found = true;
				}
			});

			if (found)
			{
				if (webpage->_fMediaSetOverlayCallback)
				{
					// Wrap pElement into a ref - that way, the callback set on webpage holds a ref to the element
					// This is cause we cannot use RefPtr<Element> in ObjC code
					webpage->_fMediaSetOverlayCallback(player, pElement, [ref = Ref{*pElement}, cb = WTFMove(overlaycallback)](void *windowPtr, int scrollX, int scrollY, int left, int top, int right, int bottom, int width, int height) {
							cb(windowPtr, scrollX, scrollY, left, top, right, bottom, width, height);
						});
				}
				return;
			}
		}
	};
	
	MediaPlayerMorphOSSettings::settings().m_overlayUpdate = [this](WebCore::MediaPlayer *player) {
		for (auto& webpage : m_pageMap.values())
		{
			WebCore::Element* pElement;
			bool found = false;
			webpage->corePage()->forEachMediaElement([player, &found, &pElement](WebCore::HTMLMediaElement&e){
				if (player == e.player().get())
				{
					pElement = &e;
					found = true;
				}
			});

			if (found)
			{
				if (webpage->_fMediaUpdateOverlayCallback)
					webpage->_fMediaUpdateOverlayCallback(player);
				return;
			}
		}
	};
#endif

#if ENABLE(GAMEPAD)
	GamepadProvider::setSharedProvider(GamepadProviderMorphOS::singleton());
#endif

#if ENABLE(SERVICE_WORKER)
    ServiceWorkerProvider::setSharedProvider(WebServiceWorkerProvider::singleton());
	DeprecatedGlobalSettings::setServiceWorkerEnabled(true);
    DeprecatedGlobalSettings::setPushAPIEnabled(true);
#endif
// TODO: check this
//    SharedWorkerProvider::setSharedProvider(WebSharedWorkerProvider::singleton());

#if USE_ADFILTER
	WTF::String easyListPath = "PROGDIR:Resources/easylist.txt"_s;
	WTF::String easyListSerializedPath = "PROGDIR:Resources/easylist.dat"_s;

	WTF::FileSystemImpl::PlatformFileHandle fh = WTF::FileSystemImpl::openFile(easyListSerializedPath, WTF::FileSystemImpl::FileOpenMode::Read);

	if (WTF::FileSystemImpl::invalidPlatformFileHandle != fh)
	{
		long long size = WTF::FileSystemImpl::fileSize(fh).value_or(0);
		if (size > 0ll)
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
			long long size = WTF::FileSystemImpl::fileSize(fh).value_or(0);
			if (size > 0ll)
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
							dprintf(">> failed opening easylist.dat for write\n");
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
	CurlCacheManager::singleton().setStorageSizeLimit(0);

#if ENABLE(SERVICE_WORKER)
    m_swServer.reset();
#endif

    m_networkSession->shutdown();

	waitForThreads();

    GCController::singleton().garbageCollectNow();
//    FontCache::singleton().invalidate(); // trashes memory like fuck on https://testdrive-archive.azurewebsites.net/Graphics/CanvasPinball/default.html
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
			LockHolder lock(Thread::allThreadsLock());
			auto& allThreads = Thread::allThreads();
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
		WTF::RunLoop::iterate();
	}
	D(dprintf("..done waiting\n"));
}

void WebProcess::handleSignals(const uint32_t /* sigmask */)
{
	WTF::RunLoop::iterate();
}

float WebProcess::timeToNextTimerEvent()
{
	return WTF::RunLoop::secondsUntilNextIterate().value();
}

void WebProcess::dispatchAllEvents()
{
	WebCore::DOMWindow::dispatchAllPendingBeforeUnloadEvents();
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
	D(dprintf("%s at %d frames %d\n", __PRETTY_FUNCTION__, m_pageMap.size(), m_frameMap.size()));
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

WebPage* WebProcess::webPageForHost(const WTF::String &host)
{
	for (auto& webpage : m_pageMap.values())
	{
		auto url = webpage->topLevelFrame().url();
		if (equalIgnoringASCIICase(host, url.host().toString()))
		{
			return webpage.get();
		}
	}
	
	return nullptr;
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
	D(dprintf("%s %p %llu\n", __PRETTY_FUNCTION__, frame, frameID.toUInt64()));

	// fallbacks if WkSettings weren't applied yet
	if (!m_hasSetCacheModel)
		setCacheModel(CacheModel::PrimaryWebBrowser);

	if (ms_diskCacheSizeUninitialized == m_diskCacheSize)
		setDiskCacheSize(ms_diskCacheSizeUninitialized);

    m_frameMap.set(frameID, frame);
}

void WebProcess::removeWebFrame(WebCore::FrameIdentifier frameID)
{
	D(dprintf("%s %llu knowsFrames %ld\n", __PRETTY_FUNCTION__, frameID.toUInt64(), m_frameMap.size()));

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
//    auto& fontCache = FontCache::singleton();
//    ss << "CachedFontDataCount " << fontCache.fontCount(); ss.nextLine();;
//    ss << "CachedFontDataInactiveCount " << fontCache.inactiveFontCount(); ss.nextLine();;
	
    // Gather glyph page statistics.
//    ss << "GlyphPageCount " << GlyphPage::count(); ss.nextLine();;
	
    // Get WebCore memory cache statistics
    getWebCoreMemoryCacheStatistics(ss);
	
    dprintf(ss.release().utf8().data());
}

void reactOnMemoryPressureInWebKit()
{
	WebProcess::singleton().clearResourceCaches();
}

void WebProcess::garbageCollectJavaScriptObjects()
{
    GCController::singleton().garbageCollectNow();
}

Ref<WebCore::LocalWebLockRegistry> WebProcess::getOrCreateWebLockRegistry(bool isPrivateBrowsingEnabled)
{
    static NeverDestroyed<WeakPtr<WebCore::LocalWebLockRegistry>> defaultRegistry;
    static NeverDestroyed<WeakPtr<WebCore::LocalWebLockRegistry>> privateRegistry;
    auto& existingRegistry = isPrivateBrowsingEnabled ? privateRegistry : defaultRegistry;
    if (existingRegistry.get())
        return *existingRegistry.get();
    auto registry = WebCore::LocalWebLockRegistry::create();
    existingRegistry.get() = registry;
    return registry;
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

    m_hasSetCacheModel = true;
    D(dprintf("CACHES SETUP, total %d\n", cacheTotalCapacity));
}

QUAD WebProcess::maxDiskCacheSize() const
{
	return calculateMaxCacheSize("PROGDIR:Cache/Curl");
}

void WebProcess::setDiskCacheSize(QUAD sizeMax)
{
	bool wasUnset = ms_diskCacheSizeUninitialized == m_diskCacheSize;

	m_diskCacheSize = std::min(sizeMax, calculateMaxCacheSize("PROGDIR:Cache/Curl"));
	CurlCacheManager::singleton().setStorageSizeLimit(m_diskCacheSize);

	D(dprintf("%s %lld %d\n", __PRETTY_FUNCTION__, m_diskCacheSize, wasUnset));

	if (wasUnset && (m_diskCacheSize > 0) && (m_diskCacheSize < ms_diskCacheSizeUninitialized))
	{
    	CurlCacheManager::singleton().setCacheDirectory("PROGDIR:Cache/Curl"_s);
	}
}

void WebProcess::setCookieJarPath(const String& path)
{
    WebCore::setCookieJarPath(path);
}

void WebProcess::signalMainThread()
{
	Signal(m_sigTask, m_sigMask);
}

bool WebProcess::shouldAllowRequest(const char *url, const char *mainPageURL, WebCore::DocumentLoader& loader)
{
#if USE_ADFILTER
	WebFrame *frame = WebFrame::fromCoreFrame(*loader.frame());
    WebPage *page = frame ? frame->page() : nullptr;

	if (page && !page->adBlockingEnabled())
		return true;

	if (m_urlFilter.matches(url, ABP::FONoFilterOption, mainPageURL))
	{
		return false;
	}
#else
	(void)url;
	(void)mainPageURL;
	(void)loader;
#endif
	return true;
}

WebCore::NetworkStorageSession* WebProcess::storageSession(PAL::SessionID) const
{
    return &NetworkStorageSessionMap::defaultStorageSession();
}

#if ENABLE(SERVICE_WORKER)
WebCore::SWServer& WebProcess::ensureSWServer()
{
    if (!m_swServer) {
        auto info = valueOrDefault(m_serviceWorkerInfo);
        auto path = info.databasePath;
        // There should already be a registered path for this PAL::SessionID.
        // If there's not, then where did this PAL::SessionID come from?
        ASSERT(m_sessionID.isEphemeral() || !path.isEmpty());

        auto appBoundDomainsCallback = [] (auto&& completionHandler) {
            completionHandler({ });
        };

        m_swServer = makeUnique<SWServer>(makeUniqueRef<WebSWOriginStore>(), info.processTerminationDelayEnabled, WTFMove(path), sessionID(), false, true/* m_networkProcess->parentProcessHasServiceWorkerEntitlement() */, [this](auto&& jobData, bool shouldRefreshCache, auto&& request, auto&& completionHandler) mutable {
            ServiceWorkerSoftUpdateLoader::start(WTFMove(jobData), shouldRefreshCache, WTFMove(request), WTFMove(completionHandler));
        }, [this](auto& registrableDomain, std::optional<ScriptExecutionContextIdentifier> serviceWorkerPageIdentifier, auto&& completionHandler) {

            establishServiceWorkerContextConnectionToNetworkProcess({}, WebCore::RegistrableDomain(registrableDomain), serviceWorkerPageIdentifier, std::move(completionHandler));
        }, WTFMove(appBoundDomainsCallback));
    }
    return *m_swServer;
}

WebSWOriginStore* WebProcess::swOriginStore() const
{
    return m_swServer ? &static_cast<WebSWOriginStore&>(m_swServer->originStore()) : nullptr;
}

void WebProcess::registerSWServerConnection(WebSWServerConnection& connection)
{
    auto* store = swOriginStore();
    ASSERT(store);
    if (store)
        store->registerSWServerConnection(connection);
}

void WebProcess::unregisterSWServerConnection(WebSWServerConnection& connection)
{
    if (auto* store = swOriginStore())
        store->unregisterSWServerConnection(connection);
}

bool WebProcess::hasServiceWorkerDatabasePath() const
{
    return m_serviceWorkerInfo && !m_serviceWorkerInfo->databasePath.isEmpty();
}

WebSWServerConnection* WebProcess::swConnection()
{
    if (!m_swConnection)
    {
        auto& server = ensureSWServer();
        auto connection = makeUnique<WebSWServerConnection>(server);

        m_swConnection = *connection;
        server.addConnection(WTFMove(connection));
    }

    return m_swConnection.get();
}

void WebProcess::establishServiceWorkerContextConnectionToNetworkProcess(WebCore::PageIdentifier pageID,
    WebCore::RegistrableDomain&& registrableDomain, std::optional<WebCore::ScriptExecutionContextIdentifier> serviceWorkerPageIdentifier, CompletionHandler<void()>&&completionHandler)
{

    D(dprintf("%s: domain %s\n", __PRETTY_FUNCTION__, registrableDomain.string().utf8().data()));

    if (nullptr == SWContextManager::singleton().connection())
    {
        SWContextManager::singleton().setConnection(makeUnique<WebSWContextManagerConnection>(WTFMove(registrableDomain), serviceWorkerPageIdentifier, pageID));
        SWContextManager::singleton().connection()->establishConnection(WTFMove(completionHandler));
    }
    else
    {
        dprintf("-- already established!\n");
        callOnMainThread(WTFMove(completionHandler));
    }
}

void WebProcess::addServiceWorkerRegistration(WebCore::ServiceWorkerRegistrationIdentifier identifier)
{
    m_swRegistrationCounts.add(identifier);
}

bool WebProcess::removeServiceWorkerRegistration(WebCore::ServiceWorkerRegistrationIdentifier identifier)
{
    ASSERT(m_swRegistrationCounts.contains(identifier));
    return m_swRegistrationCounts.remove(identifier);
}

#endif

}

RefPtr<WebCore::SharedBuffer> loadResourceIntoBuffer(const char* name);
RefPtr<WebCore::SharedBuffer> loadResourceIntoBuffer(const char* name)
{
	WTF::String path = makeString("PROGDIR:Resources/", name, ".png");
	WTF::FileSystemImpl::PlatformFileHandle fh = WTF::FileSystemImpl::openFile(path, WTF::FileSystemImpl::FileOpenMode::Read);

	if (WTF::FileSystemImpl::invalidPlatformFileHandle != fh)
	{
		long long size = WTF::FileSystemImpl::fileSize(fh).value_or(0);
		if (size > 0ll && size < (512ll * 1024ll))
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
