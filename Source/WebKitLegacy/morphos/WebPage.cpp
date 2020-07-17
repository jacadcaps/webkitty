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
//#include <WebCore/PageCache.h>
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
//#include <WebCore/SchemeRegistry.h>
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
#include <WebCore/FrameLoaderTypes.h>
#include <WebCore/UserInputBridge.h>
#include <WebCore/KeyboardEvent.h>
#include <WebCore/EventNames.h>
#include <WebCore/WindowsKeyboardCodes.h>
#include <WebCore/RenderLayerCompositor.h>
#include <WebCore/ContextMenuController.h>
#include <WebCore/MediaRecorderProvider.h>
#include <WebCore/ScriptState.h>
#include <wtf/ASCIICType.h>
#include <wtf/HexNumber.h>

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

#include "WebPage.h"
#include "WebFrame.h"
#include "../../WebCoreSupport/PageStorageSessionProvider.h"
#include "WebCoreSupport/WebEditorClient.h"
#include "WebCoreSupport/WebChromeClient.h"
#include "WebCoreSupport/WebPluginInfoProvider.h"
#include "WebCoreSupport/WebPageGroup.h"
#include "BackForwardClient.h"
#include <WebCoreSupport/WebVisitedLinkStore.h>
#include "WebCoreSupport/WebPlatformStrategies.h"
#include "WebCoreSupport/WebInspectorClient.h"
#include "WebCoreSupport/WebFrameLoaderClient.h"
#include "WebCoreSupport/WebContextMenuClient.h"
#include "WebCoreSupport/WebProgressTrackerClient.h"
#include "WebApplicationCache.h"
#include "../../Storage/WebDatabaseProvider.h"
#include "WebDocumentLoader.h"
#include "WebDragClient.h"
#include "WebProcess.h"

#include <cairo.h>

#include <utility>
#include <cstdio>

#include <proto/graphics.h>
#include <proto/exec.h>
#include <proto/cybergraphics.h>
#include <cybergraphx/cybergraphics.h>
#include <intuition/intuition.h>
#include <intuition/intuimessageclass.h>
#include <intuition/classusr.h>
#include <clib/alib_protos.h>
#include <devices/rawkeycodes.h>

// we cannot include libraries/mui.h here...
enum
{
	MUIKEY_RELEASE = -2, /* not a real key, faked when MUIKEY_PRESS is released */
	MUIKEY_NONE    = -1,
	MUIKEY_PRESS,
	MUIKEY_TOGGLE,
	MUIKEY_UP,
	MUIKEY_DOWN,
	MUIKEY_PAGEUP,
	MUIKEY_PAGEDOWN,
	MUIKEY_TOP,
	MUIKEY_BOTTOM,
	MUIKEY_LEFT,
	MUIKEY_RIGHT,
	MUIKEY_WORDLEFT,
	MUIKEY_WORDRIGHT,
	MUIKEY_LINESTART,
	MUIKEY_LINEEND,
	MUIKEY_GADGET_NEXT,
	MUIKEY_GADGET_PREV,
	MUIKEY_GADGET_OFF,
	MUIKEY_WINDOW_CLOSE,
	MUIKEY_WINDOW_NEXT,
	MUIKEY_WINDOW_PREV,
	MUIKEY_HELP,
	MUIKEY_POPUP,
	MUIKEY_CUT,
	MUIKEY_COPY,
	MUIKEY_PASTE,
	MUIKEY_UNDO,
	MUIKEY_REDO,
	MUIKEY_DELETE,
	MUIKEY_BACKSPACE,
	MUIKEY_ICONIFY,
};

extern "C" {
	void dprintf(const char *, ...);
};

#define D(x) 

using namespace std;
using namespace WebCore;

namespace WebKit {

class MediaRecorderProvider final : public WebCore::MediaRecorderProvider {
public:
    MediaRecorderProvider() = default;
};

class WebViewDrawContext
{
	int m_width;
	int m_height;
	
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
	
	void invalidate()
	{
		m_needsRepaint = true;
	}
	
	const int width() const { return m_width; }
	const int height() const { return m_height; }

	void drawLayer(WebCore::GraphicsLayer *layer, WebCore::GraphicsContext& gc, WebCore::FloatRect &fr)
	{
		layer->paintGraphicsLayerContents(gc, fr);
		
		const Vector<Ref<GraphicsLayer>>& children = layer->children();
		for (auto it = children.begin(); it != children.end(); it++)
		{
			WebCore::GraphicsLayer *child = it->ptr();
			dprintf("child %p pos %f %f size %f %f bounds %f %f backing %p\n", child, child->position().x(), child->position().y(),
				child->size().width(), child->size().height(), child->boundsOrigin().x(), child->boundsOrigin().y(), child->tiledBacking());
			WebCore::FloatRect xfr(fr.x() + child->position().x(), fr.y() + child->position().y(), child->size().width(), child->size().height());
			drawLayer(child, gc, xfr);
		}

	}

	void draw(WebCore::FrameView *frameView, RastPort *rp, const int x, const int y, const int width, const int height,
		int scrollX, int scrollY, bool update)
	{
		if (!m_platformContext)
			return;
		
		if (m_needsRepaint)
		{
			WebCore::GraphicsContext gc(m_platformContext);
			
			WebCore::IntRect ir(scrollX, scrollY, m_width, m_height);
			WebCore::FloatRect fr(0, 0, m_width, m_height);

			gc.save();
			gc.clip(fr);
			gc.translate(-scrollX, -scrollY);
			
			frameView->paintContents(gc, ir);

//			frameView->paintContentsForSnapshot(gc, ir, WebCore::FrameView::SelectionInSnapshot::ExcludeSelection,
//				WebCore::FrameView::CoordinateSpaceForSnapshot::ViewCoordinates);

#if 0
			if (frameView->renderView() && frameView->renderView()->compositor().rootGraphicsLayer())
			{
				auto *rlayer = frameView->renderView()->compositor().rootGraphicsLayer();
				const Vector<Ref<GraphicsLayer>>& children = rlayer->children();
				for (auto it = children.begin(); it != children.end(); it++)
				{
					WebCore::GraphicsLayer *layer = it->ptr();
					drawLayer(layer, gc, fr);
				}
			}

#if 0
				const FloatPoint position = layer->position();
					layer->paintGraphicsLayerContents(gc, fr);
					const Vector<Ref<GraphicsLayer>>& inchildren = rlayer->children();
					for (auto it = inchildren.begin(); it != inchildren.end(); it++)
					{
						WebCore::GraphicsLayer *xlayer = it->ptr();
						const FloatPoint position = xlayer->position();
						xlayer->paintGraphicsLayerContents(gc, fr);
						dprintf("subsublayer at %f %f\n", position.x(), position.y());
					}
				}
			}
#endif
#endif

