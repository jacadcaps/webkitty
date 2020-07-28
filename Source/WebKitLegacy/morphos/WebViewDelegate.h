#pragma once
#include <functional>
#include <wtf/text/WTFString.h>
#include <wtf/Vector.h>
#include <WebCore/IntRect.h>
#include <WebCore/FrameLoaderClient.h>

namespace WebCore {
	class Page;
	class WindowFeatures;
	class ResourceError;
	class ResourceRequest;
	class FileChooser;
	class ResourceResponse;
	class PolicyCheckIdentifier;
};

enum class WebViewDelegateOpenWindowMode
{
	Default,
	BackgroundTab,
	NewWindow
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
	std::function<void()>                                _fDidLoadInsecureContent;

	std::function<bool(const WTF::String&, const WebCore::WindowFeatures&)>      _fCanOpenWindow;
	std::function<WebCore::Page*(void)>                                          _fDoOpenWindow;
	std::function<void(const WTF::URL& url, WebViewDelegateOpenWindowMode mode)> _fNewTabWindow;
	
	std::function<int(const WebCore::IntRect&, const WTF::Vector<WTF::String>&)> _fPopup;

	std::function<void(const WTF::String&, int level, unsigned int line)>        _fConsole;
	
	std::function<void(const WTF::URL &download, const WTF::String &suggestedName)> _fDownload;
	std::function<void(WebCore::ResourceHandle*, const WebCore::ResourceRequest&, const WebCore::ResourceResponse&)> _fDownloadFromResource;
	std::function<void(const WebCore::ResourceResponse& response, const WebCore::ResourceRequest& request, WebCore::PolicyCheckIdentifier identifier, const WTF::String& downloadAttribute, WebCore::FramePolicyFunction&& function)> _fDownloadAsk;
	
	std::function<void(const WTF::String &)>                                      _fAlert;
	std::function<bool(const WTF::String &)>                                      _fConfirm;
	std::function<bool(const WTF::String &, const WTF::String &, WTF::String &) > _fPrompt;
	std::function<void(WebCore::FileChooser&)>                                    _fFile;

	std::function<void()> _fHasAutofill;
	std::function<void(const WTF::String &l, const WTF::String &p)> _fStoreAutofill;

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
		_fDidLoadInsecureContent = nullptr;
		_fAlert = nullptr;
		_fConfirm = nullptr;
		_fPrompt = nullptr;
		_fFile = nullptr;
		_fDownloadAsk = nullptr;
		_fDownloadFromResource = nullptr;
		_fHasAutofill = nullptr;
		_fStoreAutofill = nullptr;
		_fNewTabWindow = nullptr;
	};
};
