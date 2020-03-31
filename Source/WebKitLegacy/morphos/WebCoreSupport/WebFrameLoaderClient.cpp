/*
 * Copyright (C) 2006-2017 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "WebFrameLoaderClient.h"

#include "WebView.h"
#include "WebFrame.h"
#include "WebFrameNetworkingContext.h"

#include <JavaScriptCore/APICast.h>
#include <WebCore/AuthenticationChallenge.h>
#include <WebCore/BackForwardController.h>
#include <WebCore/CachedFrame.h>
#include <WebCore/DNS.h>
#include <WebCore/DocumentLoader.h>
#include <WebCore/FormState.h>
#include <WebCore/Frame.h>
#include <WebCore/FrameLoader.h>
#include <WebCore/FrameTree.h>
#include <WebCore/FrameView.h>
#include <WebCore/HTMLAppletElement.h>
#include <WebCore/HTMLFrameElement.h>
#include <WebCore/HTMLFrameOwnerElement.h>
#include <WebCore/HTMLNames.h>
#include <WebCore/HTMLParserIdioms.h>
#include <WebCore/HTMLPlugInElement.h>
#include <WebCore/HistoryItem.h>
#include <WebCore/LocalizedStrings.h>
#include <WebCore/MIMETypeRegistry.h>
#include <WebCore/NotImplemented.h>
#include <WebCore/Page.h>
#include <WebCore/PolicyChecker.h>
#include <WebCore/RenderWidget.h>
#include <WebCore/ResourceHandle.h>
#include <WebCore/ScriptController.h>
#include <WebCore/Settings.h>
#include <WebCore/SubresourceLoader.h>

using namespace WebCore;
using namespace HTMLNames;

#include <proto/exec.h>

WebFrameLoaderClient::WebFrameLoaderClient(WebFrame* webFrame)
    : m_webFrame(webFrame)
{
	printf("%s:%d. frame %p\n", __PRETTY_FUNCTION__, __LINE__, webFrame);
}

WebFrameLoaderClient::~WebFrameLoaderClient()
{
	printf("%s:%d\n", __PRETTY_FUNCTION__, __LINE__);
}

void WebFrameLoaderClient::frameLoaderDestroyed()
{
	dprintf("%s:...\n", __PRETTY_FUNCTION__);
	
	if (m_webFrame)
		m_webFrame->frameLoaderDestroyed();
//    notImplemented();
    //m_webFrame = nullptr; ??
	delete this;
}

Optional<WebCore::PageIdentifier> WebFrameLoaderClient::pageID() const
{
    return WTF::nullopt;
}

Optional<uint64_t> WebFrameLoaderClient::frameID() const
{
    return WTF::nullopt;
}

PAL::SessionID WebFrameLoaderClient::sessionID() const
{
    RELEASE_ASSERT_NOT_REACHED();
    return PAL::SessionID::defaultSessionID();
}

bool WebFrameLoaderClient::hasWebView() const
{
    return m_webFrame->webView();
}

void WebFrameLoaderClient::makeRepresentation(DocumentLoader*)
{
    notImplemented();
}

void WebFrameLoaderClient::forceLayoutForNonHTML()
{
    notImplemented();
}

void WebFrameLoaderClient::setCopiesOnScroll()
{
    notImplemented();
}

void WebFrameLoaderClient::detachedFromParent2()
{
    notImplemented();
}

void WebFrameLoaderClient::detachedFromParent3()
{
    notImplemented();
}

void WebFrameLoaderClient::convertMainResourceLoadToDownload(DocumentLoader* documentLoader, PAL::SessionID, const ResourceRequest& request, const ResourceResponse& response)
{
	notImplemented();
}

bool WebFrameLoaderClient::dispatchDidLoadResourceFromMemoryCache(DocumentLoader*, const ResourceRequest&, const ResourceResponse&, int /*length*/)
{
    notImplemented();
    return false;
}

void WebFrameLoaderClient::assignIdentifierToInitialRequest(unsigned long identifier, DocumentLoader* loader, const ResourceRequest& request)
{
	notImplemented();
}

