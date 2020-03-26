#import <mui/MUIArea.h>

class WebView;

@interface WkWebView : MUIArea
{
	WebView *_webView;
}

- (void)navigateTo:(OBString *)uri;

@end
