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
#include <WebCore/AutofillElements.h>
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
#include <iostream>
#include <vector>
#include <functional>
#include <string.h>

#include <cairo.h>

#include <utility>
#include <cstdio>

#include <proto/graphics.h>
#include <proto/exec.h>
#include <proto/cybergraphics.h>
#include <proto/intuition.h>
#include <proto/layers.h>
#include <cybergraphx/cybergraphics.h>
#include <intuition/intuition.h>
#include <intuition/pointerclass.h>
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

namespace {

	inline constexpr int ceilingDivide(const int value, const int divider)
	{
		return 1 + ((value - 1) / divider);
	}
}

class TiledDamage
{
	static constexpr int m_tileSize = 64;
	static constexpr int m_bitDivider = 32;

	class EncapsulatingRect
	{
	public:

		EncapsulatingRect() = default;
		~EncapsulatingRect() = default;

		inline int x() const { return m_x; }
		inline int y() const { return m_y; }
		inline int width() const { return m_width; }
		inline int height() const { return m_height; }

		inline void encapsulateCoords(int x, int y, int width, int height)
		{
			if (isValid())
			{
				int maxX = std::max(x + width - 1, m_x + m_width - 1);
				m_x = std::min(m_x, x);
				m_width = (maxX - m_x) + 1;
				int maxY = std::max(y + height - 1, m_y + m_height - 1);
				m_y = std::min(m_y, y);
				m_height = (maxY - m_y) + 1;
			}
			else
			{
				m_x = x;
				m_y = y;
				m_width = width;
				m_height = height;
			}
		}
		
		inline void inflateWidthToPoint(int xPlusWidth)
		{
			m_width = xPlusWidth - m_x;
		}

		inline bool isValid() const { return m_width > 0; }

		inline void reset()
		{
			m_width = -1;
		}
		
		inline void clip(const int width, const int height)
		{
			m_width = std::min(m_width, width - m_x);
			m_height = std::min(m_height, height - m_y);
		}

	protected:
		int m_x;
		int m_y;
		int m_width = -1;
		int m_height;
	};

public:

	TiledDamage() = default;
	~TiledDamage() = default;

	inline int width() const { return m_width; }
	inline int height() const { return m_height; }
	inline int rows() const { return m_rows; }
	inline int columns() const { return m_columns; }

	void resize(const int width, const int height)
	{
		if (width != m_width || height != m_height)
		{
			m_width = width;
			m_height = height;
			
			m_rows = ceilingDivide(width, m_tileSize);
			m_columns = ceilingDivide(height, m_tileSize);
			m_cells = m_rows * m_columns;
			
			m_damage.resize(m_cells);
			for (int i = 0; i < m_cells; i++)
				m_damage[i] = true;
			
			m_damageRect.reset();
			m_damageRect.encapsulateCoords(0, 0, m_rows, m_columns);
		}
	}

	void invalidate()
	{
		for (int i = 0; i < m_cells; i++)
			m_damage[i] = true;
		m_damageRect.reset();
		m_damageRect.encapsulateCoords(0, 0, m_rows, m_columns);
	}
	
	void invalidate(int x, int y, int width, int height)
	{
		if (x < 0)
		{
			width += x;
			x = 0;
		}
		
		if (y < 0)
		{
			height += y;
			y = 0;
		}

		if (x + width > m_width)
			width = m_width - x;
		if (y + height > m_height)
			height = m_height - y;

		if (width < 0 || height < 0)
			return;

		// switch to tile bits from units...
		int maxX = ceilingDivide(x + width, m_tileSize);
		int maxY = ceilingDivide(y + height, m_tileSize);
		x /= m_tileSize;
		y /= m_tileSize;
		width = maxX - x;
		height = maxY - y;

		// so now x and y are row and column numbers...
		m_damageRect.encapsulateCoords(x, y, width, height);
		
		// damage the tiles...
		int bitIndex = x + (y * m_rows);
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
				m_damage[bitIndex + j] = true;
			bitIndex += m_rows;
		}
	}
	
	void visitDamagedTiles(std::function<void(const int x, const int y, const int width, const int height)> &&visitor)
	{
		EncapsulatingRect lastRect;
		EncapsulatingRect rect;

		for (int y = m_damageRect.y(); y < m_damageRect.y() + m_damageRect.height(); y++)
		{
			for (int x = m_damageRect.x(); x < m_damageRect.x() + m_damageRect.width(); x++)
			{
				if (m_damage[(y * m_rows) + x])
				{
					if (rect.isValid())
					{
						rect.inflateWidthToPoint(((x + 1) * m_tileSize));
					}
					else
					{
						rect.encapsulateCoords(x * m_tileSize, y * m_tileSize, m_tileSize, m_tileSize);
					}
				}
				else if (rect.isValid())
				{
					rect.clip(m_width, m_height);
					visitor(rect.x(), rect.y(), rect.width(), rect.height());
					rect.reset();
				}
			}
			
			if (rect.isValid())
			{
				if (!lastRect.isValid() || (lastRect.x() == rect.x() && lastRect.width() == rect.width()))
				{
					lastRect.encapsulateCoords(rect.x(), rect.y(), rect.width(), rect.height());
				}
				else if (lastRect.isValid())
				{
					lastRect.clip(m_width, m_height);
					visitor(lastRect.x(), lastRect.y(), lastRect.width(), lastRect.height());
					lastRect.reset();
					lastRect.encapsulateCoords(rect.x(), rect.y(), rect.width(), rect.height());
				}
				
				rect.reset();
			}
		}

		if (lastRect.isValid())
		{
			lastRect.clip(m_width, m_height);
			visitor(lastRect.x(), lastRect.y(), lastRect.width(), lastRect.height());
		}
	}
	
	void clear()
	{
		m_damageRect.reset();
		for (int i = 0; i < m_cells; i++)
			m_damage[i] = false;
	}
	
	bool hasDamage() const { return m_damageRect.isValid(); }

