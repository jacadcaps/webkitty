#import <ob/OBFramework.h>
#import <mui/MUIFramework.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <cairo.h>
#include <stdio.h>

unsigned long __stack = 2 * 1024 * 1024;

extern "C" {
        void dprintf(const char *fmt, ...);
};

#import <WebKitLegacy/morphos/WkWebView.h>
#import <WebKitLegacy/morphos/WebView.h>

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
dprintf("muimain!\n");
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

dprintf("instantiate webview..\n");
			view = [[WkWebView new] autorelease];
dprintf("webview %p\n", view);

			MUIGroup *bug;
			MUIButton *button;

			win.rootObject = [MUIGroup groupWithObjects:
				bug = [MUIGroup horizontalGroupWithObjects:
					address = [MUIString stringWithContents:@"https://"],
					nil],
				view, nil];
			win.title = @"Test";
			
			[address notify:@selector(acknowledge) performSelector:@selector(navigate) withTarget:app];
			
			#define ADDBUTTON(__title__, __address__) \
				[bug addObject:button = [MUIButton buttonWithLabel:__title__]]; \
				[button notify:@selector(pressed) trigger:NO performSelector:@selector(navigateTo:) withTarget:view withObject:__address__];

			ADDBUTTON(@"Ggle", @"https://www.google.com");
			ADDBUTTON(@"WIMB", @"https://www.whatsmybrowser.org");
			ADDBUTTON(@"ReCaptcha", @"https://patrickhlauke.github.io/recaptcha/");
			ADDBUTTON(@"BBC", @"https://www.bbc.com/news/");

			[app instantiateWithWindows:win, nil];
			[win autorelease];
			win.open = YES;

			[app run];

dprintf("shutdown...\n");

			[WkWebView shutdown];

dprintf("exiting...\n");

			win.open = NO;
		}
		
dprintf("release..\n");
		[app release];
dprintf("app release done...\n");

		CloseLibrary(FreetypeBase);
	}
dprintf("destructors be called next\n");
	return 0;
}
