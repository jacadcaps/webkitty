#import <mui/MUIArea.h>

@class WkWebView;
@class WkWebViewPrivate;
@class WkMutableNetworkRequest;

@protocol WkWebViewScrollingDelegate <OBObject>

- (void)webView:(WkWebView *)view changedContentsSizeToWidth:(int)width height:(int)height;
- (void)webView:(WkWebView *)view scrolledToLeft:(int)left top:(int)top;

@end

@protocol WkWebViewNetworkDelegate <OBObject>

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

// Query whether we're ready to quit. You MUST keep calling this from OBApplication's readyToQuit delegate
// method until it returns YES. WkWebKit will break the main runloop on its own one readiness state
// changes
+ (BOOL)readyToQuit;

// Load an URL into the main frame
- (void)load:(OBURL *)url;
// Load a HTML string into the main frame
- (void)loadHTMLString:(OBString *)string baseURL:(OBURL *)base;
// Loads a WkMutableNetworkRequest
- (void)loadRequest:(WkMutableNetworkRequest *)request;

- (BOOL)loading;
- (BOOL)hasOnlySecureContent;

- (BOOL)canGoBack;
- (BOOL)canGoForward;

- (void)goBack;
- (void)goForward;

- (void)reload;

- (void)stopLoading;

- (OBString *)title;
- (OBURL *)URL;

- (void)runJavaScript:(OBString *)javascript;
- (OBString *)evaluateJavaScript:(OBString *)javascript;

- (void)scrollToLeft:(int)left top:(int)top;

- (void)setScrollingDelegate:(id<WkWebViewScrollingDelegate>)delegate;
- (void)setNetworkDelegate:(id<WkWebViewNetworkDelegate>)delegate;

- (void)dumpDebug;

@end