protected:
	std::vector<bool> m_damage;
	EncapsulatingRect m_damageRect;
	int m_width = 0;
	int m_height = 0;
	int m_rows;
	int m_columns;
	int m_cells;
};
namespace WebKit {

class MediaRecorderProvider final : public WebCore::MediaRecorderProvider {
public:
    MediaRecorderProvider() = default;
};

class WebViewDrawContext
{
	int m_width = -1;
	int m_height = -1;
	int m_scrollY = 0;
	bool m_partialDamage = false;
	bool m_didScroll = false;

	TiledDamage m_damage;

	cairo_surface_t *m_surface = nullptr ;
	cairo_t *m_cairo = nullptr ;
	WebCore::PlatformContextCairo *m_platformContext = nullptr;

public:
	WebViewDrawContext(const int width, const int height)
		: m_surface(nullptr)
		, m_cairo(nullptr)
		, m_platformContext(nullptr)
	{
		resize(width, height);
	}
	
	~WebViewDrawContext()
	{
		if (m_platformContext)
			delete m_platformContext;
		if (m_cairo)
			cairo_destroy(m_cairo);
		if (m_surface)
			cairo_surface_destroy(m_surface);
	}
	
	int width() const { return m_width; }
	int height() const { return m_height; }
	void onDidScroll()
	{
		m_didScroll = true;
	}

	void invalidate(const WebCore::IntRect& rect)
	{
		int width = rect.width();
		int height = rect.height();
		
		// WebCore does this for some reason...
		if (width == 0 || height == 0)
			return;

		if (width < m_width || height < m_height)
			m_partialDamage = true;

		m_damage.invalidate(rect.x(), rect.y(), rect.width(), rect.height());
	}
	
	void invalidate()
	{
		m_damage.invalidate();
	}
	
	void repair(WebCore::FrameView *frameView, WebCore::InterpolationQuality interpolation)
	{
		WebCore::GraphicsContext gc(m_platformContext);
		
		if (WebCore::InterpolationQuality::Default != interpolation)
		{
			gc.setImageInterpolationQuality(interpolation);
		}

		m_damage.visitDamagedTiles([&](const int x, const int y, const int width, const int height) {
			WebCore::IntRect ir(x, y, width, height);
			/// NOTE: bad shit happens when clipping is used w/o save/restore, cairo seems to be happily
			/// trashing memory w/o this
			gc.save();
			/// NOTE: clipping IS important. WebCore will paint whole elements that overlap the paint area
			/// in their actual bounds otherwise, but will not paint any children that do not (so a button
			/// frame overlapping would clear button text if the text wasn't overlapping)
			gc.clip(WebCore::FloatRect(x, y, width, height));
			frameView->paint(gc, ir);
			gc.restore();
		});
	}
	
	void repaint(RastPort *rp, const int outX, const int outY)
	{
		cairo_surface_flush(m_surface);
		const unsigned int stride = cairo_image_surface_get_stride(m_surface);
		unsigned char *src = cairo_image_surface_get_data(m_surface);

		m_damage.visitDamagedTiles([&](const int x, const int y, const int width, const int height) {
			WritePixelArray(src, x, y, stride, rp, outX + x, outY + y, width, height, RECTFMT_ARGB);
		});
	}
	
	void repaintAll(RastPort *rp, const int outX, const int outY)
	{
		cairo_surface_flush(m_surface);
		const unsigned int stride = cairo_image_surface_get_stride(m_surface);
		unsigned char *src = cairo_image_surface_get_data(m_surface);
		WritePixelArray(src, 0, 0, stride, rp, outX, outY, m_width, m_height, RECTFMT_ARGB);
	}

