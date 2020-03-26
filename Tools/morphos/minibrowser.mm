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
			WkWebView *view = [[WkWebView new] autorelease];
dprintf("webview %p\n", view);

			win.rootObject = view;
			win.title = @"Test";
			
			[app instantiateWithWindows:win, nil];
			[win autorelease];
			win.open = YES;
			[view navigateTo:@"file:///System:test.html"];

			[app run];

dprintf("exiting...\n");

			win.open = NO;
		}
		
		[app release];
		
		CloseLibrary(FreetypeBase);
	}
	
	return 0;
}
