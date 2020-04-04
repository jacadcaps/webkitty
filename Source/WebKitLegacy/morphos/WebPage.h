#pragma once
#include <wtf/RefPtr.h>
#include <wtf/HashSet.h>
#include <WebCore/PageIdentifier.h>
#include <WebCore/Color.h>
#include "WebViewDelegate.h"
#include "WebFrame.h"

namespace WebCore {
	class Page;
	class Frame;
	class FrameView;
	class IntRect;
};

struct RastPort;
struct IntuiMessage;

namespace WebKit {

class WebPage;
class WebPageGroup;
class WebFrame;
class WebViewDrawContext;
class WebChromeClient;
class WebPageCreationParameters;
class WebDocumentLoader;

WebCore::Page* core(WebPage *webView);
WebPage *kit(WebCore::Page* page);

WebCore::Frame& mainframe(WebCore::Page& page);
const WebCore::Frame& mainframe(const WebCore::Page& page);

class WebPage : public WebViewDelegate, public WTF::RefCounted<WebPage>
{
friend class WebChromeClient;
public:
    static Ref<WebPage> create(WebCore::PageIdentifier, WebPageCreationParameters&&);

    virtual ~WebPage();

	WebCore::Page *corePage();
	static WebPage *fromCorePage(WebCore::Page *corePage);

    WebCore::PageIdentifier pageID() const { return m_pageID; }
    PAL::SessionID sessionID() const;

	void go(const char *url);

	void setVisibleSize(const int width, const int height);
	void setScroll(const int x, const int y);
	void draw(struct RastPort *rp, const int x, const int y, const int width, const int height, bool updateMode);
	bool handleIntuiMessage(IntuiMessage *imsg, const int mouseX, const int mouseY, bool mouseInside);

    void addResourceRequest(unsigned long, const WebCore::ResourceRequest&);
    void removeResourceRequest(unsigned long);

    void didStartPageTransition();
    void didCompletePageTransition();
    void didCommitLoad(WebFrame*);
	void didFinishDocumentLoad(WebFrame& frame);
	void didFinishLoad(WebFrame& frame);

    Ref<WebCore::DocumentLoader> createDocumentLoader(WebCore::Frame&, const WebCore::ResourceRequest&, const WebCore::SubstituteData&);
    void updateCachedDocumentLoader(WebDocumentLoader&, WebCore::Frame&);

    void scalePage(double scale, const WebCore::IntPoint& origin);
    double pageScaleFactor() const;
    double totalScaleFactor() const;
    double viewScaleFactor() const;

    bool mainFrameIsScrollable() const { return m_mainFrameIsScrollable; }

    void setAlwaysShowsHorizontalScroller(bool);
    void setAlwaysShowsVerticalScroller(bool);

    bool alwaysShowsHorizontalScroller() const { return m_alwaysShowsHorizontalScroller; };
    bool alwaysShowsVerticalScroller() const { return m_alwaysShowsVerticalScroller; };

    const Optional<WebCore::Color>& backgroundColor() const { return m_backgroundColor; }

    WebCore::IntSize size() const;
    WebCore::IntRect bounds() const { return WebCore::IntRect(WebCore::IntPoint(), size()); }

    WebFrame* topLevelFrame() const { return m_mainFrame.get(); }

    WebCore::Frame* mainFrame() const; // May return nullptr.
    WebCore::FrameView* mainFrameView() const; // May return nullptr.

protected:
	WebPage(WebCore::PageIdentifier, WebPageCreationParameters&&);

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
    RefPtr<WebFrame> m_mainFrame { nullptr };
	std::unique_ptr<WebCore::Page> m_page;
	RefPtr<WebPageGroup> m_webPageGroup;
	WebViewDrawContext  *m_drawContext { nullptr };
    WebCore::PageIdentifier m_pageID;
    WTF::HashSet<unsigned long> m_trackedNetworkResourceRequestIdentifiers;
    uint64_t m_pendingNavigationID { 0 };
	bool m_transparent { false };
	bool m_usesLayeredWindow { false };
	int  m_clickCount { 0 };
    bool m_mainFrameProgressCompleted { false };
    bool m_alwaysShowsHorizontalScroller { false };
    bool m_alwaysShowsVerticalScroller { false };
    bool m_mainFrameIsScrollable { true };
    Optional<WebCore::Color> m_backgroundColor { WebCore::Color::white };
};

}
