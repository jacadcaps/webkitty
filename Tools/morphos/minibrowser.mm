#import <ob/OBFramework.h>
#import <mui/MUIFramework.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <cairo.h>
#include <stdio.h>

#include <signal.h>
#include <locale.h>

unsigned long __stack = 2 * 1024 * 1024;

extern "C" {
        void dprintf(const char *fmt, ...);
};

#import <WebKitLegacy/morphos/WkWebView.h>
//#import <WebKitLegacy/morphos/WebPage.h>

struct Library *FreetypeBase;

static WkWebView *view;
static MUIString *address;

@interface MUIApplication (Addons)
@end

@implementation MUIApplication (Addons)

- (void)test1
{
	[view navigateTo:@"https://www.google.com"];
}

- (void)test2
{
	[view navigateTo:@"https://www.whatsmybrowser.org"];
}

- (void)navigate
{
	[view navigateTo:[address contents]];
}

@end

int muiMain(int argc, char *argv[])
{
    signal(SIGINT, SIG_IGN);
	setlocale(LC_TIME, "C");
	setlocale(LC_NUMERIC, "C");
	setlocale(LC_CTYPE, "en-US");

	FreetypeBase = OpenLibrary("freetype.library", 0);
	if (FreetypeBase)
	{
		// Hack to make sure cairo mutex are initialized
		// TODO: fix this in the fucking cairo
		cairo_surface_t *dummysurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 4, 4);
		if (dummysurface)
				cairo_surface_destroy(dummysurface);

		MUIApplication *app = [MUIApplication new];
	
		if (app)
		{
			MUIWindow *win = [MUIWindow new];

			view = [[WkWebView new] autorelease];

			MUIGroup *bug;
			MUIButton *button;
			MUIButton *debug;

			win.rootObject = [MUIGroup groupWithObjects:
				bug = [MUIGroup horizontalGroupWithObjects:
					address = [MUIString stringWithContents:@"https://"],
					nil],
				view,
				[MUIGroup horizontalGroupWithObjects:
					[MUIRectangle rectangleWithWeight:300],
					debug = [MUIButton buttonWithLabel:@"Debug Stats"],
					nil],
				nil];
			win.title = @"Test";
			win.iD = 1;
			
			[address notify:@selector(acknowledge) performSelector:@selector(navigate) withTarget:app];
			[address setWeight:300];
			
			#define ADDBUTTON(__title__, __address__) \
				[bug addObject:button = [MUIButton buttonWithLabel:__title__]]; \
				[button notify:@selector(pressed) trigger:NO performSelector:@selector(navigateTo:) withTarget:view withObject:__address__];

			ADDBUTTON(@"Ggle", @"https://www.google.com");
			ADDBUTTON(@"WIMB", @"https://www.whatsmybrowser.org");
			ADDBUTTON(@"Cookies", @"http://browsercookielimits.squawky.net");
			ADDBUTTON(@"ReCaptcha", @"https://patrickhlauke.github.io/recaptcha/");
			ADDBUTTON(@"BBC", @"https://www.bbc.com/news/");
			ADDBUTTON(@"MZone", @"https://morph.zone/");
			ADDBUTTON(@"HTML5", @"http://html5test.com");

			[debug notify:@selector(pressed) trigger:NO performSelector:@selector(dumpDebug) withTarget:view];

			[app setBase:@"WEKBITTY"];
			[app instantiateWithWindows:win, nil];
			[win autorelease];
			win.open = YES;

			[app run];

			win.open = NO;

dprintf("exiting...\n");

		}
		
		[WkWebView shutdown];

dprintf("release..\n");
		[app release];
dprintf("app release done...\n");

		CloseLibrary(FreetypeBase);
	}
dprintf("destructors be called next\n");
	return 0;
}
