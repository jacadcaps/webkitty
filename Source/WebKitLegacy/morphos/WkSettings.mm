#import "WkSettings.h"

@interface WkSettingsPrivate : WkSettings
{
	bool _script;
	bool _adBlocker;
}
@end

@implementation WkSettingsPrivate

- (id)init
{
	if ((self = [super init]))
	{
		_script = YES;
		_adBlocker = YES;
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

@end
