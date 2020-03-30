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
class WebViewDrawContext;
class WebChromeClient;

struct RastPort;
struct IntuiMessage;

WebCore::Page* core(WebView *webView);
WebView *kit(WebCore::Page* page);

WebCore::Frame& mainframe(WebCore::Page& page);
const WebCore::Frame& mainframe(const WebCore::Page& page);

class WebView : public WebViewDelegate
{
friend class WebChromeClient;
public:
	WebView();
	~WebView();

	WebCore::Page *page();

	void go(const char *url);

	void setVisibleSize(const int width, const int height);
	void setScroll(const int x, const int y);
	void draw(struct RastPort *rp, const int x, const int y, const int width, const int height, bool updateMode);
	bool handleIntuiMessage(IntuiMessage *imsg, const int mouseX, const int mouseY, bool mouseInside);
	
    static void handleRunLoop();
    static void shutdown();
    WebFrame* topLevelFrame() const { return m_mainFrame; }

protected:
	// WebChrome methods
    void repaint(const WebCore::IntRect&);
    void internalScroll(int scrollX, int scrollY);
    void documentSizeChanged(int width, int height);

    void closeWindow();
    void closeWindowSoon();
    void closeWindowTimerFired();

    bool transparent() const { return m_transparent; }
    bool usesLayeredWindow() const { return m_usesLayeredWindow; }

private:
    WebFrame* m_mainFrame { nullptr };
    WebCore::Page* m_page { nullptr };
	RefPtr<WebViewGroup> m_webViewGroup;
	WebViewDrawContext  *m_drawContext { nullptr };
	bool m_transparent { false };
	bool m_usesLayeredWindow { false };
	int  m_clickCount { 0 };
};
