#define SYSTEM_PRIVATE

// These will spew out some warnings, but it is not possible to disable them :(
#undef __OBJC__
#import "WebKit.h"
#import "WebPage.h"
#import "WebProcess.h"
#import "WebViewDelegate.h"
#import <WebCore/ContextMenuItem.h>
#import <WebCore/CertificateInfo.h>
#import <WebCore/ResourceRequest.h>
#import <WebCore/ResourceHandle.h>
#import <WebCore/ResourceResponse.h>
#import <WebCore/FileChooser.h>
#import <WebCore/TextEncoding.h>
#import <WebCore/DOMWindow.h>
#include <WebCore/AuthenticationChallenge.h>
#include <WebCore/AuthenticationClient.h>
#define __OBJC__

#import <ob/OBFramework.h>
#import <mui/MUIFramework.h>

#import "WkWebView.h"
#import "WkHistory_private.h"
#import "WkSettings.h"
#import "WkCertificate_private.h"
#import "WkError_private.h"
#import "WkDownload_private.h"
#import "WkFileDialog_private.h"

#import <proto/dos.h>
#import <proto/exec.h>
#import <proto/intuition.h>

#import <cairo.h>
struct Library *FreetypeBase;

extern "C" { void dprintf(const char *, ...); }

#define D(x) 

namespace  {
	static int _viewInstanceCount;
	static bool _shutdown;
	static bool _readyToQuitPending;
	static bool _wasInstantiatedOnce;
	static OBSignalHandler *_signalHandler;
	APTR   _globalOBContext;
	struct Task *_mainThread;
}

@interface WkWebView ()

- (void)invalidated;
- (void)scrollToX:(int)sx y:(int)sy;
- (void)setDocumentWidth:(int)width height:(int)height;

@end

@interface WkWeakContainer : OBObject
{
        id _object;
}

+ (id)container:(id)object;
- (id)containedObject;
- (id)performSelector:(SEL)selector;
- (id)performSelector:(SEL)selector withObject:(id)object;
- (id)performSelector:(SEL)selector withObject:(id)object withObject:(id)object2;

@end

@implementation WkWeakContainer

+ (id)container:(id)object
{
        WkWeakContainer *container = [WkWeakContainer new];
        container->_object = object;
        return [container autorelease];
}

- (id)containedObject
{
        return _object;
}

- (id)performSelector:(SEL)selector
{
        return [_object performSelector:selector];
}

- (id)performSelector:(SEL)selector withObject:(id)object
{
        return [_object performSelector:selector withObject:object];
}

- (id)performSelector:(SEL)selector withObject:(id)object withObject:(id)object2
{
        return [_object performSelector:selector withObject:object withObject:object2];
}

- (BOOL)isEqual:(id)otherObject
{
        static Class myClass = [WkWeakContainer class];
        if ([otherObject isKindOfClass:myClass])
                return [_object isEqual:[(WkWeakContainer *)otherObject containedObject]];
        return [_object isEqual:otherObject];
}

- (ULONG)hash
{
        return [_object hash];
}

@end

@interface WkWebViewPrivate : OBObject
{
	WTF::RefPtr<WebKit::WebPage>         _page;
	id<WkWebViewScrollingDelegate>       _scrollingDelegate;
	id<WkWebViewNetworkDelegate>         _networkDelegate;
	id<WkWebViewBackForwardListDelegate> _backForwardDelegate;
	id<WkWebViewDebugConsoleDelegate>    _consoleDelegate;
	id<WkDownloadDelegate>               _downloadDelegate;
	id<WkWebViewDialogDelegate>          _dialogDelegate;
	id<WkWebViewAutofillDelegate>        _autofillDelegate;
	OBMutableDictionary                 *_protocolDelegates;
	bool                                 _drawPending;
	bool                                 _isActive;
	bool                                 _isLoading;
	bool                                 _isLiveResizing;
	bool                                 _hasOnlySecureContent;
	OBURL                               *_url;
	OBString                            *_title;
	int                                  _scrollX, _scrollY;
	int                                  _documentWidth, _documentHeight;
}
@end

@implementation WkWebViewPrivate

- (void)dealloc
{
	@synchronized ([WkWebView class])
	{
		_viewInstanceCount --;
		D(dprintf("%s: instances left %d pendingquit %d\n", __PRETTY_FUNCTION__, _viewInstanceCount, _readyToQuitPending));

		if (_readyToQuitPending)
		{
			[[MUIApplication currentApplication] quit];
		}
	}
	
	[_url release];
	[_title release];
	[_protocolDelegates release];
	
	[super dealloc];
}

- (void)timedOut
{
	// delay disposal just a little bit
	[self retain];
	[self autorelease];
}

- (void)setPage:(WebKit::WebPage *)page
{
	_page = page;
}

- (WebKit::WebPage *)page
{
	return _page.get();
}

- (void)setScrollingDelegate:(id<WkWebViewScrollingDelegate>)delegate
{
	_scrollingDelegate = delegate;
}

- (id<WkWebViewScrollingDelegate>)scrollingDelegate
{
	return _scrollingDelegate;
}

- (void)setNetworkDelegate:(id<WkWebViewNetworkDelegate>)delegate
{
	_networkDelegate = delegate;
}

- (id<WkWebViewNetworkDelegate>)networkDelegate
{
	return _networkDelegate;
}

- (void)setConsoleDelegate:(id<WkWebViewDebugConsoleDelegate>)consoleDelegate
{
	_consoleDelegate = consoleDelegate;
}

- (id<WkWebViewDebugConsoleDelegate>)consoleDelegate
{
	return _consoleDelegate;
}

- (void)setTitle:(OBString *)title
{
	[_title autorelease];
	_title = [title retain];
}

- (OBString *)title
{
	return _title;
}

- (void)setURL:(OBURL *)url
{
	[_url autorelease];
	_url = [url retain];
}