bool WebFrameLoaderClient::shouldUseCredentialStorage(DocumentLoader* loader, unsigned long identifier)
{
    return true;
}

void WebFrameLoaderClient::dispatchDidReceiveAuthenticationChallenge(DocumentLoader* loader, unsigned long identifier, const AuthenticationChallenge& challenge)
{
	notImplemented();

    // If the ResourceLoadDelegate doesn't exist or fails to handle the call, we tell the ResourceHandle
    // to continue without credential - this is the best approximation of Mac behavior
    challenge.authenticationClient()->receivedRequestToContinueWithoutCredential(challenge);
}

void WebFrameLoaderClient::dispatchWillSendRequest(DocumentLoader* loader, unsigned long identifier, ResourceRequest& request, const ResourceResponse& redirectResponse)
{
	notImplemented();
}

void WebFrameLoaderClient::dispatchDidReceiveResponse(DocumentLoader* loader, unsigned long identifier, const ResourceResponse& response)
{
	notImplemented();
}

void WebFrameLoaderClient::dispatchDidReceiveContentLength(DocumentLoader* loader, unsigned long identifier, int length)
{
	notImplemented();
}

void WebFrameLoaderClient::dispatchDidFinishLoading(DocumentLoader* loader, unsigned long identifier)
{
	notImplemented();
}

void WebFrameLoaderClient::dispatchDidFailLoading(DocumentLoader* loader, unsigned long identifier, const ResourceError& error)
{
	notImplemented();
}

void WebFrameLoaderClient::dispatchDidDispatchOnloadEvents()
{
	notImplemented();
}

void WebFrameLoaderClient::dispatchDidReceiveServerRedirectForProvisionalLoad()
{
	notImplemented();
}

void WebFrameLoaderClient::dispatchDidCancelClientRedirect()
{
	notImplemented();
}

void WebFrameLoaderClient::dispatchWillPerformClientRedirect(const URL& url, double delay, WallTime fireDate, WebCore::LockBackForwardList)
{
	notImplemented();
}

void WebFrameLoaderClient::dispatchDidChangeLocationWithinPage()
{
	notImplemented();
}

void WebFrameLoaderClient::dispatchDidPushStateWithinPage()
{
	notImplemented();
}

void WebFrameLoaderClient::dispatchDidReplaceStateWithinPage()
{
	notImplemented();
}

void WebFrameLoaderClient::dispatchDidPopStateWithinPage()
{
	notImplemented();
}

void WebFrameLoaderClient::dispatchWillClose()
{
	notImplemented();
}

void WebFrameLoaderClient::dispatchDidStartProvisionalLoad()
{
	notImplemented();
}

void WebFrameLoaderClient::dispatchDidReceiveTitle(const StringWithDirection& title)
{
	notImplemented();
}

void WebFrameLoaderClient::dispatchDidCommitLoad(Optional<HasInsecureContent>)
{
	notImplemented();
}

void WebFrameLoaderClient::dispatchDidFailProvisionalLoad(const ResourceError& error, WillContinueLoading)
{
	notImplemented();
//	DumpTaskState(FindTask(0));
}

void WebFrameLoaderClient::dispatchDidFailLoad(const ResourceError& error)
{
	notImplemented();
}

void WebFrameLoaderClient::dispatchDidFinishDocumentLoad()
{
	notImplemented();
}

void WebFrameLoaderClient::dispatchDidFinishLoad()
{
	notImplemented();
}

void WebFrameLoaderClient::dispatchDidReachLayoutMilestone(OptionSet<WebCore::LayoutMilestone> milestones)
{
	notImplemented();
}

Frame* WebFrameLoaderClient::dispatchCreatePage(const NavigationAction& navigationAction)
{
	notImplemented();
	return  nullptr;
}

void WebFrameLoaderClient::dispatchShow()
{
	notImplemented();}

void WebFrameLoaderClient::dispatchDecidePolicyForResponse(const ResourceResponse& response, const ResourceRequest& request, WebCore::PolicyCheckIdentifier identifier, const String&, FramePolicyFunction&& function)
{
	function(PolicyAction::Use, identifier);
	notImplemented();
}

