#import <ob/OBFramework.h>
#import <mui/MUIFramework.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <cairo.h>
#include <stdio.h>

#include <signal.h>
#include <locale.h>

#include <algorithm>

unsigned long __stack = 2 * 1024 * 1024;

extern "C" {
        void dprintf(const char *fmt, ...);
};

#import <WebKitLegacy/morphos/WkWebView.h>
#import <WebKitLegacy/morphos/WkSettings.h>

@interface BrowserWindow : MUIWindow<WkWebViewNetworkDelegate, WkWebViewBackForwardListDelegate>
{
	WkWebView *_view;
 	MUIString *_address;
	MUICycle  *_userAgents;
	MUIGroup  *_topGroup;
	MUIGroup  *_bottomGroup;
	MUIGroup  *_loading;
	MUIButton *_back;
	MUIButton *_forward;
	MUIButton *_stop;
	MUIButton *_reload;
	MUICheckmark *_adBlock;
	MUICheckmark *_script;
}
@end

@interface BrowserGroup : MUIGroup<WkWebViewScrollingDelegate>
{
	WkWebView *_view;
	MUIScrollbar *_horiz;
	MUIScrollbar *_vert;
	int           _documentWidth;
	int           _documentHeight;
	int           _viewWidth;
	int           _viewHeight;
	int           _horizStep;
	int           _vertStep;
	bool          _needsUpdate;
}

- (void)doScroll;

@end

@implementation BrowserGroup

- (id)initWithWkWebView:(WkWebView *)view
{
	MUIGroup *inner;
	if ((self = [super initHorizontalWithObjects:
		inner = [MUIGroup groupWithObjects:_view = view, _horiz = [MUIScrollbar horizontalScrollbar], nil],
		_vert = [MUIScrollbar verticalScrollbar],
		nil]))
	{
		[inner setInnerLeft:0];
		[inner setInnerRight:0];
		
		[_vert notify:@selector(first) performSelector:@selector(doScroll) withTarget:self];
		[_horiz notify:@selector(first) performSelector:@selector(doScroll) withTarget:self];
		
		[_view setScrollingDelegate:self];
		
		[_vert setDeltaFactor:10];
		[_horiz setDeltaFactor:10];
	}
	
	return self;
}

+ (id)browserGroupWithWkWebView:(WkWebView *)view
{
	return [[[self alloc] initWithWkWebView:view] autorelease];
}

- (void)updateScrollers
{
	_needsUpdate = false;
	
	if (_documentWidth > 0)
	{
		if (_documentWidth >= _viewWidth)
		{
			[_horiz noNotifySetEntries:_documentWidth];
			[_horiz noNotifySetPropVisible:_viewWidth];
		}
		else
		{
			[_horiz noNotifySetEntries:1];
		}
		
		if (_documentHeight >= _viewHeight)
		{
			[_vert noNotifySetEntries:_documentHeight];
			[_vert noNotifySetPropVisible:_viewHeight];
		}
		else
			[_vert noNotifySetEntries:1];
	}
	else
	{
		[_vert noNotifySetEntries:1];
		[_horiz noNotifySetEntries:1];
	}
}

- (void)webView:(WkWebView *)view changedContentsSizeToWidth:(int)width height:(int)height
{
	_documentWidth = width;
	_documentHeight = height;
	if (!_needsUpdate)
	{
		_needsUpdate = true;
		[[OBRunLoop mainRunLoop] performSelector:@selector(updateScrollers) target:self];
	}
}

- (void)webView:(WkWebView *)view scrolledToLeft:(int)left top:(int)top
{
	[_horiz noNotifySetFirst:left];
	[_vert noNotifySetFirst:top];
}

- (void)doScroll
{
	[_view scrollToLeft:[_horiz first] top:[_vert first]];
}

- (BOOL)show:(struct LongRect *)clip
{
	if ([super show:clip])
	{
		_viewWidth = [_view innerWidth];
		_viewHeight = [_view innerHeight];

		if (!_needsUpdate)
		{
			_needsUpdate = true;
			[[OBRunLoop mainRunLoop] performSelector:@selector(updateScrollers) target:self];
		}

		return YES;
	}
	
	return NO;
}

