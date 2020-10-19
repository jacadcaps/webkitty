#undef __OBJC__
#import "WebKit.h"
#import <WebCore/IntRect.h>
#import <WebCore/FloatRect.h>
#import <WebCore/PrintContext.h>
#define __OBJC__

#import "WkPrinting.h"
#import <libraries/ppd.h>

@class WkWebView, OBArray;

namespace WebCore {
	class Frame;
	class PrintContext;
}

@interface WkPrintingProfile (Internal)

+ (OBArray /* OBString */ *)allProfiles;
+ (OBString *)defaultProfile;

+ (WkPrintingProfile *)spoolInfoForProfile:(OBString *)profile;

@end

@interface WkPrintingProfilePrivate : WkPrintingProfile
{
	Library  *_ppdBase;
	OBString *_profile;
	PPD      *_ppd;
}

- (id)initWithProfile:(OBString *)profile;

@end

@interface WkPrintingPagePrivate : WkPrintingPage
{
	OBString *_name;
	OBString *_key;
	float     _width;
	float     _height;
	float     _marginLeft;
	float     _marginRight;
	float     _marginTop;
	float     _marginBottom;
}

+ (WkPrintingPagePrivate *)pageWithName:(OBString *)name key:(OBString *)key width:(float)width height:(float)height
	marginLeft:(float)mleft marginRight:(float)mright marginTop:(float)mtop marginBottom:(float)mbottom;

@end

@interface WkPrintingStatePrivate : WkPrintingState
{
	WkWebView             *_webView;
	WebCore::PrintContext *_context;
	WkPrintingProfile     *_profile;
	OBArray               *_profiles;
}

- (id)initWithWebView:(WkWebView *)view frame:(WebCore::Frame *)frame;

@end
