#include "WebFrame.h"

#include <JavaScriptCore/APICast.h>
#include <JavaScriptCore/JSCJSValue.h>
#include <JavaScriptCore/JSLock.h>
#include <JavaScriptCore/JSObject.h>
#include <WebCore/CSSAnimationController.h>
#include <WebCore/CSSStyleDeclaration.h>
#include <WebCore/CachedResourceLoader.h>
#include <WebCore/Chrome.h>
#include <WebCore/DatabaseManager.h>
#include <WebCore/DocumentFragment.h>
#include <WebCore/DocumentLoader.h>
#include <WebCore/DocumentMarkerController.h>
#include <WebCore/Editing.h>
#include <WebCore/Editor.h>
#include <WebCore/EventHandler.h>
#include <WebCore/EventNames.h>
#include <WebCore/Frame.h>
#include <WebCore/FrameLoadRequest.h>
#include <WebCore/FrameLoader.h>
#include <WebCore/FrameLoaderStateMachine.h>
#include <WebCore/FrameTree.h>
#include <WebCore/GraphicsContext.h>
#include <WebCore/HTMLFrameOwnerElement.h>
#include <WebCore/HTMLNames.h>
#include <WebCore/HistoryItem.h>
#include <WebCore/HitTestResult.h>
#include <WebCore/JSNode.h>
#include <WebCore/MIMETypeRegistry.h>
#include <WebCore/Page.h>
#include <WebCore/PluginData.h>
#include <WebCore/PrintContext.h>
#include <WebCore/RenderLayer.h>
#include <WebCore/RenderView.h>
#include <WebCore/RenderWidget.h>
#include <WebCore/RenderedDocumentMarker.h>
#include <WebCore/RuntimeApplicationChecks.h>
#include <WebCore/ScriptController.h>
#include <WebCore/SecurityOrigin.h>
#include <WebCore/SmartReplace.h>
#include <WebCore/StyleProperties.h>
#include <WebCore/SubframeLoader.h>
#include <WebCore/TextIterator.h>
#include <WebCore/ThreadCheck.h>
#include <WebCore/VisibleUnits.h>
#include <WebCore/markup.h>

#include "WebCoreSupport/WebFrameLoaderClient.h"

WebCore::Frame* core(WebFrame *webFrame)
{
	if (webFrame)
	{
		return webFrame->impl();
	}
	
	return nullptr;
}

class WebFrame::WebFramePrivate {
public:
    WebFramePrivate()
    {
    }

    ~WebFramePrivate() { }
    WebCore::FrameView* frameView() { return frame ? frame->view() : nullptr; }

    WebCore::Frame* frame { nullptr };
    WebView* webView { nullptr };
};

WebFrame* WebFrame::createInstance(WebCore::Frame *frame, WebView *view)
{
    WebFrame* instance = new WebFrame(frame, view);
    return instance;
}

WTF::Ref<WebCore::Frame> WebFrame::createSubframeWithOwnerElement(WebView* view, WebCore::Page* page, WebCore::HTMLFrameOwnerElement* ownerElement)
{
    WebFrame* instance = new WebFrame(nullptr, view);

    auto coreFrame = WebCore::Frame::create(page, ownerElement, new WebFrameLoaderClient(instance));
	instance->_private->frame = coreFrame.ptr();

	return coreFrame;
}

WebFrame::WebFrame(WebCore::Frame *frame, WebView *view)
{
	printf("%s:%d\n", __PRETTY_FUNCTION__, __LINE__);
	//_coreFrame = WebCore::Frame::create(nullptr, nullptr, nullptr);
	_private = new WebFramePrivate();
	if (_private)
	{
		_private->frame = frame;
		_private->webView = view;
	}
}

WebFrame::~WebFrame()
{
	printf("%s:%d\n", __PRETTY_FUNCTION__, __LINE__);
	delete _private;
}

WebView *WebFrame::webView() const
{
	return _private->webView;
}

WebCore::Frame* WebFrame::impl()
{
	return _private->frame;
}

void WebFrame::frameLoaderDestroyed()
{
    // The FrameLoader going away is equivalent to the Frame going away,
    // so we now need to clear our frame pointer.
    _private->frame = nullptr;
}
