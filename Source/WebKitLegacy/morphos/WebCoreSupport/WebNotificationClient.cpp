#include "WebNotificationClient.h"

#if ENABLE(NOTIFICATIONS)

#include <WebCore/NotificationData.h>
#include <WebCore/ScriptExecutionContext.h>
#include <WebCore/NotificationPermissionCallback.h>
#include <wtf/CompletionHandler.h>
#include "WebPage.h"

#define D(x) 

using namespace WebCore;

namespace  WebKit {

WebNotificationClient::WebNotificationClient(WebPage *webView)
    : m_webPage(webView)
{
	D(dprintf("%s(%p)\n", __PRETTY_FUNCTION__, this));
}

bool WebNotificationClient::show(WebCore::ScriptExecutionContext& context, WebCore::NotificationData&& notification, RefPtr<WebCore::NotificationResources>&&, WTF::CompletionHandler<void()>&& onCompleted)
{
	if (!m_webPage->_fShowNotification)
		return false;

	D(dprintf("%s(%p): %p\n", __PRETTY_FUNCTION__, this, notification));

	m_webPage->_fShowNotification(WTFMove(notification));

    onCompleted();
    return true;
}

void WebNotificationClient::cancel(WebCore::NotificationData&& notification)
{
	D(dprintf("%s(%p): %p\n", __PRETTY_FUNCTION__, this, notification));
	if (m_webPage->_fHideNotification)
		m_webPage->_fHideNotification(WTFMove(notification));
}

void WebNotificationClient::notificationObjectDestroyed(WebCore::NotificationData&& notification)
{
	if (m_webPage->_fHideNotification)
		m_webPage->_fHideNotification(WTFMove(notification));
}

void WebNotificationClient::notificationControllerDestroyed()
{
	D(dprintf("%s(%p):\n", __PRETTY_FUNCTION__, this));
	// Means our WkWebView gets destroyed too, so all of the client-side notifications should be
	// cleaned by the client
    delete this;
}

void WebNotificationClient::requestPermission(WebCore::ScriptExecutionContext&context, WebCore::NotificationClient::PermissionHandler&&callback)
{
	// TODO: call WkWebView via page
	D(dprintf("%s(%p): %p\n", __PRETTY_FUNCTION__, this, &context));
	if (m_webPage->_fRequestNotificationPermission)
	{
		m_webPage->_fRequestNotificationPermission(context.url(), WTFMove(callback));
	}
	else
	{
		callback(NotificationClient::Permission::Denied);
	}
}

NotificationClient::Permission WebNotificationClient::checkPermission(ScriptExecutionContext* context)
{
    if (!context || !context->isDocument() || !m_webPage->_fCheckNotificationPermission)
        return NotificationClient::Permission::Denied;

	auto permission = m_webPage->_fCheckNotificationPermission(context->url());

	if (permission == WebViewDelegate::NotificationPermission::Default)
		return NotificationClient::Permission::Default;

	if (permission == WebViewDelegate::NotificationPermission::Grant)
            return NotificationClient::Permission::Granted;

	return NotificationClient::Permission::Denied;
}

}

#endif
