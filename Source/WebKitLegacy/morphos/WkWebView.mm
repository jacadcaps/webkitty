#undef __OBJC__
#import "WebPage.h"
#import "WebProcess.h"
#define __OBJC__

#import <ob/OBFramework.h>
#import <mui/MUIFramework.h>

#import "WkWebView.h"
#import "WebViewDelegate.h"

#import <proto/dos.h>
#import <cairo.h>

extern "C" { void dprintf(const char *, ...); }

@interface WkWebView ()

- (void)invalidated;
- (void)scrollToX:(int)x y:(int)y;
- (void)setDocumentWidth:(int)width height:(int)height;

@end

@interface WkWebViewPrivate : OBObject
{
	WTF::RefPtr<WebKit::WebPage> _page;
}
@end

@implementation WkWebViewPrivate

- (void)setPage:(WebKit::WebPage *)page
{
	_page = page;
}

- (WebKit::WebPage *)page
{
	return _page.get();
}

@end

@implementation WkWebView

static int _viewInstanceCount;
static bool _shutdown;
static bool _wasInstantiatedOnce;
static OBSignalHandler *_signalHandler;

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

+ (void)shutdown
{
	if (!_shutdown && 0 == _viewInstanceCount)
	{
		[[OBRunLoop mainRunLoop] removeSignalHandler:_signalHandler];
		[_signalHandler release];
		WebKit::WebProcess::singleton().terminate();
	}

	_shutdown = YES;
}

- (id)init
{
	if ((self = [super init]))
	{
		if (_shutdown)
		{
			[self release];
			return nil;
		}
	
		@synchronized ([WkWebView class])
		{
			_viewInstanceCount ++;

			if (!_wasInstantiatedOnce)
			{
				// MUST be done before 1st WebPage is instantiated!
				cairo_surface_t *dummysurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 4, 4);
				if (dummysurface)
					cairo_surface_destroy(dummysurface);
				
				_signalHandler = [OBSignalHandler new];
				[_signalHandler setDelegate:(id)[self class]];
				[[OBRunLoop mainRunLoop] addSignalHandler:_signalHandler];
				
				[OBScheduledTimer scheduledTimerWithInterval:0.1 perform:[OBPerform performSelector:@selector(fire) target:[self class]] repeats:YES];
				
				WebKit::WebProcess::singleton().initialize(int([_signalHandler sigBit]));
				
				_wasInstantiatedOnce = true;
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
			auto webProcess = WebKit::WebProcess::singleton();
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

			webPage->_setDocumentSize = [self](int width, int height) {
				[self setDocumentWidth:width height:height];
			};
			
			webPage->_fScroll = [self](int x, int y) {
				[self scrollToX:x y:y];
			};
			
			webPage->_fActivateNext = [self]() {
				[[self windowObject] setActiveObjectSpecial:MUIV_Window_ActiveObject_Next];
			};

dprintf("done, webpage %p, %p\n", webPage, webProcess.webPage(identifier));

		} catch (...) {
dprintf("kill me\n");
			[self release];
			return nil;
		}
	}
	
	return self;
}

- (void)dealloc
{
	@synchronized ([WkWebView class])
	{
		_viewInstanceCount --;
	}

	try {
		auto webPage = [_private page];
		WebKit::WebProcess::singleton().removeWebPage(PAL::SessionID(), webPage->pageID());
	} catch (...) {};

	[_private release];

	[super dealloc];
}

- (Boopsiobject *)instantiateTagList:(struct TagItem *)tags
{

#define MADF_KNOWSACTIVE       (1<< 7) /* sigh */

	Boopsiobject *meBoopsi = [super instantiateTagList:tags];
	// prevent MUI active frame from being drawn
	_flags(meBoopsi) |= MADF_KNOWSACTIVE;
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

	_isActive = true;
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
	_isActive = false;
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
	if (_drawPending)
	{
		[self redraw:MADF_DRAWUPDATE];
		_drawPending = NO;
	}
}

- (void)invalidated
{
	if (!_drawPending)
	{
		[[OBRunLoop mainRunLoop] performSelector:@selector(lateDraw) target:self];
		_drawPending = YES;
	}
}

- (void)scrollToX:(int)x y:(int)y
{
	// update scrollers...

	if (!_drawPending)
	{
		[[OBRunLoop mainRunLoop] performSelector:@selector(lateDraw) target:self];
		_drawPending = YES;
	}
}

- (void)setDocumentWidth:(int)width height:(int)height
{
	// update scrollers...
}

- (void)navigateTo:(OBString *)uri
{
	const char *curi = [uri cString];
//	dprintf("%s:%d\n", __PRETTY_FUNCTION__, __LINE__);

	try {
		auto webPage = [_private page];
		webPage->go(curi);
	} catch (std::exception &ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
}

- (void)dumpDebug
{
	WebKit::WebProcess::singleton().dumpWebCoreStatistics();
}

@end

#if 0
@implementation WkWebViewSupport

- (void)fire
{
	static bool firstFire;
	if (!firstFire)
	{
		dprintf("%s:%d\n", __PRETTY_FUNCTION__, __LINE__);
		firstFire = YES;
	}

	try {
		WebPage::handleRunLoop();
	} catch (std::exception &ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
}

- (id)init
{
	if ((self = [super init]))
	{
		// cairo initializer hack, lame
		cairo_surface_t *dummysurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 4, 4);
		if (dummysurface)
		{
				cairo_surface_destroy(dummysurface);
		}
		else
		{
			[self release];
			return nil;
		}

		_timer = [OBScheduledTimer scheduledTimerWithInterval:0.1 perform:[OBPerform performSelector:@selector(fire) target:self] repeats:YES runLoop:[OBRunLoop mainRunLoop]];
	}
	
	return self;
}

- (void)stop
{
	[_timer invalidate];
	_timer = nil;
}

@end
#endif