- (OBURL *)url
{
	return _url;
}

- (void)setDrawPending:(bool)drawPending
{
	_drawPending = drawPending;
}

- (void)setBackForwardDelegate:(id<WkWebViewBackForwardListDelegate>)backForwardDelegate
{
	_backForwardDelegate = backForwardDelegate;
}

- (id<WkWebViewBackForwardListDelegate>)backForwardDelegate
{
	return _backForwardDelegate;
}

- (void)setDownloadDelegate:(id<WkDownloadDelegate>)delegate
{
	_downloadDelegate = delegate;
}

- (id<WkDownloadDelegate>)downloadDelegate
{
	return _downloadDelegate;
}

- (void)setDialogDelegate:(id<WkWebViewDialogDelegate>)delegate
{
	_dialogDelegate = delegate;
}

- (id<WkWebViewDialogDelegate>)dialogDelegate
{
	return _dialogDelegate;
}

- (void)setAutofillDelegate:(id<WkWebViewAutofillDelegate>)delegate
{
	_autofillDelegate = delegate;
}

- (id<WkWebViewAutofillDelegate>)autofillDelegate
{
	return _autofillDelegate;
}

- (void)setCustomProtocolHandler:(id<WkWebViewNetworkProtocolHandlerDelegate>)delegate forProtocol:(OBString *)protocol
{
	if (nil == _protocolDelegates)
		_protocolDelegates = [OBMutableDictionary new];
	
	[_protocolDelegates setObject:[WkWeakContainer container:delegate] forKey:protocol];
}

- (id<WkWebViewNetworkProtocolHandlerDelegate>)protocolDelegateForProtocol:(OBString *)protocol
{
	return [[_protocolDelegates objectForKey:protocol] containedObject];
}

- (bool)drawPending
{
	return _drawPending;
}

- (void)setIsActive:(bool)isactive
{
	_isActive = isactive;
}

- (bool)isActive
{
	return _isActive;
}

- (void)setIsLoading:(bool)isloading
{
	_isLoading = isloading;
}

- (bool)isLoading
{
	return _isLoading;
}

- (bool)isLiveResizing
{
	return _isLiveResizing;
}

- (void)setIsLiveResizing:(BOOL)resizing
{
	_isLiveResizing = resizing;
}

- (void)setDocumentWidth:(int)width height:(int)height
{
	_documentWidth = width;
	_documentHeight = height;
}

- (int)documentWidth
{
	return _documentWidth;
}

- (int)documentHeight
{
	return _documentHeight;
}

- (void)setScrollX:(int)sx y:(int)sy
{
	_scrollX = sx;
	_scrollY = sy;
}

- (int)scrollX
{
	return _scrollX;
}

- (int)scrollY
{
	return _scrollY;
}

- (BOOL)hasOnlySecureContent
{
	return _hasOnlySecureContent;
}

- (void)setHasOnlySecureContent:(BOOL)hasOnlySecureContent
{
	_hasOnlySecureContent = hasOnlySecureContent;
}

@end

@interface WkDownloadResponseDelegatePrivate : OBObject<WkConfirmDownloadResponseDelegate>
{
	WebCore::PolicyCheckIdentifier _identifier;
	WebCore::FramePolicyFunction   _function;
}
@end

@implementation WkDownloadResponseDelegatePrivate

- (id)initWithPolicyCheckIdentifier:(WebCore::PolicyCheckIdentifier)identifier function:(WebCore::FramePolicyFunction &&)function
{
	if ((self = [super init]))
	{
		_function = std::move(function);
		_identifier = identifier;
	}
	
	return self;
}

- (void)dealloc
{
	if (_function)
		_function(WebCore::PolicyAction::Ignore, _identifier);
	[super dealloc];
}

- (void)download
{
	if (_function)
	{
		_function(WebCore::PolicyAction::Download, _identifier);
		_function = nullptr;
	}
}

- (void)ignore
{
	if (_function)
	{
		_function(WebCore::PolicyAction::Ignore, _identifier);
		_function = nullptr;
	}
}

@end

class WkAuthenticationChallenge : public RefCounted<WkAuthenticationChallenge>
{
public:
    static Ref<WkAuthenticationChallenge> create(const WebCore::AuthenticationChallenge &challenge)
    {
        return WTF::adoptRef(*new WkAuthenticationChallenge(challenge));
    }
	WkAuthenticationChallenge(const WebCore::AuthenticationChallenge &challenge) :
		_challenge(challenge)
	{
	}
	WebCore::AuthenticationChallenge &challenge() { return _challenge; }
private:
	WebCore::AuthenticationChallenge _challenge;
};

@interface WkAuthenticationChallengeResponseDelegatePrivate : OBObject<WkAuthenticationChallengeResponseDelegate>
{
	// this wrapper class is only here cause GCC sucks...
	//WebCore::AuthenticationChallenge _challenge;
	RefPtr<WkAuthenticationChallenge> _challenge;
}
@end

@implementation WkAuthenticationChallengeResponseDelegatePrivate

- (id)initWithAuthenticationChallenge:(RefPtr<WkAuthenticationChallenge>)challenge
{
	if ((self = [super init]))
	{
		if (challenge)
		{
			_challenge = challenge;
		}
		else
		{
			[self release];
			return nil;
		}
	}
	
	return self;
}

- (void)dealloc
{
	if (_challenge)
		_challenge->challenge().authenticationClient()->receivedCancellation(_challenge->challenge());

	[super dealloc];
}

- (void)authenticateWithLogin:(OBString *)login password:(OBString *)password
{
	if (_challenge)
	{
		WTF::String sLogin = WTF::String::fromUTF8([login cString]);
		WTF::String sPassword = WTF::String::fromUTF8([password cString]);
		WebCore::Credential credential(sLogin, sPassword, WebCore::CredentialPersistence::CredentialPersistenceForSession);
		_challenge->challenge().authenticationClient()->receivedCredential(_challenge->challenge(), credential);
		_challenge = nullptr;
	}
}