void WebFrameLoaderClient::dispatchDecidePolicyForNewWindowAction(const NavigationAction& action, const ResourceRequest& request, FormState* formState, const String& frameName, WebCore::PolicyCheckIdentifier identifier, FramePolicyFunction&& function)
{
	notImplemented();
}

void WebFrameLoaderClient::dispatchDecidePolicyForNavigationAction(const NavigationAction& action, const ResourceRequest& request, const ResourceResponse&, FormState* formState, WebCore::PolicyDecisionMode, WebCore::PolicyCheckIdentifier identifier, FramePolicyFunction&& function)
{
	notImplemented();
	
	function(PolicyAction::Use, identifier);
	
//	RefPtr<Frame> frame = m_webFrame->impl();
//    if (frame)
//        static_cast<WebFrameLoaderClient&>(frame->loader().client()).receivedPolicyDecision(PolicyAction::Use);
	
}

void WebFrameLoaderClient::dispatchUnableToImplementPolicy(const ResourceError& error)
{
	notImplemented();
}

void WebFrameLoaderClient::dispatchWillSendSubmitEvent(Ref<WebCore::FormState>&&)
{
}

void WebFrameLoaderClient::dispatchWillSubmitForm(FormState& formState, CompletionHandler<void()>&& completionHandler)
{
	notImplemented();

    // FIXME: Add a sane default implementation
    completionHandler();
}

void WebFrameLoaderClient::setMainDocumentError(DocumentLoader*, const ResourceError& error)
{
	WTF::StringView sv(error.localizedDescription());
	dprintf("SMDE error %d %s\n", error.errorCode(), sv.utf8().data());
	// DumpTaskState(FindTask(0));
	notImplemented();
}

void WebFrameLoaderClient::progressStarted(WebCore::Frame&)
{
	notImplemented();
}

void WebFrameLoaderClient::progressEstimateChanged(WebCore::Frame&)
{
	notImplemented();
}

void WebFrameLoaderClient::progressFinished(WebCore::Frame&)
{
	notImplemented();
}

void WebFrameLoaderClient::startDownload(const ResourceRequest& request, const String& /* suggestedName */)
{
//    m_webFrame->webView()->downloadURL(request.url());
	notImplemented();
}

void WebFrameLoaderClient::willChangeTitle(DocumentLoader*)
{
    notImplemented();
}

void WebFrameLoaderClient::didChangeTitle(DocumentLoader*)
{
    notImplemented();
}

void WebFrameLoaderClient::committedLoad(DocumentLoader* loader, const char* data, int length)
{
    loader->commitData(data, length);
}

void WebFrameLoaderClient::finishedLoading(DocumentLoader*)
{
	notImplemented();
}

void WebFrameLoaderClient::updateGlobalHistory()
{
	notImplemented();
}

void WebFrameLoaderClient::updateGlobalHistoryRedirectLinks()
{
	notImplemented();
}

bool WebFrameLoaderClient::shouldGoToHistoryItem(HistoryItem&) const
{
	notImplemented();
    return true;
}

void WebFrameLoaderClient::didDisplayInsecureContent()
{
	notImplemented();
}

void WebFrameLoaderClient::didRunInsecureContent(SecurityOrigin& origin, const URL& insecureURL)
{
	notImplemented();
}

void WebFrameLoaderClient::didDetectXSS(const URL&, bool)
{
	notImplemented();
    // FIXME: propogate call into the private delegate.
}

ResourceError WebFrameLoaderClient::cancelledError(const ResourceRequest& request)
{
    // FIXME: Need ChickenCat to include CFNetwork/CFURLError.h to get these values
    // Alternatively, we could create our own error domain/codes.
    return ResourceError(String("error"), -999, request.url(), String("Cancelled"));
}

ResourceError WebFrameLoaderClient::blockedError(const ResourceRequest& request)
{
    return ResourceError(String("error"), -999, request.url(), String("Cancelled"));
}

