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

WebFrame::WebFrame()
{

	_coreFrame = WebCore::Frame::create(nullptr, nullptr, nullptr);

}

WebFrame::~WebFrame()
{

}
