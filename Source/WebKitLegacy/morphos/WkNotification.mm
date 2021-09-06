#import "WkNotification_private.h"
#import <ob/OBURL.h>

static WTF::HashMap<WebCore::Notification *, id> _notificationLookup;

@implementation WkNotificationPrivate

- (id)initWithNotification:(RefPtr<WebCore::Notification>)notification
{
	if ((self = [super init]))
	{
		_notification = notification;
		_notificationLookup.add(notification.get(), self);
	}
	
	return self;
}

- (void)dealloc
{
	if (_notification.get())
		_notificationLookup.remove(_notification.get());
	[super dealloc];
}


+ (id)notificationForNotification:(WebCore::Notification *)notification
{
	auto it = _notificationLookup.find(notification);
	if (it != _notificationLookup.end())
		return it->value;
	return nil;
}

- (OBString *)title
{
	if (_notification.get())
	{
		auto utitle = _notification->title().utf8();
		return [OBString stringWithUTF8String:utitle.data()];
	}
	return nil;
}

- (OBString *)body
{
	if (_notification.get())
	{
		auto ubody = _notification->body().utf8();
		return [OBString stringWithUTF8String:ubody.data()];
	}
	return nil;
}

- (OBString *)language
{
	if (_notification.get())
	{
		auto ulanguage = _notification->lang().utf8();
		return [OBString stringWithUTF8String:ulanguage.data()];
	}
	return nil;
}

- (OBString *)tag
{
	if (_notification.get())
	{
		auto utag = _notification->tag().utf8();
		return [OBString stringWithUTF8String:utag.data()];
	}
	return nil;
}

- (OBURL *)icon
{
	if (_notification.get())
	{
		auto uurl = _notification->icon().string().utf8();
		return [OBURL URLWithString:[OBString stringWithUTF8String:uurl.data()]];
	}
	return nil;
}

// Report state to the website
- (void)notificationShown
{
	if (_notification.get())
		_notification->dispatchShowEvent();
}

- (void)notificationClosed
{
	if (_notification.get())
		_notification->dispatchCloseEvent();
}

- (void)notificationClicked
{
	if (_notification.get())
		_notification->dispatchClickEvent();
}

- (void)notificationNotShownDueToError
{
	if (_notification.get())
		_notification->dispatchErrorEvent();
}

@end

@implementation WkNotification

- (OBString *)title
{
	return nil;
}

- (OBString *)body
{
	return nil;
}

- (OBString *)language
{
	return nil;
}

- (OBString *)tag
{
	return nil;
}

- (OBURL *)icon
{
	return nil;
}

// Report state to the website
- (void)notificationShown
{
}

- (void)notificationClosed
{
}

- (void)notificationClicked
{
}

- (void)notificationNotShownDueToError
{
}

@end

@implementation WkNotificationPermissionResponsePrivate

- (id)initWithCallback:(RefPtr<WebCore::NotificationPermissionCallback>)callback
{
	if ((self = [super init]))
	{
		_callback = callback;
	}
	
	return self;
}

- (void)respondToNotificationPermission:(WkNotificationPermission)permission
{
	if (_callback.get())
	{
		switch (permission)
		{
		case WkNotificationPermission_Default:
			_callback->handleEvent(WebCore::Notification::Permission::Default);
			break;
		case WkNotificationPermission_Deny:
			_callback->handleEvent(WebCore::Notification::Permission::Denied);
			break;
		case WkNotificationPermission_Grant:
			_callback->handleEvent(WebCore::Notification::Permission::Granted);
			break;
		}
	}
}

@end

@implementation WkNotificationPermissionResponse
- (void)respondToNotificationPermission:(WkNotificationPermission)permission
{
	(void)permission;
}
@end
