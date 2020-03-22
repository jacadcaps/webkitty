#pragma once
#include <wtf/RefPtr.h>

namespace WebCore {
	class Page;
	class Frame;
};

class WebView;
class WebViewGroup;
class WebFrame;

WebCore::Page* core(WebView *webView);
WebView *kit(WebCore::Page* page);

WebCore::Frame& mainframe(WebCore::Page& page);
const WebCore::Frame& mainframe(const WebCore::Page& page);

class WebView
{
public:
	WebView();
	~WebView();

	WebCore::Page *page();

	void go(const char *url);

private:
    WebFrame* m_mainFrame { nullptr };
    WebCore::Page* m_page { nullptr };
	RefPtr<WebViewGroup> m_webViewGroup;
};
