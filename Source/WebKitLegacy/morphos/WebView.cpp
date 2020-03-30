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
#include <WebCore/HitTestResult.h>
#include <WebCore/PlatformEvent.h>
#include <WebCore/PlatformKeyboardEvent.h>
#include <WebCore/DeprecatedGlobalSettings.h>

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
#include "WebCoreSupport/WebContextMenuClient.h"
#include "WebApplicationCache.h"
#include "../../Storage/WebDatabaseProvider.h"

#include <cairo.h>

#include <utility>
#include <cstdio>
using namespace std;

#include <proto/graphics.h>
#include <proto/cybergraphics.h>
#include <cybergraphx/cybergraphics.h>
#include <intuition/intuition.h>
#include <intuition/intuimessageclass.h>
#include <intuition/classusr.h>
#include <clib/alib_protos.h>

extern "C" {
	void dprintf(const char *, ...);
};

class WebViewDrawContext
{
	int m_width;
	int m_height;
	int m_scrollX = 0;
	int m_scrollY = 0;
	
	WebCore::IntRect m_damage;
	bool m_hasDamage = false;
	bool m_needsRepaint = true;
	
	cairo_surface_t *m_surface = nullptr ;
	cairo_t *m_cairo = nullptr ;
	WebCore::PlatformContextCairo *m_platformContext = nullptr;

public:
	WebViewDrawContext(const int width, const int height)
		: m_width(width)
		, m_height(height)
		, m_surface(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height))
		, m_cairo(cairo_create(m_surface))
		, m_platformContext(new WebCore::PlatformContextCairo(m_cairo))
	{
	
	}
	
	~WebViewDrawContext()
	{
		delete m_platformContext;
		cairo_destroy(m_cairo);
		cairo_surface_destroy(m_surface);
	}
	
	void invalidate(const WebCore::IntRect& rect)
	{
		if (m_hasDamage)
		{
			m_damage.unite(rect);
		}
		else
		{
			m_damage = rect;
			m_hasDamage = true;
		}
	}
	
	void setScroll(const int sX, const int sY)
	{
		m_scrollX = sX;
		m_scrollY = sY;
		
		m_needsRepaint = true;
	}
	
	const int scrollX() const { return m_scrollX; }
	const int scrollY() const { return m_scrollY; }
	const int width() const { return m_width; }
	const int height() const { return m_height; }

	void draw(WebCore::FrameView *frameView, RastPort *rp, const int x, const int y, const int width, const int height, bool update)
	{
		if (!m_platformContext)
			return;
		
		if (m_needsRepaint)
		{
			WebCore::GraphicsContext gc(m_platformContext);
			
			WebCore::IntRect ir(m_scrollX, m_scrollY, m_width, m_height);
			WebCore::FloatRect fr(0, 0, m_width, m_height);

			gc.save();
			gc.clip(fr);
			gc.translate(-m_scrollX, -m_scrollY);
			
			frameView->paintContents(gc, ir);

			gc.restore();
			cairo_surface_flush(m_surface);
		}
		// TODO: add damage bitmap
		else if (m_hasDamage)
		{
			WebCore::GraphicsContext gc(m_platformContext);
			
			WebCore::IntRect ir(m_scrollX + m_damage.x(), m_scrollY + m_damage.y(), m_damage.width(), m_damage.height());
			WebCore::FloatRect fr(m_damage.x(), m_damage.y(), m_damage.width(), m_damage.height());

			gc.save();
			gc.clip(fr);
			gc.translate(-m_scrollX, -m_scrollY);
			
			frameView->paintContents(gc, ir);

			gc.restore();
			cairo_surface_flush(m_surface);
		}

		const unsigned int stride = cairo_image_surface_get_stride(m_surface);
		unsigned char *src = cairo_image_surface_get_data(m_surface);

		if (update && !m_needsRepaint)
		{
			int dx = x + m_damage.x();
			int dy = y + m_damage.y();
			int dmaxx = dx + m_damage.width();
			int dmaxy = dy + m_damage.height();

			dmaxx = std::min(dmaxx, x + width);
			dmaxy = std::min(dmaxy, y + height);

			dx = std::min(dx, dmaxx);
			dy = std::min(dy, dmaxy);

			if (dx < dmaxx && dy < dmaxy)
				WritePixelArray(src, m_damage.x(), m_damage.y(), stride, rp, dx, dy, dmaxx - dx, dmaxy - dy, RECTFMT_ARGB);
		}
		else
		{
			WritePixelArray(src, 0, 0, stride, rp, x, y, width, height, RECTFMT_ARGB);
		}
		
		m_hasDamage = false;
		m_needsRepaint = false;
	}

	bool resize(const int width, const int height)
	{
		if (width != m_width || height != m_height)
		{
			delete m_platformContext;
			cairo_destroy(m_cairo);
			cairo_surface_destroy(m_surface);

			m_surface = nullptr;
			m_cairo = nullptr;
			m_platformContext = nullptr;
			
			m_width = width;
			m_height = height;
			
			if ((m_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height)))
			{
				if ((m_cairo = cairo_create(m_surface)))
				{
					if (!(m_platformContext = new WebCore::PlatformContextCairo(m_cairo)))
					{
						cairo_destroy(m_cairo);
						cairo_surface_destroy(m_surface);
						m_surface = nullptr;
						m_cairo = nullptr;
					}
					
					m_needsRepaint = true;
					return true;
				}
				else
				{
					cairo_surface_destroy(m_surface);
					m_surface = nullptr;
				}
			}
		}
		
		return false;
	}
	
