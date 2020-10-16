#import <ob/OBString.h>
#import <mui/MUIArea.h>
#import <mui/MUIGroup.h>

@class WkWebView, WkPrintingState;

@interface WkPrintingPage : OBObject

- (OBString *)name;
- (OBString *)key;

- (float)width;
- (float)height;

- (float)marginLeft;
- (float)marginRight;
- (float)marginTop;
- (float)marginBottom;

@end

@interface WkPrintingProfile : OBObject

+ (OBArray /* OBString */ *)allProfiles;
+ (OBString *)defaultProfile;

+ (WkPrintingProfile *)spoolInfoForProfile:(OBString *)profile;

- (OBArray /* WkPrintingPage */*)pageFormats;
- (WkPrintingPage *)defaultPageFormat;
- (WkPrintingPage *)selectedPageFormat;

- (OBString *)printerModel;
- (OBString *)manufacturer;
- (LONG)psLevel;

@end

@interface WkPrintingPreview : MUIArea

+ (WkPrintingPreview *)previewWithState:(WkPrintingState *)state;

- (WkPrintingState *)printingState;

@end

@interface WkPrintingSettingsGroup : MUIGroup

+ (WkPrintingSettingsGroup *)settingsWithState:(WkPrintingSettingsGroup *)state;

- (WkPrintingState *)printingState;

@end

@interface WkPrintingState : OBObject

- (WkWebView *)webView;

- (WkPrintingProfile *)profile;
- (void)setProfile:(WkPrintingProfile *)profile;

@end
