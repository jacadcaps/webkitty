#import <mui/MUIArea.h>

@class WkWebView;
@class WkWebViewPrivate;
@class WkMutableNetworkRequest;
@class WkBackForwardList;
@class WkSettings;
@class WkCertificate;
@class WkCertificateChain;
@class WkError;
@protocol WkDownloadDelegate;

@protocol WkWebViewScrollingDelegate <OBObject>

- (void)webView:(WkWebView *)view changedContentsSizeToWidth:(int)width height:(int)height;
- (void)webView:(WkWebView *)view scrolledToLeft:(int)left top:(int)top;

@end

@protocol WkWebViewNetworkDelegate <OBObject>

- (OBString *)userAgentForURL:(OBString *)url;

- (void)webView:(WkWebView *)view changedTitle:(OBString *)newtitle;
- (void)webView:(WkWebView *)view changedDocumentURL:(OBURL *)newURL;

- (void)webView:(WkWebView *)view documentReady:(BOOL)ready;

- (void)webView:(WkWebView *)view didFailLoadingWithError:(WkError *)error;

- (BOOL)webView:(WkWebView *)view wantsToCreateNewViewWithURL:(OBURL *)url options:(OBDictionary *)options;
- (void)webView:(WkWebView *)view createdNewWebView:(WkWebView *)newview;

- (void)webViewDidLoadInsecureContent:(WkWebView *)view;

@end

@protocol WkWebViewBackForwardListDelegate <OBObject>

- (void)webViewChangedBackForwardList:(WkWebView *)view;

@end

@protocol WkWebViewDebugConsoleDelegate <OBObject>

typedef enum {
    WkWebViewDebugConsoleLogLevel_Log = 1,
    WkWebViewDebugConsoleLogLevel_Warning = 2,
    WkWebViewDebugConsoleLogLevel_Error = 3,
    WkWebViewDebugConsoleLogLevel_Debug = 4,
    WkWebViewDebugConsoleLogLevel_Info = 5,
} WkWebViewDebugConsoleLogLevel;

- (void)webView:(WkWebView *)view outputConsoleMessage:(OBString *)message level:(WkWebViewDebugConsoleLogLevel)level atLine:(ULONG)lineno;

@end

@protocol WkWebViewNetworkProtocolHandlerDelegate <OBObject>

- (void)webView:(WkWebView *)view wantsToNavigateToCustomProtocol:(OBString *)protocol withArguments:(OBString *)arguments;

@end

@interface WkWebView : MUIArea
{
	WkWebViewPrivate *_private;
}

// Query whether we're ready to quit. You MUST keep calling this from OBApplication's readyToQuit delegate
// method until it returns YES. WkWebKit will break the main runloop on its own one readiness state
// changes
+ (BOOL)readyToQuit;

// Sets a custom PEM file to be used to validate a connection to the given domain
// 'key' is an optional password required to load the PEM file
+ (void)setCustomCertificate:(OBString *)pathToPEM forDomain:(OBString *)domain withKey:(OBString *)key;

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

- (BOOL)goBack;
- (BOOL)goForward;

- (WkBackForwardList *)backForwardList;

- (void)reload;

- (void)stopLoading;

- (OBString *)title;
- (OBURL *)URL;
- (WkCertificateChain *)certificateChain;

- (OBString *)html;
- (void)setHTML:(OBString *)html;

- (WkSettings *)settings;
- (void)setSettings:(WkSettings *)settings;

- (void)runJavaScript:(OBString *)javascript;
- (OBString *)evaluateJavaScript:(OBString *)javascript;

- (void)scrollToLeft:(int)left top:(int)top;

- (void)setScrollingDelegate:(id<WkWebViewScrollingDelegate>)delegate;
- (void)setNetworkDelegate:(id<WkWebViewNetworkDelegate>)delegate;
- (void)setBackForwardListDelegate:(id<WkWebViewBackForwardListDelegate>)delegate;
- (void)setDebugConsoleDelegate:(id<WkWebViewDebugConsoleDelegate>)delegate;
- (void)setDownloadDelegate:(id<WkDownloadDelegate>)delegate;

- (void)dumpDebug;

- (void)setCustomProtocolHandler:(id<WkWebViewNetworkProtocolHandlerDelegate>)delegate forProtocol:(OBString *)protocol;

@end