	void draw(WebCore::FrameView *frameView, RastPort *rp, const int x, const int y, const int width, const int height,
		int scrollX, int scrollY, bool update, WebCore::InterpolationQuality interpolation)
	{
		if (!m_platformContext)
			return;

		(void)scrollX;

#if 1
		struct Window *window = (struct Window *)rp->Layer->Window;

		// Only trigger fast path if we've scrolled from outside (by scroller, etc) and there was
		// no partial damage done to the site (meaning some components have displaced)
		if (m_scrollY != scrollY && update && window && !m_partialDamage && m_didScroll)
		{
			int delta = scrollY - m_scrollY;
			m_scrollY = scrollY;

			if (abs(delta) < m_height)
			{
				LockLayerUpdates(rp->Layer);
			
				if (delta > 0)
					m_damage.invalidate(0, m_height - delta, m_width, delta);
				else
					m_damage.invalidate(0, 0, m_width, -delta);

				repair(frameView, interpolation);
				ScrollWindowRaster(window, 0, delta, x, y, x + width - 1, y + height - 1);

				cairo_surface_flush(m_surface);
				const unsigned int stride = cairo_image_surface_get_stride(m_surface);
				unsigned char *src = cairo_image_surface_get_data(m_surface);

				if (delta > 0)
					WritePixelArray(src, 0, height - delta, stride, rp, x, y + height - delta, width, delta, RECTFMT_ARGB);
				else
					WritePixelArray(src, 0, 0, stride, rp, x, y, width, -delta, RECTFMT_ARGB);
				
				UnlockLayerUpdates(rp->Layer);
				m_damage.invalidate();
				m_partialDamage = false;
				m_didScroll = false;
				return;
			}
		}
#endif

		repair(frameView, interpolation);
		if (update)
			repaint(rp, x, y);
		else
			repaintAll(rp, x, y);
		m_damage.clear();
		m_partialDamage = false;
		m_didScroll = false;
	}

