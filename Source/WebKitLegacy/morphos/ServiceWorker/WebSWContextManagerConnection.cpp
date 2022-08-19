/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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
#include "WebSWContextManagerConnection.h"

#if ENABLE(SERVICE_WORKER)

#include "WebProcess.h"
#include "WebDatabaseProvider.h"
#include "WebPageProxyIdentifier.h"
#include <WebCore/EditorClient.h>
#include <WebCore/EmptyClients.h>
#include <WebCore/MessageWithMessagePorts.h>
#include <WebCore/PageConfiguration.h>
#include <WebCore/RuntimeEnabledFeatures.h>
#include <WebCore/ScriptExecutionContextIdentifier.h>
#include <WebCore/SerializedScriptValue.h>
#include <WebCore/ServiceWorkerClientData.h>
#include <WebCore/ServiceWorkerClientQueryOptions.h>
#include <WebCore/ServiceWorkerJobDataIdentifier.h>
#include <WebCore/SocketProvider.h>
#include "../../WebCoreSupport/WebBroadcastChannelRegistry.h"
#include <WebCore/WebLockRegistry.h>
#include <WebCore/UserAgent.h>
#include <wtf/ProcessID.h>
#include "WebSWOriginTable.h"
#include "WebSWServerToContextConnection.h"
#include <wtf/MainThread.h>

#define D(x) x

namespace WebKit {
using namespace PAL;
using namespace WebCore;

WebSWContextManagerConnection::WebSWContextManagerConnection(WebCore::RegistrableDomain&& registrableDomain, std::optional<WebCore::ScriptExecutionContextIdentifier> serviceWorkerPageIdentifier, PageIdentifier pageID)
    : m_registrableDomain(WTFMove(registrableDomain))
    , m_serviceWorkerPageIdentifier(serviceWorkerPageIdentifier)
    , m_pageID(pageID)
    , m_userAgent(standardUserAgent())
{
//TODO: ??
//    WebProcess::singleton().disableTermination();


    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
}

WebSWContextManagerConnection::~WebSWContextManagerConnection() = default;

void WebSWContextManagerConnection::establishConnection(CompletionHandler<void()>&& completionHandler)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    // m_connectionToNetworkProcess->sendWithAsyncReply(Messages::NetworkConnectionToWebProcess::EstablishSWContextConnection { m_webPageProxyID, m_registrableDomain, m_serviceWorkerPageIdentifier }, WTFMove(completionHandler), 0);
    
    if (auto* swServer = WebProcess::singleton().swServer())
        m_swContextConnection = makeUnique<WebSWServerToContextConnection>(WebPageProxyIdentifier(), WebCore::RegistrableDomain(m_registrableDomain),
            m_serviceWorkerPageIdentifier, *swServer);
    completionHandler();
}

void WebSWContextManagerConnection::updateAppInitiatedValue(WebCore::ServiceWorkerIdentifier serviceWorkerIdentifier, WebCore::LastNavigationWasAppInitiated lastNavigationWasAppInitiated)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    if (auto* serviceWorkerThreadProxy = SWContextManager::singleton().serviceWorkerThreadProxy(serviceWorkerIdentifier))
        serviceWorkerThreadProxy->setLastNavigationWasAppInitiated(lastNavigationWasAppInitiated == WebCore::LastNavigationWasAppInitiated::Yes);
}

void WebSWContextManagerConnection::installServiceWorker(ServiceWorkerContextData&& contextData, ServiceWorkerData&& workerData, String&& userAgent, WorkerThreadMode workerThreadMode)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    auto pageConfiguration = pageConfigurationWithEmptyClients(WebProcess::singleton().sessionID());

    pageConfiguration.databaseProvider = Ref{WebDatabaseProvider::singleton()};
    pageConfiguration.socketProvider = WebCore::SocketProvider::create();
    pageConfiguration.broadcastChannelRegistry = WebBroadcastChannelRegistry::getOrCreate(false);
    pageConfiguration.webLockRegistry = WebProcess::singleton().getOrCreateWebLockRegistry(false);