private:

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

	WebCore::DeprecatedGlobalSettings::setUsesOverlayScrollbars(true);

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
	pageConfiguration.contextMenuClient = new WebContextMenuClient(this);

// setUserStyleSheetLocation !

dprintf("%s:%d chromeclient %p\n", __PRETTY_FUNCTION__, __LINE__, pageConfiguration.chromeClient);

	m_page = new WebCore::Page(WTFMove(pageConfiguration));
dprintf("%s:%d Created!\n", __PRETTY_FUNCTION__, __LINE__);

    WebFrame* webFrame = WebFrame::createInstance(&m_page->mainFrame(), this);
    static_cast<WebFrameLoaderClient&>(m_page->mainFrame().loader().client()).setWebFrame(webFrame);
    m_mainFrame = webFrame;

    //m_page->mainFrame().tree().setName(toString(frameName));
    m_page->mainFrame().init();

	WebCore::Settings& settings = m_page->settings();
	settings.setJavaEnabled(true);
	settings.setJavaEnabledForLocalFiles(true);
    settings.setAllowDisplayOfInsecureContent(true);
    settings.setAllowRunningOfInsecureContent(true);
    settings.setLoadsImagesAutomatically(true);
}

WebView::~WebView()
{
	delete m_drawContext;
	delete m_page;
}

void WebView::shutdown()
{
	dprintf("%s\n", __PRETTY_FUNCTION__);
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
    coreFrame->loader().load(WebCore::FrameLoadRequest(*coreFrame, req, WebCore::ShouldOpenExternalURLsPolicy::ShouldAllow));
}

void WebView::repaint(const WebCore::IntRect& rect)
{
	if (m_drawContext)
		m_drawContext->invalidate(rect);

	if (_fInvalidate)
		_fInvalidate();
}

void WebView::internalScroll(int scrollX, int scrollY)
{
	if (m_drawContext)
	{
		const int sx = std::max(0, m_drawContext->scrollX() - scrollX);
		const int sy = std::max(0, m_drawContext->scrollY() - scrollY);

dprintf("%s: by %d %d to %d %d\n", __PRETTY_FUNCTION__, scrollX, scrollY, sx, sy);

		if (m_drawContext)
			m_drawContext->setScroll(sx, sy);

		if (_fScroll)
			_fScroll(sx, sy);
	}
}

void WebView::documentSizeChanged(int width, int height)
{
	if (_setDocumentSize)
		_setDocumentSize(width, height);
}

void WebView::closeWindow()
{

}

void WebView::closeWindowSoon()
{
	delete m_drawContext;
	m_drawContext = nullptr;
}

void WebView::closeWindowTimerFired()
{

}

void WebView::setVisibleSize(const int width, const int height)
{
	bool resized = false;
	if (nullptr == m_drawContext)
	{
		m_drawContext = new WebViewDrawContext(width, height);
		resized = true;
	}
	else
	{
		resized = m_drawContext->resize(width, height);
	}

	if (resized)
	{
		auto* coreFrame = core(m_mainFrame);
		if (coreFrame)
		{
			WebCore::FloatSize logicalSize(width, height);
			auto clientRect = enclosingIntRect(WebCore::FloatRect(WebCore::FloatPoint(), logicalSize));
			coreFrame->view()->resize(clientRect.size());
		}
	}
}

void WebView::setScroll(const int x, const int y)
{
	if (m_drawContext)
		m_drawContext->setScroll(x, y);
}

void WebView::draw(struct RastPort *rp, const int x, const int y, const int width, const int height, bool updateMode)
{
	auto* coreFrame = core(m_mainFrame);
	if (!coreFrame || !m_drawContext)
		return;

    WebCore::FrameView* frameView = coreFrame->view();
	frameView->updateLayoutAndStyleIfNeededRecursive();

//dprintf("draw to %p at %d %d : %dx%d\n", rp, x,y, width, height);

	m_drawContext->draw(frameView, rp, x, y, width, height, updateMode);
}