- (void)cancel
{
	if (_challenge)
	{
		_challenge->challenge().authenticationClient()->receivedRequestToContinueWithoutCredential(_challenge->challenge());
		_challenge = nullptr;
	}
}

@end

@implementation WkWebView

static inline bool wkIsMainThread() {
	return FindTask(0) == _mainThread;
}

static inline void validateObjCContext() {
	if (!wkIsMainThread()) {
dprintf("---------- objc fixup ------------\n");
		FindTask(0)->tc_ETask->OBContext = _globalOBContext; // fixup ObjC threading
	}
}

+ (void)performWithSignalHandler:(OBSignalHandler *)handler
{
	if (_signalHandler == handler)
	{
		static const uint32_t mask = uint32_t(1UL << [handler sigBit]);
		WebKit::WebProcess::singleton().handleSignals(mask);
	}
}

+ (void)fire
{
	[self performWithSignalHandler:_signalHandler];
}

+ (void)_lastPageClosed
{
	D(dprintf("%s: %d\n", __PRETTY_FUNCTION__,_readyToQuitPending));
	if (_readyToQuitPending)
	{
		[[MUIApplication currentApplication] quit];
	}
}

+ (BOOL)readyToQuit
{
	@synchronized ([WkWebView class])
	{
		if (!_readyToQuitPending)
			WebCore::DOMWindow::dispatchAllPendingBeforeUnloadEvents();
		_readyToQuitPending = YES;
		if (_viewInstanceCount == 0 &&
			WebKit::WebProcess::singleton().webFrameCount() == 0)
		{
			_readyToQuitPending = NO;
			_shutdown = YES;
			[[OBRunLoop mainRunLoop] removeSignalHandler:_signalHandler];
			[_signalHandler release];
			WebKit::WebProcess::singleton().terminate();
			[WkCertificate shutdown];
			CloseLibrary(FreetypeBase);
			return YES;
		}
	}
	
	return NO;
}

- (WkWebViewPrivate *)privateObject
{
	return _private;
}