//    pageConfiguration.userContentProvider = m_userContentController;

    auto effectiveUserAgent =  WTFMove(userAgent);
    if (effectiveUserAgent.isNull())
        effectiveUserAgent = m_userAgent;

    //pageConfiguration.loaderClientForMainFrame = makeUniqueRef<RemoteWorkerFrameLoaderClient>(m_webPageProxyID, m_pageID, FrameIdentifier::generate(), effectiveUserAgent);

#if !RELEASE_LOG_DISABLED
    auto serviceWorkerIdentifier = contextData.serviceWorkerIdentifier;
#endif
    
    auto lastNavigationWasAppInitiated = contextData.lastNavigationWasAppInitiated;
    auto page = makeUniqueRef<Page>(WTFMove(pageConfiguration));
/*    if (m_preferencesStore) {
        WebPage::updateSettingsGenerated(*m_preferencesStore, page->settings());
        page->settings().setStorageBlockingPolicy(static_cast<StorageBlockingPolicy>(m_preferencesStore->getUInt32ValueForKey(WebPreferencesKey::storageBlockingPolicyKey())));
    }*/
    page->setupForRemoteWorker(contextData.scriptURL, contextData.registration.key.topOrigin(), contextData.referrerPolicy);

    std::unique_ptr<WebCore::NotificationClient> notificationClient;
#if ENABLE(NOTIFICATIONS)
//    notificationClient = makeUnique<WebNotificationClient>(nullptr);
#endif

    auto serviceWorkerThreadProxy = ServiceWorkerThreadProxy::create(WTFMove(page), WTFMove(contextData), WTFMove(workerData), WTFMove(effectiveUserAgent), workerThreadMode, WebProcess::singleton().cacheStorageProvider(), WTFMove(notificationClient));

    if (lastNavigationWasAppInitiated)
        serviceWorkerThreadProxy->setLastNavigationWasAppInitiated(lastNavigationWasAppInitiated == WebCore::LastNavigationWasAppInitiated::Yes);

    SWContextManager::singleton().registerServiceWorkerThreadForInstall(WTFMove(serviceWorkerThreadProxy));

    RELEASE_LOG(ServiceWorker, "Created service worker %" PRIu64 " in process PID %i", serviceWorkerIdentifier.toUInt64(), getCurrentProcessID());
}

void WebSWContextManagerConnection::setUserAgent(String&& userAgent)
{
    m_userAgent = WTFMove(userAgent);
}

void WebSWContextManagerConnection::serviceWorkerStarted(std::optional<ServiceWorkerJobDataIdentifier> jobDataIdentifier, WebCore::ServiceWorkerIdentifier serviceWorkerIdentifier, bool doesHandleFetch)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    m_swContextConnection->scriptContextStarted(jobDataIdentifier, serviceWorkerIdentifier, doesHandleFetch);
}

void WebSWContextManagerConnection::serviceWorkerFailedToStart(std::optional<ServiceWorkerJobDataIdentifier> jobDataIdentifier, WebCore::ServiceWorkerIdentifier serviceWorkerIdentifier, const String& exceptionMessage)
{
    D(dprintf("%s(%d): %s\n", __PRETTY_FUNCTION__, WTF::isMainThread(), exceptionMessage.utf8().data()));
    m_swContextConnection->scriptContextFailedToStart(jobDataIdentifier, serviceWorkerIdentifier, exceptionMessage);
}

