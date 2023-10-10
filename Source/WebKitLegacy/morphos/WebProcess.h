#pragma once

#include <wtf/HashMap.h>
#include <WebCore/PageIdentifier.h>
#include <WebCore/FrameIdentifier.h>
#include <pal/SessionID.h>
#include "CacheModel.h"
#include <WebCore/NetworkingContext.h>
#include <WebCore/SWServer.h>
#include "ServiceWorkerSoftUpdateLoader.h"
#include "NetworkSession.h"
#include <wtf/HashCountedSet.h>

#if !MORPHOS_MINIMAL
#include "ABPFilterParser/ABPFilterParser.h"
#endif

namespace WebCore {
	class DocumentLoader;
	class CacheStorageProvider;
    class SWServer;
    class LocalWebLockRegistry;
};

namespace WebKit {

class WebFrame;
class WebPage;
class WebSWOriginStore;
class WebSWServerConnection;
class WebSWServerToContextConnection;
class ServiceWorkerFetchTask;
class CacheStorageEngineConnection;

class WebPageCreationParameters
{
public:
	WebPageCreationParameters() {};
	~WebPageCreationParameters() {};
};

class WebProcess
{
public:
    static WebProcess& singleton();

	void initialize(int sigbit);
	void terminate();
	void waitForThreads();

    WebPage* webPage(WebCore::PageIdentifier) const;
    void createWebPage(WebCore::PageIdentifier, WebPageCreationParameters&&);
    void removeWebPage(WebCore::PageIdentifier);
    WebPage* focusedWebPage() const;

    WebFrame* webFrame(WebCore::FrameIdentifier) const;
    void addWebFrame(WebCore::FrameIdentifier, WebFrame*);
    void removeWebFrame(WebCore::FrameIdentifier);
    size_t webFrameCount() const { return m_frameMap.size(); }

	WebPage* webPageForHost(const WTF::String &host);

	PAL::SessionID sessionID() const { ASSERT(m_sessionID); return *m_sessionID; }
	RefPtr<WebCore::NetworkingContext> networkingContext() { return m_dummyNetworkingContext; }

	WebCore::CacheStorageProvider& cacheStorageProvider() { return m_cacheStorageProvider.get(); }
    CacheStorageEngineConnection& cacheStorageEngineConnection() { return *m_cacheStorageEngineConnection.get(); }
    NetworkSession& networkSession() { return *m_networkSession.get(); }

    // those two are to simplify NetworkCache stuff
    NetworkSession* networkSession(PAL::SessionID) { return &networkSession(); }
    WebCore::NetworkStorageSession* storageSession(PAL::SessionID) const;

	void setCacheModel(WebKit::CacheModel cacheModel);
	WebKit::CacheModel cacheModel() const { return m_cacheModel; }
	
	void setDiskCacheSize(QUAD sizeMax);
	QUAD diskCacheSize() const { return m_diskCacheSize; }
	QUAD maxDiskCacheSize() const;
 
    void setCookieJarPath(const String& path);

	void dumpWebCoreStatistics();
	
	void garbageCollectJavaScriptObjects();

	void clearResourceCaches();
	
	void handleSignals(const uint32_t sigmask);
	float timeToNextTimerEvent();
	void signalMainThread();
	
	bool shouldAllowRequest(const char *url, const char *mainPageURL, WebCore::DocumentLoader& loader);
	
	void setLastPageClosedCallback(std::function<void()>&&func) { m_fLastPageClosed = func; }

	void returnedFromConstrainedRunLoop();
	void dispatchAllEvents();
 
    void setEasyListPath(const char *path);

