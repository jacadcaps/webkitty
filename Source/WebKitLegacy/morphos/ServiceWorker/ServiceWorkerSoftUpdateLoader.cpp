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

#include "config.h"
#include "ServiceWorkerSoftUpdateLoader.h"

#if ENABLE(SERVICE_WORKER)

#include <WebCore/ServiceWorkerJob.h>
#include <WebCore/TextResourceDecoder.h>
#include <WebCore/WorkerFetchResult.h>
#include <WebCore/WorkerScriptLoader.h>
#include <WebCore/ResourceHandle.h>
#include <WebCore/SecurityOrigin.h>
#include "WebProcess.h"

#define D(x) x

namespace WebKit {

using namespace WebCore;

void ServiceWorkerSoftUpdateLoader::start(ServiceWorkerJobData&& jobData, bool shouldRefreshCache, ResourceRequest&& request, Handler&& completionHandler)
{
    auto loader = std::unique_ptr<ServiceWorkerSoftUpdateLoader>(new ServiceWorkerSoftUpdateLoader(WTFMove(jobData), shouldRefreshCache, WTFMove(request), WTFMove(completionHandler)));
    WebProcess::singleton().addSoftUpdateLoader(WTFMove(loader));
}

ServiceWorkerSoftUpdateLoader::ServiceWorkerSoftUpdateLoader(ServiceWorkerJobData&& jobData, bool shouldRefreshCache, ResourceRequest&& request, Handler&& completionHandler)
    : m_completionHandler(WTFMove(completionHandler))
    , m_jobData(WTFMove(jobData))
{
    ASSERT(!request.isConditional());

// TODO: check if we can use curl cache
/*    if (session.cache()) {
        // We set cache policy to disable speculative loading/async revalidation from the cache.
        request.setCachePolicy(ResourceRequestCachePolicy::ReturnCacheDataDontLoad);

        session.cache()->retrieve(request, NetworkCache::GlobalFrameID { }, NavigatingToAppBoundDomain::No, [this, weakThis = WeakPtr { *this }, request, shouldRefreshCache](auto&& entry, auto&&) mutable {
            if (!weakThis)
                return;
            if (!m_session) {
                fail(ResourceError { ResourceError::Type::Cancellation });
                return;
            }
            if (!shouldRefreshCache && entry && !entry->needsValidation()) {
                loadWithCacheEntry(*entry);
                return;
            }

            request.setCachePolicy(ResourceRequestCachePolicy::RefreshAnyCacheData);
            if (entry) {
                m_cacheEntry = WTFMove(entry);

                String eTag = m_cacheEntry->response().httpHeaderField(HTTPHeaderName::ETag);
                if (!eTag.isEmpty())
                    request.setHTTPHeaderField(HTTPHeaderName::IfNoneMatch, eTag);

                String lastModified = m_cacheEntry->response().httpHeaderField(HTTPHeaderName::LastModified);
                if (!lastModified.isEmpty())
                    request.setHTTPHeaderField(HTTPHeaderName::IfModifiedSince, lastModified);
            }
            loadFromNetwork(*m_session, WTFMove(request));
        });
        return;
    }*/
    loadFromNetwork(WTFMove(request));
}

ServiceWorkerSoftUpdateLoader::~ServiceWorkerSoftUpdateLoader()
{
    if (m_completionHandler)
        m_completionHandler(workerFetchError(ResourceError { ResourceError::Type::Cancellation }));
}

void ServiceWorkerSoftUpdateLoader::fail(ResourceError&& error)
{
    if (!m_completionHandler)
        return;

    m_completionHandler(workerFetchError(WTFMove(error)));
    didComplete();
}

#if 0
void ServiceWorkerSoftUpdateLoader::loadWithCacheEntry(NetworkCache::Entry& entry)
{
    auto error = processResponse(entry.response());
    if (!error.isNull()) {
        fail(WTFMove(error));
        return;
    }

    if (entry.buffer())
        didReceiveBuffer(*entry.buffer(), 0);
    didFinishLoading({ });
}
#endif

void ServiceWorkerSoftUpdateLoader::loadFromNetwork(ResourceRequest&& request)
{
    D(dprintf("%s: %s\n", __PRETTY_FUNCTION__, request.url().string().utf8().data()));

    m_handle = WebCore::ResourceHandle::create(WebKit::WebProcess::singleton().networkingContext().get(), request,
        this, false, false, true, nullptr, false);
}

void ServiceWorkerSoftUpdateLoader::didReceiveBuffer(WebCore::ResourceHandle*, const WebCore::FragmentedSharedBuffer& buffer, int encodedDataLength)
{
    if (!m_decoder) {
        if (!m_responseEncoding.isEmpty())
            m_decoder = TextResourceDecoder::create("text/javascript"_s, m_responseEncoding);
        else
            m_decoder = TextResourceDecoder::create("text/javascript"_s, "UTF-8");
    }

    buffer.forEachSegment([&](auto& segment) {
        if (segment.size())
            m_script.append(m_decoder->decode(segment.data(), segment.size()));
    });
}

void ServiceWorkerSoftUpdateLoader::didFinishLoading(WebCore::ResourceHandle*, const WebCore::NetworkLoadMetrics&)
{
    D(dprintf("%s: \n", __PRETTY_FUNCTION__));

    if (m_decoder)
        m_script.append(m_decoder->flush());
    m_completionHandler({ ScriptBuffer { m_script.toString() }, m_jobData.scriptURL, m_certificateInfo, m_contentSecurityPolicy, m_crossOriginEmbedderPolicy, m_referrerPolicy, { } });
    didComplete();
}

void ServiceWorkerSoftUpdateLoader::didFail(WebCore::ResourceHandle*, const WebCore::ResourceError& error)
{
    D(dprintf("%s: \n", __PRETTY_FUNCTION__));
    fail(ResourceError(error));
}

void ServiceWorkerSoftUpdateLoader::didReceiveResponseAsync(WebCore::ResourceHandle*, WebCore::ResourceResponse&& response, CompletionHandler<void()>&& completion)
{
    D(dprintf("%s: \n", __PRETTY_FUNCTION__));
    m_certificateInfo = *response.certificateInfo();
    auto error = processResponse(response);
    if (!error.isNull()) {
        fail(WTFMove(error));
    }
    completion();
}

void ServiceWorkerSoftUpdateLoader::willSendRequestAsync(WebCore::ResourceHandle*, WebCore::ResourceRequest&& request, WebCore::ResourceResponse&&, CompletionHandler<void(WebCore::ResourceRequest&&)>&& completion)
{
    D(dprintf("%s: \n", __PRETTY_FUNCTION__));

    m_currentRequest = request;
    completion(std::move(request));
}

/*
void ServiceWorkerSoftUpdateLoader::willSendRedirectedRequest(ResourceRequest&&, ResourceRequest&&, ResourceResponse&&)
{
    fail(ResourceError { ResourceError::Type::Cancellation });
}
*/

// https://w3c.github.io/ServiceWorker/#update-algorithm, steps 9.7 to 9.17
ResourceError ServiceWorkerSoftUpdateLoader::processResponse(const ResourceResponse& response)
{
    auto source = m_jobData.workerType == WorkerType::Module ? WorkerScriptLoader::Source::ModuleScript : WorkerScriptLoader::Source::ClassicWorkerScript;
    auto error = WorkerScriptLoader::validateWorkerResponse(response, source, FetchOptions::Destination::Serviceworker);
    if (!error.isNull())
        return error;

    error = ServiceWorkerJob::validateServiceWorkerResponse(m_jobData, response);
    if (!error.isNull())
        return error;

    m_contentSecurityPolicy = ContentSecurityPolicyResponseHeaders { response };
    // Service workers are always secure contexts.
    m_crossOriginEmbedderPolicy = obtainCrossOriginEmbedderPolicy(response, nullptr);
    m_referrerPolicy = response.httpHeaderField(HTTPHeaderName::ReferrerPolicy);
    m_responseEncoding = response.textEncodingName();

    return { };
}

void ServiceWorkerSoftUpdateLoader::didComplete()
{
    m_handle = nullptr;
    WebProcess::singleton().removeSoftUpdateLoader(this);
}

} // namespace WebKit

#endif // ENABLE(SERVICE_WORKER)
