#define SYSTEM_PRIVATE

// These will spew out some warnings, but it is not possible to disable them :(
#undef __OBJC__
#import "WebKit.h"
#import "WebPage.h"
#import "WebProcess.h"
#import "WebViewDelegate.h"
#define __OBJC__

#import <ob/OBFramework.h>
#import <mui/MUIFramework.h>

#import "WkWebView.h"
#import "WkHistory_private.h"

#import <proto/dos.h>
#import <proto/exec.h>

#import <cairo.h>
struct Library *FreetypeBase;

extern "C" { void dprintf(const char *, ...); }

#define D(x) x

namespace  {
	static int _viewInstanceCount;
	static bool _shutdown;
	static bool _readyToQuitPending;
	static bool _wasInstantiatedOnce;
	static OBSignalHandler *_signalHandler;
	APTR   _globalOBContext;
	struct Task *_mainThread;
}

@interface WkWebView ()

- (void)invalidated;
- (void)scrollToX:(int)x y:(int)y;
- (void)setDocumentWidth:(int)width height:(int)height;

@end

@interface WkWebViewPrivate : OBObject
{
	WTF::RefPtr<WebKit::WebPage>   _page;
	id<WkWebViewScrollingDelegate> _scrollingDelegate;
	id<WkWebViewNetworkDelegate>   _networkDelegate;
	bool                           _drawPending;
	bool                           _isActive;
}
@end

@implementation WkWebViewPrivate

- (void)dealloc
{
	@synchronized ([WkWebView class])
	{
		_viewInstanceCount --;
		D(dprintf("%s: instances left %d pendingquit %d\n", __PRETTY_FUNCTION__, _viewInstanceCount, _readyToQuitPending));

		if (_readyToQuitPending)
		{
			[[MUIApplication currentApplication] quit];
		}
	}
	
	[super dealloc];
}

- (void)timedOut
{
	// delay disposal just a little bit
	[self retain];
	[self autorelease];
}

- (void)setPage:(WebKit::WebPage *)page
{
	_page = page;
}

- (WebKit::WebPage *)page
{
	return _page.get();
}

- (void)setScrollingDelegate:(id<WkWebViewScrollingDelegate>)delegate
{
	_scrollingDelegate = delegate;
}

- (id<WkWebViewScrollingDelegate>)scrollingDelegate
{
	return _scrollingDelegate;
}

- (void)setNetworkDelegate:(id<WkWebViewNetworkDelegate>)delegate
{
	_networkDelegate = delegate;
}

- (id<WkWebViewNetworkDelegate>)networkDelegate
{
	return _networkDelegate;
}

- (void)setDrawPending:(bool)drawPending
{
	_drawPending = drawPending;
}

- (bool)drawPending
{
	return _drawPending;
}

- (void)setIsActive:(bool)isactive
{
	_isActive = isactive;
}

- (bool)isActive
{
	return _isActive;
}

@end

@implementation WkWebView

static inline bool wkIsMainThread() {
	return FindTask(0) == _mainThread;
}

static inline void validateObjCContext() {
	if (!wkIsMainThread()) {
dprintf("---------- objc fixup ------------\n");
		FindTask(0)->tc_ETask->OBContext = _globalOBContext; // fixup ObjC threading
	}
}

+ (void)performWithSignalHandler:(OBSignalHandler *)handler
{
	if (_signalHandler == handler)
	{
		static const uint32_t mask = uint32_t(1UL << [handler sigBit]);
		WebKit::WebProcess::singleton().handleSignals(mask);
	}
}

+ (void)fire
{
	[self performWithSignalHandler:_signalHandler];
}

+ (void)_lastPageClosed
{
	D(dprintf("%s: %d\n", __PRETTY_FUNCTION__,_readyToQuitPending));
	if (_readyToQuitPending)
	{
		[[MUIApplication currentApplication] quit];
	}
}

+ (BOOL)readyToQuit
{
	@synchronized ([WkWebView class])
	{
		_readyToQuitPending = YES;
		if (_viewInstanceCount == 0 && WebKit::WebProcess::singleton().webFrameCount() == 0)
		{
			_readyToQuitPending = NO;
			_shutdown = YES;
			[[OBRunLoop mainRunLoop] removeSignalHandler:_signalHandler];
			[_signalHandler release];
			WebKit::WebProcess::singleton().terminate();
			CloseLibrary(FreetypeBase);
			return YES;
		}
	}
	
	return NO;
}

- (WkWebViewPrivate *)privateObject
{
	return _private;
}

