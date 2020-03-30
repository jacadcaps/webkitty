#import <ob/OBFramework.h>
#import <mui/MUIFramework.h>

#import "WkWebView.h"
#import "WebViewDelegate.h"
#import "WebView.h"

#import <proto/dos.h>
#import <cairo.h>

extern "C" { void dprintf(const char *, ...); }

@interface WkWebView ()

- (void)invalidated;
- (void)scrollToX:(int)x y:(int)y;
- (void)setDocumentWidth:(int)width height:(int)height;

@end

@implementation WkWebView

static int _viewInstanceCount;
static bool _shutdown;
static OBScheduledTimer *_timer;

+ (void)fire
{
	WebView::handleRunLoop();
}

+ (void)shutdown
{
	_shutdown = YES;
	if (0 == _viewInstanceCount)
		WebView::shutdown();
}

- (id)init
{
	if ((self = [super init]))
	{
		@synchronized ([WkWebView class])
		{
			_viewInstanceCount ++;

			if (1 == _viewInstanceCount)
			{
				// MUST be done before 1st WebView is instantiated!
				cairo_surface_t *dummysurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 4, 4);
				if (dummysurface)
					cairo_surface_destroy(dummysurface);
				
				_timer = [[OBScheduledTimer scheduledTimerWithInterval:0.1 perform:[OBPerform performSelector:@selector(fire) target:[self class]] repeats:YES runLoop:[OBRunLoop mainRunLoop]] retain];
			}
		}

		self.fillArea = NO;
		self.handledEvents = IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_RAWKEY | IDCMP_MOUSEHOVER;

		try {
			_webView = new WebView();

			_webView->_fInvalidate = [self]() { [self invalidated]; };

			_webView->_setDocumentSize = [self](int width, int height) {
				[self setDocumentWidth:width height:height];
			};
			
			_webView->_fScroll = [self](int x, int y) {
				[self scrollToX:x y:y];
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
	@synchronized ([WkWebView class])
	{
		_viewInstanceCount --;
		
		if (0 == _viewInstanceCount)
		{
			[_timer invalidate];
			[_timer release];
			_timer = nil;
			
			if (_shutdown)
			{
				WebView::shutdown();
			}
		}
	}

	try {
		delete _webView;
	} catch (...) {};

	[super dealloc];
}

- (void)askMinMax:(struct MUI_MinMax *)minmaxinfo
{
	[super askMinMax:minmaxinfo];

	minmaxinfo->MinWidth += 50;
	minmaxinfo->MinHeight += 50;
	minmaxinfo->DefWidth += 800;
	minmaxinfo->DefHeight += 600;
	minmaxinfo->MaxWidth = MUI_MAXMAX;
	minmaxinfo->MaxHeight = MUI_MAXMAX;
}

- (BOOL)draw:(ULONG)flags
{
	[super draw:flags];
	
	LONG iw = [self innerWidth];
	LONG ih = [self innerHeight];
	
	try {
		_webView->setVisibleSize(int(iw), int(ih));
		_webView->draw([self rastPort], [self left], [self top], iw, ih, MADF_DRAWUPDATE == (MADF_DRAWUPDATE & flags));
	} catch (std::exception &ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}

	return TRUE;
}

- (ULONG)handleEvent:(struct IntuiMessage *)imsg muikey:(LONG)muikey
{
	if (imsg)
	{
		return _webView->handleIntuiMessage(imsg, [self mouseX:imsg], [self mouseY:imsg], [self isInObject:imsg]) ?
			MUI_EventHandlerRC_Eat : 0;
	}
	
	return 0;
}

- (void)lateDraw
{
	[self redraw:MADF_DRAWUPDATE];
	_drawPending = NO;
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
		_webView->go(curi);
	} catch (std::exception &ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
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
		WebView::handleRunLoop();
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

