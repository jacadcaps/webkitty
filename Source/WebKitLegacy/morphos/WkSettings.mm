#import "WkSettings.h"

#undef __OBJC__
#import "WebKit.h"
#import <wtf/WallTime.h>
#import <wtf/text/WTFString.h>
#import <WebCore/CurlDownload.h>
#import <WebCore/ResourceResponse.h>
#import <WebCore/ResourceHandle.h>
#import <WebCore/ResourceResponse.h>
#import <WebCore/TextEncoding.h>
#import <wtf/FileSystem.h>
#import <WebProcess.h>
#define __OBJC__

typedef enum _cairo_antialias {
    CAIRO_ANTIALIAS_DEFAULT,
    CAIRO_ANTIALIAS_NONE,
    CAIRO_ANTIALIAS_GRAY,
    CAIRO_ANTIALIAS_SUBPIXEL,
    CAIRO_ANTIALIAS_FAST,
    CAIRO_ANTIALIAS_GOOD,
    CAIRO_ANTIALIAS_BEST
} cairo_antialias_t;

namespace WebCore {
	void setDefaultCairoFontAntialias(cairo_antialias_t aa);
}

@interface WkSettingsPrivate : WkSettings
{
	WkSettings_Throttling _throttling;
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
		_throttling = WkSettings_Throttling_InvisibleBrowsers;
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

- (WkSettings_Throttling)throttling
{
	return _throttling;
}

- (void)setThrottling:(WkSettings_Throttling)throttling
{
	_throttling = throttling;
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

- (WkSettings_Throttling)throttling
{
	return WkSettings_Throttling_InvisibleBrowsers;
}

- (void)setThrottling:(WkSettings_Throttling)throttling
{
}

@end

@implementation WkGlobalSettings

+ (OBString *)downloadPath
{
	WTF::String str = WTF::FileSystemImpl::temporaryFilePathForPrefix("download");
	auto udata = str.utf8();
	return [OBString stringWithUTF8String:udata.data()];
}

+ (void)setDownloadPath:(OBString *)path
{
	const char *cpath = [path nativeCString];
	WebCore::CurlRequest::SetDownloadPath(WTF::String(cpath, strlen(cpath), MIBENUM_SYSTEM));
	WTF::FileSystemImpl::setTemporaryFilePathForPrefix(cpath, "download");
}

static cairo_antialias_t defaultAA;

+ (WkGlobalSettings_Antialias)fontAntialias
{
	switch (defaultAA)
	{
	case CAIRO_ANTIALIAS_NONE: return WkGlobalSettings_Antialias_None;
	case CAIRO_ANTIALIAS_GRAY: return WkGlobalSettings_Antialias_Gray;
	default: return WkGlobalSettings_Antialias_Subpixel;
	}
}

+ (void)setAntialias:(WkGlobalSettings_Antialias)aa
{
	switch (aa)
	{
	case WkGlobalSettings_Antialias_None: defaultAA = CAIRO_ANTIALIAS_NONE; break;
	case WkGlobalSettings_Antialias_Gray: defaultAA = CAIRO_ANTIALIAS_GRAY; break;
	case WkGlobalSettings_Antialias_Subpixel: defaultAA = CAIRO_ANTIALIAS_SUBPIXEL; break;
	}
	WebCore::setDefaultCairoFontAntialias(defaultAA);
}

+ (void)setCustomCertificate:(OBString *)pathToPEM forHost:(OBString *)host withKey:(OBString *)key
{
	if ([pathToPEM length] && [host length])
	{
		WTF::String sPath = WTF::String::fromUTF8([[pathToPEM absolutePath] nativeCString]);
		WTF::String sDomain = WTF::String::fromUTF8([host cString]);
		WTF::String sKey = key ? WTF::String::fromUTF8([key cString]) : "";
		WebCore::ResourceHandle::setClientCertificateInfo(sDomain, sPath, sKey);
	}
	else if ([host length])
	{
		WTF::String sDomain = WTF::String::fromUTF8([host cString]);
		WebCore::ResourceHandle::clearClientCertificateInfo(sDomain);
	}
}

+ (void)ignoreSSLErrorsForHost:(OBString *)host
{
	if ([host length])
	{
		WTF::String sDomain = WTF::String::fromUTF8([host cString]);
		WebCore::ResourceHandle::setHostAllowsAnyHTTPSCertificate(sDomain);
	}
}

@end

