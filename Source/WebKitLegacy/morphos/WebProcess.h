#pragma once

#include <wtf/HashMap.h>
#include <WebCore/PageIdentifier.h>
#include <WebCore/FrameIdentifier.h>
#include <pal/SessionID.h>
#include "CacheModel.h"
#include <WebCore/NetworkingContext.h>
#include "ABPFilterParser/ABPFilterParser.h"

namespace WebCore {
	class DocumentLoader;
	class CacheStorageProvider;
};

namespace WebKit {

class WebFrame;
class WebPage;

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

	PAL::SessionID sessionID() const { ASSERT(m_sessionID); return *m_sessionID; }
	RefPtr<WebCore::NetworkingContext> networkingContext() { return m_dummyNetworkingContext; }

	WebCore::CacheStorageProvider& cacheStorageProvider() { return m_cacheStorageProvider.get(); }

	void setCacheModel(WebKit::CacheModel cacheModel);

	void dumpWebCoreStatistics();
	
	void garbageCollectJavaScriptObjects();

	void clearResourceCaches();
	
	void handleSignals(const uint32_t sigmask);
	void signalMainThread();
	
	bool shouldAllowRequest(const char *url, const char *mainPageURL, WebCore::DocumentLoader& loader);
	
	void setLastPageClosedCallback(std::function<void()>&&func) { m_fLastPageClosed = func; }

	void returnedFromConstrainedRunLoop();

protected:
    HashMap<WebCore::FrameIdentifier, WebFrame*> m_frameMap;
    HashMap<WebCore::PageIdentifier, RefPtr<WebPage>> m_pageMap;

	WebProcess();
	~WebProcess();

    bool m_hasSetCacheModel { false };
    CacheModel m_cacheModel { CacheModel::DocumentViewer };
    ABP::ABPFilterParser m_urlFilter;
    std::vector<char>    m_urlFilterData;
    Optional<PAL::SessionID> m_sessionID;
    Ref<WebCore::CacheStorageProvider> m_cacheStorageProvider;
    RefPtr<WebCore::NetworkingContext> m_dummyNetworkingContext;

    std::function<void()> m_fLastPageClosed;
	
	struct Task *m_sigTask;
    uint32_t m_sigMask;
};

}