ResourceError WebFrameLoaderClient::blockedByContentBlockerError(const ResourceRequest& request)
{
    RELEASE_ASSERT_NOT_REACHED(); // Content Blockers are not enabled for WK1.
}

ResourceError WebFrameLoaderClient::cannotShowURLError(const ResourceRequest& request)
{
    return ResourceError(String("error"), -999, request.url(), String("Cancelled"));
}

ResourceError WebFrameLoaderClient::interruptedForPolicyChangeError(const ResourceRequest& request)
{
    return ResourceError(String("error"), -999, request.url(), String("Cancelled"));
}

ResourceError WebFrameLoaderClient::cannotShowMIMETypeError(const ResourceResponse& response)
{
    return ResourceError(String("error"), -999, response.url(), String("Cancelled"));
}

ResourceError WebFrameLoaderClient::fileDoesNotExistError(const ResourceResponse& response)
{
    return ResourceError(String("error"), -999, response.url(), String("Cancelled"));
}

ResourceError WebFrameLoaderClient::pluginWillHandleLoadError(const ResourceResponse& response)
{
    return ResourceError(String("error"), -999, response.url(), String("Cancelled"));
}

bool WebFrameLoaderClient::shouldFallBack(const ResourceError& error)
{
    return true;
}

bool WebFrameLoaderClient::canHandleRequest(const ResourceRequest& request) const
{
    //return WebView::canHandleRequest(request);
    return true;
}

bool WebFrameLoaderClient::canShowMIMEType(const String& mimeType) const
{
	return true;
//    return m_webFrame->webView()->canShowMIMEType(mimeType);
}

bool WebFrameLoaderClient::canShowMIMETypeAsHTML(const String& mimeType) const
{
	return true;
//    return m_webFrame->webView()->canShowMIMETypeAsHTML(mimeType);
}

bool WebFrameLoaderClient::representationExistsForURLScheme(const String& /*URLScheme*/) const
{
    notImplemented();
    return false;
}

String WebFrameLoaderClient::generatedMIMETypeForURLScheme(const String& /*URLScheme*/) const
{
    notImplemented();
    ASSERT_NOT_REACHED();
    return String();
}

void WebFrameLoaderClient::frameLoadCompleted()
{
}

void WebFrameLoaderClient::saveViewStateToItem(HistoryItem&)
{
}

void WebFrameLoaderClient::restoreViewState()
{
}

void WebFrameLoaderClient::provisionalLoadStarted()
{
    notImplemented();
}

void WebFrameLoaderClient::didFinishLoad()
{
    notImplemented();
}

void WebFrameLoaderClient::prepareForDataSourceReplacement()
{
    notImplemented();
}

Ref<DocumentLoader> WebFrameLoaderClient::createDocumentLoader(const ResourceRequest& request, const SubstituteData& substituteData)
{
	Ref<DocumentLoader> loader = DocumentLoader::create(request, substituteData);
	return std::move(loader);
}

void WebFrameLoaderClient::setTitle(const StringWithDirection& title, const URL& url)
{
	notImplemented();
}

void WebFrameLoaderClient::savePlatformDataToCachedFrame(CachedFrame* cachedFrame)
{
    notImplemented();
}

void WebFrameLoaderClient::transitionToCommittedFromCachedFrame(CachedFrame*)
{
	notImplemented();
}

void WebFrameLoaderClient::transitionToCommittedForNewPage()
{
    //WebView* view = m_webFrame->webView();
    FloatRect logicalFrame(0, 0, 800, 600);
    logicalFrame.scale(1.0f);
    Optional<Color> backgroundColor;
    dprintf("%s: creating view..\n", __PRETTY_FUNCTION__);
    core(m_webFrame)->createView(enclosingIntRect(logicalFrame).size(), backgroundColor, /* fixedLayoutSize */ { }, /* fixedVisibleContentRect */ { });
}

void WebFrameLoaderClient::didSaveToPageCache()
{
	notImplemented();
}

void WebFrameLoaderClient::didRestoreFromPageCache()
{
	notImplemented();
}

void WebFrameLoaderClient::dispatchDidBecomeFrameset(bool)
{
	notImplemented();
}