static inline bool isValidFetch(const ResourceRequest& request, const FetchOptions& options, const URL& serviceWorkerURL, const String& referrer)
{
    // For exotic service workers, do not enforce checks.
    if (!serviceWorkerURL.protocolIsInHTTPFamily())
        return true;

    if (options.mode == FetchOptions::Mode::Navigate) {
        if (!protocolHostAndPortAreEqual(request.url(), serviceWorkerURL)) {
            RELEASE_LOG_ERROR(ServiceWorker, "Should not intercept a navigation load that is not same-origin as the service worker URL");
            RELEASE_ASSERT_WITH_MESSAGE(request.url().host() == serviceWorkerURL.host(), "Hosts do not match");
            RELEASE_ASSERT_WITH_MESSAGE(request.url().protocol() == serviceWorkerURL.protocol(), "Protocols do not match");
            RELEASE_ASSERT_WITH_MESSAGE(request.url().port() == serviceWorkerURL.port(), "Ports do not match");
            return false;
        }
        return true;
    }

    String origin = request.httpOrigin();
    URL url { URL(), origin.isEmpty() ? referrer : origin };
    if (url.protocolIsInHTTPFamily() && !protocolHostAndPortAreEqual(url, serviceWorkerURL)) {
        RELEASE_LOG_ERROR(ServiceWorker, "Should not intercept a non navigation load that is not originating from a same-origin context as the service worker URL");
        ASSERT(url.host() == serviceWorkerURL.host());
        ASSERT(url.protocol() == serviceWorkerURL.protocol());
        ASSERT(url.port() == serviceWorkerURL.port());
        return false;
    }
    return true;
}

void WebSWContextManagerConnection::cancelFetch(SWServerConnectionIdentifier serverConnectionIdentifier, WebCore::ServiceWorkerIdentifier serviceWorkerIdentifier, FetchIdentifier fetchIdentifier)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));

    if (auto* serviceWorkerThreadProxy = SWContextManager::singleton().serviceWorkerThreadProxy(serviceWorkerIdentifier))
        serviceWorkerThreadProxy->cancelFetch(serverConnectionIdentifier, fetchIdentifier);
}

void WebSWContextManagerConnection::continueDidReceiveFetchResponse(SWServerConnectionIdentifier serverConnectionIdentifier, WebCore::ServiceWorkerIdentifier serviceWorkerIdentifier, FetchIdentifier fetchIdentifier)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));

    auto* serviceWorkerThreadProxy = SWContextManager::singleton().serviceWorkerThreadProxy(serviceWorkerIdentifier);
    RELEASE_LOG(ServiceWorker, "WebSWContextManagerConnection::continueDidReceiveFetchResponse for service worker %llu, fetch identifier %llu, has service worker %d", serviceWorkerIdentifier.toUInt64(), fetchIdentifier.toUInt64(), !!serviceWorkerThreadProxy);

    if (serviceWorkerThreadProxy)
        serviceWorkerThreadProxy->continueDidReceiveFetchResponse(serverConnectionIdentifier, fetchIdentifier);
}

void WebSWContextManagerConnection::startFetch(SWServerConnectionIdentifier serverConnectionIdentifier, WebCore::ServiceWorkerIdentifier serviceWorkerIdentifier, FetchIdentifier fetchIdentifier, ResourceRequest&& request, FetchOptions&& options, String&& referrer, bool isServiceWorkerNavigationPreloadEnabled, String&& clientIdentifier, String&& resultingClientIdentifier)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    auto* serviceWorkerThreadProxy = SWContextManager::singleton().serviceWorkerThreadProxy(serviceWorkerIdentifier);
    if (!serviceWorkerThreadProxy) {
        // m_connectionToNetworkProcess->send(Messages::ServiceWorkerFetchTask::DidNotHandle { }, fetchIdentifier);
        return;
    }

    serviceWorkerThreadProxy->setLastNavigationWasAppInitiated(request.isAppInitiated());

    if (!isValidFetch(request, options, serviceWorkerThreadProxy->scriptURL(), referrer)) {
        // m_connectionToNetworkProcess->send(Messages::ServiceWorkerFetchTask::DidNotHandle { }, fetchIdentifier);
        return;
    }

/*
    auto client = WebServiceWorkerFetchTaskClient::create(// m_connectionToNetworkProcess.copyRef(), serviceWorkerIdentifier, serverConnectionIdentifier, fetchIdentifier, request.requester() == ResourceRequest::Requester::Main);

    request.setHTTPBody(formData.takeData());
    serviceWorkerThreadProxy->startFetch(serverConnectionIdentifier, fetchIdentifier, WTFMove(client), WTFMove(request), WTFMove(referrer), WTFMove(options), isServiceWorkerNavigationPreloadEnabled, WTFMove(clientIdentifier), WTFMove(resultingClientIdentifier));
*/
}

