#include "WebKit.h"

#include <WebCore/ApplicationCacheStorage.h>
#include <WebCore/BackForwardController.h>
#include <WebCore/CSSAnimationController.h>
#include <WebCore/CacheStorageProvider.h>
#include <WebCore/Chrome.h>
#include <WebCore/CookieJar.h>
#include <WebCore/DatabaseManager.h>
#include <WebCore/DeprecatedGlobalSettings.h>
#include <WebCore/Document.h>
#include <WebCore/DocumentLoader.h>
#include <WebCore/DragController.h>
#include <WebCore/DragData.h>
#include <WebCore/DragItem.h>
#include <WebCore/Editing.h>
#include <WebCore/Editor.h>
#include <WebCore/Event.h>
#include <WebCore/EventHandler.h>
#include <WebCore/FocusController.h>
#include <WebCore/FontAttributes.h>
#include <WebCore/FontCache.h>
#include <WebCore/Frame.h>
#include <WebCore/FrameLoader.h>
#include <WebCore/FrameSelection.h>
#include <WebCore/FrameTree.h>
#include <WebCore/FrameView.h>
#include <WebCore/FullscreenManager.h>
#include <WebCore/GCController.h>
#include <WebCore/GeolocationController.h>
#include <WebCore/GeolocationError.h>
#include <WebCore/HTMLNames.h>
#include <WebCore/HTMLOListElement.h>
#include <WebCore/HTMLUListElement.h>
#include <WebCore/HTMLVideoElement.h>
#include <WebCore/HistoryController.h>
#include <WebCore/HistoryItem.h>
#include <WebCore/JSCSSStyleDeclaration.h>
#include <WebCore/JSDocument.h>
#include <WebCore/JSElement.h>
#include <WebCore/JSNodeList.h>
#include <WebCore/JSNotification.h>
#include <WebCore/LibWebRTCProvider.h>
#include <WebCore/LocalizedStrings.h>
#include <WebCore/LogInitialization.h>
#include <WebCore/MIMETypeRegistry.h>
#include <WebCore/MemoryCache.h>
#include <WebCore/MemoryRelease.h>
#include <WebCore/NetworkStorageSession.h>
#include <WebCore/NodeList.h>
#include <WebCore/Notification.h>
#include <WebCore/NotificationController.h>
#include <WebCore/Page.h>
#include <WebCore/PageCache.h>
#include <WebCore/PageConfiguration.h>
#include <WebCore/PageGroup.h>
#include <WebCore/PathUtilities.h>
#include <WebCore/ProgressTracker.h>
#include <WebCore/RenderTheme.h>
#include <WebCore/RenderView.h>
#include <WebCore/RenderWidget.h>
#include <WebCore/ResourceHandle.h>
#include <WebCore/ResourceLoadObserver.h>
#include <WebCore/ResourceRequest.h>
#include <WebCore/RuntimeApplicationChecks.h>
#include <WebCore/RuntimeEnabledFeatures.h>
#include <WebCore/SchemeRegistry.h>
#include <WebCore/ScriptController.h>
#include <WebCore/SecurityOrigin.h>
#include <WebCore/SecurityPolicy.h>
#include <WebCore/Settings.h>
#include <WebCore/ShouldTreatAsContinuingLoad.h>
#include <WebCore/SocketProvider.h>
#include <WebCore/StyleProperties.h>
#include <WebCore/TextResourceDecoder.h>
#include <WebCore/ThreadCheck.h>
#include <WebCore/UserAgent.h>
#include <WebCore/UserContentController.h>
#include <WebCore/UserGestureIndicator.h>
#include <WebCore/UserScript.h>
#include <WebCore/UserStyleSheet.h>
#include <WebCore/ValidationBubble.h>
#include <WebCore/Widget.h>
#include <WebCore/StorageNamespaceProvider.h>
#include <WebCore/FrameLoadRequest.h>

