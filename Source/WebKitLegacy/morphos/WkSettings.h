#import <ob/OBString.h>

@interface WkSettings : OBObject

+ (WkSettings *)settings;

- (BOOL)javaScriptEnabled;
- (void)setJavaScriptEnabled:(BOOL)enabled;

- (BOOL)adBlockerEnabled;
- (void)setAdBlockerEnabled:(BOOL)enabled;

- (BOOL)thirdPartyCookiesAllowed;
- (void)setThirdPartyCookiesAllowed:(BOOL)allowCookies;

@end
