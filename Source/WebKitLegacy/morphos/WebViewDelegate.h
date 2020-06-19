#pragma once
#include <functional>
#include <wtf/text/WTFString.h>
#include <wtf/Vector.h>
#include <WebCore/IntRect.h>

namespace WebCore {
	class Page;
	class WindowFeatures;
	class ResourceError;
	class ResourceRequest;
};

struct WebViewDelegate
{
	std::function<void()>             _fInvalidate;
	std::function<void(int, int)>     _fScroll;
	std::function<void(int, int)>     _fSetDocumentSize;
	std::function<void()>             _fActivateNext;
	std::function<void()>             _fActivatePrevious;
	std::function<void()>             _fGoActive;

	std::function<WTF::String(const WTF::String&)>       _fUserAgentForURL;
	std::function<void(const WTF::String&)>              _fChangedTitle;
	std::function<void(const WTF::String&)>              _fChangedURL;
	std::function<void(void)>                            _fDidStartLoading;
	std::function<void(void)>                            _fDidStopLoading;
	std::function<void(void)>                            _fHistoryChanged;
	std::function<void(const WebCore::ResourceError &)>  _fDidFailWithError;
	std::function<bool(const WebCore::ResourceRequest&)> _fCanHandleRequest;

	std::function<bool(const WTF::String&, const WebCore::WindowFeatures&)> _fCanOpenWindow;
	std::function<WebCore::Page*(void)> _fDoOpenWindow;
	
	std::function<int(const WebCore::IntRect&, const WTF::Vector<WTF::String>&)> _fPopup;

	std::function<void(const WTF::String&, int level, unsigned int line)> _fConsole;
	
	std::function<void(const WTF::URL &download, const WTF::String &suggestedName)> _fDownload;

	void clearDelegateCallbacks() {
		_fInvalidate = nullptr;
		_fScroll = nullptr;
		_fSetDocumentSize = nullptr;
		_fActivateNext = nullptr;
		_fActivatePrevious = nullptr;
		_fGoActive = nullptr;
		_fUserAgentForURL = nullptr;
		_fChangedTitle = nullptr;
		_fChangedURL = nullptr;
		_fDidStartLoading = nullptr;
		_fDidStopLoading = nullptr;
		_fCanOpenWindow = nullptr;
		_fDoOpenWindow = nullptr;
		_fPopup = nullptr;
		_fHistoryChanged = nullptr;
		_fConsole = nullptr;
		_fDidFailWithError = nullptr;
		_fCanHandleRequest = nullptr;
		_fDownload = nullptr;
	};
};
