#pragma once
#include <functional>
#include <wtf/text/WTFString.h>

namespace WebCore {
	class Page;
	class WindowFeatures;
};

struct WebViewDelegate
{
	std::function<void()>             _fInvalidate;
	std::function<void(int, int)>     _fScroll;
	std::function<void(int, int)>     _setDocumentSize;
	std::function<void(int&, int&)>   _getViewSize;
	std::function<void()>             _fActivateNext;
	std::function<void()>             _fActivatePrevious;
	std::function<void()>             _fGoActive;

	std::function<WTF::String(const WTF::String&)> _fUserAgentForURL;
	std::function<void(const WTF::String&)>        _fChangedTitle;
	std::function<void(const WTF::String&)>        _fChangedURL;
	std::function<void(void)>                      _fDidStartLoading;
	std::function<void(void)>                      _fDidStopLoading;

	std::function<bool(const WTF::String&, const WebCore::WindowFeatures&)> _fCanOpenWindow;
	std::function<WebCore::Page*(void)> _fDoOpenWindow;
};