#include <JavaScriptCore/APICast.h>
#include <JavaScriptCore/ArrayPrototype.h>
#include <JavaScriptCore/CatchScope.h>
#include <JavaScriptCore/DateInstance.h>
#include <JavaScriptCore/Exception.h>
#include <JavaScriptCore/InitializeThreading.h>
#include <JavaScriptCore/JSCJSValue.h>
#include <JavaScriptCore/JSLock.h>
#include <JavaScriptCore/JSValueRef.h>

#include <wtf/URLHelpers.h>
#include <wtf/RunLoop.h>

#include "WebView.h"
#include "WebFrame.h"
#include "../../WebCoreSupport/PageStorageSessionProvider.h"
#include "WebCoreSupport/WebEditorClient.h"
#include "WebCoreSupport/WebChromeClient.h"
#include "WebCoreSupport/WebPluginInfoProvider.h"
#include "../../WebCoreSupport/WebViewGroup.h"
#include "BackForwardClient.h"
#include <WebCoreSupport/WebVisitedLinkStore.h>
#include "WebCoreSupport/WebPlatformStrategies.h"
#include "WebCoreSupport/WebInspectorClient.h"
#include "WebCoreSupport/WebFrameLoaderClient.h"
#include "WebApplicationCache.h"
#include "../../Storage/WebDatabaseProvider.h"

#include <cairo.h>

#include <utility>
using namespace std;

#include <proto/graphics.h>
#include <proto/cybergraphics.h>
#include <cybergraphx/cybergraphics.h>

extern "C" {
	void dprintf(const char *, ...);
};

WebCore::Page* core(WebView *webView)
{
	if (webView)
    	return webView->page();
	return nullptr;
}

WebCore::Frame& mainframe(WebCore::Page& page)
{
	return page.mainFrame();
}

const WebCore::Frame& mainframe(const WebCore::Page& page)
{
	return page.mainFrame();
}

WebView *kit(WebCore::Page* page)
{
    if (!page)
        return nullptr;

	WebEditorClient &editorClient = static_cast<WebEditorClient &>(page->editorClient());
    return editorClient.webView();
}

WebView::WebView()
{
dprintf("%s:%d\n", __PRETTY_FUNCTION__, __LINE__);
    JSC::initializeThreading();
    RunLoop::initializeMainRunLoop();
    WebCore::NetworkStorageSession::permitProcessToUseCookieAPI(true);

dprintf("%s:%d\n", __PRETTY_FUNCTION__, __LINE__);

    static bool didOneTimeInitialization;
    if (!didOneTimeInitialization) {
#if !LOG_DISABLED || !RELEASE_LOG_DISABLED
        initializeLogChannelsIfNecessary();
#endif // !LOG_DISABLED || !RELEASE_LOG_DISABLED

        // Initialize our platform strategies first before invoking the rest
        // of the initialization code which may depend on the strategies.
        WebPlatformStrategies::initialize();

        auto& memoryPressureHandler = MemoryPressureHandler::singleton();
        memoryPressureHandler.setLowMemoryHandler([] (Critical critical, Synchronous synchronous) {
            WebCore::releaseMemory(critical, synchronous);
        });
        memoryPressureHandler.install();

        didOneTimeInitialization = true;
     }

	m_webViewGroup = WebViewGroup::getOrCreate("test", "T:");

	auto storageProvider = PageStorageSessionProvider::create();
dprintf("%s:%d\n", __PRETTY_FUNCTION__, __LINE__);
	WebCore::PageConfiguration pageConfiguration(
        makeUniqueRef<WebEditorClient>(this),
        WebCore::SocketProvider::create(),
        WebCore::LibWebRTCProvider::create(),
        WebCore::CacheStorageProvider::create(),
        BackForwardClientMorphOS::create(this),
        WebCore::CookieJar::create(storageProvider.copyRef())
        );

	pageConfiguration.chromeClient = new WebChromeClient(this);
	pageConfiguration.inspectorClient = new WebInspectorClient(this);
    pageConfiguration.loaderClientForMainFrame = new WebFrameLoaderClient;
    pageConfiguration.progressTrackerClient = static_cast<WebFrameLoaderClient*>(pageConfiguration.loaderClientForMainFrame);
    pageConfiguration.storageNamespaceProvider = &m_webViewGroup->storageNamespaceProvider();
    pageConfiguration.userContentProvider = &m_webViewGroup->userContentController();
    pageConfiguration.visitedLinkStore = &m_webViewGroup->visitedLinkStore();
    pageConfiguration.pluginInfoProvider = &WebPluginInfoProvider::singleton();
    pageConfiguration.applicationCacheStorage = &WebApplicationCache::storage();
    pageConfiguration.databaseProvider = &WebDatabaseProvider::singleton();
    pageConfiguration.userContentProvider = &m_webViewGroup->userContentController();;
	
dprintf("%s:%d chromeclient %p\n", __PRETTY_FUNCTION__, __LINE__, pageConfiguration.chromeClient);

	m_page = new WebCore::Page(WTFMove(pageConfiguration));
dprintf("%s:%d Created!\n", __PRETTY_FUNCTION__, __LINE__);

    WebFrame* webFrame = WebFrame::createInstance(&m_page->mainFrame(), this);
    static_cast<WebFrameLoaderClient&>(m_page->mainFrame().loader().client()).setWebFrame(webFrame);
    m_mainFrame = webFrame;

    //m_page->mainFrame().tree().setName(toString(frameName));
    m_page->mainFrame().init();
}

