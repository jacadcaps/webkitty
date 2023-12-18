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

#pragma once
#include "WebKit.h"

#if ENABLE(SERVICE_WORKER)

#include <WebCore/ServiceWorkerProvider.h>
#include <WebCore/SWClientConnection.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/UniqueRef.h>
#include "WebSWOriginTable.h"

namespace WebKit {

class WebServiceWorkerProvider final : public WebCore::ServiceWorkerProvider, public WebCore::SWClientConnection {
public:
    static WebServiceWorkerProvider& singleton();

    void addServiceWorkerRegistrationInServer(WebCore::ServiceWorkerRegistrationIdentifier) final;
    void removeServiceWorkerRegistrationInServer(WebCore::ServiceWorkerRegistrationIdentifier) final;

    WebCore::SWServerConnectionIdentifier serverConnectionIdentifier() const final { return m_identifier; }
    bool mayHaveServiceWorkerRegisteredForOrigin(const WebCore::SecurityOriginData&) const final;

public:
    friend NeverDestroyed<WebServiceWorkerProvider>;
    WebServiceWorkerProvider();

    WebCore::SWClientConnection& serviceWorkerConnection() final;
    void terminateWorkerForTesting(WebCore::ServiceWorkerIdentifier, CompletionHandler<void()>&&) final;

    void scheduleJobInServer(const WebCore::ServiceWorkerJobData&) final;
    void finishFetchingScriptInServer(const WebCore::ServiceWorkerJobDataIdentifier&, const WebCore::ServiceWorkerRegistrationKey&, const WebCore::WorkerFetchResult&) final;
    void postMessageToServiceWorker(WebCore::ServiceWorkerIdentifier destinationIdentifier, WebCore::MessageWithMessagePorts&&, const WebCore::ServiceWorkerOrClientIdentifier& source) final;
    void registerServiceWorkerClient(const WebCore::SecurityOrigin& topOrigin, const WebCore::ServiceWorkerClientData&, const std::optional<WebCore::ServiceWorkerRegistrationIdentifier>&, const String& userAgent) final;
    void unregisterServiceWorkerClient(WebCore::ScriptExecutionContextIdentifier) final;
    void scheduleUnregisterJobInServer(WebCore::ServiceWorkerRegistrationIdentifier, WebCore::ServiceWorkerOrClientIdentifier, CompletionHandler<void(WebCore::ExceptionOr<bool>&&)>&&) final;

    void matchRegistration(WebCore::SecurityOriginData&& topOrigin, const URL& clientURL, RegistrationCallback&&) final;
    void didMatchRegistration(uint64_t matchRequestIdentifier, std::optional<WebCore::ServiceWorkerRegistrationData>&&);
    void didGetRegistrations(uint64_t matchRequestIdentifier, Vector<WebCore::ServiceWorkerRegistrationData>&&);
    void whenRegistrationReady(const WebCore::SecurityOriginData& topOrigin, const URL& clientURL, WhenRegistrationReadyCallback&&) final;

    void setDocumentIsControlled(WebCore::ScriptExecutionContextIdentifier, WebCore::ServiceWorkerRegistrationData&&, CompletionHandler<void(bool)>&&);

    void getRegistrations(WebCore::SecurityOriginData&& topOrigin, const URL& clientURL, GetRegistrationsCallback&&) final;
    void whenServiceWorkerIsTerminatedForTesting(WebCore::ServiceWorkerIdentifier, CompletionHandler<void()>&&) final;

    void didResolveRegistrationPromise(const WebCore::ServiceWorkerRegistrationKey&) final;
    void storeRegistrationsOnDiskForTesting(CompletionHandler<void()>&&) final;
    
    void subscribeToPushService(WebCore::ServiceWorkerRegistrationIdentifier, const Vector<uint8_t>& applicationServerKey, SubscribeToPushServiceCallback&&) final;
    void unsubscribeFromPushService(WebCore::ServiceWorkerRegistrationIdentifier, WebCore::PushSubscriptionIdentifier, UnsubscribeFromPushServiceCallback&&) final;
    void getPushSubscription(WebCore::ServiceWorkerRegistrationIdentifier, GetPushSubscriptionCallback&&) final;
    void getPushPermissionState(WebCore::ServiceWorkerRegistrationIdentifier, GetPushPermissionStateCallback&&) final;

    void enableNavigationPreload(WebCore::ServiceWorkerRegistrationIdentifier, ExceptionOrVoidCallback&&) final;
    void disableNavigationPreload(WebCore::ServiceWorkerRegistrationIdentifier, ExceptionOrVoidCallback&&) final;
    void setNavigationPreloadHeaderValue(WebCore::ServiceWorkerRegistrationIdentifier, String&&, ExceptionOrVoidCallback&&) final;
    void getNavigationPreloadState(WebCore::ServiceWorkerRegistrationIdentifier, ExceptionOrNavigationPreloadStateCallback&&) final;

private:
    WebCore::SWServerConnectionIdentifier m_identifier;
    WTF::UniqueRef<WebKit::WebSWOriginTable> m_swOriginTable;

}; // class WebServiceWorkerProvider

} // namespace WebKit

#endif // ENABLE(SERVICE_WORKER)