static inline WebCore::MouseButton imsgToButton(IntuiMessage *imsg)
{
	if (IDCMP_MOUSEBUTTONS == imsg->Class)
	{
		switch (imsg->Code)
		{
		case SELECTUP:
		case SELECTDOWN: return WebCore::MouseButton::LeftButton;
		case MENUUP:
		case MENUDOWN: return WebCore::MouseButton::RightButton;
		case MIDDLEUP:
		case MIDDLEDOWN: return WebCore::MouseButton::MiddleButton;
		}
	}
	
	return WebCore::MouseButton::NoButton;
}

static inline WebCore::PlatformEvent::Type imsgToEventType(IntuiMessage *imsg)
{
	switch (imsg->Class)
	{
	case IDCMP_MOUSEBUTTONS:
		switch (imsg->Code)
		{
		case SELECTDOWN:
		case MENUDOWN:
		case MIDDLEDOWN:
			return WebCore::PlatformEvent::Type::MousePressed;
		default:
			return WebCore::PlatformEvent::Type::MouseReleased;
		}
	}
	
	return WebCore::PlatformEvent::Type::MouseMoved;
}

bool WebView::handleIntuiMessage(IntuiMessage *imsg, const int mouseX, const int mouseY, bool mouseInside)
{
	switch (imsg->Class)
	{
	case IDCMP_MOUSEMOVE:
	case IDCMP_MOUSEBUTTONS:
	case IDCMP_MOUSEHOVER:
		{
			if (imsg->Class == IDCMP_MOUSEMOVE)
				m_clickCount = 0;
			else
				m_clickCount = 1;

			WebCore::PlatformMouseEvent pme(
				WebCore::IntPoint(mouseX, mouseY),
				WebCore::IntPoint(imsg->IDCMPWindow->LeftEdge + imsg->MouseX, imsg->IDCMPWindow->TopEdge + imsg->MouseY),
				imsgToButton(imsg),
				imsgToEventType(imsg),
				m_clickCount,
				(imsg->Qualifier & (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT)) != 0,
				(imsg->Qualifier & IEQUALIFIER_CONTROL) != 0,
				(imsg->Qualifier & (IEQUALIFIER_LALT|IEQUALIFIER_RALT)) != 0,
				(imsg->Qualifier & (IEQUALIFIER_LCOMMAND|IEQUALIFIER_RCOMMAND)) != 0,
				WTF::WallTime::fromRawSeconds(imsg->Seconds),
				0.0,
				WebCore::SyntheticClickType::NoTap);
		
			WebCore::FocusController& focusController = m_page->focusController();
		
		bool rc;
		
			switch (imsg->Class)
			{
			case IDCMP_MOUSEBUTTONS:
				switch (imsg->Code)
				{
				case SELECTDOWN:
				case MENUDOWN:
				case MIDDLEDOWN:
					focusController.setFocused(mouseInside);
					rc = m_page->mainFrame().eventHandler().handleMousePressEvent(pme);
					printf("press at %d %d -> %d\n", mouseX, mouseY, rc);
					break;
				default:
					rc = m_page->mainFrame().eventHandler().handleMouseReleaseEvent(pme);
					printf("release at %d %d -> %d\n", mouseX, mouseY, rc);
					break;
				}
				break;
			case IDCMP_MOUSEMOVE:
			case IDCMP_MOUSEHOVER:
				{
//				WebCore::HitTestResult ht;
				rc = m_page->mainFrame().eventHandler().handleMouseMoveEvent(pme);
//				auto p = ht.pointInMainFrame();
//				printf("%p %p %d %d\n", ht.innerNode(), ht.URLElement(), int(p.x()), int(p.y()));
//				}
//				printf("move at %d %d -> %d\n", mouseX, mouseY, rc);
				break;
				}
			}
		}
		break;
		
	case IDCMP_RAWKEY:
		{
			Boopsiobject *oimsg = (Boopsiobject *)imsg;
			ULONG key = 0;
			DoMethod(oimsg, OM_GET, IMSGA_UCS4, &key);
			if (key >= 32)
			{
				UChar ch[2] = { key, 0 };
				WTF::String s(ch, 2);
				dprintf("type char %d\n", key);
				auto ke = WebCore::PlatformKeyboardEvent((imsg->Code & IECODE_UP_PREFIX) ? WebCore::PlatformEvent::KeyUp : WebCore::PlatformEvent::KeyDown,s,s,s,s,s,
					 0, false, false, false, WTF::OptionSet<WebCore::PlatformEvent::Modifier>(), WTF::WallTime::fromRawSeconds(imsg->Seconds));
				m_page->mainFrame().eventHandler().keyEvent(ke);
			}
		}
		break;
	}

	return false;
}

void WebView::handleRunLoop()
{
	WTF::RunLoop::iterate();
}