@end


@interface MiniMenuitem : MUIMenuitem

+ (id)itemWithTitle:(OBString *)title shortcut:(OBString *)shortcut selector:(SEL)selector;
+ (id)checkmarkItemWithTitle:(OBString *)title shortcut:(OBString *)shortcut selector:(SEL)selector;

@end

@implementation MiniMenuitem

+ (id)itemWithTitle:(OBString *)title shortcut:(OBString *)shortcut selector:(SEL)selector
{
	return [[[self alloc] initWithTitle:title shortcut:shortcut selector:selector] autorelease];
}

+ (id)checkmarkItemWithTitle:(OBString *)title shortcut:(OBString *)shortcut selector:(SEL)selector
{
	MiniMenuitem *item = [[[self alloc] initWithTitle:title shortcut:shortcut selector:selector] autorelease];
	item.checkit = YES;
	return item;
}

- (void)about
{
	MUIApplication *app = [MUIApplication currentApplication];
	OBArray *windows = [app objects];
	
	for (int i = 0; i < [windows count]; i++)
	{
		MUIWindow *window = [windows objectAtIndex:i];
		if ([window isKindOfClass:[MCCAboutbox class]])
		{
			window.open = YES;
			return;
		}
	}
	
	MCCAboutbox *about = [MCCAboutbox new];
	[app addObject:about];
	about.open = YES;
}

- (void)quit
{
	[[MUIApplication currentApplication] quit];
}

- (void)newWindow
{
	BrowserWindow *w = [[BrowserWindow new] autorelease];
	[[MUIApplication currentApplication] addObject:w];
	[w setOpen:YES];
}

@end

@implementation BrowserWindow

static int _windowID = 1;

- (void)navigate
{
	[_view load:[OBURL URLWithString:[_address contents]]];
}

- (void)postClose
{
	OBEnumerator *e = [[[MUIApplication currentApplication] objects] objectEnumerator];
	MUIWindow *win;

	while ((win = [e nextObject]))
	{
		if ([win isKindOfClass:[self class]] && win != self)
		{
			[[MUIApplication currentApplication] removeObject:self];
			return;
		}
	}

	[[OBRunLoop currentRunLoop] performSelector:@selector(quit) target:[MUIApplication currentApplication]];
}

- (void)doClose
{
	[self setOpen:NO];
	[[OBRunLoop mainRunLoop] performSelector:@selector(postClose) target:self];
}

- (void)navigateTo:(OBString *)to
{
	[_view load:[OBURL URLWithString:to]];
}

- (void)settingsUpdated
{
	WkSettings *settings = [WkSettings settings];
	[settings setJavaScriptEnabled:[_script selected]];
	[settings setAdBlockerEnabled:[_adBlock selected]];
	[_view setSettings:settings];
}

