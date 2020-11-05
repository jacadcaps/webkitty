#import <ob/OBString.h>
#import <mui/MUIArea.h>
#import <mui/MUIGroup.h>

@class WkWebView, WkPrintingState;

@interface WkPrintingPage : OBObject

- (OBString *)name;
- (OBString *)key;

// page sizes in inches
- (float)width;
- (float)height;

// without margins
- (float)contentWidth;
- (float)contentHeight;

// margin sizes in inches
- (float)marginLeft;
- (float)marginRight;
- (float)marginTop;
- (float)marginBottom;

@end

@interface WkPrintingProfile : OBObject

- (OBString *)name;

- (OBArray /* WkPrintingPage */*)pageFormats;
- (WkPrintingPage *)defaultPageFormat;
- (WkPrintingPage *)selectedPageFormat;

- (OBString *)printerModel;
- (OBString *)manufacturer;
- (LONG)psLevel;

- (BOOL)canSelectPageFormat;
- (void)setSelectedPageFormat:(WkPrintingPage *)page;

- (BOOL)isPDFFilePrinter;

@end

@interface WkPrintingRange : OBObject

// Do note that page numbering begins from 1!

+ (WkPrintingRange *)rangeWithPage:(LONG)pageNo;
+ (WkPrintingRange *)rangeFromPage:(LONG)pageStart toPage:(LONG)pageEnd;
+ (WkPrintingRange *)rangeFromPage:(LONG)pageStart count:(LONG)count;

- (LONG)pageStart;
- (LONG)pageEnd;
- (LONG)count;

@end

@interface WkPrintingState : OBObject

// Associated WkWebView
- (WkWebView *)webView;

// Selected printer
- (WkPrintingProfile *)profile;
- (void)setProfile:(WkPrintingProfile *)profile;

- (OBArray * /* WkPrintingProfile */)allProfiles;

// Calculated # of pages
- (LONG)pages;

// Previewed page no
- (LONG)previevedPage;
- (void)setPrevievedPage:(LONG)page;

// 1.0 for 100%
- (float)userScalingFactor;
- (void)setUserScalingFactor:(float)scaling;

// margin sizes in inches
- (float)marginLeft;
- (float)marginTop;
- (float)marginRight;
- (float)marginBottom;

- (void)setMarginLeft:(float)left top:(float)top right:(float)right bottom:(float)bottom;
- (void)resetMarginsToPaperDefaults;

- (WkPrintingPage *)pageWithMarginsApplied;

- (void)setLandscape:(BOOL)landscape;
- (BOOL)landscape;

// Only when printing to PostScript targets (no PDF)
- (LONG)pagesPerSheet;
- (void)setPagesPerSheet:(LONG)pps;

@end
