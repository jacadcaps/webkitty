#import <mui/MUIArea.h>

@class WkWebView;
@class WkWebViewPrivate;
@class WkMutableNetworkRequest;
@class WkBackForwardList;
@class WkBackForwardListItem;
@class WkSettings;
@class WkCertificate;
@class WkCertificateChain;
@class WkError;
@protocol WkFileDialogSettings;
@protocol WkFileDialogResponseHandler;
@protocol WkDownloadDelegate;

#define kWebViewNetworkDelegateOption @"mode"
#define kWebViewNetworkDelegateOption_NewWindow @"window"
#define kWebViewNetworkDelegateOption_NewTab @"tab"

@protocol WkWebViewScrollingDelegate <OBObject>

- (void)webView:(WkWebView *)view changedContentsSizeToWidth:(int)width height:(int)height;
- (void)webView:(WkWebView *)view scrolledToLeft:(int)left top:(int)top;

@end

@protocol WkConfirmDownloadResponseDelegate <OBObject>

- (void)download;
- (void)ignore;

@end

@protocol WkAuthenticationChallengeResponseDelegate <OBObject>

- (void)authenticateWithLogin:(OBString *)login password:(OBString *)password;
- (void)cancel;

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

- (void)webView:(WkWebView *)view confirmDownloadOfURL:(OBURL *)url mimeType:(OBString *)mime size:(size_t) size withSuggestedName:(OBString *)suggestedName withResponseDelegate:(id<WkConfirmDownloadResponseDelegate>)delegate;

- (void)webView:(WkWebView *)view issuedAuthenticationChallengeAtURL:(OBURL *)url withResponseDelegate:(id<WkAuthenticationChallengeResponseDelegate>)delegate;

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

@protocol WkWebViewDialogDelegate <OBObject>

- (void)webView:(WkWebView *)view wantsToOpenFileSelectionPanelWithSettings:(id<WkFileDialogSettings>)settings responseHandler:(id<WkFileDialogResponseHandler>)handler;
- (void)webView:(WkWebView *)view wantsToShowJavaScriptAlertWithMessage:(OBString *)message;
- (BOOL)webView:(WkWebView *)view wantsToShowJavaScriptConfirmPanelWithMessage:(OBString *)message;
- (OBString *)webView:(WkWebView *)view wantsToShowJavaScriptPromptPanelWithMessage:(OBString *)message defaultValue:(OBString *)defaultValue;

@end

@protocol WkWebViewAutofillDelegate <OBObject>

- (void)webView:(WkWebView *)view willSubmitFormWithLogin:(OBString *)login password:(OBString *)password atURL:(OBURL *)url;
- (void)webView:(WkWebView *)view selectedAutofillFieldAtURL:(OBURL *)url withPrefilledLogin:(OBString *)login;

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

- (BOOL)goBack;
- (BOOL)goForward;

- (void)goToItem:(WkBackForwardListItem *)item;
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

- (BOOL)hasAutofillElements;
- (void)autofillElementsWithLogin:(OBString *)login password:(OBString *)password;

- (float)textZoomFactor;
- (float)pageZoomFactor;
- (void)setPageZoomFactor:(float)pageFactor textZoomFactor:(float)textFactor;

- (void)setScrollingDelegate:(id<WkWebViewScrollingDelegate>)delegate;
- (void)setNetworkDelegate:(id<WkWebViewNetworkDelegate>)delegate;
- (void)setBackForwardListDelegate:(id<WkWebViewBackForwardListDelegate>)delegate;
- (void)setDebugConsoleDelegate:(id<WkWebViewDebugConsoleDelegate>)delegate;
- (void)setDownloadDelegate:(id<WkDownloadDelegate>)delegate;
- (void)setDialogDelegate:(id<WkWebViewDialogDelegate>)delegate;
- (void)setAutofillDelegate:(id<WkWebViewAutofillDelegate>)delegate;

- (void)dumpDebug;

- (void)setCustomProtocolHandler:(id<WkWebViewNetworkProtocolHandlerDelegate>)delegate forProtocol:(OBString *)protocol;

@end
