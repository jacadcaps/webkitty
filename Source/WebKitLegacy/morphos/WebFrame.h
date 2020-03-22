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

class WebView;

class WebFrame
{
public:
    static WebFrame* createInstance(WebCore::Frame *frame, WebView *view);

    WebCore::Frame* impl();

protected:
    WebFrame(WebCore::Frame *frame, WebView *view);
    ~WebFrame();

private:
	class WebFramePrivate;
	WebFramePrivate *_private;
};

WebCore::Frame* core(WebFrame *webFrame);