WebView::~WebView()
{
	delete m_page;
}

WebCore::Page *WebView::page()
{
	return m_page;
}

void WebView::go(const char *url)
{
	WTF::URL baseCoreURL = WTF::URL(WTF::URL(), WTF::String(url));
	WebCore::ResourceRequest req(baseCoreURL);

    auto* coreFrame = core(m_mainFrame);
	
//	dprintf("go to %s, %s...\n", wurl.protocol().utf8().data(), wurl.host().utf8().data());
    coreFrame->loader().load(WebCore::FrameLoadRequest(*coreFrame, req, WebCore::ShouldOpenExternalURLsPolicy::ShouldNotAllow));
}

void WebView::repaint(const WebCore::IntRect& rect, bool contentChanged, bool immediate, bool repaintContentOnly)
{
	if (_fInvalidateRect)
		_fInvalidateRect(rect.x(), rect.y(), rect.width(), rect.height());
}

void WebView::closeWindow()
{

}

void WebView::closeWindowSoon()
{

}

void WebView::closeWindowTimerFired()
{

}

void WebView::drawToRP(struct RastPort *rp, const int x, const int y, const int width, const int height)
{
	auto* coreFrame = core(m_mainFrame);
	if (!coreFrame)
		return;

    WebCore::FrameView* frameView = coreFrame->view();
	frameView->updateLayoutAndStyleIfNeededRecursive();

dprintf("draw to %p at %d %d : %dx%d\n", rp, x,y, width, height);

	auto *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
	auto *cr = cairo_create(surface);

	{
		WebCore::PlatformContextCairo pcc(cr);
		WebCore::GraphicsContext gc(&pcc);
		
		WebCore::IntRect ir(0, 0, width, height);
		WebCore::FloatRect fr(0, 0, width, height);

		gc.save();
		gc.clip(fr);
		
		frameView->paintContents(gc, ir);

		gc.restore();
	}

	unsigned int stride;
	unsigned char *src;
	stride = cairo_image_surface_get_stride(surface);
    src = cairo_image_surface_get_data(surface);

	WritePixelArray(src, 0, 0, stride, rp, x, y, width, height, RECTFMT_ARGB);

	cairo_destroy(cr);
	cairo_surface_destroy(surface);
}

void WebView::handleRunLoop()
{
	WTF::RunLoop::iterate();
}
