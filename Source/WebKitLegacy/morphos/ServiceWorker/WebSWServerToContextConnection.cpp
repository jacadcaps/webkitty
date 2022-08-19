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
#include "WebSWServerToContextConnection.h"

#if ENABLE(SERVICE_WORKER)

#include "ServiceWorkerFetchTask.h"
#include "WebSWServerConnection.h"
#include "WebSWContextManagerConnection.h"
#include <WebCore/SWServer.h>
#include <WebCore/ServiceWorkerContextData.h>
#include <WebCore/SWContextManager.h>

#define D(x) x

namespace WebKit {
using namespace WebCore;

WebSWServerToContextConnection::WebSWServerToContextConnection(WebPageProxyIdentifier webPageProxyID, RegistrableDomain&& registrableDomain, std::optional<ScriptExecutionContextIdentifier> serviceWorkerPageIdentifier, SWServer& server)
    : SWServerToContextConnection(WTFMove(registrableDomain), serviceWorkerPageIdentifier)
    , m_server(server)
    , m_webPageProxyID(webPageProxyID)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));

    server.addContextConnection(*this);
}

WebSWServerToContextConnection::~WebSWServerToContextConnection()
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    auto fetches = WTFMove(m_ongoingFetches);
//    for (auto& fetch : fetches.values())
//        fetch->contextClosed();

    auto downloads = WTFMove(m_ongoingDownloads);
//    for (auto& download : downloads.values())
//        download->contextClosed();

    if (m_server && m_server->contextConnectionForRegistrableDomain(registrableDomain()) == this)
        m_server->removeContextConnection(*this);
}

void WebSWServerToContextConnection::postMessageToServiceWorkerClient(const ScriptExecutionContextIdentifier& destinationIdentifier, const MessageWithMessagePorts& message, ServiceWorkerIdentifier sourceIdentifier, const String& sourceOrigin)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    if (!m_server)
        return;

    if (auto* connection = m_server->connection(destinationIdentifier.processIdentifier()))
        connection->postMessageToServiceWorkerClient(destinationIdentifier, message, sourceIdentifier, sourceOrigin);
}

void WebSWServerToContextConnection::installServiceWorkerContext(const ServiceWorkerContextData& contextData, const ServiceWorkerData& workerData, const String& userAgent, WorkerThreadMode workerThreadMode)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    static_cast<WebSWContextManagerConnection *>(SWContextManager::singleton().connection())->installServiceWorker(ServiceWorkerContextData(contextData), ServiceWorkerData(workerData), String(userAgent), workerThreadMode);

//    send(Messages::WebSWContextManagerConnection::InstallServiceWorker { contextData, workerData, userAgent, workerThreadMode });
}

void WebSWServerToContextConnection::updateAppInitiatedValue(ServiceWorkerIdentifier serviceWorkerIdentifier, WebCore::LastNavigationWasAppInitiated lastNavigationWasAppInitiated)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
//    send(Messages::WebSWContextManagerConnection::UpdateAppInitiatedValue(serviceWorkerIdentifier, lastNavigationWasAppInitiated));
}

void WebSWServerToContextConnection::fireInstallEvent(ServiceWorkerIdentifier serviceWorkerIdentifier)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    static_cast<WebSWContextManagerConnection *>(SWContextManager::singleton().connection())->fireInstallEvent(serviceWorkerIdentifier);
//    send(Messages::WebSWContextManagerConnection::FireInstallEvent(serviceWorkerIdentifier));
}

void WebSWServerToContextConnection::fireActivateEvent(ServiceWorkerIdentifier serviceWorkerIdentifier)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    static_cast<WebSWContextManagerConnection *>(SWContextManager::singleton().connection())->fireActivateEvent(serviceWorkerIdentifier);
//    send(Messages::WebSWContextManagerConnection::FireActivateEvent(serviceWorkerIdentifier));
}

