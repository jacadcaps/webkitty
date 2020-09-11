#import <ob/OBString.h>

typedef enum
{
	WkSettings_Throttling_None,
	WkSettings_Throttling_InvisibleBrowsers,
	WkSettings_Throttling_All,
} WkSettings_Throttling;

typedef enum
{
	WkSettings_Interpolation_Low,
	WkSettings_Interpolation_Medium,
	WkSettings_Interpolation_High,
} WkSettings_Interpolation;

@interface WkSettings : OBObject

+ (WkSettings *)settings;

- (BOOL)javaScriptEnabled;
- (void)setJavaScriptEnabled:(BOOL)enabled;

- (BOOL)adBlockerEnabled;
- (void)setAdBlockerEnabled:(BOOL)enabled;

- (BOOL)thirdPartyCookiesAllowed;
- (void)setThirdPartyCookiesAllowed:(BOOL)allowCookies;

- (WkSettings_Throttling)throttling;
- (void)setThrottling:(WkSettings_Throttling)throttling;

- (WkSettings_Interpolation)interpolation;
- (void)setInterpolation:(WkSettings_Interpolation)interpolation;

@end

typedef enum
{
	WkGlobalSettings_Antialias_None,
	WkGlobalSettings_Antialias_Gray,
	WkGlobalSettings_Antialias_Subpixel
} WkGlobalSettings_Antialias;

typedef enum
{
	// Most of the in-memory caches are off or at a low threshold, whole-page cache is off
	WkGlobalSettings_Caching_Minimal,
	// Cache a couple of whole-pages to allow faster back/forward navigation, cache more
	// resources in RAM, depending on system total RAM available
	WkGlobalSettings_Caching_Balanced,
} WkGlobalSettings_Caching;

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

// Set the caching model, defaults to Balanced
+ (void)setCaching:(WkGlobalSettings_Caching)caching;
+ (WkGlobalSettings_Caching)caching;

@end
