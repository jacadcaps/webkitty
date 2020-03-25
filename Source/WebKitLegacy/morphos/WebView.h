#pragma once
#include <wtf/RefPtr.h>

namespace WebCore {
	class Page;
	class Frame;
	class IntRect;
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

    void repaint(const WebCore::IntRect&, bool contentChanged, bool immediate = false, bool repaintContentOnly = false);

    void closeWindow();
    void closeWindowSoon();
    void closeWindowTimerFired();

    bool transparent() const { return m_transparent; }
    bool usesLayeredWindow() const { return m_usesLayeredWindow; }
    bool needsDisplay() const { return m_needsDisplay; }

    // Convenient to be able to violate the rules of COM here for easy movement to the frame.
    WebFrame* topLevelFrame() const { return m_mainFrame; }

private:
    WebFrame* m_mainFrame { nullptr };
    WebCore::Page* m_page { nullptr };
	RefPtr<WebViewGroup> m_webViewGroup;
	bool m_transparent { false };
	bool m_usesLayeredWindow { false };
	bool m_needsDisplay { false };
};