void WebSWContextManagerConnection::postMessageToServiceWorker(WebCore::ServiceWorkerIdentifier destinationIdentifier, MessageWithMessagePorts&& message, ServiceWorkerOrClientData&& sourceData)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));

    SWContextManager::singleton().postMessageToServiceWorker(destinationIdentifier, WTFMove(message), WTFMove(sourceData));
}

void WebSWContextManagerConnection::fireInstallEvent(WebCore::ServiceWorkerIdentifier identifier)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    SWContextManager::singleton().fireInstallEvent(identifier);
}

void WebSWContextManagerConnection::fireActivateEvent(WebCore::ServiceWorkerIdentifier identifier)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    SWContextManager::singleton().fireActivateEvent(identifier);
}

void WebSWContextManagerConnection::firePushEvent(WebCore::ServiceWorkerIdentifier identifier,  CompletionHandler<void(bool)>&& callback)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));

    std::optional<Vector<uint8_t>> data;
//    if (ipcData)
//        data = Vector<uint8_t> { ipcData->data(), ipcData->size() };
    SWContextManager::singleton().firePushEvent(identifier, WTFMove(data), WTFMove(callback));
}

void WebSWContextManagerConnection::terminateWorker(WebCore::ServiceWorkerIdentifier identifier)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    SWContextManager::singleton().terminateWorker(identifier, SWContextManager::workerTerminationTimeout, nullptr);
}

void WebSWContextManagerConnection::postMessageToServiceWorkerClient(const ScriptExecutionContextIdentifier& destinationIdentifier, const MessageWithMessagePorts& message, WebCore::ServiceWorkerIdentifier sourceIdentifier, const String& sourceOrigin)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));

    m_swContextConnection->postMessageToServiceWorkerClient(destinationIdentifier, message, sourceIdentifier, sourceOrigin);
}

void WebSWContextManagerConnection::didFinishInstall(std::optional<ServiceWorkerJobDataIdentifier> jobDataIdentifier, WebCore::ServiceWorkerIdentifier serviceWorkerIdentifier, bool wasSuccessful)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    m_swContextConnection->didFinishInstall(jobDataIdentifier, serviceWorkerIdentifier, wasSuccessful);
}

void WebSWContextManagerConnection::didFinishActivation(WebCore::ServiceWorkerIdentifier serviceWorkerIdentifier)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    m_swContextConnection->didFinishActivation(serviceWorkerIdentifier);
}

void WebSWContextManagerConnection::setServiceWorkerHasPendingEvents(WebCore::ServiceWorkerIdentifier serviceWorkerIdentifier, bool hasPendingEvents)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));

    m_swContextConnection->setServiceWorkerHasPendingEvents(serviceWorkerIdentifier, hasPendingEvents);
}

void WebSWContextManagerConnection::skipWaiting(WebCore::ServiceWorkerIdentifier serviceWorkerIdentifier, CompletionHandler<void()>&& callback)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    m_swContextConnection->skipWaiting(serviceWorkerIdentifier, WTFMove(callback));
    // m_connectionToNetworkProcess->sendWithAsyncReply(Messages::WebSWServerToContextConnection::SkipWaiting(serviceWorkerIdentifier), WTFMove(callback));
}

void WebSWContextManagerConnection::setScriptResource(WebCore::ServiceWorkerIdentifier serviceWorkerIdentifier, const URL& url, const ServiceWorkerContextData::ImportedScript& script)
{
    m_swContextConnection->setScriptResource(serviceWorkerIdentifier, URL(url), ServiceWorkerContextData::ImportedScript(script));
    // m_connectionToNetworkProcess->send(Messages::WebSWServerToContextConnection::SetScriptResource { serviceWorkerIdentifier, url, script }, 0);
}

void WebSWContextManagerConnection::workerTerminated(WebCore::ServiceWorkerIdentifier serviceWorkerIdentifier)
{
    m_swContextConnection->workerTerminated(serviceWorkerIdentifier);
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    // m_connectionToNetworkProcess->send(Messages::WebSWServerToContextConnection::WorkerTerminated(serviceWorkerIdentifier), 0);
}