- (id)init
{
	if ((self = [super init]))
	{
		@synchronized ([WkWebView class])
		{
			if (_shutdown || _readyToQuitPending)
			{
				[self release];
				return nil;
			}

			_viewInstanceCount ++;

			if (!_wasInstantiatedOnce)
			{
				FreetypeBase = OpenLibrary("freetype.library", 0);
				if (FreetypeBase)
				{
					BPTR icuDir = Lock("MOSSYS:Data/ICU/icudt54b", ACCESS_READ);
					if (0 == icuDir)
					{
						[MUIRequest request:nil title:@"WebKit Installation Error"
							message:@"ICU data files must be present in MOSSYS:Data/ICU/icudt54b"
							buttons:[OBArray arrayWithObject:@"Exit"]];
						CloseLibrary(FreetypeBase);
						FreetypeBase = NULL;
						[self release];
						return nil;
					}
					else
					{
						UnLock(icuDir);
					}
					
					// MUST be done before 1st WebPage is instantiated!
					cairo_surface_t *dummysurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 4, 4);
					if (dummysurface)
						cairo_surface_destroy(dummysurface);
					
					_signalHandler = [OBSignalHandler new];
					[_signalHandler setDelegate:(id)[self class]];
					[[OBRunLoop mainRunLoop] addSignalHandler:_signalHandler];
					
					[OBScheduledTimer scheduledTimerWithInterval:0.1 perform:[OBPerform performSelector:@selector(fire) target:[self class]] repeats:YES];
					
					WebKit::WebProcess::singleton().initialize(int([_signalHandler sigBit]));
					WebKit::WebProcess::singleton().setLastPageClosedCallback([]() {
						[WkWebView _lastPageClosed];
					});
					
					_wasInstantiatedOnce = true;
					_mainThread = FindTask(0);
					_globalOBContext = _mainThread->tc_ETask->OBContext;
				}
			}
		}

		self.fillArea = NO;
		self.handledEvents = IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_MOUSEHOVER | IDCMP_RAWKEY;
		[self setEventHandlerGUIMode:YES];
		self.cycleChain = YES;

		_private = [WkWebViewPrivate new];
		if (!_private)
		{
			[self release];
			return nil;
		}

		try {
			auto& webProcess = WebKit::WebProcess::singleton();
			auto identifier = WebCore::PageIdentifier::generate();

			WebKit::WebPageCreationParameters parameters;
			webProcess.createWebPage(identifier, WTFMove(parameters));
    		[_private setPage:webProcess.webPage(identifier)];

			if (![_private page])
			{
				[self release];
				return nil;
			}
			
			auto webPage = [_private page];

			webPage->_fInvalidate = [self]() { [self invalidated]; };

			webPage->_fSetDocumentSize = [self](int width, int height) {
				[self setDocumentWidth:width height:height];
			};
			
			webPage->_fScroll = [self](int sx, int sy) {
				[self scrollToX:sx y:sy];
			};
			
			webPage->_fActivateNext = [self]() {
				[[self windowObject] setActiveObjectSpecial:MUIV_Window_ActiveObject_Next];
			};
			
			webPage->_fActivatePrevious = [self]() {
				[[self windowObject] setActiveObjectSpecial:MUIV_Window_ActiveObject_Prev];
			};

			webPage->_fGoActive = [self]() {
				[[self windowObject] setActiveObject:self];
			};
			
			webPage->_fUserAgentForURL = [self](const WTF::String& url) -> WTF::String {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkWebViewNetworkDelegate> networkDelegate = [privateObject networkDelegate];
				if (networkDelegate)
				{
					auto uurl = url.utf8();
					OBString *overload = [networkDelegate userAgentForURL:[OBString stringWithUTF8String:uurl.data()]];

					if (overload)
						return WTF::String::fromUTF8([overload cString]);
				}
				return WTF::String::fromUTF8("Mozilla/5.0 (Macintosh; Intel Mac OS X 10_13_6) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/13.1 Safari/605.1.15");
			};
			
			webPage->_fChangedTitle = [self](const WTF::String& title) {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkWebViewNetworkDelegate> networkDelegate = [privateObject networkDelegate];
				auto utitle = title.utf8();
				OBString *otitle = [OBString stringWithUTF8String:utitle.data()];
				[privateObject setTitle:otitle];
				[networkDelegate webView:self changedTitle:otitle];
			};

			webPage->_fChangedURL = [self](const WTF::String& url) {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkWebViewNetworkDelegate> networkDelegate = [privateObject networkDelegate];
				auto uurl = url.utf8();
				OBURL *ourl = [OBURL URLWithString:[OBString stringWithUTF8String:uurl.data()]];
				[privateObject setHasOnlySecureContent:YES];
				[privateObject setURL:ourl];
				[networkDelegate webView:self changedDocumentURL:ourl];
			};
			
			webPage->_fDidStartLoading = [self]() {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkWebViewNetworkDelegate> networkDelegate = [privateObject networkDelegate];
				[privateObject setIsLoading:YES];
				[networkDelegate webView:self documentReady:NO];
			};

			webPage->_fDidStopLoading = [self]() {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkWebViewNetworkDelegate> networkDelegate = [privateObject networkDelegate];
				[privateObject setIsLoading:NO];
				[networkDelegate webView:self documentReady:YES];
			};

			webPage->_fDidLoadInsecureContent = [self]() {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkWebViewNetworkDelegate> networkDelegate = [privateObject networkDelegate];
				[privateObject setHasOnlySecureContent:NO];
				[networkDelegate webViewDidLoadInsecureContent:self];
			};
			
			webPage->_fCanOpenWindow = [self](const WTF::String& url, const WebCore::WindowFeatures&) -> bool {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkWebViewNetworkDelegate> networkDelegate = [privateObject networkDelegate];
				if (networkDelegate)
				{
					auto uurl = url.utf8();
					OBURL *url = [OBURL URLWithString:[OBString stringWithUTF8String:uurl.data()]];
					return [networkDelegate webView:self wantsToCreateNewViewWithURL:url options:nil];
				}
				return NO;
			};
			
			webPage->_fDoOpenWindow = [self]() -> WebCore::Page * {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkWebViewNetworkDelegate> networkDelegate = [privateObject networkDelegate];
				if (!networkDelegate)
					return nullptr;

				WkWebView *newView = [[[self class] new] autorelease];
				WkWebViewPrivate *newPrivateObject = [newView privateObject];
				WebKit::WebPage *page = [newPrivateObject page];
				if (page && page->corePage())
				{
					[networkDelegate webView:self createdNewWebView:newView];
					return page->corePage();
				}
				return nullptr;
			};
			
			webPage->_fPopup = [self](const WebCore::IntRect& pos, const WTF::Vector<WTF::String>& items) -> int {
				validateObjCContext();
				MUIMenu *menu = [[MUIMenu new] autorelease];
				int index = 0;
				for (const WTF::String &entry : items)
				{
					auto uentry = entry.utf8();
					[menu addObject:[MUIMenuitem itemWithTitle:[OBString stringWithUTF8String:uentry.data()] shortcut:nil userData:++index]];
				}
				MUIMenustrip *strip = [[MUIMenustrip menustripWithObjects:menu, nil] retain];
				if (strip)
				{
					// 0 on failure, all our menus return 1, 2...
					int rc = [strip popup:self flags:0 x:[self left] + pos.x() y:[self top] + pos.y()];
					[strip release];
					// 0 = first entry, -1 = error
					return rc - 1;
				}
				return -1;
			};
			
			webPage->_fHistoryChanged = [self]() {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkWebViewBackForwardListDelegate> delegate = [privateObject backForwardDelegate];
				[delegate webViewChangedBackForwardList:self];
			};
			
			webPage->_fConsole = [self](const WTF::String&message, int level, unsigned int line) {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkWebViewDebugConsoleDelegate> delegate = [privateObject consoleDelegate];
				if (delegate)
				{
					auto umessage = message.utf8();
					OBString *log = [OBString stringWithUTF8String:umessage.data()];

					[delegate webView:self outputConsoleMessage:log level:(WkWebViewDebugConsoleLogLevel)level atLine:line];
				}
			};
			
			webPage->_fDidFailWithError = [self](const WebCore::ResourceError &error) {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkWebViewNetworkDelegate> networkDelegate = [privateObject networkDelegate];
				if (networkDelegate)
				{
					[networkDelegate webView:self documentReady:YES];
					[networkDelegate webView:self didFailLoadingWithError:[WkError errorWithResourceError:error]];
				}
			};
			
			webPage->_fCanHandleRequest = [self](const WebCore::ResourceRequest &request) {
				if (request.httpMethod() == "GET")
				{
					const WTF::URL &url = request.url();
					WTF::String protocol = url.protocol().toString();

					// bypass standard protocols...
					if (protocol == "http" || protocol == "https" || protocol == "ftp")
						return true;
					
					validateObjCContext();
					WkWebViewPrivate *privateObject = [self privateObject];

					auto uprotocol = protocol.utf8();
					OBString *oprotocol = [OBString stringWithUTF8String:uprotocol.data()];
					id<WkWebViewNetworkProtocolHandlerDelegate> delegate = [privateObject protocolDelegateForProtocol:oprotocol];
					if (delegate)
					{
						auto uurl = url.string().utf8();
						OBString *args = @"";
						if (uurl.length() > protocol.length() + 1)
							args = [OBString stringWithUTF8String:uurl.data() + protocol.length() + 1];
						[delegate webView:self wantsToNavigateToCustomProtocol:oprotocol withArguments:args];
						return false;
					}
				}

				return true;
			};
			
			webPage->_fDownload = [self](const WTF::URL &url, const WTF::String &suggestedName) {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkDownloadDelegate> downloadDelegate = [privateObject downloadDelegate];
				if (downloadDelegate)
				{
					auto uurl = url.string().utf8();
					WkDownload *download = [WkDownload download:[OBURL URLWithString:[OBString stringWithUTF8String:uurl.data()]] withDelegate:downloadDelegate];
					[download start];
				}
			};
			
			webPage->_fDownloadFromResource = [self](WebCore::ResourceHandle* handle, const WebCore::ResourceRequest& request, const WebCore::ResourceResponse& response) {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkDownloadDelegate> downloadDelegate = [privateObject downloadDelegate];
				if (downloadDelegate)
				{
					WkDownload *download = [WkDownload downloadWithHandle:handle request:request response:response withDelegate:downloadDelegate];
					[download start];
				}
			};
			
			webPage->_fDownloadAsk = [self](const WebCore::ResourceResponse& response, const WebCore::ResourceRequest& request,
				WebCore::PolicyCheckIdentifier identifier, const WTF::String& downloadAttribute, WebCore::FramePolicyFunction&& function) {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkWebViewNetworkDelegate> networkDelegate = [privateObject networkDelegate];
				if (networkDelegate)
				{
					WkDownloadResponseDelegatePrivate *responsePrivate = [[[WkDownloadResponseDelegatePrivate alloc] initWithPolicyCheckIdentifier:identifier function:std::move(function)] autorelease];
					auto uurl = response.url().string().utf8();
					auto umime = response.mimeType().utf8();
					auto uname = response.suggestedFilename().utf8();
					
					if (0 == uname.length())
						uname = WebCore::decodeURLEscapeSequences(response.url().lastPathComponent()).utf8();
					
					[networkDelegate webView:self
						confirmDownloadOfURL:[OBURL URLWithString:[OBString stringWithUTF8String:uurl.data()]]
						mimeType:[OBString stringWithUTF8String:umime.data()]
						size:response.expectedContentLength()
						withSuggestedName:[OBString stringWithUTF8String:uname.data()]
						withResponseDelegate:responsePrivate];
					return;
				}
				function(WebCore::PolicyAction::Ignore, identifier);
			};

			webPage->_fAlert = [self](const WTF::String &alert) {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkWebViewDialogDelegate> dialogDelegate = [privateObject dialogDelegate];
				if (dialogDelegate)
				{
					auto uurl = alert.utf8();
					[dialogDelegate webView:self wantsToShowJavaScriptAlertWithMessage:[OBString stringWithUTF8String:uurl.data()]];
				}
			};

			webPage->_fConfirm = [self](const WTF::String &confirm) -> bool {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkWebViewDialogDelegate> dialogDelegate = [privateObject dialogDelegate];
				if (dialogDelegate)
				{
					auto uurl = confirm.utf8();
					return [dialogDelegate webView:self wantsToShowJavaScriptConfirmPanelWithMessage:[OBString stringWithUTF8String:uurl.data()]];
				}
				return false;
			};
			
			webPage->_fPrompt = [self](const WTF::String &prompt, const WTF::String &defaults, WTF::String &out) -> bool {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkWebViewDialogDelegate> dialogDelegate = [privateObject dialogDelegate];
				if (dialogDelegate)
				{
					auto uprompt = prompt.utf8();
					auto udefaults = defaults.utf8();
					OBString *rc = [dialogDelegate webView:self wantsToShowJavaScriptPromptPanelWithMessage:[OBString stringWithUTF8String:uprompt.data()] defaultValue:[OBString stringWithUTF8String:udefaults.data()]];
					if (rc)
					{
						out = WTF::String::fromUTF8([rc cString]);
						return true;
					}
				}
				return false;
			};
			
			webPage->_fFile = [self](WebCore::FileChooser&chooser) {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkWebViewDialogDelegate> dialogDelegate = [privateObject dialogDelegate];
				if (dialogDelegate)
				{
					WkFileDialogResponseHandlerPrivate *fd = [[[WkFileDialogResponseHandlerPrivate alloc] initWithChooser:chooser] autorelease];
					[dialogDelegate webView:self wantsToOpenFileSelectionPanelWithSettings:fd responseHandler:fd];
				}
			};
			
			webPage->_fHasAutofill = [self]() {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkWebViewAutofillDelegate> autoDelegate = [privateObject autofillDelegate];
				WebKit::WebPage *page = [privateObject page];
				if (autoDelegate && page)
				{
					WTF::String wLogin, wPassword;
					page->getAutofillElements(wLogin, wPassword);
					OBString *login = nil;
					if (!wLogin.isEmpty())
					{
						auto ulogin = wLogin.utf8();
						login = [OBString stringWithUTF8String:ulogin.data()];
					}
					[autoDelegate webView:self selectedAutofillFieldAtURL:[self URL] withPrefilledLogin:login];
				}
			};
			
			webPage->_fStoreAutofill = [self](const WTF::String &l, const WTF::String &p) {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkWebViewAutofillDelegate> autoDelegate = [privateObject autofillDelegate];
				if (autoDelegate)
				{
					auto ul = l.utf8();
					auto up = p.utf8();
					[autoDelegate webView:self willSubmitFormWithLogin:[OBString stringWithUTF8String:ul.data()] password:[OBString stringWithUTF8String:up.data()] atURL:[self URL]];
				}
			};
			
			webPage->_fNewTabWindow = [self](const WTF::URL& inurl, WebViewDelegateOpenWindowMode mode) {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkWebViewNetworkDelegate> networkDelegate = [privateObject networkDelegate];
				if (networkDelegate)
				{
					auto uurl = inurl.string().utf8();
					OBURL *url = [OBURL URLWithString:[OBString stringWithUTF8String:uurl.data()]];
					OBString *modeKey;
					
					switch (mode)
					{
					case WebViewDelegateOpenWindowMode::NewWindow:
						modeKey = kWebViewNetworkDelegateOption_NewWindow;
						break;
					case WebViewDelegateOpenWindowMode::BackgroundTab:
						modeKey = kWebViewNetworkDelegateOption_NewTab;
						break;
					default:
						modeKey = kWebViewNetworkDelegateOption;
						break;
					}

					if ([networkDelegate webView:self wantsToCreateNewViewWithURL:url
						options:[OBDictionary dictionaryWithObject:modeKey forKey:kWebViewNetworkDelegateOption]])
					{
						WkWebView *newView = [[[self class] new] autorelease];
						[newView load:url];
						[networkDelegate webView:self createdNewWebView:newView];
						return YES;
					}
				}
				return NO;
			};
			
			webPage->_fAuthChallenge = [self](const WebCore::AuthenticationChallenge &challenge) -> bool {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				id<WkWebViewNetworkDelegate> networkDelegate = [privateObject networkDelegate];
				if (networkDelegate)
				{
					WkAuthenticationChallengeResponseDelegatePrivate *responseDelegate =
						[[[WkAuthenticationChallengeResponseDelegatePrivate alloc] initWithAuthenticationChallenge:WkAuthenticationChallenge::create(challenge)] autorelease];
					if (responseDelegate)
					{
						[[OBRunLoop mainRunLoop] performSelector:@selector(webView:issuedAuthenticationChallengeAtURL:withResponseDelegate:)
							target:networkDelegate withObject:self withObject:[self URL] withObject:responseDelegate];
						return YES;
					}
				}
				return NO;
			};
			
			webPage->_fSetCursor = [self](int cursor) {
				validateObjCContext();
				WkWebViewPrivate *privateObject = [self privateObject];
				if ([self window])
				{
					struct TagItem tags[] = { { WA_PointerType, (IPTR)cursor }, { TAG_DONE } };
					SetWindowPointerA([self window], tags);
				}
			};
			
		} catch (...) {
			[self release];
			return nil;
		}
	}
	
	return self;
}

