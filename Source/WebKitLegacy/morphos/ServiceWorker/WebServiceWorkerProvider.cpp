#include "WebServiceWorkerProvider.h"

#if ENABLE(SERVICE_WORKER)

#include <WebCore/ServiceWorkerProvider.h>
#include <WebCore/WorkerSWClientConnection.h>
#include <WebCore/ProcessIdentifier.h>
#include <WebCore/SecurityOrigin.h>
#include <WebCore/ExceptionData.h>
#include <wtf/NeverDestroyed.h>
#include "WebProcess.h"
#include "WebSWServerConnection.h"

#define D(x) x

namespace WebKit {

WebServiceWorkerProvider& WebServiceWorkerProvider::singleton()
{
    static NeverDestroyed<WebServiceWorkerProvider> provider;
    return provider;
}

WebServiceWorkerProvider::WebServiceWorkerProvider()
    : m_identifier(WebCore::Process::identifier())
    , m_swOriginTable(makeUniqueRef<WebSWOriginTable>())
{
    D(dprintf("%s(%d): identifier %s\n", __PRETTY_FUNCTION__, WTF::isMainThread(), m_identifier.loggingString().utf8().data()));
}

WebCore::SWClientConnection& WebServiceWorkerProvider::serviceWorkerConnection()
{
	return *this;
}

void WebServiceWorkerProvider::terminateWorkerForTesting(WebCore::ServiceWorkerIdentifier, CompletionHandler<void()>&&)
{
}

bool WebServiceWorkerProvider::mayHaveServiceWorkerRegisteredForOrigin(const WebCore::SecurityOriginData&) const
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    return false;
}

void WebServiceWorkerProvider::addServiceWorkerRegistrationInServer(WebCore::ServiceWorkerRegistrationIdentifier identifier)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    
    WebProcess::singleton().addServiceWorkerRegistration(identifier);
    WebProcess::singleton().swConnection()->addServiceWorkerRegistrationInServer(identifier);
}

void WebServiceWorkerProvider::removeServiceWorkerRegistrationInServer(WebCore::ServiceWorkerRegistrationIdentifier identifier)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    if (WebProcess::singleton().removeServiceWorkerRegistration(identifier))
        WebProcess::singleton().swConnection()->removeServiceWorkerRegistrationInServer(identifier);
}

void WebServiceWorkerProvider::scheduleJobInServer(const WebCore::ServiceWorkerJobData& data)
{
// C
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    WebProcess::singleton().swConnection()->scheduleJobInServer(std::move(WebCore::ServiceWorkerJobData(data)));
}

void WebServiceWorkerProvider::finishFetchingScriptInServer(const WebCore::ServiceWorkerJobDataIdentifier& identifier, const WebCore::ServiceWorkerRegistrationKey& regKey, const WebCore::WorkerFetchResult& fetchResult)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    WebProcess::singleton().swConnection()->finishFetchingScriptInServer(identifier, regKey, fetchResult);
}

void WebServiceWorkerProvider::postMessageToServiceWorker(WebCore::ServiceWorkerIdentifier destinationIdentifier, WebCore::MessageWithMessagePorts&& message, const WebCore::ServiceWorkerOrClientIdentifier& source)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    WebProcess::singleton().swConnection()->postMessageToServiceWorker(destinationIdentifier, WTFMove(message), source);
}

void WebServiceWorkerProvider::registerServiceWorkerClient(const WebCore::SecurityOrigin& topOrigin, const WebCore::ServiceWorkerClientData& data, const std::optional<WebCore::ServiceWorkerRegistrationIdentifier>& identifier, const String& userAgent)
{
//A
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    WebProcess::singleton().swConnection()->registerServiceWorkerClient(std::move(WebCore::SecurityOriginData(topOrigin.data())),
        std::move(WebCore::ServiceWorkerClientData(data)), identifier, std::move(WTF::String(userAgent)));
}

void WebServiceWorkerProvider::unregisterServiceWorkerClient(WebCore::ScriptExecutionContextIdentifier identifier)
{
//B
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    WebProcess::singleton().swConnection()->unregisterServiceWorkerClient(identifier);
}