void WebSWServerToContextConnection::firePushEvent(WebCore::ServiceWorkerIdentifier serviceWorkerIdentifier, const std::optional<Vector<uint8_t>>& data, CompletionHandler<void(bool)>&& callback)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
//    if (!m_processingPushEventsCount++)
//        m_connection.networkProcess().parentProcessConnection()->send(Messages::NetworkProcessProxy::StartServiceWorkerBackgroundProcessing { webProcessIdentifier() }, 0);
#if 0
    std::optional<IPC::DataReference> ipcData;
    if (data)
        ipcData = IPC::DataReference { data->data(), data->size() };
    sendWithAsyncReply(Messages::WebSWContextManagerConnection::FirePushEvent(serviceWorkerIdentifier, ipcData), [weakThis = WeakPtr { *this }, callback = WTFMove(callback)](bool wasProcessed) mutable {
        if (weakThis && !--weakThis->m_processingPushEventsCount)
            weakThis->m_connection.networkProcess().parentProcessConnection()->send(Messages::NetworkProcessProxy::EndServiceWorkerBackgroundProcessing { weakThis->webProcessIdentifier() }, 0);
        callback(wasProcessed);
    });
#endif
}

void WebSWServerToContextConnection::terminateWorker(ServiceWorkerIdentifier serviceWorkerIdentifier)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    //send(Messages::WebSWContextManagerConnection::TerminateWorker(serviceWorkerIdentifier));
}

void WebSWServerToContextConnection::didSaveScriptsToDisk(ServiceWorkerIdentifier serviceWorkerIdentifier, const ScriptBuffer& script, const HashMap<URL, ScriptBuffer>& importedScripts)
{
}

void WebSWServerToContextConnection::terminateDueToUnresponsiveness()
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
//    m_connection.networkProcess().parentProcessConnection()->send(Messages::NetworkProcessProxy::TerminateUnresponsiveServiceWorkerProcesses { webProcessIdentifier() }, 0);
}

void WebSWServerToContextConnection::matchAllCompleted(uint64_t requestIdentifier, const Vector<ServiceWorkerClientData>& clientsData)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    //send(Messages::WebSWContextManagerConnection::MatchAllCompleted { requestIdentifier, clientsData });
}

void WebSWServerToContextConnection::connectionIsNoLongerNeeded()
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
   // m_connection.serviceWorkerServerToContextConnectionNoLongerNeeded();
}

void WebSWServerToContextConnection::setThrottleState(bool isThrottleable)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    m_isThrottleable = isThrottleable;
    //send(Messages::WebSWContextManagerConnection::SetThrottleState { isThrottleable });
}

void WebSWServerToContextConnection::startFetch(ServiceWorkerFetchTask& task)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
//    task.start(*this);
}

void WebSWServerToContextConnection::registerFetch(ServiceWorkerFetchTask& task)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    ASSERT(!m_ongoingFetches.contains(task.fetchIdentifier()));
//    m_ongoingFetches.add(task.fetchIdentifier(), task);
}

void WebSWServerToContextConnection::unregisterFetch(ServiceWorkerFetchTask& task)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    ASSERT(m_ongoingFetches.contains(task.fetchIdentifier()));
//    m_ongoingFetches.remove(task.fetchIdentifier());
}

void WebSWServerToContextConnection::registerDownload(ServiceWorkerDownloadTask& task)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
   // ASSERT(!m_ongoingDownloads.contains(task.fetchIdentifier()));
 //   m_ongoingDownloads.add(task.fetchIdentifier(), task);
    //m_connection.connection().addThreadMessageReceiver(Messages::ServiceWorkerDownloadTask::messageReceiverName(), &task, task.fetchIdentifier().toUInt64());
}

void WebSWServerToContextConnection::unregisterDownload(ServiceWorkerDownloadTask& task)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
  //  m_ongoingDownloads.remove(task.fetchIdentifier());
    //m_connection.connection().removeThreadMessageReceiver(Messages::ServiceWorkerDownloadTask::messageReceiverName(), task.fetchIdentifier().toUInt64());
}

} // namespace WebKit

#endif // ENABLE(SERVICE_WORKER)
