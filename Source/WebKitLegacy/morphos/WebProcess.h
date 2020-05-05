#pragma once

#include <wtf/HashMap.h>
#include <WebCore/PageIdentifier.h>
#include <pal/SessionID.h>
#include "CacheModel.h"
#include "ABPFilterParser/ABPFilterParser.h"

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
    void removeWebPage(PAL::SessionID, WebCore::PageIdentifier);
    WebPage* focusedWebPage() const;

    WebFrame* webFrame(uint64_t) const;
    void addWebFrame(uint64_t, WebFrame*);
    void removeWebFrame(uint64_t);
    size_t webFrameCount() const { return m_frameMap.size(); }

	void setCacheModel(WebKit::CacheModel cacheModel);

	void dumpWebCoreStatistics();
	
	void garbageCollectJavaScriptObjects();

	void clearResourceCaches();
	
	void handleSignals(const uint32_t sigmask);
	void signalMainThread();
	
	bool shouldAllowRequest(const char *url, const char *mainPageURL);
	
	void setLastPageClosedCallback(std::function<void()>&&func) { m_fLastPageClosed = func; }

protected:
    HashMap<uint64_t, WebFrame*> m_frameMap;
    HashMap<WebCore::PageIdentifier, RefPtr<WebPage>> m_pageMap;

	~WebProcess();

    bool m_hasSetCacheModel { false };
    CacheModel m_cacheModel { CacheModel::DocumentViewer };
    ABP::ABPFilterParser m_urlFilter;
    std::vector<char>    m_urlFilterData;
	
    std::function<void()> m_fLastPageClosed;
	
	struct Task *m_sigTask;
    uint32_t m_sigMask;
};

}

