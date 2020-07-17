#import "WkSettings.h"

@interface WkSettingsPrivate : WkSettings
{
	bool _script;
	bool _adBlocker;
	bool _thCookies;
}
@end

@implementation WkSettingsPrivate

- (id)init
{
	if ((self = [super init]))
	{
		_script = YES;
		_adBlocker = YES;
		_thCookies = YES;
	}
	
	return self;
}

- (BOOL)javaScriptEnabled
{
	return _script;
}

- (void)setJavaScriptEnabled:(BOOL)enabled
{
	_script  = enabled;
}

- (BOOL)adBlockerEnabled
{
	return _adBlocker;
}

- (void)setAdBlockerEnabled:(BOOL)enabled
{
	_adBlocker = enabled;
}

- (BOOL)thirdPartyCookiesAllowed
{
	return _thCookies;
}

- (void)setThirdPartyCookiesAllowed:(BOOL)allowCookies
{
	_thCookies = allowCookies;
}

@end

@implementation WkSettings

+ (WkSettings *)settings
{
	return [[WkSettingsPrivate new] autorelease];
}

- (BOOL)javaScriptEnabled
{
	return YES;
}

- (void)setJavaScriptEnabled:(BOOL)enabled
{
}

- (BOOL)adBlockerEnabled
{
	return YES;
}

- (void)setAdBlockerEnabled:(BOOL)enabled
{
}

- (BOOL)thirdPartyCookiesAllowed
{
	return NO;
}

- (void)setThirdPartyCookiesAllowed:(BOOL)allowCookies
{
}

@end