String WebFrameLoaderClient::userAgent(const URL& url)
{
    //return m_webFrame->webView()->userAgentForKURL(url);
    //return String("Mozilla/5.0 (Macintosh; Intel Mac OS X 10_13_6) AppleWebKit/605.1.15 (KHTML, like Gecko)");
    return String("Mozilla/5.0 (MorphOS; PowerPC 3.14) AppleWebKit/605.1.15 (KHTML, like Gecko) Chrome/78.0.3904.87 Version/13.1 Safari/605.1.15");
}

bool WebFrameLoaderClient::canCachePage() const
{
    return true;
}

RefPtr<Frame> WebFrameLoaderClient::createFrame(const URL& url, const String& name, HTMLFrameOwnerElement& ownerElement,
    const String& referrer)
{
    Frame* coreFrame = core(m_webFrame);
    ASSERT(coreFrame);

dprintf("create frame, core %p\n", coreFrame);

    RefPtr<WebCore::Frame> childFrame = WebFrame::createSubframeWithOwnerElement(m_webFrame->webView(), coreFrame->page(), &ownerElement);

dprintf("create frame, child %p\n", childFrame);

    childFrame->tree().setName(name);

    ownerElement.document().frame()->tree().appendChild(*childFrame);

//    coreFrame->tree().appendChild(*childFrame);
    childFrame->init();

    if (!childFrame->page())
        return nullptr;

    coreFrame->loader().loadURLIntoChildFrame(url, referrer, childFrame.get());

    // The frame's onload handler may have removed it from the document.
    if (!childFrame->tree().parent())
        return nullptr;

    return childFrame;
}

ObjectContentType WebFrameLoaderClient::objectContentType(const URL& url, const String& mimeTypeIn)
{
    String mimeType = mimeTypeIn;

    if (mimeType.isEmpty())
        return ObjectContentType::Frame; // Go ahead and hope that we can display the content.

    if (MIMETypeRegistry::isSupportedImageMIMEType(mimeType))
        return WebCore::ObjectContentType::Image;

    if (MIMETypeRegistry::isSupportedNonImageMIMEType(mimeType))
        return WebCore::ObjectContentType::Frame;

    return WebCore::ObjectContentType::None;
}

void WebFrameLoaderClient::dispatchDidFailToStartPlugin(const PluginView& pluginView) const
{
}

RefPtr<Widget> WebFrameLoaderClient::createPlugin(const IntSize& pluginSize, HTMLPlugInElement& element, const URL& url, const Vector<String>& paramNames, const Vector<String>& paramValues, const String& mimeType, bool loadManually)
{
    return nullptr;
}

void WebFrameLoaderClient::redirectDataToPlugin(Widget& pluginWidget)
{
}

RefPtr<Widget> WebFrameLoaderClient::createJavaAppletWidget(const IntSize& pluginSize, HTMLAppletElement& element, const URL& /*baseURL*/, const Vector<String>& paramNames, const Vector<String>& paramValues)
{
    return nullptr;
}

String WebFrameLoaderClient::overrideMediaType() const
{
    notImplemented();
    return String();
}

void WebFrameLoaderClient::dispatchDidClearWindowObjectInWorld(DOMWrapperWorld& world)
{
}

Ref<FrameNetworkingContext> WebFrameLoaderClient::createNetworkingContext()
{
    return WebFrameNetworkingContext::create(core(m_webFrame));
}

bool WebFrameLoaderClient::shouldAlwaysUsePluginDocument(const String& mimeType) const
{
    return false;
}

void WebFrameLoaderClient::revertToProvisionalState(DocumentLoader*)
{
    notImplemented();
}

void WebFrameLoaderClient::setMainFrameDocumentReady(bool)
{
    notImplemented();
}

void WebFrameLoaderClient::cancelPolicyCheck()
{
}

void WebFrameLoaderClient::prefetchDNS(const String& hostname)
{
    WebCore::prefetchDNS(hostname);
}

bool WebFrameLoaderClient::allowScript(bool enabledPerSettings)
{
	notImplemented();
	return true;
}