    Ref<WebCore::LocalWebLockRegistry> getOrCreateWebLockRegistry(bool isPrivateBrowsingEnabled);

#if ENABLE(SERVICE_WORKER)
    void addSoftUpdateLoader(std::unique_ptr<ServiceWorkerSoftUpdateLoader>&& loader) { m_softUpdateLoaders.add(WTFMove(loader)); }
    void removeSoftUpdateLoader(ServiceWorkerSoftUpdateLoader* loader) { m_softUpdateLoaders.remove(loader); }
//    void addNavigationPreloaderTask(ServiceWorkerFetchTask&);
//    ServiceWorkerFetchTask* navigationPreloaderTaskFromFetchIdentifier(WebCore::FetchIdentifier);
//    void removeNavigationPreloaderTask(ServiceWorkerFetchTask&);

    WebCore::SWServer* swServer() { return m_swServer.get(); }
    WebCore::SWServer& ensureSWServer();
    WebSWOriginStore* swOriginStore() const; // FIXME: Can be private?
    void registerSWServerConnection(WebSWServerConnection&);
    void unregisterSWServerConnection(WebSWServerConnection&);

    bool hasServiceWorkerDatabasePath() const;

    void establishServiceWorkerContextConnectionToNetworkProcess(WebCore::PageIdentifier, WebCore::RegistrableDomain&&, std::optional<WebCore::ScriptExecutionContextIdentifier> serviceWorkerPageIdentifier, CompletionHandler<void()>&&);
    void addServiceWorkerRegistration(WebCore::ServiceWorkerRegistrationIdentifier identifier);
    bool removeServiceWorkerRegistration(WebCore::ServiceWorkerRegistrationIdentifier identifier);

//    void addServiceWorkerSession(bool processTerminationDelayEnabled, String&& serviceWorkerRegistrationDirectory, const SandboxExtension::Handle&);

    /// -------------------------------------
    WebSWServerConnection* swConnection();
//    std::unique_ptr<ServiceWorkerFetchTask> createFetchTask(NetworkResourceLoader&, const WebCore::ResourceRequest&);
#endif

protected:
    HashMap<WebCore::FrameIdentifier, WebFrame*> m_frameMap;
    HashMap<WebCore::PageIdentifier, RefPtr<WebPage>> m_pageMap;

	WebProcess();
	~WebProcess();

    bool m_hasSetCacheModel { false };
    CacheModel m_cacheModel { CacheModel::DocumentViewer };
    static const QUAD ms_diskCacheSizeUninitialized = 0x7FFFFFFFFFFFFFFFll;
    QUAD m_diskCacheSize { ms_diskCacheSizeUninitialized };
#if (!MORPHOS_MINIMAL)
    ABP::ABPFilterParser m_urlFilter;
    std::vector<char>    m_urlFilterData;
#endif
    std::optional<PAL::SessionID> m_sessionID;
    Ref<WebCore::CacheStorageProvider> m_cacheStorageProvider;
    RefPtr<WebCore::NetworkingContext> m_dummyNetworkingContext;
    RefPtr<CacheStorageEngineConnection> m_cacheStorageEngineConnection;

    std::function<void()> m_fLastPageClosed;
    std::unique_ptr<NetworkSession> m_networkSession;
	
	struct Task *m_sigTask;
    uint32_t m_sigMask;
    
    WTF::String m_easyListPath;

#if ENABLE(SERVICE_WORKER)
    HashSet<std::unique_ptr<ServiceWorkerSoftUpdateLoader>> m_softUpdateLoaders;
//    HashMap<WebCore::FetchIdentifier, WeakPtr<ServiceWorkerFetchTask>> m_navigationPreloaders;

    struct ServiceWorkerInfo {
        String databasePath;
        bool processTerminationDelayEnabled { true };
    };
    std::optional<ServiceWorkerInfo> m_serviceWorkerInfo;
    std::unique_ptr<WebCore::SWServer> m_swServer;
    
    //------------------------------
    WeakPtr<WebSWServerConnection> m_swConnection;
//    std::unique_ptr<WebSWServerToContextConnection> m_swContextConnection;
    HashCountedSet<WebCore::ServiceWorkerRegistrationIdentifier> m_swRegistrationCounts;
#endif
};

}