void WebSWContextManagerConnection::findClientByVisibleIdentifier(WebCore::ServiceWorkerIdentifier serviceWorkerIdentifier, const String& clientIdentifier, FindClientByIdentifierCallback&& callback)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    m_swContextConnection->findClientByVisibleIdentifier(serviceWorkerIdentifier, clientIdentifier, WTFMove(callback));
    // m_connectionToNetworkProcess->sendWithAsyncReply(Messages::WebSWServerToContextConnection::FindClientByVisibleIdentifier { serviceWorkerIdentifier, clientIdentifier }, WTFMove(callback));
}

void WebSWContextManagerConnection::matchAll(WebCore::ServiceWorkerIdentifier serviceWorkerIdentifier, const ServiceWorkerClientQueryOptions& options, ServiceWorkerClientsMatchAllCallback&& callback)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));

    auto requestIdentifier = ++m_previousRequestIdentifier;
    m_matchAllRequests.add(requestIdentifier, WTFMove(callback));
    m_swContextConnection->matchAll(requestIdentifier, serviceWorkerIdentifier, options);
    // m_connectionToNetworkProcess->send(Messages::WebSWServerToContextConnection::MatchAll { requestIdentifier, serviceWorkerIdentifier, options }, 0);
}

void WebSWContextManagerConnection::matchAllCompleted(uint64_t requestIdentifier, Vector<ServiceWorkerClientData>&& clientsData)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));

    if (auto callback = m_matchAllRequests.take(requestIdentifier))
    {
        callback(WTFMove(clientsData));
    }
    else
    {
        D(dprintf("--!no matchall callback!\n"));
    }
}

void WebSWContextManagerConnection::claim(WebCore::ServiceWorkerIdentifier serviceWorkerIdentifier, CompletionHandler<void(ExceptionOr<void>&&)>&& callback)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));

    m_swContextConnection->claim(serviceWorkerIdentifier, [callback = WTFMove(callback)](auto&& result) mutable {
        callback(result ? result->toException() : ExceptionOr<void> { });
    });

    // m_connectionToNetworkProcess->sendWithAsyncReply(Messages::WebSWServerToContextConnection::Claim { serviceWorkerIdentifier }, [callback = WTFMove(callback)](auto&& result) mutable {
//        callback(result ? result->toException() : ExceptionOr<void> { });
//    });
}

void WebSWContextManagerConnection::close()
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));

    RELEASE_LOG(ServiceWorker, "Service worker process is requested to stop all service workers");
    setAsClosed();

    // m_connectionToNetworkProcess->send(Messages::NetworkConnectionToWebProcess::CloseSWContextConnection { }, 0);
    SWContextManager::singleton().stopAllServiceWorkers();
//??
//    WebProcess::singleton().enableTermination();
}

void WebSWContextManagerConnection::setThrottleState(bool isThrottleable)
{
    RELEASE_LOG(ServiceWorker, "Service worker throttleable state is set to %d", isThrottleable);
    m_isThrottleable = isThrottleable;
//    WebProcess::singleton().setProcessSuppressionEnabled(isThrottleable);
}

bool WebSWContextManagerConnection::isThrottleable() const
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));

    return m_isThrottleable;
}

void WebSWContextManagerConnection::convertFetchToDownload(SWServerConnectionIdentifier serverConnectionIdentifier, WebCore::ServiceWorkerIdentifier serviceWorkerIdentifier, FetchIdentifier fetchIdentifier)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));

    if (auto* serviceWorkerThreadProxy = SWContextManager::singleton().serviceWorkerThreadProxy(serviceWorkerIdentifier))
        serviceWorkerThreadProxy->convertFetchToDownload(serverConnectionIdentifier, fetchIdentifier);
}

void WebSWContextManagerConnection::didFailHeartBeatCheck(WebCore::ServiceWorkerIdentifier serviceWorkerIdentifier)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));

    // m_connectionToNetworkProcess->send(Messages::WebSWServerToContextConnection::DidFailHeartBeatCheck { serviceWorkerIdentifier }, 0);
}

} // namespace WebCore

#endif // ENABLE(SERVICE_WORKER)
