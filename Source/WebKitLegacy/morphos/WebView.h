#pragma once
#include <wtf/RefPtr.h>
#include "WebViewDelegate.h"

namespace WebCore {
	class Page;
	class Frame;
	class IntRect;
};

class WebView;
class WebViewGroup;
class WebFrame;

struct RastPort;

WebCore::Page* core(WebView *webView);
WebView *kit(WebCore::Page* page);

WebCore::Frame& mainframe(WebCore::Page& page);
const WebCore::Frame& mainframe(const WebCore::Page& page);

class WebView : public WebViewDelegate
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

    WebFrame* topLevelFrame() const { return m_mainFrame; }
	
	void drawToRP(struct RastPort *rp, const int x, const int y, const int width, const int height);
	
    static void handleRunLoop();

private:
    WebFrame* m_mainFrame { nullptr };
    WebCore::Page* m_page { nullptr };
	RefPtr<WebViewGroup> m_webViewGroup;
	bool m_transparent { false };
	bool m_usesLayeredWindow { false };
	bool m_needsDisplay { false };
};
