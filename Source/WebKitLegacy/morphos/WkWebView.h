#import <mui/MUIArea.h>

@class WkWebViewPrivate;

@interface WkWebView : MUIArea
{
	WkWebViewPrivate *_private;
	bool              _drawPending;
}

+ (void)shutdown;

- (void)navigateTo:(OBString *)uri;
- (void)dumpDebug;

@end
