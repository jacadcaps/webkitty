#include <exec/types.h>
#include <cstdint>
#include <WebKitLegacy/morphos/WebView.h>
#include <WebKitLegacy/morphos/WebFrame.h>

#include <proto/exec.h>
#include <stdio.h>

unsigned long __stack = 2 * 1024 * 1024;

struct Library *FreetypeBase;

int main(void)
{
	printf("Hello\n");
	FreetypeBase = OpenLibrary("freetype.library", 0);
	if (FreetypeBase)
	{
                printf("Creating frame...\n");
		WebFrame *frame = new WebFrame();
                printf("Creating webview...\n");
		WebView *view = new WebView();
		printf("page %p\n", view->page());
		CloseLibrary(FreetypeBase);
	}
	return 0;
}

