/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#if !__has_feature(objc_arc)
#error This file requires ARC. Add the "-fobjc-arc" compiler flag for this file.
#endif

#import "config.h"
#import "WebExtensionAPIDevToolsInspectedWindow.h"

#import "CocoaHelpers.h"
#import "JSWebExtensionWrapper.h"
#import "MessageSenderInlines.h"

#if ENABLE(WK_WEB_EXTENSIONS) && ENABLE(INSPECTOR_EXTENSIONS)

namespace WebKit {

bool WebExtensionAPIDevToolsInspectedWindow::isPropertyAllowed(const ASCIILiteral& propertyName, WebPage&)
{
    // FIXME: <https://webkit.org/b/246485> Implement.

    return true;
}

void WebExtensionAPIDevToolsInspectedWindow::eval(NSString *expression, NSDictionary *options, Ref<WebExtensionCallbackHandler>&& callback, NSString **outExceptionString)
{
    // Documentation: https://developer.mozilla.org/docs/Mozilla/Add-ons/WebExtensions/API/devtools/inspectedWindow/eval

    // FIXME: <https://webkit.org/b/246485> Implement.

    callback->call();
}

void WebExtensionAPIDevToolsInspectedWindow::reload(NSDictionary *options, NSString **outExceptionString)
{
    // Documentation: https://developer.chrome.com/docs/extensions/mv2/reference/devtools/inspectedWindow#method-reload

    // FIXME: <https://webkit.org/b/246485> Implement.
}

void WebExtensionAPIDevToolsInspectedWindow::getResources(NSDictionary *options, Ref<WebExtensionCallbackHandler>&& callback, NSString **outExceptionString)
{
    // Documentation: https://developer.mozilla.org/docs/Mozilla/Add-ons/WebExtensions/API/devtools/inspectedWindow/eval

    // FIXME: <https://webkit.org/b/246485> Implement.

    callback->call();
}

double WebExtensionAPIDevToolsInspectedWindow::tabId()
{
    // Documentation: https://developer.mozilla.org/docs/Mozilla/Add-ons/WebExtensions/API/devtools/inspectedWindow/tabId

    // FIXME: <https://webkit.org/b/246485> Implement.

    return -1;
}

WebExtensionAPIEvent& WebExtensionAPIDevToolsInspectedWindow::onResourceAdded()
{
    // Documentation: https://developer.chrome.com/docs/extensions/mv2/reference/devtools/inspectedWindow#event-onResourceAdded

    if (!m_onResourceAdded)
        m_onResourceAdded = WebExtensionAPIEvent::create(forMainWorld(), runtime(), extensionContext(), WebExtensionEventListenerType::DevToolsInspectedWindowOnResourceAdded);

    return *m_onResourceAdded;
}

} // namespace WebKit

#endif // ENABLE(WK_WEB_EXTENSIONS) && ENABLE(INSPECTOR_EXTENSIONS)
