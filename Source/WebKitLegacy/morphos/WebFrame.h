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

class WebFrame
{
public:
	WebFrame();
	~WebFrame();

private:
    RefPtr<WebCore::Frame> _coreFrame;
};
