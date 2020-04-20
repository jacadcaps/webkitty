#import <mui/MUIArea.h>

@class WkWebView;
@class WkWebViewPrivate;

@protocol WkWebViewScrollingDelegate <OBObject>

- (void)webView:(WkWebView *)view changedContentsSizeToWidth:(int)width height:(int)height;
- (void)webView:(WkWebView *)view scrolledToLeft:(int)left top:(int)top;

@end

@protocol WkWebViewNetworkDelegate <OBObject>

// CAUTION: may be called on the network thread
- (OBString *)userAgentForURL:(OBString *)url;

- (void)webView:(WkWebView *)view changedTitle:(OBString *)newtitle;
- (void)webView:(WkWebView *)view changedDocumentURL:(OBString *)newURL;
- (void)webViewDidStartProvisionalLoading:(WkWebView *)view;
- (void)webViewDidFinishProvisionalLoading:(WkWebView *)view;

- (BOOL)webView:(WkWebView *)view wantsToCreateNewViewWithURL:(OBString *)url options:(OBDictionary *)options;
- (void)webView:(WkWebView *)view createdNewWebView:(WkWebView *)newview;

@end

@interface WkWebView : MUIArea
{
	WkWebViewPrivate *_private;
}

+ (void)shutdown;

- (void)navigateTo:(OBString *)uri;
- (void)stop;
- (void)reload;

- (void)scrollToLeft:(int)left top:(int)top;

- (void)setScrollingDelegate:(id<WkWebViewScrollingDelegate>)delegate;
- (void)setNetworkDelegate:(id<WkWebViewNetworkDelegate>)delegate;

- (void)dumpDebug;

@end