- (id)initWithView:(WkWebView *)view
{
	if ((self = [super init]))
	{
		MUIButton *button;
		MUIButton *debug;

		self.rootObject = [MUIGroup groupWithObjects:
			_topGroup = [MUIGroup horizontalGroupWithObjects:
				_back = [MUIButton buttonWithLabel:@"\33I[5:PROGDIR:MiniResources/icons8-go-back-20.png]"],
				_forward = [MUIButton buttonWithLabel:@"\33I[5:PROGDIR:MiniResources/icons8-circled-right-20.png]"],
				_stop = [MUIButton buttonWithLabel:@"\33I[5:PROGDIR:MiniResources/icons8-no-entry-20.png]"],
				_reload = [MUIButton buttonWithLabel:@"\33I[5:PROGDIR:MiniResources/icons8-restart-20.png]"],
				_address = [MUIString stringWithContents:@"https://"],
				nil],
			[BrowserGroup browserGroupWithWkWebView:_view = view],
			_bottomGroup = [MUIGroup horizontalGroupWithObjects:
				_userAgents = [MUICycle cycleWithEntries:[OBArray arrayWithObjects:@"Safari", @"Chrome", @"WebKitty", @"iPad 12.2", @"IE10", nil]],
				debug = [MUIButton buttonWithLabel:@"Debug Stats"],
				[MUICheckmark checkmarkWithLabel:@"AdBlocker" checkmark:&_adBlock],
				[MUICheckmark checkmarkWithLabel:@"JS" checkmark:&_script],
				[MUIRectangle rectangleWithWeight:300],
				_loading = [MUIGroup groupWithPages:[MUIRectangle rectangleWithWeight:20], [[MCCBusy new] autorelease], nil],
				nil],
			nil];
		self.title = @"Test";
		self.iD = _windowID++;

		[self notify:@selector(closeRequest) performSelector:@selector(doClose) withTarget:self];

		[_address notify:@selector(acknowledge) performSelector:@selector(navigate) withTarget:self];
		[_address setWeight:300];
		[_address setMaxLen:4000];
		
		[_view setNetworkDelegate:self];
		[_view setBackForwardListDelegate:self];

		[_back notify:@selector(pressed) trigger:NO performSelector:@selector(goBack) withTarget:_view];
		[_forward notify:@selector(pressed) trigger:NO performSelector:@selector(goForward) withTarget:_view];
		[_stop notify:@selector(pressed) trigger:NO performSelector:@selector(stopLoading) withTarget:_view];
		[_reload notify:@selector(pressed) trigger:NO performSelector:@selector(reload) withTarget:_view];
		
		[_back setHorizWeight:0];
		[_forward setHorizWeight:0];
		[_stop setHorizWeight:0];
		[_reload setHorizWeight:0];
		
		[_adBlock setSelected:YES];
		[_script setSelected:YES];
		
		[_adBlock notify:@selector(selected) performSelector:@selector(settingsUpdated) withTarget:self];
		[_script notify:@selector(selected) performSelector:@selector(settingsUpdated) withTarget:self];

		[_back setDisabled:YES];
		[_forward setDisabled:YES];

		#define ADDBUTTON(__title__, __address__) \
			[_topGroup addObject:button = [MUIButton buttonWithLabel:__title__]]; \
			[button notify:@selector(pressed) trigger:NO performSelector:@selector(navigateTo:) withTarget:self withObject:__address__];

		ADDBUTTON(@"Ggle", @"https://www.google.com");
		ADDBUTTON(@"ReCaptcha", @"https://patrickhlauke.github.io/recaptcha/");
		ADDBUTTON(@"BBC", @"https://www.bbc.com/news/");
		ADDBUTTON(@"MZone", @"https://morph.zone/");
		ADDBUTTON(@"HTML5", @"http://html5test.com");
		ADDBUTTON(@"WhatsApp", @"https://web.whatsapp.com");
		ADDBUTTON(@"Teleg", @"https://web.telegram.org");
		ADDBUTTON(@"Gif", @"https://media.giphy.com/media/dC4FTacOCkOKRYIRqw/source.gif");

		[debug notify:@selector(pressed) trigger:NO performSelector:@selector(dumpDebug) withTarget:_view];

		[self setMenustrip:[MUIMenustrip menustripWithObjects:
			[MUIMenu menuWithTitle:@"MiniBrowser" objects:
				[MiniMenuitem itemWithTitle:@"New Window..." shortcut:@"N" selector:@selector(newWindow)],
				[MUIMenuitem barItem],
				[MiniMenuitem itemWithTitle:@"About..." shortcut:@"?" selector:@selector(about)],
				[MiniMenuitem itemWithTitle:@"Quit" shortcut:@"Q" selector:@selector(quit)],
				nil],
			nil]];
	
	}
	
	return self;
}

- (id)init
{
	return [self initWithView:[[WkWebView new] autorelease]];
}

- (void)dealloc
{
	dprintf("%s\n", __PRETTY_FUNCTION__);
	[_view setNetworkDelegate:nil];
	[_view setScrollingDelegate:nil];
	[super dealloc];
}

