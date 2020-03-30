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
	[view navigateTo:@"file:///System:test.html"];
}

- (void)test2
{
	[view navigateTo:@"file:///System:Applications/OWB/Resource/about.html"];
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

			MUIButton *test1, *test2;

			win.rootObject = [MUIGroup groupWithObjects:
				[MUIGroup horizontalGroupWithObjects:
					address = [MUIString stringWithContents:@"file:///"],
					test1 = [MUIButton buttonWithLabel:@"test1"],
					test2 = [MUIButton buttonWithLabel:@"test2"],
					nil],
				view, nil];
			win.title = @"Test";
			
			[address notify:@selector(acknowledge) performSelector:@selector(navigate) withTarget:app];
			[test1 notify:@selector(pressed) trigger:NO performSelector:@selector(test1) withTarget:app];
			[test2 notify:@selector(pressed) trigger:NO performSelector:@selector(test2) withTarget:app];

			[app instantiateWithWindows:win, nil];
			[win autorelease];
			win.open = YES;
//			[view navigateTo:@"file:///System:test.html"];
			[view navigateTo:@"file:///System:test.html"];

			[app run];
			
			[WkWebView shutdown];

dprintf("exiting...\n");

			win.open = NO;
		}
		
dprintf("release..\n");
		[app release];
dprintf("app release done...\n");

		CloseLibrary(FreetypeBase);
	}
	
	return 0;
}