			gc.restore();
			cairo_surface_flush(m_surface);
		}
		// TODO: add damage bitmap
		else if (m_hasDamage)
		{
			WebCore::GraphicsContext gc(m_platformContext);
			
			m_damage.intersect({ 0, 0, m_width, m_height });
			
			WebCore::IntRect ir(scrollX + m_damage.x(), scrollY + m_damage.y(), m_damage.width(), m_damage.height());
			WebCore::FloatRect fr(m_damage.x(), m_damage.y(), m_damage.width(), m_damage.height());

			gc.save();
			gc.clip(fr);
			gc.translate(-scrollX, -scrollY);
			
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

WebCore::Page* core(WebPage *webpage)
{
	if (webpage)
    	return webpage->corePage();
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

WebPage *kit(WebCore::Page* page)
{
	return WebPage::fromCorePage(page);
}

Ref<WebPage> WebPage::create(WebCore::PageIdentifier pageID, WebPageCreationParameters&& parameters)
{
    Ref<WebPage> page = adoptRef(*new WebPage(pageID, WTFMove(parameters)));
    return page;
}

WebPage::WebPage(WebCore::PageIdentifier pageID, WebPageCreationParameters&& parameters)
	: m_pageID(pageID)
{
    JSC::initializeThreading();
    RunLoop::initializeMainRunLoop();
    WebCore::NetworkStorageSession::permitProcessToUseCookieAPI(true);

//	WebCore::DeprecatedGlobalSettings::setUsesOverlayScrollbars(true);
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

	m_webPageGroup = WebPageGroup::getOrCreate("test", "PROGDIR:Cache/Storage");
	auto storageProvider = PageStorageSessionProvider::create();

	WebCore::PageConfiguration pageConfiguration(
		WebProcess::singleton().sessionID(),
        makeUniqueRef<WebEditorClient>(this),
        WebCore::SocketProvider::create(),
        makeUniqueRef<WebCore::LibWebRTCProvider>(),
        WebProcess::singleton().cacheStorageProvider(),
        BackForwardClientMorphOS::create(this),
        WebCore::CookieJar::create(storageProvider.copyRef()),
        makeUniqueRef<WebProgressTrackerClient>(*this),
        makeUniqueRef<MediaRecorderProvider>()
        );

	pageConfiguration.chromeClient = new WebChromeClient(*this);
	pageConfiguration.inspectorClient = new WebInspectorClient(this);
    pageConfiguration.loaderClientForMainFrame = new WebFrameLoaderClient;
    pageConfiguration.storageNamespaceProvider = &m_webPageGroup->storageNamespaceProvider();
    pageConfiguration.userContentProvider = &m_webPageGroup->userContentController();
    pageConfiguration.visitedLinkStore = &m_webPageGroup->visitedLinkStore();
    pageConfiguration.pluginInfoProvider = &WebPluginInfoProvider::singleton();
    pageConfiguration.applicationCacheStorage = &WebApplicationCache::storage();
    pageConfiguration.databaseProvider = &WebDatabaseProvider::singleton();
	pageConfiguration.contextMenuClient = new WebContextMenuClient(this);
	pageConfiguration.dragClient = makeUnique<WebDragClient>(this);

//dprintf("%s:%d chromeclient %p\n", __PRETTY_FUNCTION__, __LINE__, pageConfiguration.chromeClient);

	m_page = std::make_unique<WebCore::Page>(WTFMove(pageConfiguration));
	storageProvider->setPage(*m_page);

	WebCore::Settings& settings = m_page->settings();
    settings.setAllowDisplayOfInsecureContent(true);
    settings.setAllowRunningOfInsecureContent(false);
    settings.setLoadsImagesAutomatically(true);
    settings.setScriptEnabled(true);
    settings.setScriptMarkupEnabled(true);
    settings.setDeferredCSSParserEnabled(true);
    settings.setDeviceWidth(1920);
    settings.setDeviceHeight(1080);
    settings.setDiagnosticLoggingEnabled(true);
    settings.setEditableImagesEnabled(true);
    settings.setEnforceCSSMIMETypeInNoQuirksMode(true);
    settings.setShrinksStandaloneImagesToFit(true);
    settings.setSubpixelAntialiasedLayerTextEnabled(true);

#if 1
	settings.setForceCompositingMode(false);
	settings.setAcceleratedCompositingEnabled(false);
	settings.setAcceleratedDrawingEnabled(false);
	settings.setAccelerated2dCanvasEnabled(false);
	settings.setAcceleratedCompositedAnimationsEnabled(false);
	settings.setAcceleratedCompositingForFixedPositionEnabled(false);
	settings.setAcceleratedFiltersEnabled(false);
    settings.setFrameFlattening(FrameFlattening::FullyEnabled);
#else
    settings.setFrameFlattening(FrameFlattening::FullyEnabled);
#endif

//	settings.setTreatsAnyTextCSSLinkAsStylesheet(true);
//	settings.setUsePreHTML5ParserQuirks(true);

	settings.setWebGLEnabled(false);

	settings.setWebAudioEnabled(true);
	settings.setMediaEnabled(true);

	settings.setLocalStorageDatabasePath(String("PROGDIR:Cache/LocalStorage"));
	settings.setLocalStorageEnabled(true);
	
//	settings.setLogsPageMessagesToSystemConsoleEnabled(true);
	
	settings.setRequestAnimationFrameEnabled(true);
//	settings.setUserStyleSheetLocation(WTF::URL(WTF::URL(), WTF::String("file:///PROGDIR:resource/userStyleSheet.css")));

    m_mainFrame = WebFrame::createWithCoreMainFrame(this, &m_page->mainFrame());
    static_cast<WebFrameLoaderClient&>(m_page->mainFrame().loader().client()).setWebFrame(m_mainFrame.get());

//    m_page->mainFrame().tree().setName(toString("frameName"));
//    m_page->mainFrame().init();

    m_page->layoutIfNeeded();

    m_page->setIsVisible(true);
    m_page->setIsInWindow(true);
	m_page->setActivityState(ActivityState::WindowIsActive);

//    m_page->addLayoutMilestones({ DidFirstLayout, DidFirstVisuallyNonEmptyLayout });
}

WebPage::~WebPage()
{
	D(dprintf("%s\n", __PRETTY_FUNCTION__));
	delete m_drawContext;
}

WebCore::Page *WebPage::corePage()
{
	return m_page.get();
}

WebPage* WebPage::fromCorePage(WebCore::Page* page)
{
    return &static_cast<WebChromeClient&>(page->chrome().client()).page();
}

void WebPage::load(const char *url)
{
	WTF::URL baseCoreURL = WTF::URL(WTF::URL(), WTF::String(url));
	WebCore::ResourceRequest req(baseCoreURL);

	if (!m_mainFrame)
		return;

    auto* coreFrame = m_mainFrame->coreFrame();

	static uint64_t navid = 1;
	
	corePage()->userInputBridge().stopLoadingFrame(coreFrame);

	m_pendingNavigationID = navid ++;

    coreFrame->loader().urlSelected(baseCoreURL, { }, nullptr, LockHistory::No, LockBackForwardList::No, MaybeSendReferrer, ShouldOpenExternalURLsPolicy::ShouldNotAllow);
}

void WebPage::loadData(const char *data, size_t length, const char *url)
{
	WTF::URL baseURL = url ? WTF::URL(WTF::URL(), WTF::String(url)) : WTF::blankURL();

    ResourceRequest request(baseURL);
    ResourceResponse response(WTF::blankURL(), "text/html", length, "UTF-8");
    SubstituteData substituteData(WebCore::SharedBuffer::create(data, length), WTF::blankURL(), response, SubstituteData::SessionHistoryVisibility::Hidden);

	auto* coreFrame = m_mainFrame->coreFrame();
    coreFrame->loader().load(FrameLoadRequest(*coreFrame, request, ShouldOpenExternalURLsPolicy::ShouldNotAllow, substituteData));
}

void WebPage::reload()
{
	auto *mainframe = mainFrame();
	if (mainframe)
		mainframe->loader().reload();
}

void WebPage::stop()
{
	auto *mainframe = mainFrame();
	if (mainframe)
		mainframe->loader().stopForUserCancel();
}

WebCore::CertificateInfo WebPage::getCertificate(void)
{
	auto* coreFrame = m_mainFrame->coreFrame();
	if (!coreFrame)
		return { };

    DocumentLoader* documentLoader = coreFrame->loader().documentLoader();
    if (!documentLoader)
        return { };

    return valueOrCompute(documentLoader->response().certificateInfo(), [] { return CertificateInfo(); });
}

void WebPage::run(const char *js)
{
	if (!m_mainFrame)
		return;

	WTF::RefPtr<WebPage> protect(this);

	auto* coreFrame = m_mainFrame->coreFrame();
	if (!coreFrame)
		return;

	coreFrame->script().executeScriptIgnoringException(js, true);
}

void *WebPage::evaluate(const char *js, WTF::Function<void *(const char *)>&& cb)
{
	if (!m_mainFrame)
		return nullptr;
	WTF::RefPtr<WebPage> protect(this);

	auto* coreFrame = m_mainFrame->coreFrame();
	if (!coreFrame)
		return nullptr;

	auto result = coreFrame->script().executeScriptIgnoringException(js, true);
	if (!m_mainFrame || !m_mainFrame->coreFrame() || !result || (!result.isBoolean() && !result.isString() && !result.isNumber()))
	{
		return cb("");
	}

    auto state = mainWorldExecState(coreFrame);
    JSC::JSLockHolder lock(state);
	WTF::String string = result.toWTFString(state);
	auto ustring = string.utf8();
	return cb(ustring.data());
}

void *WebPage::getInnerHTML(WTF::Function<void *(const char *)>&& cb)
{
    WebCore::Frame* coreFrame = m_mainFrame->coreFrame();
	if (coreFrame)
	{
		WTF::String inner = coreFrame->document()->documentElement()->innerHTML();
		auto uinner = inner.utf8();
		return cb(uinner.data());
	}
	
	return nullptr;
}

void WebPage::setInnerHTML(const char *html)
{
    WebCore::Frame* coreFrame = m_mainFrame->coreFrame();
	if (coreFrame)
	{
		coreFrame->document()->documentElement()->setInnerHTML(WTF::String::fromUTF8(html));
	}
}

bool WebPage::goBack()
{
	return m_page->backForward().goBack();
}

bool WebPage::goForward()
{
	return m_page->backForward().goForward();
}

bool WebPage::canGoBack()
{
	return m_page->backForward().canGoBackOrForward(-1);
}

bool WebPage::canGoForward()
{
	return m_page->backForward().canGoBackOrForward(1);
}

WTF::RefPtr<WebKit::BackForwardClientMorphOS> WebPage::backForwardClient()
{
	Ref<BackForwardClientMorphOS> client(static_cast<BackForwardClientMorphOS&>(m_page->backForward().client()));
	return client;
}

void WebPage::willBeDisposed()
{
	D(dprintf("%s\n", __PRETTY_FUNCTION__));
	m_orphaned = true;
	auto *mainframe = mainFrame();
	clearDelegateCallbacks();
	stop();
	if (mainframe)
		mainframe->loader().detachFromParent();
	D(dprintf("%s done mf %p\n", __PRETTY_FUNCTION__, mainframe));
}

Frame* WebPage::mainFrame() const
{
    return m_page ? &m_page->mainFrame() : nullptr;
}

FrameView* WebPage::mainFrameView() const
{
    if (Frame* frame = mainFrame())
        return frame->view();
	
    return nullptr;
}

PAL::SessionID WebPage::sessionID() const
{
	return m_page->sessionID();
}

bool WebPage::javaScriptEnabled() const
{
	return m_page->settings().isScriptEnabled();
}

void WebPage::setJavaScriptEnabled(bool enabled)
{
	return m_page->settings().setScriptEnabled(enabled);
}

bool WebPage::adBlockingEnabled() const
{
	return m_adBlocking;
}

void WebPage::setAdBlockingEnabled(bool enabled)
{
	m_adBlocking = enabled;
}

bool WebPage::thirdPartyCookiesAllowed() const
{
	return m_page->settings().isThirdPartyCookieBlockingDisabled();
}

void WebPage::setThirdPartyCookiesAllowed(bool blocked)
{
	m_page->settings().setIsThirdPartyCookieBlockingDisabled(blocked);
}

void WebPage::goActive()
{
	corePage()->userInputBridge().focusSetActive(true);
	corePage()->userInputBridge().focusSetFocused(true);
	m_justWentActive = true;
	m_isActive = true;
}

void WebPage::goInactive()
{
	m_justWentActive = false;
	m_isActive = false;
	corePage()->userInputBridge().focusSetFocused(false);
}

void WebPage::goVisible()
{
	corePage()->userInputBridge().focusSetActive(true);
}

void WebPage::goHidden()
{
	corePage()->userInputBridge().focusSetActive(false);
}

void WebPage::startLiveResize()
{
	auto* coreFrame = m_mainFrame->coreFrame();
	coreFrame->view()->willStartLiveResize();
}

void WebPage::endLiveResize()
{
	auto* coreFrame = m_mainFrame->coreFrame();
	coreFrame->view()->willEndLiveResize();
}

void WebPage::setFocusedElement(WebCore::Element *element)
{
	// this is called by the Chrome
	m_focusedElement = element;
}

void WebPage::addResourceRequest(unsigned long identifier, const WebCore::ResourceRequest& request)
{
    if (!request.url().protocolIsInHTTPFamily())
        return;

    if (m_mainFrameProgressCompleted || m_orphaned)
        return;

    ASSERT(!m_trackedNetworkResourceRequestIdentifiers.contains(identifier));
    bool wasEmpty = m_trackedNetworkResourceRequestIdentifiers.isEmpty();
    m_trackedNetworkResourceRequestIdentifiers.add(identifier);

    if (wasEmpty && _fDidStartLoading)
    	_fDidStartLoading();
}

void WebPage::removeResourceRequest(unsigned long identifier)
{
    if (!m_trackedNetworkResourceRequestIdentifiers.remove(identifier))
        return;

	if (m_trackedNetworkResourceRequestIdentifiers.isEmpty() && _fDidStopLoading)
		_fDidStopLoading();
}

void WebPage::didStartPageTransition()
{
}

void WebPage::didCompletePageTransition()
{
}

void WebPage::didCommitLoad(WebFrame* frame)
{
//    resetFocusedElementForFrame(frame);

    if (!frame->isMainFrame())
        return;

#if 0
    // If previous URL is invalid, then it's not a real page that's being navigated away from.
    // Most likely, this is actually the first load to be committed in this page.
    if (frame->coreFrame()->loader().previousURL().isValid())
        reportUsedFeatures();
#endif

    // Only restore the scale factor for standard frame loads (of the main frame).
    if (frame->coreFrame()->loader().loadType() == WebCore::FrameLoadType::Standard) {
        Page* page = frame->coreFrame()->page();

        if (page && page->pageScaleFactor() != 1)
            scalePage(1, IntPoint());
    }

#if 0
#if ENABLE(VIEWPORT_RESIZING)
    m_shrinkToFitContentTimer.stop();
#endif

#if ENABLE(TEXT_AUTOSIZING)
    m_textAutoSizingAdjustmentTimer.stop();
#endif

    WebProcess::singleton().updateActivePages();

    updateMainFrameScrollOffsetPinning();

    updateMockAccessibilityElementAfterCommittingLoad();
#endif
}

void WebPage::didFinishDocumentLoad(WebFrame& frame)
{
    if (!frame.isMainFrame())
        return;
	
	corePage()->resumeActiveDOMObjectsAndAnimations();

#if ENABLE(VIEWPORT_RESIZING) && 0
    scheduleShrinkToFitContent();
#endif
}

void WebPage::didFinishLoad(WebFrame& frame)
{
    if (!frame.isMainFrame())
        return;

#if ENABLE(VIEWPORT_RESIZING) && 0
    scheduleShrinkToFitContent();
#endif
}

void WebPage::didFailLoad(const WebCore::ResourceError& error)
{
	if (_fDidFailWithError)
		_fDidFailWithError(error);
}

Ref<DocumentLoader> WebPage::createDocumentLoader(Frame& frame, const ResourceRequest& request, const SubstituteData& substituteData)
{
    Ref<WebDocumentLoader> documentLoader = WebDocumentLoader::create(request, substituteData);

    if (frame.isMainFrame()) {
        if (m_pendingNavigationID) {
            documentLoader->setNavigationID(m_pendingNavigationID);
            m_pendingNavigationID = 0;
        }

#if 0
        if (m_pendingWebsitePolicies) {
            WebsitePoliciesData::applyToDocumentLoader(WTFMove(*m_pendingWebsitePolicies), documentLoader);
            m_pendingWebsitePolicies = WTF::nullopt;
        }
#endif
    }

    return documentLoader;
}

void WebPage::updateCachedDocumentLoader(WebDocumentLoader& documentLoader, Frame& frame)
{
    if (m_pendingNavigationID && frame.isMainFrame()) {
        documentLoader.setNavigationID(m_pendingNavigationID);
        m_pendingNavigationID = 0;
    }
}

WebCore::IntSize WebPage::size() const
{
	if (m_drawContext)
		return WebCore::IntSize(m_drawContext->width(), m_drawContext->height());
	return WebCore::IntSize();
}

double WebPage::totalScaleFactor() const
{
    return m_page->pageScaleFactor();
}

double WebPage::pageScaleFactor() const
{
    return totalScaleFactor() / viewScaleFactor();
}

double WebPage::viewScaleFactor() const
{
    return m_page->viewScaleFactor();
}

void WebPage::scalePage(double scale, const IntPoint& origin)
{
    double totalScale = scale * viewScaleFactor();
    bool willChangeScaleFactor = totalScale != totalScaleFactor();

    m_page->setPageScaleFactor(totalScale, origin);

    // We can't early return before setPageScaleFactor because the origin might be different.
    if (!willChangeScaleFactor)
        return;

	if (m_drawContext)
		m_drawContext->invalidate();

	if (_fInvalidate)
		_fInvalidate();
}

void WebPage::setAlwaysShowsHorizontalScroller(bool alwaysShowsHorizontalScroller)
{
    if (alwaysShowsHorizontalScroller == m_alwaysShowsHorizontalScroller)
        return;

    m_alwaysShowsHorizontalScroller = alwaysShowsHorizontalScroller;
    auto view = corePage()->mainFrame().view();
    if (!alwaysShowsHorizontalScroller)
        view->setHorizontalScrollbarLock(false);
    view->setHorizontalScrollbarMode(alwaysShowsHorizontalScroller ? ScrollbarAlwaysOn : m_mainFrameIsScrollable ? ScrollbarAuto : ScrollbarAlwaysOff, alwaysShowsHorizontalScroller || !m_mainFrameIsScrollable);
}

void WebPage::setAlwaysShowsVerticalScroller(bool alwaysShowsVerticalScroller)
{
    if (alwaysShowsVerticalScroller == m_alwaysShowsVerticalScroller)
        return;

    m_alwaysShowsVerticalScroller = alwaysShowsVerticalScroller;
    auto view = corePage()->mainFrame().view();
    if (!alwaysShowsVerticalScroller)
        view->setVerticalScrollbarLock(false);
    view->setVerticalScrollbarMode(alwaysShowsVerticalScroller ? ScrollbarAlwaysOn : m_mainFrameIsScrollable ? ScrollbarAuto : ScrollbarAlwaysOff, alwaysShowsVerticalScroller || !m_mainFrameIsScrollable);
}

void WebPage::repaint(const WebCore::IntRect& rect)
{
	if (!m_drawContext)
		return;

	WebCore::IntRect realRect(0, 0, m_drawContext->width(), m_drawContext->height());
	realRect.intersect(rect);

//	if (rect.x() < 0)
//	DumpTaskState(FindTask(0));

#if 0
	dprintf("%s %ld:%ld x %ld:%ld\n", __PRETTY_FUNCTION__, rect.x(), rect.y(), rect.width(), rect.height());
	dprintf("%s %ld:%ld x %ld:%ld\n", __PRETTY_FUNCTION__, realRect.x(), realRect.y(),
		realRect.width(), realRect.height());
#endif

	if (m_drawContext)
		m_drawContext->invalidate(realRect);

	if (_fInvalidate)
		_fInvalidate();
}

void WebPage::internalScroll(int scrollX, int scrollY)
{
	if (!m_ignoreScroll)
	{
		if (!m_mainFrame)
			return;
		auto* coreFrame = m_mainFrame->coreFrame();
		if (!coreFrame)
			return;
		WebCore::FrameView *view = coreFrame->view();
		WebCore::ScrollPosition sp = view->scrollPosition();
		if (_fScroll)
			_fScroll(sp.x(), sp.y());
		if (_fInvalidate)
			_fInvalidate();
	}
}

void WebPage::frameSizeChanged(WebCore::Frame& frame, int width, int height)
{
	if (!m_mainFrame)
		return;
	auto* coreFrame = m_mainFrame->coreFrame();
	if (!coreFrame)
		return;
	WebCore::FrameView *view = coreFrame->view();

	if (coreFrame == &frame)
	{
		WebCore::ScrollPosition sp = view->scrollPosition();
		if (_fSetDocumentSize)
			_fSetDocumentSize(width, height);
		if (_fScroll)
			_fScroll(sp.x(), sp.y());
	}
}

void WebPage::closeWindow()
{

}

void WebPage::closeWindowSoon()
{
	delete m_drawContext;
	m_drawContext = nullptr;
}

void WebPage::closeWindowTimerFired()
{

}

void WebPage::setVisibleSize(const int width, const int height)
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

  		auto* coreFrame = m_mainFrame->coreFrame();
		coreFrame->view()->resize(width, height);

//		corePage()->mainFrame().view()->enableAutoSizeMode(true, { width, height });
#if 0
		WebCore::FrameView* view = m_page->mainFrame().view();
		if (view)
		{
			view->resize(width, height);
		}

  		auto* coreFrame = m_mainFrame->coreFrame();

		if (coreFrame && coreFrame->view())
		{
			WebCore::FloatSize logicalSize(width, height);
			auto clientRect = enclosingIntRect(WebCore::FloatRect(WebCore::FloatPoint(), logicalSize));
			coreFrame->view()->resize(clientRect.size());
		}
#endif
	}
}

void WebPage::setScroll(const int x, const int y)
{
	if (!m_mainFrame)
		return;
	auto* coreFrame = m_mainFrame->coreFrame();
	if (!coreFrame)
		return;
	WebCore::FrameView *view = coreFrame->view();

	m_ignoreScroll = true;
	view->setScrollPosition(WebCore::ScrollPosition(x, y));
	m_ignoreScroll = false;

	if (_fInvalidate)
		_fInvalidate();
}

void WebPage::scrollBy(const int xDelta, const int yDelta)
{
	if (!m_mainFrame)
		return;
	auto* coreFrame = m_mainFrame->coreFrame();
	if (!coreFrame)
		return;
	WebCore::FrameView *view = coreFrame->view();
	WebCore::ScrollPosition sp = view->scrollPosition();
	WebCore::ScrollPosition spMin = view->minimumScrollPosition();
	WebCore::ScrollPosition spMax = view->maximumScrollPosition();

	int x = sp.x() - xDelta;
	int y = sp.y() - yDelta;

	x = std::max(x, spMin.x());
	x = std::min(x, spMax.x());
	
	y = std::max(y, spMin.y());
	y = std::min(y, spMax.y());

	view->setScrollPosition(WebCore::ScrollPosition(x, y));

	if (_fInvalidate)
		_fInvalidate();
}

void WebPage::draw(struct RastPort *rp, const int x, const int y, const int width, const int height, bool updateMode)
{
	auto* coreFrame = m_mainFrame->coreFrame();
	if (!coreFrame || !m_drawContext)
		return;
    WebCore::FrameView* frameView = coreFrame->view();
    if (!frameView)
    	return;

    m_page->updateRendering();

	frameView->updateLayoutAndStyleIfNeededRecursive();
//	frameView->updateCompositingLayersAfterLayout();
	frameView->setPaintBehavior(PaintBehavior::FlattenCompositingLayers);
	IntSize s = frameView->autoSizingIntrinsicContentSize();
	auto scroll = frameView->scrollPosition();

//	dprintf("draw to %p at %d %d : %dx%d, %d %d renderable %d\n", rp, x,y, width, height, s.width(), s.height(), frameView->isSoftwareRenderable());

	FrameTree& tree = coreFrame->tree();

#if 0
    for (Frame* child = tree.firstRenderedChild(); child; child = child->tree().traverseNextRendered(coreFrame)) {
        if (!child->view())
            continue;
		dprintf("maybe render child frame %p\n", child);
    }
#endif

#if 0
	if (frameView->renderView())
	{
    	dprintf("rv compositior 3d %d rgl %p\n", frameView->renderView()->compositor().has3DContent(), frameView->renderView()->compositor().rootGraphicsLayer());
    	if (frameView->renderView()->compositor().rootGraphicsLayer())
    	{
    		auto *rlayer = frameView->renderView()->compositor().rootGraphicsLayer();
    		const Vector<Ref<GraphicsLayer>>& children = rlayer->children();
    		for (auto it = children.begin(); it != children.end(); it++)
    		{
    			const FloatPoint position = it->ptr()->position();
				dprintf("child layer %p at %f %f\n", it->ptr(), position.x(), position.y());
			}
		}
	}
#endif
	m_drawContext->draw(frameView, rp, x, y, width, height, scroll.x(), scroll.y(), updateMode);
}

static inline WebCore::MouseButton imsgToButton(IntuiMessage *imsg)
{
	if (IDCMP_MOUSEBUTTONS == imsg->Class || IDCMP_MOUSEMOVE == imsg->Class)
	{
		switch (imsg->Code)
		{
		case SELECTUP:
		case SELECTDOWN: return WebCore::MouseButton::LeftButton;
		case MENUUP:
		case MENUDOWN: return WebCore::MouseButton::RightButton;
		case MIDDLEUP:
		case MIDDLEDOWN: return WebCore::MouseButton::MiddleButton;
		default:
			if (imsg->Qualifier & IEQUALIFIER_LEFTBUTTON)
				return WebCore::MouseButton::LeftButton;
			if (imsg->Qualifier & IEQUALIFIER_RBUTTON)
				return WebCore::MouseButton::RightButton;
			if (imsg->Qualifier & IEQUALIFIER_MIDBUTTON)
				return WebCore::MouseButton::MiddleButton;
			break;
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

static const unsigned CommandKey = 1 << 0;
static const unsigned AltKey = 1 << 1;
static const unsigned ShiftKey = 1 << 2;


struct KeyDownEntry {
    unsigned virtualKey;
    unsigned modifiers;
    const char* name;
};

struct KeyPressEntry {
    unsigned charCode;
    unsigned modifiers;
    const char* name;
};

static const KeyDownEntry keyDownEntries[] = {
    { VK_LEFT,   0,                  "MoveLeft"                                    },
    { VK_LEFT,   ShiftKey,           "MoveLeftAndModifySelection"                  },
    { VK_LEFT,   CommandKey,            "MoveWordLeft"                                },
    { VK_LEFT,   CommandKey | ShiftKey, "MoveWordLeftAndModifySelection"              },
    { VK_RIGHT,  0,                  "MoveRight"                                   },
    { VK_RIGHT,  ShiftKey,           "MoveRightAndModifySelection"                 },
    { VK_RIGHT,  CommandKey,            "MoveWordRight"                               },
    { VK_RIGHT,  CommandKey | ShiftKey, "MoveWordRightAndModifySelection"             },
    { VK_UP,     0,                  "MoveUp"                                      },
    { VK_UP,     ShiftKey,           "MoveUpAndModifySelection"                    },
    { VK_PRIOR,  ShiftKey,           "MovePageUpAndModifySelection"                },
    { VK_DOWN,   0,                  "MoveDown"                                    },
    { VK_DOWN,   ShiftKey,           "MoveDownAndModifySelection"                  },
    { VK_NEXT,   ShiftKey,           "MovePageDownAndModifySelection"              },
    { VK_PRIOR,  0,                  "MovePageUp"                                  },
    { VK_NEXT,   0,                  "MovePageDown"                                },
    { VK_HOME,   0,                  "MoveToBeginningOfLine"                       },
    { VK_HOME,   ShiftKey,           "MoveToBeginningOfLineAndModifySelection"     },
    { VK_HOME,   CommandKey,            "MoveToBeginningOfDocument"                   },
    { VK_HOME,   CommandKey | ShiftKey, "MoveToBeginningOfDocumentAndModifySelection" },

    { VK_END,    0,                  "MoveToEndOfLine"                             },
    { VK_END,    ShiftKey,           "MoveToEndOfLineAndModifySelection"           },
    { VK_END,    CommandKey,            "MoveToEndOfDocument"                         },
    { VK_END,    CommandKey | ShiftKey, "MoveToEndOfDocumentAndModifySelection"       },

    { VK_BACK,   0,                  "DeleteBackward"                              },
    { VK_BACK,   ShiftKey,           "DeleteBackward"                              },
    { VK_DELETE, 0,                  "DeleteForward"                               },
    { VK_BACK,   CommandKey,            "DeleteWordBackward"                          },
    { VK_DELETE, CommandKey,            "DeleteWordForward"                           },
	
    { 'B',       CommandKey,            "ToggleBold"                                  },
    { 'I',       CommandKey,            "ToggleItalic"                                },

    { VK_ESCAPE, 0,                  "Cancel"                                      },
    { VK_OEM_PERIOD, CommandKey,        "Cancel"                                      },
    { VK_TAB,    0,                  "InsertTab"                                   },
    { VK_TAB,    ShiftKey,           "InsertBacktab"                               },
    { VK_RETURN, 0,                  "InsertNewline"                               },
    { VK_RETURN, CommandKey,            "InsertNewline"                               },
    { VK_RETURN, AltKey,             "InsertNewline"                               },
    { VK_RETURN, ShiftKey,           "InsertNewline"                               },
    { VK_RETURN, AltKey | ShiftKey,  "InsertNewline"                               },

    // It's not quite clear whether clipboard shortcuts and Undo/Redo should be handled
    // in the application or in WebKit. We chose WebKit.
    { 'C',       CommandKey,            "Copy"                                        },
    { 'V',       CommandKey,            "Paste"                                       },
    { 'X',       CommandKey,            "Cut"                                         },
    { 'A',       CommandKey,            "SelectAll"                                   },
    { VK_INSERT, CommandKey,            "Copy"                                        },
    { VK_DELETE, ShiftKey,           "Cut"                                         },
    { VK_INSERT, ShiftKey,           "Paste"                                       },
    { 'Z',       CommandKey,            "Undo"                                        },
    { 'Z',       CommandKey | ShiftKey, "Redo"                                        },
};

static const KeyPressEntry keyPressEntries[] = {
    { '\t',   0,                  "InsertTab"                                   },
    { '\t',   ShiftKey,           "InsertBacktab"                               },
    { '\r',   0,                  "InsertNewline"                               },
    { '\r',   CommandKey,            "InsertNewline"                               },
    { '\r',   AltKey,             "InsertNewline"                               },
    { '\r',   ShiftKey,           "InsertNewline"                               },
    { '\r',   AltKey | ShiftKey,  "InsertNewline"                               },
};

static const char* interpretKeyEvent(const KeyboardEvent* evt)
{
    ASSERT(evt->type() == eventNames().keydownEvent || evt->type() == eventNames().keypressEvent);

    static HashMap<int, const char*>* keyDownCommandsMap = 0;
    static HashMap<int, const char*>* keyPressCommandsMap = 0;

    if (!keyDownCommandsMap) {
        keyDownCommandsMap = new HashMap<int, const char*>;
        keyPressCommandsMap = new HashMap<int, const char*>;

        for (size_t i = 0; i < WTF_ARRAY_LENGTH(keyDownEntries); ++i)
            keyDownCommandsMap->set(keyDownEntries[i].modifiers << 16 | keyDownEntries[i].virtualKey, keyDownEntries[i].name);

        for (size_t i = 0; i < WTF_ARRAY_LENGTH(keyPressEntries); ++i)
            keyPressCommandsMap->set(keyPressEntries[i].modifiers << 16 | keyPressEntries[i].charCode, keyPressEntries[i].name);
    }

    unsigned modifiers = 0;
    if (evt->shiftKey())
        modifiers |= ShiftKey;
    if (evt->altKey())
        modifiers |= AltKey;
    if (evt->metaKey())
        modifiers |= CommandKey;

    if (evt->type() == eventNames().keydownEvent) {
        int mapKey = modifiers << 16 | evt->keyCode();
        return mapKey ? keyDownCommandsMap->get(mapKey) : 0;
    }

    int mapKey = modifiers << 16 | evt->charCode();
    return mapKey ? keyPressCommandsMap->get(mapKey) : 0;
}

bool WebPage::handleEditingKeyboardEvent(WebCore::KeyboardEvent& event)
{
    auto* frame = downcast<WebCore::Node>(event.target())->document().frame();
    ASSERT(frame);

    auto* keyEvent = event.underlyingPlatformEvent();
    if (!keyEvent || keyEvent->isSystemKey())  // do not treat this as text input if it's a system key event
        return false;

    auto command = frame->editor().command(interpretKeyEvent(&event));

    if (keyEvent->type() == PlatformEvent::RawKeyDown) {
        // WebKit doesn't have enough information about mode to decide how commands that just insert text if executed via Editor should be treated,
        // so we leave it upon WebCore to either handle them immediately (e.g. Tab that changes focus) or let a keypress event be generated
        // (e.g. Tab that inserts a Tab character, or Enter).
        return !command.isTextInsertion() && command.execute(&event);
    }

    if (command.execute(&event))
        return true;

    // Don't insert null or control characters as they can result in unexpected behaviour
    if (event.charCode() < ' ')
        return false;

    return frame->editor().insertText(keyEvent->text(), &event);
}

bool WebPage::handleIntuiMessage(IntuiMessage *imsg, const int mouseX, const int mouseY, bool mouseInside)
{
	WebCore::Page *cp = corePage();
	if (!cp)
		return false;

	auto& bridge = cp->userInputBridge();
	auto& focusController = m_page->focusController();

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
			
			m_lastQualifier = imsg->Qualifier;
			
			switch (imsg->Class)
			{
			case IDCMP_MOUSEBUTTONS:
				switch (imsg->Code)
				{
				case SELECTDOWN:
				case MENUDOWN:
				case MIDDLEDOWN:
					if (mouseInside)
					{
						if (_fGoActive)
							_fGoActive();

						bool eat = bridge.handleMousePressEvent(pme);
						m_trackMouse = true;
						return eat;
					}
					break;
				default:
					if (mouseInside || m_trackMouse)
					{
						bool eat = bridge.handleMouseReleaseEvent(pme);
						m_trackMouse = false;
						return eat;
					}
					break;
				}
				break;
			case IDCMP_MOUSEMOVE:
			case IDCMP_MOUSEHOVER:
				if (mouseInside || m_trackMouse)
				{
					bridge.handleMouseMoveEvent(pme);
					return m_trackMouse;
				}
				break;
			}
		}
		break;
		
	case IDCMP_RAWKEY:
		{
			Boopsiobject *oimsg = (Boopsiobject *)imsg;
			ULONG key = 0;
			ULONG code = imsg->Code & ~IECODE_UP_PREFIX;
			BOOL up = (imsg->Code & IECODE_UP_PREFIX) == IECODE_UP_PREFIX;

// dprintf("rawkey %lx up %d\n", imsg->Code& ~IECODE_UP_PREFIX, up);

			m_lastQualifier = imsg->Qualifier;

			switch (code)
			{
			case NM_WHEEL_UP:
			case NM_WHEEL_DOWN:
				if (mouseInside && !up)
				{
					if (m_isActive)
					{
						float wheelTicksY = 1;
						float deltaY = (code == NM_WHEEL_UP) ? 50.0f : -50.0f;
						WebCore::PlatformWheelEvent pke(WebCore::IntPoint(mouseX, mouseY),
							WebCore::IntPoint(imsg->IDCMPWindow->LeftEdge + imsg->MouseX, imsg->IDCMPWindow->TopEdge + imsg->MouseY),
							0, deltaY,
							0, wheelTicksY,
							ScrollByPixelWheelEvent,
							(imsg->Qualifier & (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT)) != 0,
							(imsg->Qualifier & IEQUALIFIER_CONTROL) != 0,
							(imsg->Qualifier & (IEQUALIFIER_LALT|IEQUALIFIER_RALT)) != 0,
							(imsg->Qualifier & (IEQUALIFIER_LCOMMAND|IEQUALIFIER_RCOMMAND)) != 0
							);
						bool handled = bridge.handleWheelEvent(pke);
						if (!handled)
							scrollBy(0, (code == NM_WHEEL_UP) ? 50 : -50);
					}
					else
					{
						scrollBy(0, (code == NM_WHEEL_UP) ? 50 : -50);
					}
					return true;
				}
				break;
			
			case RAWKEY_TAB:
				if (!m_isActive)
					return false;
				if (!up)
				{
					if (m_justWentActive)
					{
						Frame& frame = m_page->focusController().focusedOrMainFrame();
						frame.document()->setFocusedElement(0);
						m_page->focusController().setInitialFocus((imsg->Qualifier & (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT)) ?
							FocusDirectionBackward : FocusDirectionForward, nullptr);
						m_justWentActive = false;
					}
					else
					{
						bool rc = focusController.advanceFocus((imsg->Qualifier & (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT)) ? FocusDirection::FocusDirectionBackward : FocusDirection::FocusDirectionForward, nullptr);

						if ((!rc || !m_focusedElement) && _fActivateNext && _fActivatePrevious)
						{
							if (imsg->Qualifier & (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT))
								_fActivatePrevious();
							else
								_fActivateNext();
						}
					}
				}
				return true;
			
			default:
				if (m_isActive)
				{
					bridge.handleKeyEvent(WebCore::PlatformKeyboardEvent(imsg));
					return true;
				}
				break;
			}
		}
		break;
	}

	return false;
}

bool WebPage::handleMUIKey(int muikey)
{
	if (!m_page)
		return false;

	auto& focusController = m_page->focusController();
	switch (muikey)
	{
	case MUIKEY_GADGET_NEXT:
		focusController.advanceFocus(FocusDirection::FocusDirectionForward, nullptr);
		return true;
	case MUIKEY_GADGET_PREV:
		focusController.advanceFocus(FocusDirection::FocusDirectionBackward, nullptr);
		return true;
	case MUIKEY_GADGET_OFF:
		return true;
	default:
		break;
	}
	
	return false;
}

const WTF::Vector<WebCore::ContextMenuItem>& WebPage::buildContextMenu(const int x, const int y)
{
	static WTF::Vector<WebCore::ContextMenuItem> _empty;
	WebCore::PlatformMouseEvent pme(
		WebCore::IntPoint(x, y),
		WebCore::IntPoint(x, y),
		WebCore::MouseButton::RightButton,
		WebCore::PlatformEvent::Type::MousePressed,
		m_clickCount,
		(m_lastQualifier & (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT)) != 0,
		(m_lastQualifier & IEQUALIFIER_CONTROL) != 0,
		(m_lastQualifier & (IEQUALIFIER_LALT|IEQUALIFIER_RALT)) != 0,
		(m_lastQualifier & (IEQUALIFIER_LCOMMAND|IEQUALIFIER_RCOMMAND)) != 0,
		WTF::WallTime::now(),
		0.0,
		WebCore::SyntheticClickType::NoTap);


	WebCore::Page *page = corePage();
	if (!page)
		return _empty;
    auto* coreFrame = m_mainFrame->coreFrame();

	// kill any previous context menu
    page->contextMenuController().clearContextMenu();
	bool handledEvent = coreFrame->eventHandler().sendContextMenuEvent(pme);

	if (!handledEvent)
		return _empty;

	// Re-get page, since it might have gone away during event handling.
	page = coreFrame->page();
	if (!page)
		return _empty;

	auto* contextMenu = page->contextMenuController().contextMenu();
	if (!contextMenu)
		return _empty;

	return contextMenu->items();
}

void WebPage::onContextMenuItemSelected(ULONG action, const char *title)
{
	WebCore::ContextMenuAction cmaction = WebCore::ContextMenuAction(action);
	WTF::String wtftitle = WTF::String::fromUTF8(title);
	WebCore::Page *page = corePage();
	if (!page)
		return;
	page->contextMenuController().contextMenuItemSelected(cmaction, wtftitle);
}

}