- (id)init
{
	if ((self = [super init]))
	{
		@synchronized ([WkWebView class])
		{
			if (_shutdown || _readyToQuitPending)
			{
				[self release];
				return nil;
			}

			_viewInstanceCount ++;

			if (!_wasInstantiatedOnce)
			{
				FreetypeBase = OpenLibrary("freetype.library", 0);
				if (FreetypeBase)
				{
					BPTR icuDir = Lock("MOSSYS:Data/ICU/icudt54b", ACCESS_READ);
					if (0 == icuDir)
					{
						[MUIRequest requestWithTitle:@"WebKit Installation Error" message:@"ICU data files must be presend in MOSSYS:Data/ICU"
							buttons:[OBArray arrayWithObject:@"Exit"]];
						CloseLibrary(FreetypeBase);
						FreetypeBase = NULL;
						[self release];
						return nil;
					}
					else
					{
						UnLock(icuDir);
					}
					
					// MUST be done before 1st WebPage is instantiated!
					cairo_surface_t *dummysurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 4, 4);
					if (dummysurface)
						cairo_surface_destroy(dummysurface);
					
					_signalHandler = [OBSignalHandler new];
					[_signalHandler setDelegate:(id)[self class]];
					[[OBRunLoop mainRunLoop] addSignalHandler:_signalHandler];
					
					[OBScheduledTimer scheduledTimerWithInterval:0.1 perform:[OBPerform performSelector:@selector(fire) target:[self class]] repeats:YES];
					
					WebKit::WebProcess::singleton().initialize(int([_signalHandler sigBit]));
					WebKit::WebProcess::singleton().setLastPageClosedCallback([]() {
						[WkWebView _lastPageClosed];
					});
					
					_wasInstantiatedOnce = true;
					_mainThread = FindTask(0);
					_globalOBContext = _mainThread->tc_ETask->OBContext;
				}
			}
		}

		self.fillArea = NO;
		self.handledEvents = IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_MOUSEHOVER;
		[self setEventHandlerGUIMode:YES];
		self.cycleChain = YES;

		_private = [WkWebViewPrivate new];
		if (!_private)
		{
			[self release];
			return nil;
		}

		try {
			auto& webProcess = WebKit::WebProcess::singleton();
			auto identifier = WebCore::PageIdentifier::generate();

			WebKit::WebPageCreationParameters parameters;
			webProcess.createWebPage(identifier, WTFMove(parameters));
    		[_private setPage:webProcess.webPage(identifier)];

			if (![_private page])
			{
				[self release];
				return nil;
			}
			
			auto webPage = [_private page];

			webPage->_fInvalidate = [self]() { [self invalidated]; };

			webPage->_fSetDocumentSize = [self](int width, int height) {
				[self setDocumentWidth:width height:height];
			};
			
			webPage->_fScroll = [self](int x, int y) {
				[self scrollToX:x y:y];
			};
			
			webPage->_fActivateNext = [self]() {
				[[self windowObject] setActiveObjectSpecial:MUIV_Window_ActiveObject_Next];
			};
			
			webPage->_fActivatePrevious = [self]() {
				[[self windowObject] setActiveObjectSpecial:MUIV_Window_ActiveObject_Prev];
			};

			webPage->_fGoActive = [self]() {
				[[self windowObject] setActiveObject:self];
			};
			
			webPage->_fUserAgentForURL = [self](const WTF::String& url) -> WTF::String {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkWebViewNetworkDelegate> networkDelegate = [privateObject networkDelegate];
				if (networkDelegate)
				{
					OBString *overload = [networkDelegate userAgentForURL:[OBString stringWithUTF8String:url.utf8().data()]];

					if (overload)
						return WTF::String::fromUTF8([overload cString]);
				}
				return WTF::String::fromUTF8("Mozilla/5.0 (Macintosh; Intel Mac OS X 10_13_6) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/13.1 Safari/605.1.15");
			};
			
			webPage->_fChangedTitle = [self](const WTF::String& title) {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkWebViewNetworkDelegate> networkDelegate = [privateObject networkDelegate];
				if (networkDelegate)
				{
					[networkDelegate webView:self changedTitle:[OBString stringWithUTF8String:title.utf8().data()]];
				}
			};

			webPage->_fChangedURL = [self](const WTF::String& title) {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkWebViewNetworkDelegate> networkDelegate = [privateObject networkDelegate];
				if (networkDelegate)
				{
					[networkDelegate webView:self changedDocumentURL:[OBString stringWithUTF8String:title.utf8().data()]];
				}
			};
			
			webPage->_fDidStartLoading = [self]() {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkWebViewNetworkDelegate> networkDelegate = [privateObject networkDelegate];
				[networkDelegate webViewDidStartProvisionalLoading:self];
			};

			webPage->_fDidStopLoading = [self]() {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkWebViewNetworkDelegate> networkDelegate = [privateObject networkDelegate];
				[networkDelegate webViewDidFinishProvisionalLoading:self];
			};
			
			webPage->_fCanOpenWindow = [self](const WTF::String& url, const WebCore::WindowFeatures&) -> bool {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkWebViewNetworkDelegate> networkDelegate = [privateObject networkDelegate];
				return [networkDelegate webView:self wantsToCreateNewViewWithURL:[OBString stringWithUTF8String:url.utf8().data()] options:nil];
			};
			
			webPage->_fDoOpenWindow = [self]() -> WebCore::Page * {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkWebViewNetworkDelegate> networkDelegate = [privateObject networkDelegate];
				if (!networkDelegate)
					return nullptr;

				WkWebView *newView = [[WkWebView new] autorelease];
				WkWebViewPrivate *newPrivateObject = [newView privateObject];
				WebKit::WebPage *page = [newPrivateObject page];
				if (page && page->corePage())
				{
					[networkDelegate webView:self createdNewWebView:newView];
					return page->corePage();
				}
				return nullptr;
			};
			
			webPage->_fPopup = [self](const WebCore::IntRect& pos, const WTF::Vector<WTF::String>& items) -> int {
				validateObjCContext();
				MUIMenu *menu = [[MUIMenu new] autorelease];
				int index = 0;
				for (const WTF::String &entry : items)
				{
					[menu addObject:[MUIMenuitem itemWithTitle:[OBString stringWithUTF8String:entry.utf8().data()] shortcut:nil userData:++index]];
				}
				MUIMenustrip *strip = [[MUIMenustrip menustripWithObjects:menu, nil] retain];
				if (strip)
				{
					int rc = [strip popup:self flags:0 x:pos.x() y:pos.y()];
					[strip release];
					// 0 on failure, all our menus return 1, 2...
					return rc - 1;
				}
				return -1;
			};

		} catch (...) {
			[self release];
			return nil;
		}
	}
	
	return self;
}