- (void)dealloc
{
	try {
		auto webPage = [_private page];
		if (webPage)
		{
			webPage->willBeDisposed();
			WebKit::WebProcess::singleton().removeWebPage(webPage->pageID());
		}
	} catch (...) {};

	[OBScheduledTimer scheduledTimerWithInterval:2.0 perform:[OBPerform performSelector:@selector(timedOut) target:_private] repeats:NO];
	[_private release];

	[super dealloc];
}

- (void)scrollToLeft:(int)left top:(int)top
{
	try {
		auto webPage = [_private page];
		webPage->setScroll(left, top);
	} catch (std::exception& ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
}

- (void)setScrollingDelegate:(id<WkWebViewScrollingDelegate>)delegate
{
	[_private setScrollingDelegate:delegate];
}

- (void)setNetworkDelegate:(id<WkWebViewNetworkDelegate>)delegate
{
	[_private setNetworkDelegate:delegate];
}

- (void)load:(OBURL *)url
{
	OBString *scheme = [url scheme];
	
	if (0 == [scheme length] && url)
	{
		url = [OBURL URLWithString:[OBString stringWithFormat:@"http://%@", [url absoluteString]]];
	}

	try {
		const char *curi = [[url absoluteString] cString];
		if (nullptr == curi)
			curi = "about:blank";
		auto webPage = [_private page];
		[_private setURL:url];
		[[_private networkDelegate] webView:self changedTitle:[url absoluteString]];
		webPage->load(curi);
	} catch (std::exception &ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
}

- (void)loadHTMLString:(OBString *)string baseURL:(OBURL *)base
{
	OBString *scheme = [base scheme];
	if (0 == [scheme length])
	{
		base = [OBURL URLWithString:[OBString stringWithFormat:@"file:///%@", [base absoluteString]]];
	}

	OBData *data = [string dataWithEncoding:MIBENUM_UTF_8];

	try {
		const char *curi = [[base absoluteString] cString];
		auto webPage = [_private page];
		webPage->loadData(reinterpret_cast<const char*>([data bytes]), [data length], curi);
	} catch (std::exception &ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
}

- (void)loadRequest:(WkMutableNetworkRequest *)request
{

}

- (BOOL)loading
{
	return [_private isLoading];
}

- (BOOL)hasOnlySecureContent
{
	return [_private hasOnlySecureContent];
}

- (BOOL)canGoBack
{
	try {
		auto webPage = [_private page];
		return webPage->canGoBack();
	} catch (std::exception &ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
	return NO;
}

- (BOOL)canGoForward
{
	try {
		auto webPage = [_private page];
		return webPage->canGoForward();
	} catch (std::exception &ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
	return NO;
}

- (BOOL)goBack
{
	try {
		auto webPage = [_private page];
		return webPage->goBack();
	} catch (std::exception &ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
	return NO;
}

- (BOOL)goForward
{
	try {
		auto webPage = [_private page];
		return webPage->goForward();
	} catch (std::exception &ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
	return NO;
}

- (WkBackForwardList *)backForwardList
{
	try {
		auto webPage = [_private page];
		WTF::RefPtr<WebKit::BackForwardClientMorphOS> client = webPage->backForwardClient();
		if (client.get())
			return [WkBackForwardListPrivate backForwardListPrivate:client];
	} catch (std::exception &ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
	return nil;
}

- (void)reload
{
	try {
		auto webPage = [_private page];
		webPage->reload();
	} catch (std::exception &ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
}

- (void)stopLoading
{
	try {
		auto webPage = [_private page];
		webPage->stop();
	} catch (std::exception &ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
}

- (OBString *)title
{
	return [_private title];
}

- (OBURL *)URL
{
	return [_private url];
}

- (WkCertificateChain *)certificateChain
{
	auto webPage = [_private page];
	WebCore::CertificateInfo info = webPage->getCertificate();
	if (info.isEmpty())
		return nil;
	const Vector<WebCore::CertificateInfo::Certificate>& chain = info.certificateChain();

	OBMutableArray *certArray = [OBMutableArray arrayWithCapacity:chain.size()];
	// NOTE: we want root > intermediate > client cert order
	for (int i = chain.size() - 1; i >= 0; i--)
	{
		const auto &cert = chain[i];
		[certArray addObject:[WkCertificate certificateWithData:(const char*)cert.data() length:cert.size()]];
	}

	return [WkCertificateChain certificateChainWithCertificates:certArray];
}

- (OBString *)html
{
	auto webPage = [_private page];
	return (OBString *)webPage->getInnerHTML([](const char *res) {
		return (void *)[OBString stringWithUTF8String:res];
	});
}

- (void)setHTML:(OBString *)html
{
	auto webPage = [_private page];
	webPage->setInnerHTML([html cString]);
}

- (void)runJavaScript:(OBString *)javascript
{
	try {
		auto webPage = [_private page];
		[self retain];
		webPage->run([javascript cString]);
		[self autorelease];
	} catch (std::exception &ex) {
		[self autorelease];
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
}

- (OBString *)evaluateJavaScript:(OBString *)javascript
{
	try {
		[self retain];
		auto webPage = [_private page];
		OBString *out = (id)webPage->evaluate([javascript cString], [](const char *res) {
			return (void *)[OBString stringWithUTF8String:res];
		});
		[self autorelease];
		return out;
	} catch (std::exception &ex) {
		[self autorelease];
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
	return nil;
}

- (WkSettings *)settings
{
	WkSettings *settings = [[WkSettings new] autorelease];
	auto webPage = [_private page];
	[settings setAdBlockerEnabled:webPage->adBlockingEnabled()];
	[settings setJavaScriptEnabled:webPage->javaScriptEnabled()];
	[settings setThirdPartyCookiesAllowed:webPage->thirdPartyCookiesAllowed()];
	return nil;
}

- (void)setSettings:(WkSettings *)settings
{
	auto webPage = [_private page];
	webPage->setJavaScriptEnabled([settings javaScriptEnabled]);
	webPage->setAdBlockingEnabled([settings adBlockerEnabled]);
	webPage->setThirdPartyCookiesAllowed([settings thirdPartyCookiesAllowed]);
}

- (void)dumpDebug
{
	WebKit::WebProcess::singleton().dumpWebCoreStatistics();
}

- (Boopsiobject *)instantiateTagList:(struct TagItem *)tags
{

#define MADF_KNOWSACTIVE       (1<< 7) /* sigh */

	Boopsiobject *meBoopsi = [super instantiateTagList:tags];
	if (meBoopsi)
	{
		// prevent MUI active frame from being drawn
		_flags(meBoopsi) |= MADF_KNOWSACTIVE;
	}
	return meBoopsi;
}

- (void)askMinMax:(struct MUI_MinMax *)minmaxinfo
{
	[super askMinMax:minmaxinfo];

	minmaxinfo->MinWidth += 100;
	minmaxinfo->MinHeight += 100;
	minmaxinfo->DefWidth += 1024;
	minmaxinfo->DefHeight += 740;
	minmaxinfo->MaxWidth = MUI_MAXMAX;
	minmaxinfo->MaxHeight = MUI_MAXMAX;
}

- (BOOL)draw:(ULONG)flags
{
	[super draw:flags];
	
	LONG iw = [self innerWidth];
	LONG ih = [self innerHeight];
	
	try {
		auto webPage = [_private page];
		webPage->setVisibleSize(int(iw), int(ih));
		webPage->draw([self rastPort], [self left], [self top], iw, ih, MADF_DRAWUPDATE == (MADF_DRAWUPDATE & flags));
	} catch (std::exception &ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}

	return TRUE;
}

- (BOOL)show:(struct LongRect *)clip
{
	if ([super show:clip])
	{
		try {
			auto webPage = [_private page];
			webPage->goVisible();
			
			if ([_private documentWidth])
			{
				[[_private scrollingDelegate] webView:self changedContentsSizeToWidth:[_private documentWidth]
					height:[_private documentHeight]];
				[[_private scrollingDelegate] webView:self scrolledToLeft:[_private scrollX] top:[_private scrollY]];
			}
		} catch (std::exception &ex) {
			dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
		}
		return YES;
	}
	
	return NO;
}

- (void)hide
{
	if ([self window])
	{
		struct TagItem tags[] = { { WA_PointerType, (IPTR)0 }, { TAG_DONE } };
		SetWindowPointerA([self window], tags);
	}

	[super hide];

	try {
		auto webPage = [_private page];
		webPage->goHidden();
	} catch (std::exception &ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
}

- (void)initResize:(ULONG)flags
{
	[super initResize:flags];
	[_private setIsLiveResizing:YES];
	auto webPage = [_private page];
	webPage->startLiveResize();
}

- (void)postExitResize
{
	[self redraw:MADF_DRAWOBJECT];
}

- (void)exitResize
{
	[super exitResize];
	[_private setIsLiveResizing:NO];
	auto webPage = [_private page];
	webPage->endLiveResize();
	[[OBRunLoop mainRunLoop] performSelector:@selector(postExitResize) target:self];
}

- (void)goActive:(ULONG)flags
{
	[super goActive:flags];

	[_private setIsActive:true];
	[[self windowObject] setDisableKeys:(1<<MUIKEY_WINDOW_CLOSE)|(1<<MUIKEY_GADGET_NEXT)|(1<<MUIKEY_GADGET_PREV)];

	try {
		auto webPage = [_private page];
		webPage->goActive();
	} catch (std::exception &ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
}

- (void)becomeInactive
{
	[_private setIsActive:false];
	[[self windowObject] setDisableKeys:0];

	try {
		auto webPage = [_private page];
		webPage->goInactive();
	} catch (std::exception &ex) {
		dprintf("%s: exception %s\n", __PRETTY_FUNCTION__, ex.what());
	}
}

- (void)goInactive:(ULONG)flags
{
	[super goInactive:flags];
	[self becomeInactive];
}

- (void)cleanup
{
	[self becomeInactive];
	[super cleanup];
}

- (ULONG)handleEvent:(struct IntuiMessage *)imsg muikey:(LONG)muikey
{
	auto webPage = [_private page];

	if (muikey != MUIKEY_NONE && webPage->handleMUIKey(int(muikey), [[self windowObject] defaultObject] == self))
		return MUI_EventHandlerRC_Eat;

	if (imsg)
		if (webPage->handleIntuiMessage(imsg, [self mouseX:imsg], [self mouseY:imsg], [self isInObject:imsg], [[self windowObject] defaultObject] == self))
			return MUI_EventHandlerRC_Eat;

	return 0;
}

static void populateContextMenu(MUIMenu *menu, const WTF::Vector<WebCore::ContextMenuItem> &items)
{
	for (const WebCore::ContextMenuItem& item : items)
	{
		auto title = item.title().utf8();

		switch (item.type())
		{
		case WebCore::ContextMenuItemType::ActionType:
			[menu addObject:[MUIMenuitem itemWithTitle:[OBString stringWithUTF8String:title.data()] shortcut:nil userData:ULONG(item.action())]];
			break;
		case WebCore::ContextMenuItemType::CheckableActionType:
			[menu addObject:[MUIMenuitem checkmarkItemWithTitle:[OBString stringWithUTF8String:title.data()] shortcut:nil userData:int(item.action()) checked:item.checked()]];
			break;
		case WebCore::ContextMenuItemType::SeparatorType:
			[menu addObject:[MUIMenuitem barItem]];
			break;
		case WebCore::ContextMenuItemType::SubmenuType:
			{
				MUIMenu *submenu = [MUIMenu menuWithTitle:[OBString stringWithUTF8String:title.data()] objects:nil, nil];
				[menu addObject:submenu];
				populateContextMenu(submenu, item.subMenuItems());
			}
			break;
		}
	}
}

- (IPTR)contextMenuAdd:(MUIMenustrip *)menustrip mx:(LONG)mx my:(LONG)my mxp:(LONG *)mxp myp:(LONG *)myp
{
	auto webPage = [_private page];
	auto items = webPage->buildContextMenu(mx - [self left], my - [self top]);

	if (0 == items.size())
		return 0;

	MUIMenu *menu = [MUIMenu menuWithTitle:[[MUIApplication currentApplication] title] objects:nil, nil];
	[menustrip addObject:menu];
	populateContextMenu(menu, items);

	return 0;
}

- (void)contextMenuChoice:(MUIMenuitem *)item
{
	auto webPage = [_private page];
	webPage->onContextMenuItemSelected([item userData], [[item title] cString]);
}

- (void)lateDraw
{
	if ([_private drawPending])
	{
		[self redraw:MADF_DRAWUPDATE];
		[_private setDrawPending:false];
	}
}

- (void)invalidated
{
	if (![_private drawPending])
	{
		[[OBRunLoop mainRunLoop] performSelector:@selector(lateDraw) target:self];
		[_private setDrawPending:true];
	}
}

- (void)scrollToX:(int)sx y:(int)sy
{
	[_private setScrollX:sx y:sy];
	[[_private scrollingDelegate] webView:self scrolledToLeft:sx top:sy];

	if (![_private drawPending])
	{
		[[OBRunLoop mainRunLoop] performSelector:@selector(lateDraw) target:self];
		[_private setDrawPending:true];
	}
}

- (void)setDocumentWidth:(int)width height:(int)height
{
	[_private setDocumentWidth:width height:height];
	[[_private scrollingDelegate] webView:self changedContentsSizeToWidth:width height:height];
}

- (void)setBackForwardListDelegate:(id<WkWebViewBackForwardListDelegate>)delegate
{
	[_private setBackForwardDelegate:delegate];
}

- (void)setDebugConsoleDelegate:(id<WkWebViewDebugConsoleDelegate>)delegate
{
	[_private setConsoleDelegate:delegate];
}

- (void)setCustomProtocolHandler:(id<WkWebViewNetworkProtocolHandlerDelegate>)delegate forProtocol:(OBString *)protocol
{
	[_private setCustomProtocolHandler:delegate forProtocol:protocol];
}

- (void)setDownloadDelegate:(id<WkDownloadDelegate>)delegate
{
	[_private setDownloadDelegate:delegate];
}

- (void)setDialogDelegate:(id<WkWebViewDialogDelegate>)delegate
{
	[_private setDialogDelegate:delegate];
}

- (void)setAutofillDelegate:(id<WkWebViewAutofillDelegate>)delegate
{
	[_private setAutofillDelegate:delegate];
}

- (BOOL)hasAutofillElements
{
	auto webPage = [_private page];
	return webPage->hasAutofillElements();
}

- (void)autofillElementsWithLogin:(OBString *)login password:(OBString *)password
{
	auto webPage = [_private page];
	return webPage->setAutofillElements(WTF::String::fromUTF8([login cString]), WTF::String::fromUTF8([password cString]));
}

- (float)textZoomFactor
{
	auto webPage = [_private page];
	return webPage->textZoomFactor();
}

- (float)pageZoomFactor
{
	auto webPage = [_private page];
	return webPage->pageZoomFactor();
}

- (void)setPageZoomFactor:(float)pageFactor textZoomFactor:(float)textFactor
{
	auto webPage = [_private page];
	webPage->setPageAndTextZoomFactors(pageFactor, textFactor);
}

@end
