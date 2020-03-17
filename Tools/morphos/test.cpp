#include <exec/types.h>
#include <cstdint>
#include <WebKitLegacy/morphos/WebView.h>

#include <proto/exec.h>

struct Library *FreetypeBase;

int main(void)
{
	FreetypeBase = OpenLibrary("freetype.library", 0);
	if (FreetypeBase)
	{
		WebView view;
		CloseLibrary(FreetypeBase);
	}
	return 0;
}

