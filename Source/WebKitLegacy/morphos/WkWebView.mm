#import <ob/OBFramework.h>
#import <mui/MUIFramework.h>

#import "WkWebView.h"
#import "WebViewDelegate.h"
#import "WebView.h"

#import <proto/dos.h>

extern "C" { void dprintf(const char *, ...); }

@interface WkWebView ()

- (void)invalidateRectX:(LONG)x y:(LONG)y width:(LONG)w height:(LONG)h;

@end

@interface WkWebViewSupport : OBObject
{
	OBScheduledTimer *_timer;
}

- (void)stop;

@end

@implementation WkWebView

static WkWebViewSupport *_support;
static int _viewInstanceCount;

- (id)init
{
	if ((self = [super init]))
	{
		_webView = new WebView();
		_webView->_fInvalidate = [self]() { [self redraw:MADF_DRAWOBJECT]; };
		
		self.fillArea = NO;

		_webView->_fInvalidateRect = [self](int x, int y, int width, int height) {
			[self invalidateRectX:x y:y width:width height:height];
		};
		
		@synchronized ([WkWebView class])
		{
			_viewInstanceCount ++;

			if (1 == _viewInstanceCount)
			{
				_support = [WkWebViewSupport new];
			}
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
			[_support stop];
			[_support release];
			_support = nil;
		}
	}

	delete _webView;
	[super dealloc];
}

- (void)askMinMax:(struct MUI_MinMax *)minmaxinfo
{
	[super askMinMax:minmaxinfo];

	minmaxinfo->MinWidth += 800;
	minmaxinfo->MinHeight += 600;
	minmaxinfo->DefWidth += 800;
	minmaxinfo->DefHeight += 600;
	minmaxinfo->MaxWidth += 800;
	minmaxinfo->MaxHeight += 600;
}

- (BOOL)draw:(ULONG)flags
{
	[super draw:flags];
	_webView->drawToRP([self rastPort], [self left], [self top], [self innerWidth], [self innerHeight]);
	Delay(200);
	return TRUE;
}

- (void)invalidateRectX:(LONG)x y:(LONG)y width:(LONG)w height:(LONG)h
{
	[self redraw:MADF_DRAWUPDATE];
}

- (void)navigateTo:(OBString *)uri
{
	const char *curi = [uri cString];
	dprintf("%s:%d\n", __PRETTY_FUNCTION__, __LINE__);
	_webView->go(curi);
}

@end

@implementation WkWebViewSupport

- (void)fire
{
	static bool firstFire;
	if (!firstFire)
	{
		dprintf("%s:%d\n", __PRETTY_FUNCTION__, __LINE__);
		firstFire = YES;
	}

	WebView::handleRunLoop();
}

- (id)init
{
	if ((self = [super init]))
	{
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
