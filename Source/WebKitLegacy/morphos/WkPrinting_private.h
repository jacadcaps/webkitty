#import "WkPrinting.h"
#import <libraries/ppd.h>

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
