/*
 * Copyright (C) 2006-2017 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "WebChromeClient.h"

//#include "WebElementPropertyBag.h"
#include "WebFrame.h"
//#include "WebHistory.h"
//#include "WebMutableURLRequest.h"
//#include "WebDesktopNotificationsDelegate.h"
//#include "WebSecurityOrigin.h"
#include "WebView.h"
#include <WebCore/ContextMenu.h>
#include <WebCore/Cursor.h>
#include <WebCore/FileChooser.h>
#include <WebCore/FileIconLoader.h>
#include <WebCore/FloatRect.h>
#include <WebCore/Frame.h>
#include <WebCore/FrameLoadRequest.h>
#include <WebCore/FrameView.h>
//#include <WebCore/FullScreenController.h>
//#include <WebCore/FullscreenManager.h>
#include <WebCore/GraphicsLayer.h>
#include <WebCore/HTMLNames.h>
#include <WebCore/HTMLVideoElement.h>
#include <WebCore/Icon.h>
//#include <WebCore/LocalWindowsContext.h>
#include <WebCore/LocalizedStrings.h>
#include <WebCore/NavigationAction.h>
#include <WebCore/NotImplemented.h>
#include <WebCore/Page.h>
#include <WebCore/SecurityOrigin.h>
#include <WebCore/WindowFeatures.h>
#include <wchar.h>

using namespace WebCore;

// When you call GetOpenFileName, if the size of the buffer is too small,
// MSDN says that the first two bytes of the buffer contain the required size for the file selection, in bytes or characters
// So we can assume the required size can't be more than the maximum value for a short.
static const size_t maxFilePathsListSize = USHRT_MAX;

WebChromeClient::WebChromeClient(WebView* webView)
    : m_webView(webView)
#if ENABLE(NOTIFICATIONS)
    , m_notificationsDelegate(std::make_unique<WebDesktopNotificationsDelegate>(webView))
#endif
{
}

void WebChromeClient::chromeDestroyed()
{
    delete this;
}

void WebChromeClient::setWindowRect(const FloatRect& r)
{
	notImplemented();
}

FloatRect WebChromeClient::windowRect()
{
	notImplemented();
    return FloatRect();
}

FloatRect WebChromeClient::pageRect()
{
	notImplemented();
	return FloatRect(0, 0, 800, 600);
}

void WebChromeClient::focus()
{
	notImplemented();
    // Normally this would happen on a timer, but JS might need to know this earlier, so we'll update here.
//    m_webView->updateActiveState();
}

void WebChromeClient::unfocus()
{
	notImplemented();
    // Normally this would happen on a timer, but JS might need to know this earlier, so we'll update here.
//    m_webView->updateActiveState();
}

bool WebChromeClient::canTakeFocus(FocusDirection direction)
{
	notImplemented();
	return false;
}

void WebChromeClient::takeFocus(FocusDirection direction)
{
	notImplemented();
}

void WebChromeClient::focusedElementChanged(Element*)
{
}

void WebChromeClient::focusedFrameChanged(Frame*)
{
}

Page* WebChromeClient::createWindow(Frame& frame, const FrameLoadRequest&, const WindowFeatures& features, const NavigationAction& navigationAction)
{
	return nullptr;
}

void WebChromeClient::show()
{
	notImplemented();
}

bool WebChromeClient::canRunModal()
{
	notImplemented();
	return false;
}

void WebChromeClient::runModal()
{
	notImplemented();
}

void WebChromeClient::setToolbarsVisible(bool visible)
{
	notImplemented();
}

bool WebChromeClient::toolbarsVisible()
{
	notImplemented();
	return false;
}

void WebChromeClient::setStatusbarVisible(bool visible)
{
	notImplemented();
}

bool WebChromeClient::statusbarVisible()
{
	notImplemented();
	return false;
}

void WebChromeClient::setScrollbarsVisible(bool b)
{
	notImplemented();
}

bool WebChromeClient::scrollbarsVisible()
{
	notImplemented();
	return false;
}

void WebChromeClient::setMenubarVisible(bool visible)
{
	notImplemented();

}

bool WebChromeClient::menubarVisible()
{
	notImplemented();
	return true;
}

void WebChromeClient::setResizable(bool resizable)
{
	notImplemented();
}

static BOOL messageIsError(MessageLevel level)
{
    return level == MessageLevel::Error;
}

void WebChromeClient::addMessageToConsole(MessageSource source, MessageLevel level, const String& message, unsigned lineNumber, unsigned columnNumber, const String& url)
{
	notImplemented();
}

bool WebChromeClient::canRunBeforeUnloadConfirmPanel()
{
	notImplemented();
    return false;
}

bool WebChromeClient::runBeforeUnloadConfirmPanel(const String& message, Frame& frame)
{
	notImplemented();
	return true;
}

void WebChromeClient::closeWindowSoon()
{
    // We need to remove the parent WebView from WebViewSets here, before it actually
    // closes, to make sure that JavaScript code that executes before it closes
    // can't find it. Otherwise, window.open will select a closed WebView instead of 
    // opening a new one <rdar://problem/3572585>.

    // We also need to stop the load to prevent further parsing or JavaScript execution
    // after the window has torn down <rdar://problem/4161660>.
  
    // FIXME: This code assumes that the UI delegate will respond to a webViewClose
    // message by actually closing the WebView. Safari guarantees this behavior, but other apps might not.
    // This approach is an inherent limitation of not making a close execute immediately
    // after a call to window.close.
#if 0
    m_webView->setGroupName(0);
    m_webView->stopLoading(0);
    m_webView->closeWindowSoon();
#endif
}

void WebChromeClient::runJavaScriptAlert(Frame&, const String& message)
{
	notImplemented();

}

bool WebChromeClient::runJavaScriptConfirm(Frame&, const String& message)
{
	notImplemented();
	return false;
}

bool WebChromeClient::runJavaScriptPrompt(Frame&, const String& message, const String& defaultValue, String& result)
{
	notImplemented();
	return false;
#if 0
    COMPtr<IWebUIDelegate> ui;
    if (FAILED(m_webView->uiDelegate(&ui)))
        return false;

    TimerBase::fireTimersInNestedEventLoop();

    BString resultBSTR;
    if (FAILED(ui->runJavaScriptTextInputPanelWithPrompt(m_webView, BString(message), BString(defaultValue), &resultBSTR)))
        return false;

    if (!resultBSTR)
        return false;

    result = String(resultBSTR, SysStringLen(resultBSTR));
    return true;
#endif
}

void WebChromeClient::setStatusbarText(const String& statusText)
{
	notImplemented();
}

KeyboardUIMode WebChromeClient::keyboardUIMode()
{
	bool enabled = false;
    return enabled ? KeyboardAccessTabsToLinks : KeyboardAccessDefault;
}

void WebChromeClient::invalidateRootView(const IntRect& windowRect)
{
	notImplemented();
}

void WebChromeClient::invalidateContentsAndRootView(const IntRect& windowRect)
{
	notImplemented();
}

void WebChromeClient::invalidateContentsForSlowScroll(const IntRect& windowRect)
{
	notImplemented();
}

void WebChromeClient::scroll(const IntSize& delta, const IntRect& scrollViewRect, const IntRect& clipRect)
{
	notImplemented();
}

IntPoint WebChromeClient::accessibilityScreenToRootView(const WebCore::IntPoint& point) const
{
    return screenToRootView(point);
}

IntRect WebChromeClient::rootViewToAccessibilityScreen(const WebCore::IntRect& rect) const
{
    return rootViewToScreen(rect);
}

IntRect WebChromeClient::rootViewToScreen(const IntRect& rect) const
{
	return IntRect();
}

IntPoint WebChromeClient::screenToRootView(const IntPoint& point) const
{
	return IntPoint();
}

PlatformPageClient WebChromeClient::platformPageClient() const
{
	return 0;
}

void WebChromeClient::contentsSizeChanged(Frame&, const IntSize&) const
{
    notImplemented();
}

void WebChromeClient::intrinsicContentsSizeChanged(const IntSize&) const
{
    notImplemented();
}

void WebChromeClient::mouseDidMoveOverElement(const HitTestResult& result, unsigned modifierFlags)
{
	notImplemented();
}

bool WebChromeClient::shouldUnavailablePluginMessageBeButton(RenderEmbeddedObject::PluginUnavailabilityReason pluginUnavailabilityReason) const
{
	return false;
}

void WebChromeClient::unavailablePluginButtonClicked(Element& element, RenderEmbeddedObject::PluginUnavailabilityReason pluginUnavailabilityReason) const
{
	notImplemented();
}

void WebChromeClient::setToolTip(const String& toolTip, TextDirection)
{
	notImplemented();
}

void WebChromeClient::print(Frame& frame)
{
	notImplemented();
}

void WebChromeClient::exceededDatabaseQuota(Frame& frame, const String& databaseIdentifier, DatabaseDetails)
{
	notImplemented();
}

// FIXME: Move this include to the top of the file with the other includes.
#include <WebCore/ApplicationCacheStorage.h>

void WebChromeClient::reachedMaxAppCacheSize(int64_t spaceNeeded)
{
    // FIXME: Free some space.
    notImplemented();
}

void WebChromeClient::reachedApplicationCacheOriginQuota(SecurityOrigin&, int64_t)
{
    notImplemented();
}

void WebChromeClient::runOpenPanel(Frame&, FileChooser& fileChooser)
{
}

void WebChromeClient::loadIconForFiles(const Vector<WTF::String>& filenames, WebCore::FileIconLoader& loader)
{
    loader.iconLoaded(Icon::createIconForFiles(filenames));
}

RefPtr<Icon> WebChromeClient::createIconForFiles(const Vector<String>& filenames)
{
    return Icon::createIconForFiles(filenames);
}

void WebChromeClient::didFinishLoadingImageForElement(WebCore::HTMLImageElement&)
{
}

void WebChromeClient::setCursor(const Cursor& cursor)
{
	notImplemented();

//    setLastSetCursorToCurrentCursor();
}

void WebChromeClient::setCursorHiddenUntilMouseMoves(bool)
{
    notImplemented();
}

void WebChromeClient::attachRootGraphicsLayer(Frame&, GraphicsLayer* graphicsLayer)
{
//    m_webView->setRootChildLayer(graphicsLayer);
}

void WebChromeClient::attachViewOverlayGraphicsLayer(GraphicsLayer*)
{
    // FIXME: If we want view-relative page overlays in Legacy WebKit on Windows, this would be the place to hook them up.
}

void WebChromeClient::scheduleCompositingLayerFlush()
{
//    m_webView->flushPendingGraphicsLayerChangesSoon();
}

bool WebChromeClient::selectItemWritingDirectionIsNatural()
{
    return false;
}

bool WebChromeClient::selectItemAlignmentFollowsMenuWritingDirection()
{
    return true;
}

RefPtr<PopupMenu> WebChromeClient::createPopupMenu(PopupMenuClient& client) const
{
	return nullptr;
//    return adoptRef(new PopupMenuWin(&client));
}

RefPtr<SearchPopupMenu> WebChromeClient::createSearchPopupMenu(PopupMenuClient& client) const
{
	return nullptr;
//    return adoptRef(new SearchPopupMenuWin(&client));
}

#if ENABLE(FULLSCREEN_API)

bool WebChromeClient::supportsFullScreenForElement(const Element& element, bool requestingKeyboardAccess)
{
	return false;
}

void WebChromeClient::enterFullScreenForElement(Element& element)
{
}

void WebChromeClient::exitFullScreenForElement(Element* element)
{
}

#endif

bool WebChromeClient::shouldUseTiledBackingForFrameView(const FrameView& frameView) const
{
    return false;
}
