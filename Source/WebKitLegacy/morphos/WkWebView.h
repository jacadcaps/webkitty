#import <mui/MUIArea.h>

class WebView;

@interface WkWebView : MUIArea
{
	WebView *_webView;
	bool _drawPending;
}

+ (void)shutdown;

- (void)navigateTo:(OBString *)uri;

@end