- (OBString *)userAgentForURL:(OBString *)url
{
	switch ([_userAgents active])
	{
	case 1:
		return @"Mozilla/5.0 (Macintosh; Intel Mac OS X 10_13_6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.3987.100 Safari/537.36";
	case 2:
		return @"Mozilla/5.0 (MorphOS; PowerPC 3_14) WebKitty/605.1.15 (KHTML, like Gecko)";
	case 3:
		return @"Mozilla/5.0 (iPad; CPU OS 12_2 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/12.1 Mobile/15E148 Safari/604.1";
	case 4:
		return @"Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.2)";
	}

	return nil;
}

- (void)webView:(WkWebView *)view changedTitle:(OBString *)newtitle
{
	[self setTitle:newtitle];
}

- (void)webView:(WkWebView *)view changedDocumentURL:(OBString *)newurl
{
	[_address noNotifySetContents:newurl];
}

- (void)webViewDidStartProvisionalLoading:(WkWebView *)view
{
	[_loading setActivePage:1];
}

- (void)webViewDidFinishProvisionalLoading:(WkWebView *)view
{
	[_loading setActivePage:0];
}

- (BOOL)webView:(WkWebView *)view wantsToCreateNewViewWithURL:(OBString *)url options:(OBDictionary *)options
{
	return YES;
}

- (void)webView:(WkWebView *)view createdNewWebView:(WkWebView *)newview
{
	BrowserWindow *newWindow = [[[BrowserWindow alloc] initWithView:newview] autorelease];
	[[MUIApplication currentApplication] addObject:newWindow];
	[newWindow setOpen:YES];
}

- (void)webViewChangedBackForwardList:(WkWebView *)view
{
	[_back setDisabled:![view canGoBack]];
	[_forward setDisabled:![_view canGoForward]];
}

@end

@interface MiniAppDelegate : OBObject<OBApplicationDelegate>
@end

@implementation MiniAppDelegate

- (void)dealloc
{
	dprintf("%s\n", __PRETTY_FUNCTION__);
	[super dealloc];
}

- (void)applicationWillRun
{
	dprintf("%s\n", __PRETTY_FUNCTION__);
}

- (BOOL)applicationShouldTerminate
{
	OBArray *allWindows = [[MUIApplication currentApplication] objects];
	OBEnumerator *e = [allWindows objectEnumerator];
	MUIWindow *w;
	
	while ((w = [e nextObject]))
	{
		[w setOpen:NO];
	}
	
	[[MUIApplication currentApplication] removeAllObjects];

	BOOL shouldTerminate = [WkWebView readyToQuit];
	dprintf("%s %d\n", __PRETTY_FUNCTION__, shouldTerminate);
	return shouldTerminate;
}

- (void)applicationDidTerminate
{
	dprintf("%s\n", __PRETTY_FUNCTION__);
}

@end

#define VERSION   "$VER: MiniBrowser 1.0 (19.04.2020) (C)2020 Jacek Piszczek, Harry Sintonen / MorphOS Team"
#define COPYRIGHT @"2020 Jacek Piszczek, Harry Sintonen / MorphOS Team"

int muiMain(int argc, char *argv[])
{
    signal(SIGINT, SIG_IGN);
	setlocale(LC_TIME, "C");
	setlocale(LC_NUMERIC, "C");
	setlocale(LC_CTYPE, "en-US");

	MUIApplication *app = [MUIApplication new];

	if (app)
	{
		MiniAppDelegate *delegate = [MiniAppDelegate new];
		[app setBase:@"WEKBITTY"];

		app.title = @"WebKitty MiniBrowser";
		app.author = @"Jacek Piszczek, Harry Sintonen";
		app.copyright = COPYRIGHT;
		app.applicationVersion = [OBString stringWithCString:VERSION encoding:MIBENUM_ISO_8859_1];
		[[OBApplication currentApplication] setDelegate:delegate];

		MUIWindow *win = [[BrowserWindow new] autorelease];
		[app instantiateWithWindows:win, nil];

		win.open = YES;

		[app run];
		
		dprintf("%s: runloop exited!\n", __PRETTY_FUNCTION__);
		
		[[OBApplication currentApplication] setDelegate:nil];
		[delegate release];
	}
	
dprintf("release..\n");
	[app release];

dprintf("destructors be called next\n");
	return 0;
}
