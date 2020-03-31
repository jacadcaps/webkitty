#pragma once

namespace WebCore {
    class CSSStyleDeclaration;
    class Document;
    class DocumentLoader;
    class Element;
    class Frame;
    class Frame;
    class HistoryItem;
    class HTMLElement;
    class HTMLFrameOwnerElement;
    class Node;
    class Page;
    class Range;
}

#include <wtf/RefPtr.h>

class WebView;

class WebFrame
{
public:
    static WebFrame* createInstance(WebCore::Frame *frame, WebView *view);
	static WTF::Ref<WebCore::Frame> createSubframeWithOwnerElement(WebView* webView, WebCore::Page* page, WebCore::HTMLFrameOwnerElement* ownerElement);

    WebCore::Frame* impl();
    WebView *webView() const;

	void frameLoaderDestroyed();

protected:
    WebFrame(WebCore::Frame *frame, WebView *view);
    ~WebFrame();

private:
	class WebFramePrivate;
	WebFramePrivate *_private;
};

WebCore::Frame* core(WebFrame *webFrame);