- (void)dealloc
{
	try {
		auto webPage = [_private page];
		webPage->willBeDisposed();
		WebKit::WebProcess::singleton().removeWebPage(PAL::SessionID(), webPage->pageID());
	} catch (...) {};

	[OBScheduledTimer scheduledTimerWithInterval:2.0 perform:[OBPerform performSelector:@selector(timedOut) target:_private] repeats:NO];
	[_private release];

	[super dealloc];
}

- (void)scrollToLeft:(int)left top:(int)top
{
	try {
		auto webPage = [_private page];
		webPage->setScroll(left, top);
	} catch (std::exception& ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
}

- (void)setScrollingDelegate:(id<WkWebViewScrollingDelegate>)delegate
{
	[_private setScrollingDelegate:delegate];
}

- (void)setNetworkDelegate:(id<WkWebViewNetworkDelegate>)delegate
{
	[_private setNetworkDelegate:delegate];
}

- (void)load:(OBURL *)url
{
	const char *curi = [[url absoluteString] cString];
//	dprintf("%s:%d\n", __PRETTY_FUNCTION__, __LINE__);

	try {
		auto webPage = [_private page];
		webPage->go(curi);
	} catch (std::exception &ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
}

- (void)loadHTMLString:(OBString *)string baseURL:(OBURL *)base
{

}

- (void)loadRequest:(WkMutableNetworkRequest *)request
{

}

- (BOOL)loading
{
	return NO;
}

- (BOOL)hasOnlySecureContent
{
	return NO;
}

- (BOOL)canGoBack
{
	try {
		auto webPage = [_private page];
		return webPage->canGoBack();
	} catch (std::exception &ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
	return NO;
}

- (BOOL)canGoForward
{
	try {
		auto webPage = [_private page];
		return webPage->canGoForward();
	} catch (std::exception &ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
	return NO;
}

- (BOOL)goBack
{
	try {
		auto webPage = [_private page];
		return webPage->goBack();
	} catch (std::exception &ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
	return NO;
}

- (BOOL)goForward
{
	try {
		auto webPage = [_private page];
		return webPage->goForward();
	} catch (std::exception &ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
	return NO;
}

- (WkBackForwardList *)backForwardList
{
	try {
		auto webPage = [_private page];
		WTF::RefPtr<WebKit::BackForwardClientMorphOS> client = webPage->backForwardClient();
		if (client.get())
			return [WkBackForwardListPrivate backForwardListPrivate:client];
	} catch (std::exception &ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
	return nil;
}

- (void)reload
{
	try {
		auto webPage = [_private page];
		webPage->reload();
	} catch (std::exception &ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
}

- (void)stopLoading
{
	try {
		auto webPage = [_private page];
		webPage->stop();
	} catch (std::exception &ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
}

- (OBString *)title
{
	return nil;
}

- (OBURL *)URL
{
	return nil;
}

- (void)runJavaScript:(OBString *)javascript
{

}

- (OBString *)evaluateJavaScript:(OBString *)javascript
{
	return nil;
}

- (void)navigateTo:(OBString *)uri
{

}

- (void)dumpDebug
{
	WebKit::WebProcess::singleton().dumpWebCoreStatistics();
}

- (Boopsiobject *)instantiateTagList:(struct TagItem *)tags
{

#define MADF_KNOWSACTIVE       (1<< 7) /* sigh */

	Boopsiobject *meBoopsi = [super instantiateTagList:tags];
	if (meBoopsi)
	{
		// prevent MUI active frame from being drawn
		_flags(meBoopsi) |= MADF_KNOWSACTIVE;
	}
	return meBoopsi;
}

- (void)askMinMax:(struct MUI_MinMax *)minmaxinfo
{
	[super askMinMax:minmaxinfo];

	minmaxinfo->MinWidth += 100;
	minmaxinfo->MinHeight += 100;
	minmaxinfo->DefWidth += 1024;
	minmaxinfo->DefHeight += 740;
	minmaxinfo->MaxWidth = MUI_MAXMAX;
	minmaxinfo->MaxHeight = MUI_MAXMAX;
}

- (BOOL)draw:(ULONG)flags
{
	[super draw:flags];
	
	LONG iw = [self innerWidth];
	LONG ih = [self innerHeight];
	
	try {
		auto webPage = [_private page];
		webPage->setVisibleSize(int(iw), int(ih));
		webPage->draw([self rastPort], [self left], [self top], iw, ih, MADF_DRAWUPDATE == (MADF_DRAWUPDATE & flags));
	} catch (std::exception &ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}

	return TRUE;
}

- (void)goActive:(ULONG)flags
{
	[super goActive:flags];

	[_private setIsActive:true];
	self.handledEvents = self.handledEvents | IDCMP_RAWKEY;
	[[self windowObject] setDisableKeys:(1<<MUIKEY_WINDOW_CLOSE)|(1<<MUIKEY_GADGET_NEXT)|(1<<MUIKEY_GADGET_PREV)];

	try {
		auto webPage = [_private page];
		webPage->goActive();
	} catch (std::exception &ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
}

- (void)becomeInactive
{
	[_private setIsActive:false];
	self.handledEvents = self.handledEvents & ~IDCMP_RAWKEY;
	[[self windowObject] setDisableKeys:0];

	try {
		auto webPage = [_private page];
		webPage->goInactive();
	} catch (std::exception &ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
}

- (void)goInactive:(ULONG)flags
{
	[super goInactive:flags];
	[self becomeInactive];
}

- (void)cleanup
{
	[self becomeInactive];
	[super cleanup];
}

- (ULONG)handleEvent:(struct IntuiMessage *)imsg muikey:(LONG)muikey
{
	if (imsg)
	{
		auto webPage = [_private page];
		return webPage->handleIntuiMessage(imsg, [self mouseX:imsg], [self mouseY:imsg], [self isInObject:imsg]) ?
			MUI_EventHandlerRC_Eat : 0;
	}
	else
	{
		auto webPage = [_private page];
		return webPage->handleMUIKey(int(muikey));
	}
	
	return 0;
}

- (void)lateDraw
{
	if ([_private drawPending])
	{
		[self redraw:MADF_DRAWUPDATE];
		[_private setDrawPending:false];
	}
}

- (void)invalidated
{
	if (![_private drawPending])
	{
		[[OBRunLoop mainRunLoop] performSelector:@selector(lateDraw) target:self];
		[_private setDrawPending:true];
	}
}

- (void)scrollToX:(int)x y:(int)y
{
	[[_private scrollingDelegate] webView:self scrolledToLeft:x top:y];

	if (![_private drawPending])
	{
		[[OBRunLoop mainRunLoop] performSelector:@selector(lateDraw) target:self];
		[_private setDrawPending:true];
	}
}

- (void)setDocumentWidth:(int)width height:(int)height
{
	[[_private scrollingDelegate] webView:self changedContentsSizeToWidth:width height:height];
}

- (void)setBackForwardListDelegate:(id<WkWebViewBackForwardListDelegate>)delegate
{

}

@end