	bool resize(const int width, const int height)
	{
		if (width != m_width || height != m_height)
		{
			if (m_platformContext)
				delete m_platformContext;
			if (m_cairo)
				cairo_destroy(m_cairo);
			if (m_surface)
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
					

					m_damage.resize(m_width, m_height);
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


#if 0
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
#endif

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

	(void)parameters;

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

#if 0
        [[self preferences] privateBrowsingEnabled] ? PAL::SessionID::legacyPrivateSessionID() : PAL::SessionID::defaultSessionID(),
#endif

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
    settings.setAuthorAndUserStylesEnabled(true);
    settings.setFixedFontFamily("Courier New");
    settings.setDefaultFixedFontSize(13);
    settings.setResizeObserverEnabled(true);
	settings.setEditingBehaviorType(EditingBehaviorType::EditingUnixBehavior);

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
	
// 	settings.setDeveloperExtrasEnabled(true);
	settings.setXSSAuditorEnabled(true);
	settings.setVisualViewportAPIEnabled(true);
	
	settings.setHiddenPageCSSAnimationSuspensionEnabled(true);
	settings.setAnimatedImageAsyncDecodingEnabled(false);

	settings.setViewportFitEnabled(true);
	settings.setConstantPropertiesEnabled(true);
	
//	settings.setLogsPageMessagesToSystemConsoleEnabled(true);
	
	settings.setRequestAnimationFrameEnabled(true);
	settings.setUserStyleSheetLocation(WTF::URL(WTF::URL(), WTF::String("file:///PROGDIR:Resources/morphos.css")));

    m_mainFrame = WebFrame::createWithCoreMainFrame(this, &m_page->mainFrame());
    static_cast<WebFrameLoaderClient&>(m_page->mainFrame().loader().client()).setWebFrame(m_mainFrame.get());

//    m_page->mainFrame().tree().setName(toString("frameName"));
//    m_page->mainFrame().init();

    m_page->layoutIfNeeded();

    m_page->setIsVisible(true);
    m_page->setIsInWindow(true);
	m_page->setActivityState(ActivityState::WindowIsActive);
//	m_page->setLowPowerModeEnabledOverrideForTesting(true);

//    m_page->addLayoutMilestones({ DidFirstLayout, DidFirstVisuallyNonEmptyLayout });
}

WebPage::~WebPage()
{
	D(dprintf("%s(%p)\n", __PRETTY_FUNCTION__, this));
	clearDelegateCallbacks();
	delete m_drawContext;
	delete m_autofillElements;
}

WebCore::Page *WebPage::corePage()
{
	return m_page.get();
}

const WebCore::Page *WebPage::corePage() const
{
	return m_page.get();
}

WebPage* WebPage::fromCorePage(WebCore::Page* page)
{
    return &static_cast<WebChromeClient&>(page->chrome().client()).page();
}

void WebPage::load(const char *url)
{
	static uint64_t navid = 1;

	if (!m_mainFrame)
		return;

    auto* coreFrame = m_mainFrame->coreFrame();
	WTF::URL baseCoreURL = WTF::URL(WTF::URL(), WTF::String(url));
	WebCore::ResourceRequest request(baseCoreURL);
	
	corePage()->userInputBridge().stopLoadingFrame(coreFrame);
	m_pendingNavigationID = navid ++;
	coreFrame->loader().load(FrameLoadRequest(*coreFrame, request, ShouldOpenExternalURLsPolicy::ShouldNotAllow));

//    coreFrame->loader().urlSelected(baseCoreURL, { }, nullptr, LockHistory::No, LockBackForwardList::No, MaybeSendReferrer, ShouldOpenExternalURLsPolicy::ShouldNotAllow);
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

void WebPage::goToItem(WebCore::HistoryItem& item)
{
	m_page->goToItem(item, FrameLoadType::IndexedBackForward, ShouldTreatAsContinuingLoad::No);
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
//	stop();
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
	m_isVisible = true;
	corePage()->setIsVisible(true);
	corePage()->userInputBridge().focusSetActive(true);
}

void WebPage::goHidden()
{
	m_isVisible = false;
	corePage()->setIsVisible(false);
	corePage()->userInputBridge().focusSetActive(false);
}

void WebPage::setLowPowerMode(bool lowPowerMode)
{
	corePage()->setLowPowerModeEnabledOverrideForTesting(lowPowerMode);
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
	corePage()->setIsVisible(true);
	corePage()->userInputBridge().focusSetActive(true);
}

void WebPage::setFocusedElement(WebCore::Element *element)
{
	// this is called by the Chrome
	m_focusedElement = element;
}

void WebPage::startedEditingElement(WebCore::HTMLInputElement *input)
{
	if (nullptr == input)
		return;
	if (nullptr == m_autofillElements)
		m_autofillElements = new WebCore::AutofillElements();
	if (m_autofillElements)
	{
		if (m_autofillElements->computeAutofillElements(*input))
		{
			if (_fHasAutofill)
				_fHasAutofill();
		}
		else
		{
			delete m_autofillElements;
			m_autofillElements = nullptr;
		}
	}
}

bool WebPage::hasAutofillElements()
{
	if (m_autofillElements)
		return true;
	return false;
}

void WebPage::clearAutofillElements()
{
	if (m_autofillElements)
		delete m_autofillElements;
	m_autofillElements = nullptr;
}

void WebPage::setAutofillElements(const WTF::String &login, const WTF::String &password)
{
	if (m_autofillElements)
		m_autofillElements->autofill(login, password);
}

bool WebPage::getAutofillElements(WTF::String &outlogin, WTF::String &outPassword)
{
	if (m_autofillElements)
	{
		if (m_autofillElements->username())
		{
			outlogin = m_autofillElements->username()->value();
		}
		
		if (m_autofillElements->password())
		{
			outPassword = m_autofillElements->password()->value();
		}
		
		return true;
	}

	return false;
}

void WebPage::setCursor(int cursor)
{
	if (m_cursor != cursor)
	{
		m_cursor = cursor;

		if (!m_trackMouse && _fSetCursor)
			_fSetCursor(m_cursor);
	}
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
    {
        return;
	}

	if (m_trackedNetworkResourceRequestIdentifiers.isEmpty() && _fDidStopLoading)
	{
		_fDidStopLoading();
	}
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
    {
        return;
	}

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

float WebPage::pageZoomFactor() const
{
	if (m_mainFrame)
	{
		auto* coreFrame = m_mainFrame->coreFrame();
		return coreFrame->pageZoomFactor();
	}
	return 1.f;
}

float WebPage::textZoomFactor() const
{
	if (m_mainFrame)
	{
		auto* coreFrame = m_mainFrame->coreFrame();
		return coreFrame->textZoomFactor();
	}
	return 1.f;
}

void WebPage::setPageAndTextZoomFactors(float pageZoomFactor, float textZoomFactor)
{
	if (m_mainFrame)
	{
		auto* coreFrame = m_mainFrame->coreFrame();
		coreFrame->setPageAndTextZoomFactors(pageZoomFactor, textZoomFactor);
	}
}

void WebPage::scalePage(double scale, const IntPoint& origin)
{
    double totalScale = scale * viewScaleFactor();
    bool willChangeScaleFactor = totalScale != totalScaleFactor();

    m_page->setPageScaleFactor(totalScale, origin);

    // We can't early return before setPageScaleFactor because the origin might be different.
    if (!willChangeScaleFactor)
    {
        return;
	}

	if (m_drawContext)
	{
		m_drawContext->invalidate();
	}
	
	if (_fInvalidate)
	{
		_fInvalidate(false);
	}
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
	// dprintf("%s %ld:%ld x %ld:%ld\n", __PRETTY_FUNCTION__, rect.x(), rect.y(), rect.width(), rect.height());
	dprintf("%s %ld:%ld x %ld:%ld\n", __PRETTY_FUNCTION__, realRect.x(), realRect.y(),
		realRect.width(), realRect.height());
#endif

	if (m_drawContext)
		m_drawContext->invalidate(realRect);

	if (_fInvalidate)
		_fInvalidate(false);
}

void WebPage::internalScroll(int scrollX, int scrollY)
{
	(void)scrollX;
	(void)scrollY;

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
			_fInvalidate(false);
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
//  		coreFrame->document()->updateStyleIfNeeded();
		coreFrame->view()->resize(width, height);
		coreFrame->view()->availableContentSizeChanged(WebCore::ScrollableArea::AvailableSizeChangeReason::AreaSizeChanged);
		coreFrame->view()->updateLayoutAndStyleIfNeededRecursive();

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
	if (!m_mainFrame || !m_drawContext)
		return;
	auto* coreFrame = m_mainFrame->coreFrame();
	if (!coreFrame)
		return;
	WebCore::FrameView *view = coreFrame->view();

	m_ignoreScroll = true;
	view->setScrollPosition(WebCore::ScrollPosition(x, y));
	m_drawContext->onDidScroll();
	m_ignoreScroll = false;

	if (_fInvalidate)
		_fInvalidate(false);
}

void WebPage::scrollBy(const int xDelta, const int yDelta, WebCore::Frame *inFrame)
{
	if (!m_mainFrame)
		return;
	auto* coreFrame = inFrame ? inFrame : m_mainFrame->coreFrame();
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
		_fInvalidate(false);
}

void WebPage::wheelScrollOrZoomBy(const int xDelta, const int yDelta, ULONG qualifiers, WebCore::Frame *inFrame)
{
	if (qualifiers & IEQUALIFIER_CONTROL)
	{
		float factor = pageZoomFactor();
		if (yDelta < 0)
			factor -= 0.05;
		else
			factor += 0.05;
		setPageAndTextZoomFactors(factor, textZoomFactor());
	}
	else
	{
		scrollBy(xDelta, yDelta, inFrame);
	}
}

void WebPage::draw(struct RastPort *rp, const int x, const int y, const int width, const int height, bool updateMode)
{
	auto* coreFrame = m_mainFrame->coreFrame();
	if (!coreFrame || !m_drawContext)
	{
		return;
	}
	
    WebCore::FrameView* frameView = coreFrame->view();
    if (!frameView)
	{
		return;
	}

    m_page->updateRendering();

#if 0
	frameView->updateLayoutAndStyleIfNeededRecursive();
//	frameView->updateCompositingLayersAfterLayout();
	frameView->setPaintBehavior(PaintBehavior::FlattenCompositingLayers);
#endif
//	IntSize s = frameView->autoSizingIntrinsicContentSize();
	auto scroll = frameView->scrollPosition();

//	dprintf("draw to %p at %d %d : %dx%d, %d %d renderable %d\n", rp, x,y, width, height, s.width(), s.height(), frameView->isSoftwareRenderable());

	// FrameTree& tree = coreFrame->tree();

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
	m_drawContext->draw(frameView, rp, x, y, width, height, scroll.x(), scroll.y(), updateMode, m_interpolation);
}

void WebPage::invalidate()
{
	if (m_drawContext)
		m_drawContext->invalidate();
	if (_fInvalidate)
		_fInvalidate(true);
}

bool WebPage::search(const WTF::String &string, WebCore::FindOptions &options, bool& outWrapped)
{
	WebCore::Page *cp = corePage();

	if (cp)
	{
		WebCore::DidWrap didWrap(WebCore::DidWrap::No);
		bool found = cp->findString(string, options, &didWrap);
		outWrapped = didWrap == WebCore::DidWrap::Yes;
		return found;
	}
	
	return false;
}

void WebPage::loadUserStyleSheet(const WTF::String &path)
{
	WebCore::Settings& settings = m_page->settings();
	if (path.length() == 0)
	{
		settings.setUserStyleSheetLocation(WTF::URL(WTF::URL(), WTF::String("file:///PROGDIR:Resources/morphos.css")));
	}
	else
	{
		settings.setUserStyleSheetLocation(WTF::URL(WTF::URL(), path));
	}
}

bool WebPage::allowsScrolling()
{
	auto* coreFrame = m_mainFrame->coreFrame();
	if (coreFrame)
		return coreFrame->view()->canHaveScrollbars();
	return false;
}

void WebPage::setAllowsScrolling(bool allows)
{
	auto* coreFrame = m_mainFrame->coreFrame();
	if (coreFrame)
		coreFrame->view()->setCanHaveScrollbars(allows);
}

#if 0
void WebPage::flushCompositing()
{
dprintf("flushing compositing...\n");
	m_page->updateRendering();

	auto* coreFrame = m_mainFrame->coreFrame();
	if (coreFrame)
		coreFrame->view()->flushCompositingStateIncludingSubframes();
}
#endif

bool WebPage::drawRect(const int x, const int y, const int width, const int height, struct RastPort *rp)
{
	auto* coreFrame = m_mainFrame->coreFrame();
	if (!coreFrame || !m_drawContext)
	{
		return false;
	}
	
    WebCore::FrameView* frameView = coreFrame->view();
    if (!frameView)
    {
    	return false;
	}

	auto *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
	if (nullptr == surface)
	{
		return false;
	}

	auto *cairo = cairo_create(surface);
	if (nullptr == cairo)
	{
		cairo_surface_destroy(surface);
		return false;
	}

	WebCore::PlatformContextCairo context(cairo);
	WebCore::GraphicsContext gc(&context);
	WebCore::IntRect rect(0, 0,width, height);
	gc.save();
	gc.clip(rect);
	gc.setImageInterpolationQuality(WebCore::InterpolationQuality::Default);

	OptionSet<WebCore::PaintBehavior> oldBehavior = frameView->paintBehavior();
	OptionSet<WebCore::PaintBehavior> paintBehavior = oldBehavior;
	auto oldScroll = frameView->scrollPosition();

	paintBehavior.add(WebCore::PaintBehavior::FlattenCompositingLayers);
	paintBehavior.add(WebCore::PaintBehavior::Snapshotting);
	frameView->setPaintBehavior(paintBehavior);
	m_ignoreScroll = true;
	frameView->WebCore::ScrollView::scrollTo(WebCore::ScrollPosition(x, y));

	frameView->paint(gc, rect);

	frameView->setPaintBehavior(oldBehavior);
	frameView->WebCore::ScrollView::scrollTo(oldScroll);
	gc.restore();
	m_ignoreScroll = false;

	cairo_surface_flush(surface);
	const unsigned int stride = cairo_image_surface_get_stride(surface);
	unsigned char *src = cairo_image_surface_get_data(surface);

	WritePixelArray(src, 0, 0, stride, rp, 0, 0, width, height, RECTFMT_ARGB);

	cairo_destroy(cairo);
	cairo_surface_destroy(surface);

	return true;
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

bool WebPage::checkDownloadable(IntuiMessage *imsg, const int mouseX, const int mouseY, WTF::URL &outURL)
{
	auto position = m_mainFrame->coreFrame()->view()->windowToContents(WebCore::IntPoint(mouseX, mouseY));
	auto hitTestResult = m_mainFrame->coreFrame()->eventHandler().hitTestResultAtPoint(position, WebCore::HitTestRequest::ReadOnly | WebCore::HitTestRequest::Active | WebCore::HitTestRequest::DisallowUserAgentShadowContent | WebCore::HitTestRequest::AllowChildFrameContent);
	(void)imsg;
	if (hitTestResult.isOverLink())
		outURL = hitTestResult.absoluteLinkURL();
	else if (hitTestResult.image())
		outURL = hitTestResult.absoluteImageURL();
	return hitTestResult.isOverLink() || hitTestResult.image();
}

bool WebPage::handleIntuiMessage(IntuiMessage *imsg, const int mouseX, const int mouseY, bool mouseInside, bool isDefaultHandler)
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
			else if (imsg->Code == SELECTDOWN || imsg->Code == MIDDLEDOWN)
				m_clickCount ++;
			
			WebCore::SyntheticClickType clickType = WebCore::SyntheticClickType::NoTap;
			if (imsg->Code == SELECTDOWN)
				clickType = WebCore::SyntheticClickType::OneFingerTap;

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
				clickType);
			
			m_lastQualifier = imsg->Qualifier;

			switch (imsg->Class)
			{
			case IDCMP_MOUSEBUTTONS:
				switch (imsg->Code)
				{
				case SELECTDOWN:
					if (mouseInside)
					{
						if (_fGoActive)
							_fGoActive();

						bridge.handleMousePressEvent(pme);
						m_trackMouse = true;
						return true;
					}
					break;

				case MIDDLEDOWN:
					if (mouseInside)
						return true;
					break;
				case MIDDLEUP:
					if (mouseInside || m_trackMouse)
					{
						auto position = m_mainFrame->coreFrame()->view()->windowToContents(pme.position());
						auto hitTestResult = m_mainFrame->coreFrame()->eventHandler().hitTestResultAtPoint(position, WebCore::HitTestRequest::ReadOnly | WebCore::HitTestRequest::Active | WebCore::HitTestRequest::DisallowUserAgentShadowContent | WebCore::HitTestRequest::AllowChildFrameContent);
						bool isMouseDownOnLinkOrImage = hitTestResult.isOverLink() || hitTestResult.image();

						if (isMouseDownOnLinkOrImage)
						{
							if (imsg->Qualifier & (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT))
							{
								// open in new window...
								if (_fNewTabWindow)
								{
									if (hitTestResult.isOverLink())
										_fNewTabWindow(hitTestResult.absoluteLinkURL(), WebViewDelegateOpenWindowMode::NewWindow);
									else
										_fNewTabWindow(hitTestResult.absoluteImageURL(), WebViewDelegateOpenWindowMode::NewWindow);
								}
							}
							else if (imsg->Qualifier & (IEQUALIFIER_LALT|IEQUALIFIER_RALT))
							{
								// download
								if (_fDownload)
								{
									if (hitTestResult.isOverLink())
										_fDownload(hitTestResult.absoluteLinkURL(), hitTestResult.linkSuggestedFilename());
									else
										_fDownload(hitTestResult.absoluteImageURL(), { });
								}
							}
							else
							{
								// open in new bg tab...
								if (_fNewTabWindow)
								{
									if (hitTestResult.isOverLink())
										_fNewTabWindow(hitTestResult.absoluteLinkURL(), WebViewDelegateOpenWindowMode::BackgroundTab);
									else
										_fNewTabWindow(hitTestResult.absoluteImageURL(), WebViewDelegateOpenWindowMode::BackgroundTab);
								}
							}
						}
						m_trackMouse = false;
						return true;
					}
					break;
				case MENUDOWN:
					// This is consistent with Safari
					// Other browsers are a mess: Vivaldi does Down, Up, ContextMenu, Firefox & Chrome do Down, ContextMenu, Up
					if (mouseInside)
					{
						if (_fGoActive)
							_fGoActive();

						bool doEvent = true;
						
						switch (m_cmHandling)
						{
						case ContextMenuHandling::Override:
							doEvent = false;
							break;
						case ContextMenuHandling::OverrideWithControl:
							doEvent = (imsg->Qualifier & IEQUALIFIER_CONTROL) == 0;
							break;
						case ContextMenuHandling::OverrideWithAlt:
							doEvent = (imsg->Qualifier & (IEQUALIFIER_LALT|IEQUALIFIER_RALT)) == 0;
							break;
						case ContextMenuHandling::OverrideWithShift:
							doEvent = (imsg->Qualifier & (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT)) == 0;
							break;
						case ContextMenuHandling::Default:
						default:
							break;
						}

						auto position = m_mainFrame->coreFrame()->view()->windowToContents(pme.position());
						auto result = m_mainFrame->coreFrame()->eventHandler().hitTestResultAtPoint(position, WebCore::HitTestRequest::ReadOnly | WebCore::HitTestRequest::Active | WebCore::HitTestRequest::DisallowUserAgentShadowContent | WebCore::HitTestRequest::AllowChildFrameContent);
						m_page->contextMenuController().clearContextMenu();
						Frame* targetFrame = result.innerNonSharedNode() ? result.innerNonSharedNode()->document().frame() : &m_page->focusController().focusedOrMainFrame();
						if (targetFrame)
						{
							if (doEvent)
							{
								bridge.handleContextMenuEvent(pme, *targetFrame);
							}
						}

						if (m_page->contextMenuController().contextMenu() &&
							m_page->contextMenuController().contextMenu()->items().size())
						{
							if (_fContextMenu)
							{
								_fContextMenu(WebCore::IntPoint(mouseX, mouseY), m_page->contextMenuController().contextMenu()->items(), result);
								WebKit::WebProcess::singleton().returnedFromConstrainedRunLoop();
							}
						}
						else if (_fContextMenu && !doEvent)
						{
							// force a context menu!
							_fContextMenu(WebCore::IntPoint(mouseX, mouseY), { }, result);
							WebKit::WebProcess::singleton().returnedFromConstrainedRunLoop();
						}
						else if (doEvent)
						{
							bridge.handleMousePressEvent(pme);
							m_trackMouse = true;
						}

						return true;
					}
					break;
				case MENUUP:
					if (m_trackMouse)
					{
						m_trackMouse = false;
						return bridge.handleMouseReleaseEvent(pme);
					}
					break;
				default:
					if (mouseInside || m_trackMouse)
					{
						bridge.handleMouseReleaseEvent(pme);
						m_trackMouse = false;
						return true;
					}
					break;
				}
				break;
			case IDCMP_MOUSEMOVE:
			case IDCMP_MOUSEHOVER:
				if (mouseInside || m_trackMouse)
				{
					bridge.handleMouseMoveEvent(pme);
					WTF::URL hoverURL;
					bool downloadable = checkDownloadable(imsg, mouseX, mouseY, hoverURL);

					if (m_hoveredURL != hoverURL)
					{
						m_hoveredURL = hoverURL;
						if (_fHoveredURLChanged)
						{
							_fHoveredURLChanged(m_hoveredURL);
						}
					}

					if (_fSetCursor && (imsg->Qualifier & (IEQUALIFIER_LALT|IEQUALIFIER_RALT)) && downloadable)
					{
						_fSetCursor(POINTERTYPE_ALTERNATIVECHOICE);
						return m_trackMouse;
					}

					if (_fSetCursor)
						_fSetCursor(m_cursor);
					return m_trackMouse;
				}
				else
				{
					if (_fSetCursor)
						_fSetCursor(0);
				}
				break;
			}
		}
		break;
		
	case IDCMP_RAWKEY:
		{
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
					if (1) //m_isActive || isDefaultHandler)
					{
						float wheelTicksY = (code == NM_WHEEL_UP) ? 1 : -1;
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
						
						auto position = m_mainFrame->coreFrame()->view()->windowToContents(pke.position());
						auto result = m_mainFrame->coreFrame()->eventHandler().hitTestResultAtPoint(position, WebCore::HitTestRequest::ReadOnly | WebCore::HitTestRequest::Active | WebCore::HitTestRequest::DisallowUserAgentShadowContent | WebCore::HitTestRequest::AllowChildFrameContent);
						Frame* targetFrame = result.innerNonSharedNode() ? result.innerNonSharedNode()->document().frame() : &m_page->focusController().focusedOrMainFrame();
						bool handled = bridge.handleWheelEvent(pke);
						if (!handled)
							wheelScrollOrZoomBy(0, (code == NM_WHEEL_UP) ? 50 : -50, imsg->Qualifier, targetFrame);
					}
					else
					{
						wheelScrollOrZoomBy(0, (code == NM_WHEEL_UP) ? 50 : -50, imsg->Qualifier);
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
				if (m_isActive || isDefaultHandler)
				{
					bool handled = bridge.handleKeyEvent(WebCore::PlatformKeyboardEvent(imsg));

					#define KEYQUALIFIERS (IEQUALIFIER_LALT|IEQUALIFIER_RALT|IEQUALIFIER_LSHIFT|IEQUALIFIER_LSHIFT|IEQUALIFIER_LCOMMAND|IEQUALIFIER_RCOMMAND|IEQUALIFIER_CONTROL)

					if (!handled)
					{
						WTF::URL ignored;

						switch (code)
						{
						case RAWKEY_LALT:
						case RAWKEY_RALT:
							if (!up && _fSetCursor && (imsg->Qualifier & (IEQUALIFIER_LALT|IEQUALIFIER_RALT)) && checkDownloadable(imsg, mouseX, mouseY, ignored))
							{
								_fSetCursor(POINTERTYPE_ALTERNATIVECHOICE);
							}
							else if (up && _fSetCursor)
							{
								_fSetCursor(m_cursor);
							}
							break;
						
						case RAWKEY_PAGEUP:
							if (!up && m_drawContext && (0 == (imsg->Qualifier & KEYQUALIFIERS)))
							{
								scrollBy(0, m_drawContext->height(), m_page->focusController().focusedFrame());
								return true;
							}
							break;

						case RAWKEY_PAGEDOWN:
						case RAWKEY_SPACE:
							if (!up && m_drawContext&& (0 == (imsg->Qualifier & KEYQUALIFIERS)))
							{
								scrollBy(0, -m_drawContext->height(), m_page->focusController().focusedFrame());
								return true;
							}
							break;

						case RAWKEY_DOWN:
							if (!up && m_drawContext&& (0 == (imsg->Qualifier & KEYQUALIFIERS)))
							{
								scrollBy(0, -50, m_page->focusController().focusedFrame());
								return true;
							}
							break;

						case RAWKEY_UP:
							if (!up && m_drawContext&& (0 == (imsg->Qualifier & KEYQUALIFIERS)))
							{
								scrollBy(0, 50, m_page->focusController().focusedFrame());
								return true;
							}
							break;

						case RAWKEY_HOME:
						case RAWKEY_END:
							if (!up && m_mainFrame && (0 == (imsg->Qualifier & KEYQUALIFIERS)))
							{
								auto* coreFrame = m_page->focusController().focusedFrame() ? m_page->focusController().focusedFrame() : m_mainFrame->coreFrame();
								
								if (coreFrame)
								{
									WebCore::FrameView *view = coreFrame->view();
									WebCore::ScrollPosition sp = view->scrollPosition();
									WebCore::ScrollPosition spMin = view->minimumScrollPosition();
									WebCore::ScrollPosition spMax = view->maximumScrollPosition();

									view->setScrollPosition(WebCore::ScrollPosition(sp.x(), code == RAWKEY_HOME ? spMin.y() : spMax.y()));
									
									if (_fInvalidate)
										_fInvalidate(false);
								}
								return true;
							}
							break;
						}
						return false;
					}
					return true;
				}
				break;
			}
		}
		break;
	}

	return false;
}

bool WebPage::handleMUIKey(int muikey, bool isDefaultHandler)
{
	if (!m_isActive && !isDefaultHandler)
		return false;
	if (!m_page)
		return false;
	const char *editorCommand = nullptr;

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
	case MUIKEY_CUT:
		editorCommand = "Cut";
		break;
	case MUIKEY_COPY:
		editorCommand = "Copy";
		break;
	case MUIKEY_PASTE:
		editorCommand = "Paste";
		break;
	case MUIKEY_UNDO:
		editorCommand = "Undo";
		break;
	case MUIKEY_REDO:
		editorCommand = "Redo";
		break;
	default:
		break;
	}
	
	if (editorCommand && focusController.focusedFrame())
	{
		auto command = focusController.focusedFrame()->editor().command(editorCommand);
		if (command.execute())
			return true;
	}

	return false;
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

