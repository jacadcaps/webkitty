/*
 * Copyright (C) 2019 Apple Inc. All rights reserved.
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

#pragma once
#include "WebKit.h"

#if ENABLE(SERVICE_WORKER)

#include <WebCore/ContentSecurityPolicyResponseHeaders.h>
#include <WebCore/CrossOriginEmbedderPolicy.h>
#include <WebCore/FetchOptions.h>
#include <WebCore/ServiceWorkerJobData.h>
#include <WebCore/ResourceHandleClient.h>
#include <WebCore/ResourceRequest.h>
#include <WebCore/CertificateInfo.h>
#include <wtf/CompletionHandler.h>
#include <wtf/WeakPtr.h>

namespace WebCore {
struct ServiceWorkerJobData;
struct WorkerFetchResult;
class TextResourceDecoder;
class SharedBuffer;
class ResourceHandle;
}

namespace WebKit {

class NetworkLoad;
class NetworkSession;

class ServiceWorkerSoftUpdateLoader final : public WebCore::ResourceHandleClient, public CanMakeWeakPtr<ServiceWorkerSoftUpdateLoader> {
    WTF_MAKE_FAST_ALLOCATED;
public:
    using Handler = CompletionHandler<void(const WebCore::WorkerFetchResult&)>;
    static void start(WebCore::ServiceWorkerJobData&&, bool shouldRefreshCache, WebCore::ResourceRequest&&, Handler&&);

    ~ServiceWorkerSoftUpdateLoader();
    
private:
    ServiceWorkerSoftUpdateLoader(WebCore::ServiceWorkerJobData&&, bool shouldRefreshCache, WebCore::ResourceRequest&&, Handler&&);

    // ResourceHandleClient.
    void didReceiveBuffer(WebCore::ResourceHandle*, const WebCore::FragmentedSharedBuffer&, int encodedDataLength) final;
    void didFinishLoading(WebCore::ResourceHandle*, const WebCore::NetworkLoadMetrics&) final;
    void didFail(WebCore::ResourceHandle*, const WebCore::ResourceError&) final;
    void didReceiveResponseAsync(WebCore::ResourceHandle*, WebCore::ResourceResponse&&, CompletionHandler<void()>&&) final;
    void willSendRequestAsync(WebCore::ResourceHandle*, WebCore::ResourceRequest&&, WebCore::ResourceResponse&&, CompletionHandler<void(WebCore::ResourceRequest&&)>&&) final;
/*
    void didSendData(unsigned long long bytesSent, unsigned long long totalBytesToBeSent) final { }
    bool isSynchronous() const final { return false; }
    bool isAllowedToAskUserForCredentials() const final { return false; }
    void willSendRedirectedRequest(WebCore::ResourceRequest&&, WebCore::ResourceRequest&& redirectRequest, WebCore::ResourceResponse&& redirectResponse) final;
    void didReceiveResponse(WebCore::ResourceResponse&&, PrivateRelayed, ResponseCompletionHandler&&) final;
    void didReceiveBuffer(const WebCore::FragmentedSharedBuffer&, int reportedEncodedDataLength) final;
    void didFinishLoading(const WebCore::NetworkLoadMetrics&) final;
    void didFailLoading(const WebCore::ResourceError&) final;
    void loadWithCacheEntry(NetworkCache::Entry&);
*/
    void loadFromNetwork(WebCore::ResourceRequest&&);

    void fail(WebCore::ResourceError&&);
    void didComplete();
    WebCore::ResourceError processResponse(const WebCore::ResourceResponse&);
    WebCore::ResourceRequest m_currentRequest;
    RefPtr<WebCore::ResourceHandle> m_handle;

    Handler m_completionHandler;
    WebCore::ServiceWorkerJobData m_jobData;

    String m_responseEncoding;
    String m_referrerPolicy;
    WebCore::ContentSecurityPolicyResponseHeaders m_contentSecurityPolicy;
    WebCore::CrossOriginEmbedderPolicy m_crossOriginEmbedderPolicy;

    RefPtr<WebCore::TextResourceDecoder> m_decoder;
    StringBuilder m_script;
    WebCore::CertificateInfo m_certificateInfo;
};

} // namespace WebKit

#endif // ENABLE(SERVICE_WORKER)

