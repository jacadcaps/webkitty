#pragma once

namespace WebCore {
	class Page;
	class Frame;
};

class WebView;

WebCore::Page* core(WebView *webView);
WebView *kit(WebCore::Page* page);

WebCore::Frame& mainframe(WebCore::Page& page);
const WebCore::Frame& mainframe(const WebCore::Page& page);

class WebView
{
public:
	WebView();

	WebCore::Page *page();

private:
	WebCore::Page *m_page;
};