void WebServiceWorkerProvider::scheduleUnregisterJobInServer(WebCore::ServiceWorkerRegistrationIdentifier identifier, WebCore::ServiceWorkerOrClientIdentifier workerIdentifier, CompletionHandler<void(WebCore::ExceptionOr<bool>&&)>&& callback)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    WebProcess::singleton().swConnection()->scheduleUnregisterJobInServer(WebCore::ServiceWorkerJobIdentifier::generateThreadSafe(), identifier, workerIdentifier, [completionHandler = WTFMove(callback)](auto&& result) mutable {
        if (!result.has_value())
            return completionHandler(result.error().toException());
        completionHandler(result.value());
    });
}

void WebServiceWorkerProvider::matchRegistration(WebCore::SecurityOriginData&& topOrigin, const URL& clientURL, RegistrationCallback&&)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
}

void WebServiceWorkerProvider::didMatchRegistration(uint64_t matchRequestIdentifier, std::optional<WebCore::ServiceWorkerRegistrationData>&&)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
}

void WebServiceWorkerProvider::didGetRegistrations(uint64_t matchRequestIdentifier, Vector<WebCore::ServiceWorkerRegistrationData>&&)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
}

void WebServiceWorkerProvider::whenRegistrationReady(const WebCore::SecurityOriginData& topOrigin, const URL& clientURL, WhenRegistrationReadyCallback&& callback)
{
// D
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    CompletionHandler<void(std::optional<WebCore::ServiceWorkerRegistrationData>&&)> completion = [call = std::move(callback)](std::optional<WebCore::ServiceWorkerRegistrationData>&& data) {
    D(dprintf("[whenRegistrationReady] CB!\n"));
        call(*std::move(data));
    };
    WebProcess::singleton().swConnection()->whenRegistrationReady(topOrigin, clientURL, std::move(completion));
}


void WebServiceWorkerProvider::setDocumentIsControlled(WebCore::ScriptExecutionContextIdentifier, WebCore::ServiceWorkerRegistrationData&&, CompletionHandler<void(bool)>&&)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
}


void WebServiceWorkerProvider::getRegistrations(WebCore::SecurityOriginData&& topOrigin, const URL& clientURL, GetRegistrationsCallback&&)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
}

void WebServiceWorkerProvider::whenServiceWorkerIsTerminatedForTesting(WebCore::ServiceWorkerIdentifier, CompletionHandler<void()>&&)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
}


void WebServiceWorkerProvider::didResolveRegistrationPromise(const WebCore::ServiceWorkerRegistrationKey& key)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
    WebProcess::singleton().swConnection()->didResolveRegistrationPromise(key);
}

void WebServiceWorkerProvider::storeRegistrationsOnDiskForTesting(CompletionHandler<void()>&&)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
}

void WebServiceWorkerProvider::subscribeToPushService(WebCore::ServiceWorkerRegistrationIdentifier, const Vector<uint8_t>& applicationServerKey, SubscribeToPushServiceCallback&&)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
}

void WebServiceWorkerProvider::unsubscribeFromPushService(WebCore::ServiceWorkerRegistrationIdentifier, WebCore::PushSubscriptionIdentifier, UnsubscribeFromPushServiceCallback&&)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
}

void WebServiceWorkerProvider::getPushSubscription(WebCore::ServiceWorkerRegistrationIdentifier, GetPushSubscriptionCallback&&)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
}

void WebServiceWorkerProvider::getPushPermissionState(WebCore::ServiceWorkerRegistrationIdentifier, GetPushPermissionStateCallback&&)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
}


void WebServiceWorkerProvider::enableNavigationPreload(WebCore::ServiceWorkerRegistrationIdentifier, ExceptionOrVoidCallback&&)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
}

void WebServiceWorkerProvider::disableNavigationPreload(WebCore::ServiceWorkerRegistrationIdentifier, ExceptionOrVoidCallback&&)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
}

void WebServiceWorkerProvider::setNavigationPreloadHeaderValue(WebCore::ServiceWorkerRegistrationIdentifier, String&&, ExceptionOrVoidCallback&&)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
}

void WebServiceWorkerProvider::getNavigationPreloadState(WebCore::ServiceWorkerRegistrationIdentifier, ExceptionOrNavigationPreloadStateCallback&&)
{
    D(dprintf("%s(%d): \n", __PRETTY_FUNCTION__, WTF::isMainThread()));
}

} // namespace WebKit

#endif // ENABLE(SERVICE_WORKER)
