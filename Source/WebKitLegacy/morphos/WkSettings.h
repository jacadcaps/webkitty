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

typedef enum
{
	WkGlobalSettings_Antialias_None,
	WkGlobalSettings_Antialias_Gray,
	WkGlobalSettings_Antialias_Subpixel
} WkGlobalSettings_Antialias;

@interface WkGlobalSettings : OBObject

// Set the default download path for all new downloads, they'll be downloaded with a tmp name
// and renamed to the name returned by decideFilenameForDownload:withSuggestedName: on success
+ (OBString *)downloadPath;
+ (void)setDownloadPath:(OBString *)path;

+ (WkGlobalSettings_Antialias)fontAntialias;
+ (void)setAntialias:(WkGlobalSettings_Antialias)aa;

// Sets a custom PEM file to be used to validate a connection to the given domain
// 'key' is an optional password required to load the PEM file
// Pass a nil pathToPEM to remove
+ (void)setCustomCertificate:(OBString *)pathToPEM forHost:(OBString *)host withKey:(OBString *)key;
// Ignore SSL errors for this host. Persistent until app's demise
+ (void)ignoreSSLErrorsForHost:(OBString *)host;

@end
