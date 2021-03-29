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
#import <WebCore/FindOptions.h>
#import <WebCore/AuthenticationChallenge.h>
#import <WebCore/AuthenticationClient.h>
#import <WebCore/HitTestResult.h>
#import <WebCore/MediaPlayerMorphOS.h>
#define __OBJC__

#import "WkHitTest_private.h"

#import <ob/OBFramework.h>
#import <mui/MUIFramework.h>
#import <proto/muimaster.h>
#import <proto/graphics.h>
#import <proto/layers.h>

#import "WkWebView.h"
#import "WkHistory_private.h"
#import "WkSettings.h"
#import "WkCertificate_private.h"
#import "WkError_private.h"
#import "WkDownload_private.h"
#import "WkFileDialog_private.h"
#import "WkFavIcon_private.h"
#import "WkPrinting_private.h"
#import "WkUserScript_private.h"

#import <proto/dos.h>
#import <proto/exec.h>
#import <proto/intuition.h>
#import <proto/openurl.h>
#import <libraries/openurl.h>
#import <pthread.h>
#import <hardware/atomic.h>
#import <exec/system.h>

#import <cairo.h>
struct Library *FreetypeBase;

#include "libeventprofiler.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmisleading-indentation"

extern "C" { void dprintf(const char *, ...); }

#define D(x) 

// #define VALIDATE_ALLOCS 15.f
#ifdef VALIDATE_ALLOCS
extern "C" { ULONG MEM_ValidateAllocs(ULONG flags); }
#endif

namespace  {
	static int _viewInstanceCount;
	static bool _shutdown;
	static bool _readyToQuitPending;
	static bool _initializedOK;
	static OBSignalHandler *_signalHandler;
	static OBScheduledTimer *_heartBeatTimer;
	static OBScheduledTimer *_fastSingleBeatTimer;
	static OBPerform        *_timerPerform;
	APTR   _globalOBContext;
	struct Task *_mainThread;
}

@interface WkWebView ()

- (void)invalidated:(BOOL)force;
- (void)scrollToX:(int)sx y:(int)sy;
- (void)setDocumentWidth:(int)width height:(int)height;
- (void)recalculatePrinting;
- (void)lateDraw;

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

#if ENABLE(VIDEO)
@interface WkMediaLoadResponseHandlerPrivate : OBObject<WkMediaLoadResponseHandler>
{
	WTF::Function<void(bool doLoad)>  _loadFunction;
	WTF::Function<void()>             _yieldFunction;
	WebCore::MediaPlayerMorphOSInfo   _info;
	OBURL                            *_url;
	OBURL                            *_pageURL;
	void                             *_playerRef;
}
@end

@implementation WkMediaLoadResponseHandlerPrivate

- (id)initWithPlayer:(void *)playerRef url:(OBURL *)url pageURL:(OBURL *)pageurl info:(WebCore::MediaPlayerMorphOSInfo &)info
	callback:(WTF::Function<void(bool doLoad)>&&)func
	yieldCallback:(WTF::Function<void()>&&)yield
{
	if ((self = [super init]))
	{
		_loadFunction = WTFMove(func);
		_yieldFunction = WTFMove(yield);
		_url = [url retain];
		_pageURL = [pageurl retain];
		_playerRef = playerRef;
		_info = info;
	}
	else
	{
		func(true);
	}
	
	return self;
}

- (void)invalidate
{
	_playerRef = nullptr;
	_loadFunction = nullptr;
	_yieldFunction = nullptr;
}

- (void)dealloc
{
	[self invalidate];
	[_url release];
	[_pageURL release];
	[super dealloc];
}

- (OBURL *)mediaURL
{
	return _url;
}

- (OBURL *)pageURL
{
	return  _pageURL;
}

- (IPTR)playerRef
{
	return (IPTR)_playerRef;
}

- (void)proceed
{
	if (_loadFunction)
		_loadFunction(true);
	_loadFunction = nullptr;
}

- (void)cancel
{
	if (_loadFunction)
		_loadFunction(false);
	[self invalidate];
}

- (void)yield:(void *)playerRef
{
dprintf("yield: pr %p myref %p\n", playerRef, _playerRef);
	if (_yieldFunction && playerRef != _playerRef)
		_yieldFunction();
}

- (BOOL)hasAudio
{
	return _info.m_frequency != 0;
}

- (BOOL)hasVideo
{
	return _info.m_width != 0;
}

- (float)duration
{
	return _info.m_duration;
}

- (BOOL)isLive
{
	return _info.m_isLive;
}

- (int)audioRate
{
	return _info.m_frequency;
}

- (int)audioChannels
{
	return _info.m_channels;
}

- (int)audioBits
{
	return _info.m_bits;
}

- (int)videoWidth
{
	return _info.m_width;
}

- (int)videoHeight
{
	return _info.m_height;
}

@end
#endif

@interface WkWebViewPrivate : OBObject
{
	WTF::RefPtr<WebKit::WebPage>            _page;
	id<WkWebViewScrollingDelegate>          _scrollingDelegate;
	id<WkWebViewClientDelegate>             _clientDelegate;
	id<WkWebViewBackForwardListDelegate>    _backForwardDelegate;
	id<WkWebViewDebugConsoleDelegate>       _consoleDelegate;
	id<WkDownloadDelegate>                  _downloadDelegate;
	id<WkWebViewDialogDelegate>             _dialogDelegate;
	id<WkWebViewAutofillDelegate>           _autofillDelegate;
	id<WkWebViewProgressDelegate>           _progressDelegate;
	id<WkWebViewContextMenuDelegate>        _contextMenuDelegate;
	id<WkWebViewAllRequestsHandlerDelegate> _allRequestsDelegate;
	id<WkWebViewEditorDelegate>             _editorDelegate;
	id<WkWebViewMediaDelegate>              _mediaDelegate;
	OBMutableDictionary                    *_protocolDelegates;
#if ENABLE(VIDEO)
	OBMutableDictionary                    *_mediaPlayers;
#endif
	WkBackForwardListPrivate               *_backForwardList;
	WkSettings_Throttling                   _throttling;
	WkSettings_UserStyleSheet               _userStyleSheet;
	OBString                               *_userStyleSheetFile;
	WkPrintingStatePrivate                 *_printingState;
	bool                                    _drawPending;
	bool                                    _isActive;
	bool                                    _isLoading;
	bool                                    _isLiveResizing;
	bool                                    _hasOnlySecureContent;
	bool                                    _isHandlingUserInput;
	OBURL                                  *_url;
	OBString                               *_title;
	OBURL                                  *_hover;
	int                                     _scrollX, _scrollY;
	int                                     _documentWidth, _documentHeight;
	float                                   _savedPageZoom, _savedTextZoom;
	UQUAD                                   _drawTime;
	ULONG                                   _drawTimeBase;
	UQUAD                                   _drawTimeLast;
	OBPerform                              *_paintPerform;
	OBScheduledTimer                       *_paintTimer;
	struct Window                          *_window;
	int                                     _mleft, _mtop, _mbottom, _mright;
#if ENABLE(VIDEO)
	Function<void(void *windowPtr, int scrollX, int scrollY, int left, int top, int right, int bottom, int width, int height)> _overlayCallback;
	WebCore::Element *_overlayElement;
	void *_overlayPlayer;
#endif
}
@end

@implementation WkWebViewPrivate

- (id)init
{
	if ((self = [super init]))
	{
		_throttling = WkSettings_Throttling_InvisibleBrowsers;
		_hasOnlySecureContent = YES;
		_userStyleSheet = WkSettings_UserStyleSheet_MUI;
		_userStyleSheetFile = @"PROGDIR:Resources/morphos.css";
		
		NewGetSystemAttrsA(&_drawTimeBase, sizeof(_drawTimeBase), SYSTEMINFOTYPE_TBCLOCKFREQUENCY, NULL);
	}
	return self;
}

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
	[_hover release];
	[_protocolDelegates release];
	[_backForwardList release];
	[_userStyleSheetFile release];
	[_printingState invalidate];
	[_printingState release];
	[_paintPerform release];
	[_paintTimer invalidate];
	[_paintTimer release];
#if ENABLE(VIDEO)
	[[_mediaPlayers allValues] makeObjectsPerformSelector:@selector(invalidate)];
	[_mediaPlayers release];
#endif

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

- (WTF::RefPtr<WebKit::WebPage>)pageRefPtr
{
	return _page;
}

- (void)setScrollingDelegate:(id<WkWebViewScrollingDelegate>)delegate
{
	_scrollingDelegate = delegate;
}

- (id<WkWebViewScrollingDelegate>)scrollingDelegate
{
	return _scrollingDelegate;
}

- (void)setClientDelegate:(id<WkWebViewClientDelegate>)delegate
{
	_clientDelegate = delegate;
}

- (id<WkWebViewClientDelegate>)clientDelegate
{
	return _clientDelegate;
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

- (void)setDrawPendingWithSchedule:(BOOL)schedule
{
	if (!_drawPending)
	{
		[_paintTimer invalidate];
		[_paintTimer release];
		_paintTimer = nil;

		_drawPending = YES;

		if (schedule && _isHandlingUserInput)
		{
			[[OBRunLoop mainRunLoop] perform:_paintPerform];
			return;
		}
		
		if (schedule && _paintPerform)
		{
			static double divider = (double)_drawTimeBase;
			double interval = _drawTime;
			interval /= divider;
			interval *= 3.0;
			double timeSinceLast = (__builtin_ppc_get_timebase() - _drawTimeLast);
			timeSinceLast /= divider;
			if (timeSinceLast > interval)
				interval = (1.0 / 60.0);
			if (interval < (1.0 / 60.0))
				interval = (1.0 / 60.0);
			if (interval > 0.7)
				interval = 0.7;

			_paintTimer = [[OBScheduledTimer scheduledTimerWithInterval:interval perform:_paintPerform repeats:NO] retain];
		}
	}
}

- (void)drawFinishedIn:(UQUAD)timebaseticks
{
	_drawPending = NO;
	_drawTime = timebaseticks;
	_drawTimeLast = __builtin_ppc_get_timebase();
}

- (void)setPaintPerform:(OBPerform *)paint
{
	[_paintTimer invalidate];
	[_paintTimer release];
	_paintTimer = nil;

	[_paintPerform autorelease];
	_paintPerform = [paint retain];
}

- (BOOL)isHandlingUserInput
{
	return _isHandlingUserInput;
}

- (void)setIsHandlingUserInput:(BOOL)handling
{
	_isHandlingUserInput = handling;
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

- (void)setProgressDelegate:(id<WkWebViewProgressDelegate>)delegate
{
	_progressDelegate = delegate;
}

- (id<WkWebViewProgressDelegate>)progressDelegate
{
	return _progressDelegate;
}

- (void)setContextMenuDelegate:(id<WkWebViewContextMenuDelegate>)delegate
{
	_contextMenuDelegate = delegate;
}

- (id<WkWebViewContextMenuDelegate>)contextMenuDelegate
{
	return _contextMenuDelegate;
}

- (void)setAllRequestsHandler:(id<WkWebViewAllRequestsHandlerDelegate>)delegate
{
	_allRequestsDelegate = delegate;
}

- (id<WkWebViewAllRequestsHandlerDelegate>)allRequestsHandler
{
	return _allRequestsDelegate;
}

- (void)setEditorDelegate:(id<WkWebViewEditorDelegate>)delegate
{
	_editorDelegate = delegate;
}

- (id<WkWebViewEditorDelegate>)editorDelegate
{
	return _editorDelegate;
}

- (void)setMediaDelegate:(id<WkWebViewMediaDelegate>)md
{
	_mediaDelegate = md;
}

- (id<WkWebViewMediaDelegate>)mediaDelegate
{
	return _mediaDelegate;
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

- (bool)drawPendingWithSchedule
{
	return _drawPending && _paintTimer;
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

#if ENABLE(VIDEO)
- (void)callOverlayCallback
{
	if (!!_overlayCallback && _overlayElement)
	{
		if (_window)
		{
			WebCore::IntRect ePos = _page->getElementBounds(_overlayElement);
			ULONG il = _mleft - _window->BorderLeft;
			ULONG it = _mtop - _window->BorderTop;
			ULONG iw, ih;
	
//
//			ePos.scale(_page->pageZoomFactor());
			
			GetAttr(WA_InnerWidth, (Boopsiobject *)_window, &iw);
			GetAttr(WA_InnerHeight, (Boopsiobject *)_window, &ih);

			_overlayCallback(_window, _scrollX, _scrollY,
				il + ePos.x(),
				it + ePos.y(),
				iw - (il + ePos.maxX()),
				ih - (it + ePos.maxY()),
				ePos.width(),
				ePos.height());
		}
		else
		{
			_overlayCallback(nullptr, _scrollX, _scrollY, 0, 0, 0, 0, 0, 0);
		}
	}
}
#endif

- (void)setMLeft:(int)left mTop:(int)mtop mRight:(int)mright mBottom:(int)mbottom
{
	_mleft = left;
	_mtop = mtop;
	_mright = mright;
	_mbottom = mbottom;

#if ENABLE(VIDEO)
	[self callOverlayCallback];
#endif
}

- (void)setDocumentWidth:(int)width height:(int)height
{
	_documentWidth = width;
	_documentHeight = height;

#if ENABLE(VIDEO)
	[self callOverlayCallback];
#endif
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

#if ENABLE(VIDEO)
	[self callOverlayCallback];
#endif
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

- (WkBackForwardListPrivate *)backForwardList
{
	if (nil == _backForwardList)
	{
		WTF::RefPtr<WebKit::BackForwardClientMorphOS> client = _page->backForwardClient();
		if (client.get())
		{
			_backForwardList = [[WkBackForwardListPrivate backForwardListPrivate:client] retain];
		}
	}
	
	return _backForwardList;
}

- (WkSettings_Throttling)throttling
{
	return _throttling;
}

- (void)setThrottling:(WkSettings_Throttling)throttling
{
	_throttling = throttling;
}

- (void)setHover:(OBURL *)hover
{
	[_hover autorelease];
	_hover = [hover retain];
}

- (OBURL *)hover
{
	return _hover;
}

- (WkSettings_UserStyleSheet)styleSheet
{
	return _userStyleSheet;
}

- (void)setStyleSheet:(WkSettings_UserStyleSheet)styleSheet
{
	_userStyleSheet = styleSheet;
}

- (OBString *)customStyleSheetPath
{
	return _userStyleSheetFile;
}

- (void)setCustomStyleSheetPath:(OBString *)path
{
	[_userStyleSheetFile autorelease];
	_userStyleSheetFile = [path copy];
}

- (WkPrintingState *)beingPrintingWithWebView:(WkWebView *)view settings:(OBDictionary *)settings
{
	if (nil == _printingState)
	{
		_savedPageZoom = _page->pageZoomFactor();
		_savedTextZoom = _page->textZoomFactor();

		_drawPending = YES; // ignore some repaints..

		auto *state = [[WkPrintingStatePrivate alloc] initWithWebView:view frame:_page->mainFrame()];
		[state setSettings:(id)settings];
		_printingState = state;
	}
	return _printingState;
}

- (WkPrintingStatePrivate *)printingState
{
	return _printingState;
}

- (void)endPrinting
{
	if (_printingState)
	{
		_page->setPageAndTextZoomFactors(_savedPageZoom, _savedTextZoom);
		_page->printingFinished();
		[_printingState invalidate];
		[_printingState release];
		_printingState = nil;
	}
}

- (void)setSavedPageZoom:(float)page textZoom:(float)text
{
	_savedTextZoom = text;
	_savedPageZoom = page;
}

#if ENABLE(VIDEO)
- (void)playerAdded:(WkMediaLoadResponseHandlerPrivate *)handler
{
	if (handler)
	{
		OBNumber *ref = [OBNumber numberWithUnsignedLong:[handler playerRef]];
		WkMediaLoadResponseHandlerPrivate *existing = [_mediaPlayers objectForKey:ref];
		if (existing)
		{
			[existing invalidate];
		}
		if (nil == _mediaPlayers)
			_mediaPlayers = [OBMutableDictionary new];
		[_mediaPlayers setObject:handler forKey:ref];
	}
}

- (WkMediaLoadResponseHandlerPrivate *)handlerForPlayer:(void *)playerRef
{
	OBNumber *ref = [OBNumber numberWithUnsignedLong:(IPTR)playerRef];
	return [_mediaPlayers objectForKey:ref];
}

- (void)playerRemoved:(void *)playerRef
{
	OBNumber *ref = [OBNumber numberWithUnsignedLong:(IPTR)playerRef];
	WkMediaLoadResponseHandlerPrivate *handler = [_mediaPlayers objectForKey:ref];

	if (handler)
	{
		[handler invalidate];
		[_mediaPlayers removeObjectForKey:ref];
	}
	
	if (playerRef == _overlayPlayer)
	{
		_overlayCallback = nullptr;
		_overlayElement = nullptr;
	}
}

- (void)playerWillPlay:(void *)playerRef
{
	OBEnumerator *e = [_mediaPlayers objectEnumerator];
	WkMediaLoadResponseHandlerPrivate *handler;
	
	while (handler = [e nextObject])
	{
		[handler yield:playerRef];
	}
}

- (void)setWindow:(struct Window *)window
{
	if (_window != window)
	{
		_window = window;
		[self callOverlayCallback];
	}
}

- (void)setOverlayCallback:(void *)playerRef element:(WebCore::Element *)element
	callback:(Function<void(void *windowPtr, int scrollX, int scrollY, int left, int right, int rigth, int bottom, int width, int height)>&&)cb
{
	if (!!_overlayCallback)
	{
		_overlayCallback(nullptr, 0, 0, 0, 0, 0, 0, 0, 0);
	}
	
	_overlayCallback = WTFMove(cb);
	_overlayElement = element;
	
	[self callOverlayCallback];
	
	// Workaround some positioning issues
	[[OBRunLoop mainRunLoop] performSelector:@selector(callOverlayCallback) target:self];
}

#endif

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

		EP_BEGIN(WebKitSignals);
		WebKit::WebProcess::singleton().handleSignals(mask);
		EP_END(WebKitSignals);

		float nextTimerEvent = WebKit::WebProcess::singleton().timeToNextTimerEvent();

		if (nextTimerEvent <= 0.001)
		{
			for (int i = 0; i < 5 && nextTimerEvent <= 0.001 ; i++)
			{
				WebKit::WebProcess::singleton().handleSignals(mask);
				nextTimerEvent = WebKit::WebProcess::singleton().timeToNextTimerEvent();
			}

			if (nextTimerEvent <= 0.001)
			{
				// yield and repeat
				[_signalHandler fire];
				return;
			}
		}
		
		// note: we have an additional 4Hz heartbeat timer running all the time!
		if (nextTimerEvent < 0.25)
		{
			[_fastSingleBeatTimer invalidate];
			[_fastSingleBeatTimer release];
			_fastSingleBeatTimer = [[OBScheduledTimer scheduledTimerWithInterval:nextTimerEvent perform:_timerPerform repeats:NO] retain];
		}
		else
		{
			[_fastSingleBeatTimer invalidate];
			[_fastSingleBeatTimer release];
			_fastSingleBeatTimer = nil;
		}
	}
}

+ (void)fire
{
	[self performWithSignalHandler:_signalHandler];
}

#ifdef VALIDATE_ALLOCS
+ (void)validateAllocs
{
	MEM_ValidateAllocs(1);
}
#endif

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
		{
			WebCore::DOMWindow::dispatchAllPendingBeforeUnloadEvents();
			// As soon as possible ask all sub-threads to stop processing. This
			// will send SIGBREAKF_CTRL_C to all sub-threads. They will take some
			// time to shut down on the background, so better start the process
			// as soon as possible.
			pthread_t tid, self = pthread_self();
			for (tid = 1; tid < PTHREAD_THREADS_MAX; tid++)
			{
				if (tid != self)
					pthread_kill(tid, SIGINT);
			}
		}
		_readyToQuitPending = YES;
		if (_viewInstanceCount == 0 &&
			WebKit::WebProcess::singleton().webFrameCount() == 0)
		{
			_readyToQuitPending = NO;
			_shutdown = YES;
			[_heartBeatTimer invalidate];
			[_heartBeatTimer release];
			_heartBeatTimer = nil;
			[_fastSingleBeatTimer invalidate];
			[_fastSingleBeatTimer release];
			_fastSingleBeatTimer = nil;
			[_timerPerform release];
			_timerPerform = nil;
			[[OBRunLoop mainRunLoop] removeSignalHandler:_signalHandler];
			[_signalHandler release];
			WebKit::WebProcess::singleton().terminate();
			[WkCertificate shutdown];
			[WkUserScripts shutdown];
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

+ (void)initialize
{
	@synchronized (self)
	{
		_mainThread = FindTask(0);
		_globalOBContext = _mainThread->tc_ETask->OBContext;

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
				return;
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
			
			_timerPerform = [[OBPerform performSelector:@selector(fire) target:[self class]] retain];
			_heartBeatTimer = [[OBScheduledTimer scheduledTimerWithInterval:0.25 perform:_timerPerform repeats:YES] retain];

#ifdef VALIDATE_ALLOCS
			[OBScheduledTimer scheduledTimerWithInterval:VALIDATE_ALLOCS perform:[OBPerform performSelector:@selector(validateAllocs) target:[self class]] repeats:YES];
#endif

			WebKit::WebProcess::singleton().initialize(int([_signalHandler sigBit]));
			WebKit::WebProcess::singleton().setLastPageClosedCallback([]() {
				[WkWebView _lastPageClosed];
			});

			_initializedOK = YES;
		}
	}
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
		}
		
		if (!_initializedOK)
		{
			[self release];
			return nil;
		}

		self.fillArea = NO;
		self.handledEvents = IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_MOUSEHOVER | IDCMP_RAWKEY | IDCMP_INACTIVEWINDOW;
		[self setEventHandlerGUIMode:YES];
		self.cycleChain = YES;

		_private = [WkWebViewPrivate new];
		if (!_private)
		{
			[self release];
			return nil;
		}

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

		webPage->_fInvalidate = [self](bool force) { [self invalidated:force]; };

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

		webPage->_fGoInactive = [self]() {
			[[self windowObject] setActiveObject:nil];
		};
		
		webPage->_fZoomChangedByWheel = [self]() {
			WkWebViewPrivate *privateObject = [self privateObject];
			[privateObject callOverlayCallback];
		};

		webPage->_fUserAgentForURL = [self](const WTF::String& url) -> WTF::String {
			validateObjCContext();
			WkWebViewPrivate *privateObject = [self privateObject];
			id<WkWebViewClientDelegate> clientDelegate = [privateObject clientDelegate];
			if (clientDelegate)
			{
				auto uurl = url.utf8();
				OBString *overload = [clientDelegate userAgentForURL:[OBString stringWithUTF8String:uurl.data()]];

				if (overload)
					return WTF::String::fromUTF8([overload cString]);
			}
			return WTF::String::fromUTF8("Mozilla/5.0 (Macintosh; Intel Mac OS X 10_13_6) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/13.1 Safari/605.1.15");
		};
		
		webPage->_fChangedTitle = [self](const WTF::String& title) {
			validateObjCContext();
			WkWebViewPrivate *privateObject = [self privateObject];
			id<WkWebViewClientDelegate> clientDelegate = [privateObject clientDelegate];
			auto utitle = title.utf8();
			OBString *otitle = [OBString stringWithUTF8String:utitle.data()];
			[privateObject setTitle:otitle];
			[clientDelegate webView:self changedTitle:otitle];
		};

		webPage->_fChangedURL = [self](const WTF::String& url) {
			validateObjCContext();
			WkWebViewPrivate *privateObject = [self privateObject];
			id<WkWebViewClientDelegate> clientDelegate = [privateObject clientDelegate];
			auto uurl = url.utf8();
			OBURL *ourl = [OBURL URLWithString:[OBString stringWithUTF8String:uurl.data()]];
			[privateObject setHasOnlySecureContent:YES];
			[privateObject setURL:ourl];
			[clientDelegate webView:self changedDocumentURL:ourl];
		};
		
		webPage->_fDidStartLoading = [self]() {
			validateObjCContext();
			WkWebViewPrivate *privateObject = [self privateObject];
			id<WkWebViewClientDelegate> clientDelegate = [privateObject clientDelegate];
			[clientDelegate webView:self documentReady:NO];
		};

		webPage->_fPrint = [self]() {
			validateObjCContext();
			WkWebViewPrivate *privateObject = [self privateObject];
			id<WkWebViewClientDelegate> clientDelegate = [privateObject clientDelegate];
			[clientDelegate webViewRequestedPrinting:self];
		};
		
		webPage->_fDidStopLoading = [self]() {
			validateObjCContext();
			WkWebViewPrivate *privateObject = [self privateObject];
			id<WkWebViewClientDelegate> clientDelegate = [privateObject clientDelegate];
			[self recalculatePrinting];
			[clientDelegate webView:self documentReady:YES];
		};

		webPage->_fDidLoadInsecureContent = [self]() {
			validateObjCContext();
			WkWebViewPrivate *privateObject = [self privateObject];
			id<WkWebViewClientDelegate> clientDelegate = [privateObject clientDelegate];
			[privateObject setHasOnlySecureContent:NO];
			[clientDelegate webViewDidLoadInsecureContent:self];
		};
		
		webPage->_fCanOpenWindow = [self](const WTF::String& url, const WebCore::WindowFeatures&) -> bool {
			validateObjCContext();
			WkWebViewPrivate *privateObject = [self privateObject];
			id<WkWebViewClientDelegate> clientDelegate = [privateObject clientDelegate];
			if (clientDelegate)
			{
				auto uurl = url.utf8();
				OBURL *url = [OBURL URLWithString:[OBString stringWithUTF8String:uurl.data()]];
				return [clientDelegate webView:self wantsToCreateNewViewWithURL:url options:nil];
			}
			return NO;
		};
		
		webPage->_fDoOpenWindow = [self]() -> WebCore::Page * {
			validateObjCContext();
			WkWebViewPrivate *privateObject = [self privateObject];
			id<WkWebViewClientDelegate> clientDelegate = [privateObject clientDelegate];
			if (!clientDelegate)
				return nullptr;

			WkWebView *newView = [[[self class] new] autorelease];
			WkWebViewPrivate *newPrivateObject = [newView privateObject];
			WebKit::WebPage *page = [newPrivateObject page];
			if (page && page->corePage())
			{
				[clientDelegate webView:self createdNewWebView:newView];
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
		
		webPage->_fContextMenu = [self](const WebCore::IntPoint& pos, const WTF::Vector<WebCore::ContextMenuItem> &items, const WebCore::HitTestResult &hitTest) -> bool {
			validateObjCContext();
			WkWebViewPrivate *privateObject = [self privateObject];
			id<WkWebViewContextMenuDelegate> contextMenuDelegate = [privateObject contextMenuDelegate];
			WebKit::WebPage *page = [privateObject page];
			MUIMenu *menu = [MUIMenu new];
			WkHitTestPrivate *wkhit = contextMenuDelegate ? [[WkHitTestPrivate hitTestFromHitTestResult:hitTest onWebPage:[privateObject pageRefPtr]] retain] : nil;

			if ([wkhit isContentEditable] && 0 == [[wkhit selectedText] length])
			{
				page->markWord(*[wkhit hitTestInternal]);
				[self redraw:MADF_DRAWUPDATE];
			}

			if (contextMenuDelegate)
				[contextMenuDelegate webView:self needsToPopulateMenu:menu withHitTest:wkhit];
			else
				populateContextMenu(menu, items);
	
			if ([menu count])
			{
				MUIMenustrip *strip = [[MUIMenustrip menustripWithObjects:menu, nil] retain];
				if (strip)
				{
					// 0 on failure, all our menus return 1, 2...
					int rc = [strip popup:self flags:0 x:[self left] + pos.x() y:[self top] + pos.y()];
					if (rc)
					{
						if (contextMenuDelegate)
						{
							[contextMenuDelegate webView:self didSelectMenuitemWithUserDatra:rc withHitTest:wkhit];
						}
						else
						{
							MUIMenuitem *item = [strip findUData:rc];
							page->onContextMenuItemSelected(rc, [[item title] cString]);
						}
					}
					[strip release];
				}
			}
			
			bool ok = [menu count] > 0;
			[wkhit release];
			[menu release];
			return ok;
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
			else
			{
				// dprintf("CONSOLE(%d@%d): %s\n", level, line, message.utf8().data());
			}
		};
		
		webPage->_fDidFailWithError = [self](const WebCore::ResourceError &error) {
			validateObjCContext();
			WkWebViewPrivate *privateObject = [self privateObject];
			id<WkWebViewClientDelegate> clientDelegate = [privateObject clientDelegate];
			if (clientDelegate)
			{
				[clientDelegate webView:self documentReady:YES];
				[clientDelegate webView:self didFailLoadingWithError:[WkError errorWithResourceError:error]];
			}
		};
		
		webPage->_fCanHandleRequest = [self](const WebCore::ResourceRequest &request) {
			validateObjCContext();
			WkWebViewPrivate *privateObject = [self privateObject];
			const WTF::URL &url = request.url();

			WTF::String protocol = url.protocol().toString();

			// bypass standard protocols...
			if (protocol == "http" || protocol == "https" || protocol == "file" || protocol == "about" || protocol == "blob")
			{
				return true;
			}

			auto uprotocol = protocol.utf8();
			OBString *oprotocol = [OBString stringWithUTF8String:uprotocol.data()];
			id<WkWebViewNetworkProtocolHandlerDelegate> delegate = [privateObject protocolDelegateForProtocol:oprotocol];
			if (delegate)
			{
				return true;
			}

			return false;
		};
		
		webPage->_fShouldNavigateToURL = [self](const WTF::URL &url, bool window) -> bool {
			validateObjCContext();
			WkWebViewPrivate *privateObject = [self privateObject];
			WTF::String protocol = url.protocol().toString();

			id<WkWebViewAllRequestsHandlerDelegate> allHandler = [privateObject allRequestsHandler];
			if (allHandler)
			{
				auto uurl = url.string().utf8();
				OBURL *url = [OBURL URLWithString:[OBString stringWithUTF8String:uurl.data()]];
				if (![allHandler webView:self wantsToNavigateToURL:url])
					return false;
			}

			if (protocol == "ftp" || protocol == "mailto" || protocol == "ftps")
			{
				auto udata = url.string().ascii();
				struct TagItem urltags[] = { { URL_Launch, TRUE }, { URL_Show, TRUE }, { TAG_DONE, 0 } };
				URL_OpenA((STRPTR)udata.data(), urltags);
				return false;
			}

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

			return true;
		};
		
		webPage->_fDownload = [self](const WTF::URL &url, const WTF::String &) {
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
		
		webPage->_fDownloadAsk = [self](const WebCore::ResourceResponse& response, const WebCore::ResourceRequest&,
			WebCore::PolicyCheckIdentifier identifier, const WTF::String&, WebCore::FramePolicyFunction&& function) {
			validateObjCContext();
			WkWebViewPrivate *privateObject = [self privateObject];
			id<WkWebViewClientDelegate> clientDelegate = [privateObject clientDelegate];
			if (clientDelegate)
			{
				WkDownloadResponseDelegatePrivate *responsePrivate = [[[WkDownloadResponseDelegatePrivate alloc] initWithPolicyCheckIdentifier:identifier function:std::move(function)] autorelease];
				auto uurl = response.url().string().utf8();
				auto umime = response.mimeType().utf8();
				auto uname = response.suggestedFilename().utf8();
				
				if (0 == uname.length())
					uname = WebCore::decodeURLEscapeSequences(response.url().lastPathComponent()).utf8();
				
				[clientDelegate webView:self
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
				[[OBRunLoop mainRunLoop] performSelector:@selector(webView:wantsToOpenFileSelectionPanelWithSettings:responseHandler:) target:dialogDelegate withObject:self withObject:fd withObject:fd];
//				[dialogDelegate webView:self wantsToOpenFileSelectionPanelWithSettings:fd responseHandler:fd];
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
			id<WkWebViewClientDelegate> clientDelegate = [privateObject clientDelegate];
			if (clientDelegate)
			{
				auto uurl = inurl.string().utf8();
				OBURL *url = [OBURL URLWithString:[OBString stringWithUTF8String:uurl.data()]];
				OBString *modeKey;
				
				switch (mode)
				{
				case WebViewDelegateOpenWindowMode::NewWindow:
					modeKey = kWebViewClientDelegateOption_NewWindow;
					break;
				case WebViewDelegateOpenWindowMode::BackgroundTab:
					modeKey = kWebViewClientDelegateOption_NewTab;
					break;
				default:
					modeKey = kWebViewClientDelegateOption;
					break;
				}

				if ([clientDelegate webView:self wantsToCreateNewViewWithURL:url
					options:[OBDictionary dictionaryWithObject:modeKey forKey:kWebViewClientDelegateOption]])
				{
					WkWebView *newView = [[[self class] new] autorelease];
					[newView load:url];
					[clientDelegate webView:self createdNewWebView:newView];
					return YES;
				}
			}
			return NO;
		};
		
		webPage->_fAuthChallenge = [self](const WebCore::AuthenticationChallenge &challenge) -> bool {
			validateObjCContext();
			WkWebViewPrivate *privateObject = [self privateObject];
			id<WkWebViewClientDelegate> clientDelegate = [privateObject clientDelegate];
			if (clientDelegate)
			{
				WkAuthenticationChallengeResponseDelegatePrivate *responseDelegate =
					[[[WkAuthenticationChallengeResponseDelegatePrivate alloc] initWithAuthenticationChallenge:WkAuthenticationChallenge::create(challenge)] autorelease];
				if (responseDelegate)
				{
					[[OBRunLoop mainRunLoop] performSelector:@selector(webView:issuedAuthenticationChallengeAtURL:withResponseDelegate:)
						target:clientDelegate withObject:self withObject:[self URL] withObject:responseDelegate];
					return YES;
				}
			}
			return NO;
		};
		
		webPage->_fSetCursor = [self](int cursor) {
			validateObjCContext();
			if ([self window])
			{
				struct TagItem tags[] = { { WA_PointerType, (IPTR)cursor }, { TAG_DONE, 0 } };
				SetWindowPointerA([self window], tags);
			}
		};
		
		webPage->_fProgressStarted = [self]() {
			validateObjCContext();
			WkWebViewPrivate *privateObject = [self privateObject];
			id<WkWebViewProgressDelegate> progressDelegate = [privateObject progressDelegate];
			[privateObject setIsLoading:YES];
			[progressDelegate webViewDidStartProgress:self];
		};

		webPage->_fProgressUpdated = [self](float progress) {
			validateObjCContext();
			WkWebViewPrivate *privateObject = [self privateObject];
			id<WkWebViewProgressDelegate> progressDelegate = [privateObject progressDelegate];
			[progressDelegate webView:self didUpdateProgress:progress];
		};

		webPage->_fProgressFinished = [self]() {
			validateObjCContext();
			WkWebViewPrivate *privateObject = [self privateObject];
			id<WkWebViewProgressDelegate> progressDelegate = [privateObject progressDelegate];
			[privateObject setIsLoading:NO];
			[self recalculatePrinting];
			[progressDelegate webViewDidFinishProgress:self];
		};
		
		webPage->_fHoveredURLChanged = [self](const WTF::URL &url) {
			validateObjCContext();
			WkWebViewPrivate *privateObject = [self privateObject];
			if (url.string().length() > 0)
			{
				auto uurl = url.string().utf8();
				[privateObject setHover:[OBURL URLWithString:[OBString stringWithUTF8String:uurl.data()]]];
			}
			else
			{
				[privateObject setHover:nil];
			}
			[[privateObject clientDelegate] webView:self changedHoveredURL:[privateObject hover]];
		};
		
		webPage->_fFavIconLoad = [self](const WTF::URL &url) -> bool {
			validateObjCContext();
			WkWebViewPrivate *privateObject = [self privateObject];
			id<WkWebViewClientDelegate> clientDelegate = [privateObject clientDelegate];
			if (clientDelegate)
			{
				auto uurl = url.string().utf8();
				return [clientDelegate webView:self shouldLoadFavIconForURL:(OBURL *)[OBURL URLWithString:[OBString stringWithUTF8String:uurl.data()]]];
			}
			return false;
		};
		
		webPage->_fFavIconLoaded = [self](WebCore::SharedBuffer *data, const WTF::URL &url) {
			validateObjCContext();
			WkWebViewPrivate *privateObject = [self privateObject];
			id<WkWebViewClientDelegate> clientDelegate = [privateObject clientDelegate];
			if (clientDelegate)
			{
				auto uurl = url.host().toString().utf8();
				[clientDelegate webView:self changedFavIcon:[WkFavIconPrivate cacheIconWithData:data forHost:[OBString stringWithUTF8String:uurl.data()]]];
			}
		};

#if ENABLE(VIDEO)
		webPage->_fAttemptMedia = [self](void *player, const String &url) -> bool {
			validateObjCContext();
			WkWebViewPrivate *privateObject = [self privateObject];
			id<WkWebViewMediaDelegate> mediaDelegate = [privateObject mediaDelegate];
			if (mediaDelegate)
			{
				auto uurl = url.utf8();
				return [mediaDelegate webView:self shouldBePermittedToPreloadMediaWithURL:[OBURL URLWithString:[OBString stringWithUTF8String:uurl.data()]]];
			}
			return YES;
		};
	
		webPage->_fMediaAdded = [self](void *player, const String &url, WebCore::MediaPlayerMorphOSInfo &info,
			WTF::Function<void(bool doLoad)> &&loadFunc, WTF::Function<void()> &&yieldFunc) {
			validateObjCContext();
			WkWebViewPrivate *privateObject = [self privateObject];
			id<WkWebViewMediaDelegate> mediaDelegate = [privateObject mediaDelegate];
			if (mediaDelegate)
			{
				auto uurl = url.utf8();
				WkMediaLoadResponseHandlerPrivate *handler = [[WkMediaLoadResponseHandlerPrivate alloc] initWithPlayer:player
					url:[OBURL URLWithString:[OBString stringWithUTF8String:uurl.data()]] pageURL:[self URL] info:info callback:WTFMove(loadFunc)
					yieldCallback:WTFMove(yieldFunc)];
				if (handler)
				{
					[privateObject playerAdded:handler];
					[mediaDelegate webView:self wantsToLoadMediaWithURL:[handler mediaURL] withResponseHandler:handler];
					[handler release];
				}
			}
		};
		
		webPage->_fMediaRemoved = [self](void *player) {
			validateObjCContext();
			WkWebViewPrivate *privateObject = [self privateObject];
			id<WkWebViewMediaDelegate> mediaDelegate = [privateObject mediaDelegate];
			if (mediaDelegate)
			{
				WkMediaLoadResponseHandlerPrivate *handler = [privateObject handlerForPlayer:player];
				if (handler)
					[mediaDelegate webView:self cancelledMediaLoadWithResponseHandler:handler];
			}
			[privateObject playerRemoved:player];
		};
		
		webPage->_fMediaSetOverlayCallback = [self](void *player, WebCore::Element* element, WTF::Function<void(void *windowPtr,
			int scrollX, int scrollY, int left, int top, int right, int bottom, int width, int height)> && callback) {
			validateObjCContext();
			WkWebViewPrivate *privateObject = [self privateObject];
			[privateObject setOverlayCallback:player element:element callback:WTFMove(callback)];
		};
		
		webPage->_fMediaWillPlay = [self](void *player) {
			validateObjCContext();
			WkWebViewPrivate *privateObject = [self privateObject];
			[privateObject playerWillPlay:player];
		};
#endif
	}
	
	return self;
}

- (id)release
{
	int32_t count = ATOMIC_SUB((LONG*)&_useCount, 1);
	
	if (0 == count)
	{
		if (wkIsMainThread())
		{
			[self dealloc];
		}
		else
		{
			ATOMIC_ADD((LONG*)&_useCount, 1);
			WTF::callOnMainThread([self]() {
				[self release];
			});
		}
		return nil;
	}
	
	return self;
}

- (void)dealloc
{
	auto webPage = [_private page];
	if (webPage)
	{
		WebKit::WebProcess::singleton().removeWebPage(webPage->pageID());
		webPage->willBeDisposed();
	}

	[OBScheduledTimer scheduledTimerWithInterval:2.0 perform:[OBPerform performSelector:@selector(timedOut) target:_private] repeats:NO];
	[_private release];

	[super dealloc];
}

- (void)scrollToLeft:(int)left top:(int)top
{
	auto webPage = [_private page];
	[_private setIsHandlingUserInput:YES];
	webPage->setScroll(left, top);
	[_private setIsHandlingUserInput:NO];
}

- (void)setScrollingDelegate:(id<WkWebViewScrollingDelegate>)delegate
{
	[_private setScrollingDelegate:delegate];
}

- (void)setClientDelegate:(id<WkWebViewClientDelegate>)delegate
{
	[_private setClientDelegate:delegate];
}

- (void)load:(OBURL *)url
{
	OBString *scheme = [url scheme];
	
	if (0 == [scheme length] && url)
	{
		url = [OBURL URLWithString:[OBString stringWithFormat:@"http://%@", [url absoluteString]]];
	}

	const char *curi = [[url absoluteString] cString];
	if (nullptr == curi)
		curi = "about:blank";

	EP_EVENTSTR(curi);

	auto webPage = [_private page];
	[_private setURL:url];
	[[_private clientDelegate] webView:self changedTitle:[url absoluteString]];
	webPage->load(curi);
}

- (void)loadHTMLString:(OBString *)string baseURL:(OBURL *)base
{
	OBString *scheme = [base scheme];
	if (0 == [scheme length])
	{
		base = [OBURL URLWithString:[OBString stringWithFormat:@"file:///%@", [base absoluteString]]];
	}

	OBData *data = [string dataWithEncoding:MIBENUM_UTF_8];

	const char *curi = [[base absoluteString] cString];
	auto webPage = [_private page];
	webPage->loadData(reinterpret_cast<const char*>([data bytes]), [data length], curi);
}

- (void)loadRequest:(WkMutableNetworkRequest *)request
{
	(void)request;
	dprintf("%s: not implemented\n", __PRETTY_FUNCTION__);
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
	auto webPage = [_private page];
	return webPage->canGoBack();
}

- (BOOL)canGoForward
{
	auto webPage = [_private page];
	return webPage->canGoForward();
}

- (BOOL)goBack
{
	auto webPage = [_private page];
	return webPage->goBack();
	return NO;
}

- (BOOL)goForward
{
	auto webPage = [_private page];
	return webPage->goForward();
}

- (void)goToItem:(WkBackForwardListItem *)item
{
	auto webPage = [_private page];
	return webPage->goToItem([(WkBackForwardListItemPrivate *)item item]);
}

- (WkBackForwardList *)backForwardList
{
	return [_private backForwardList];
}

- (void)reload
{
	auto webPage = [_private page];
	webPage->reload();
}

- (void)stopLoading
{
	auto webPage = [_private page];
	webPage->stop();
}

- (OBString *)title
{
	return [_private title];
}

- (OBURL *)URL
{
	return [_private url];
}

- (OBURL *)hoveredURL
{
	return [_private hover];
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
	auto webPage = [_private page];
	[self retain];
	bool wasEnabled = webPage->javaScriptEnabled();
	if (!wasEnabled)
		webPage->setJavaScriptEnabled(true);
	webPage->run([javascript cString]);
	if (!wasEnabled)
		webPage->setJavaScriptEnabled(false);
	[self autorelease];
}

- (OBString *)evaluateJavaScript:(OBString *)javascript
{
	[self retain];
	auto webPage = [_private page];
	bool wasEnabled = webPage->javaScriptEnabled();
	if (!wasEnabled)
		webPage->setJavaScriptEnabled(true);
	OBString *out = (id)webPage->evaluate([javascript cString], [](const char *res) {
		return (void *)[OBString stringWithUTF8String:res];
	});
	if (!wasEnabled)
		webPage->setJavaScriptEnabled(false);
	[self autorelease];
	return out;
}

- (void)setEditable:(BOOL)editable
{
	auto webPage = [_private page];
	webPage->setEditable(editable);
}

- (BOOL)editable
{
	auto webPage = [_private page];
	return webPage->editable();
}

- (WkSettings_Interpolation)interpolation
{
	auto webPage = [_private page];
	switch (webPage->interpolationQuality())
	{
	case WebCore::InterpolationQuality::High:
		return WkSettings_Interpolation_High;
	case WebCore::InterpolationQuality::DoNotInterpolate:
	case WebCore::InterpolationQuality::Low:
		return WkSettings_Interpolation_Low;
	case WebCore::InterpolationQuality::Default:
	case WebCore::InterpolationQuality::Medium:
	default:
		return WkSettings_Interpolation_Medium;
	}
}

- (WkSettings_Interpolation)interpolationForImageViews
{
	auto webPage = [_private page];
	switch (webPage->interpolationQualityForImageViews())
	{
	case WebCore::InterpolationQuality::High:
		return WkSettings_Interpolation_High;
	case WebCore::InterpolationQuality::DoNotInterpolate:
	case WebCore::InterpolationQuality::Low:
		return WkSettings_Interpolation_Low;
	case WebCore::InterpolationQuality::Default:
	case WebCore::InterpolationQuality::Medium:
	default:
		return WkSettings_Interpolation_Medium;
	}
}

- (OBString *)resolveCSSFilePath
{
	// empty string always results in WebPage reverting to the built-in morphos.css!
	switch ([_private styleSheet])
	{
	case WkSettings_UserStyleSheet_MUI:
		return [self cSSFilePath];
	case WkSettings_UserStyleSheet_Custom:
		return [_private customStyleSheetPath];
	case WkSettings_UserStyleSheet_Builtin:
	default:
		return @"";
	}
}

- (WkSettings *)settings
{
	WkSettings *settings = [[WkSettings new] autorelease];
	auto webPage = [_private page];
	[settings setAdBlockerEnabled:webPage->adBlockingEnabled()];
	[settings setJavaScriptEnabled:webPage->javaScriptEnabled()];
	[settings setThirdPartyCookiesAllowed:webPage->thirdPartyCookiesAllowed()];
	[settings setThrottling:[_private throttling]];
	[settings setInterpolation:[self interpolation]];
	[settings setInterpolationForImageViews:[self interpolationForImageViews]];
	[settings setStyleSheet:[_private styleSheet]];
	[settings setCustomStyleSheetPath:[_private customStyleSheetPath]];
	[settings setContextMenuHandling:WkSettings_ContextMenuHandling(webPage->contextMenuHandling())];
	[settings setLocalStorageEnabled:webPage->localStorageEnabled()];
	[settings setOfflineWebApplicationCacheEnabled:webPage->offlineCacheEnabled()];
	[settings setInvisiblePlaybackNotAllowed:webPage->invisiblePlaybackNotAllowed()];
	[settings setRequiresUserGestureForMediaPlayback:webPage->requiresUserGestureForMediaPlayback()];
	return settings;
}

- (void)setSettings:(WkSettings *)settings
{
	auto webPage = [_private page];
	webPage->setJavaScriptEnabled([settings javaScriptEnabled]);
	webPage->setAdBlockingEnabled([settings adBlockerEnabled]);
	webPage->setThirdPartyCookiesAllowed([settings thirdPartyCookiesAllowed]);
	webPage->setContextMenuHandling(WebKit::WebPage::ContextMenuHandling([settings contextMenuHandling]));
	webPage->setLocalStorageEnabled([settings localStorageEnabled]);
	webPage->setOfflineCacheEnabled([settings offlineWebApplicationCacheEnabled]);
	webPage->setInvisiblePlaybackNotAllowed([settings invisiblePlaybackNotAllowed]);
	webPage->setRequiresUserGestureForMediaPlayback([settings requiresUserGestureForMediaPlayback]);

	[_private setThrottling:[settings throttling]];
	[_private setCustomStyleSheetPath:[settings customStyleSheetPath]];
	[_private setStyleSheet:[settings styleSheet]];

	switch ([settings throttling])
	{
	case WkSettings_Throttling_None:
		webPage->setLowPowerMode(false);
		break;
	case WkSettings_Throttling_InvisibleBrowsers:
		webPage->setLowPowerMode(!webPage->isVisible());
		break;
	case WkSettings_Throttling_All:
		webPage->setLowPowerMode(true);
		break;
	}
	
	switch ([settings interpolation])
	{
	case WkSettings_Interpolation_Low:
		webPage->setInterpolationQuality(WebCore::InterpolationQuality::DoNotInterpolate);
		break;

	case WkSettings_Interpolation_High:
		webPage->setInterpolationQuality(WebCore::InterpolationQuality::High);
		break;

	case WkSettings_Interpolation_Medium:
	default:
		webPage->setInterpolationQuality(WebCore::InterpolationQuality::Default);
		break;
	}

	switch ([settings interpolationForImageViews])
	{
	case WkSettings_Interpolation_Low:
		webPage->setInterpolationQualityForImageViews(WebCore::InterpolationQuality::DoNotInterpolate);
		break;

	case WkSettings_Interpolation_High:
		webPage->setInterpolationQualityForImageViews(WebCore::InterpolationQuality::High);
		break;

	case WkSettings_Interpolation_Medium:
	default:
		webPage->setInterpolationQualityForImageViews(WebCore::InterpolationQuality::Default);
		break;
	}
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

- (BOOL)setup
{
	OBString *cssPath = [self resolveCSSFilePath];
	auto webPage = [_private page];
	if (webPage)
	{
		if ([cssPath length])
			cssPath = [OBString stringWithFormat:@"file:///%@", cssPath];
		webPage->loadUserStyleSheet(WTF::String::fromUTF8([cssPath cString]));
	}
	return [super setup];
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

#define DIM2WIDTH(dim) ( ((LONG)dim) & 0xffff )
#define DIM2HEIGHT(dim)( ((LONG)dim) >> 16 )

- (BOOL)draw:(ULONG)flags
{
	EP_SCOPE(draw);
	UQUAD drawStartTS = __builtin_ppc_get_timebase();

	[super draw:flags];
	
	LONG iw = [self innerWidth];
	LONG ih = [self innerHeight];

	EP_BEGIN(setVisibleSize);
	auto webPage = [_private page];
	webPage->setVisibleSize(int(iw), int(ih));
	EP_END(setVisibleSize);
	
	WkPrintingStatePrivate *printingState = [_private printingState];
	if (printingState)
	{
		EP_SCOPE(printPreview);
		WkPrintingPage *page = [printingState pageWithMarginsApplied];

		float contentWidth = [page contentWidth] * 72.f;
		float contentHeight = [page contentHeight] * 72.f;

		webPage->printPreview([self rastPort], [self left], [self top], iw, ih, [printingState previevedSheet] - 1,
			[printingState pagesPerSheet], contentWidth, contentHeight, [printingState landscape],
			[printingState printMargins], [printingState context], [printingState shouldPrintBackgrounds]);

		OBString *info = [OBString stringWithFormat:@"%d/%d", [printingState previevedSheet], [printingState sheets]];
		ULONG dim = [self textDim:info len:-1 preparse:0 flags:0];
		[self text:[self left] + ((iw-DIM2WIDTH(dim))/2) top:[self bottom] - 2 - DIM2HEIGHT(dim) width:DIM2WIDTH(dim) height:DIM2HEIGHT(dim) text:info len:-1 preparse:0 flags:0];
	}
	else
	{
		EP_SCOPE(pageDraw);
		webPage->draw([self rastPort], [self left], [self top], iw, ih, MADF_DRAWUPDATE == (MADF_DRAWUPDATE & flags));
	}

	UQUAD drawEndTS = __builtin_ppc_get_timebase();
	[_private drawFinishedIn:(UQUAD)drawEndTS - drawStartTS];
	[_private setWindow:[self window]];

	return TRUE;
}

- (BOOL)show:(struct LongRect *)clip
{
	if ([super show:clip])
	{
		auto webPage = [_private page];

		// isVisible check workarounds an out-of-order initResize/hide in 3.15 :|
		if (![_private isLiveResizing] || !webPage->isVisible())
		{
			webPage->goVisible();
			
			if (WkSettings_Throttling_InvisibleBrowsers == [_private throttling])
				webPage->setLowPowerMode(false);
			
			[_private setPaintPerform:[OBPerform performSelector:@selector(lateDraw) target:self]];
			[_private setMLeft:[self left] mTop:[self top] mRight:[self right] mBottom:[self bottom]];
			
			if ([_private documentWidth])
			{
				if ([_private printingState])
				{
					[[_private scrollingDelegate] webView:self changedContentsSizeToWidth:1 height:1];
				}
				else
				{
					[[_private scrollingDelegate] webView:self changedContentsSizeToWidth:[_private documentWidth]
						height:[_private documentHeight]];
					[[_private scrollingDelegate] webView:self scrolledToLeft:[_private scrollX] top:[_private scrollY]];
				}
			}
		}
		else
		{
			[_private setMLeft:[self left] mTop:[self top] mRight:[self right] mBottom:[self bottom]];
		}

		return YES;
	}
	
	return NO;
}

- (void)hide
{
	if ([self window])
	{
		struct TagItem tags[] = { { WA_PointerType, (IPTR)0 }, { TAG_DONE, 0 } };
		SetWindowPointerA([self window], tags);
	}

	[super hide];

	if (![_private isLiveResizing])
	{
		auto webPage = [_private page];
		webPage->goHidden();

		[_private setPaintPerform:nil];
		[_private setWindow:nullptr];

		if (WkSettings_Throttling_InvisibleBrowsers == [_private throttling])
			webPage->setLowPowerMode(true);
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

	if (![_private printingState])
	{
		[_private setIsActive:true];
		[[self windowObject] setDisableKeys:(1<<MUIKEY_WINDOW_CLOSE)|(1<<MUIKEY_GADGET_NEXT)|(1<<MUIKEY_GADGET_PREV)];

		auto webPage = [_private page];
		webPage->goActive();
	}
}

- (void)becomeInactive
{
	[_private setIsActive:false];
	[[self windowObject] setDisableKeys:0];

	auto webPage = [_private page];
	webPage->goInactive();
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
	WkPrintingState *printingState = [_private printingState];
	auto webPage = [_private page];

	if (printingState)
	{
		if (imsg && imsg->Class == IDCMP_RAWKEY)
		{
			LONG previewedSheet = [printingState previevedSheet];
			if (imsg->Code == NM_WHEEL_UP)
			{
				if (previewedSheet > 1)
					[printingState setPrevievedSheet:previewedSheet - 1];
			}
			else if (imsg->Code == NM_WHEEL_DOWN)
			{
				if (previewedSheet < [printingState sheets])
					[printingState setPrevievedSheet:previewedSheet + 1];
			}
			else
			{
				return 0;
			}
			
			return MUI_EventHandlerRC_Eat;
		}
		return 0;
	}
	
	[_private setIsHandlingUserInput:YES];
	
	if (muikey != MUIKEY_NONE && webPage->handleMUIKey(int(muikey), [[self windowObject] defaultObject] == self))
	{
		[_private setIsHandlingUserInput:NO];
		return MUI_EventHandlerRC_Eat;
	}

	if (imsg)
	{
		if (webPage->handleIntuiMessage(imsg, [self mouseX:imsg], [self mouseY:imsg],
			[self isInObject:imsg], [[self windowObject] defaultObject] == self))
		{
			[_private setIsHandlingUserInput:NO];
			return MUI_EventHandlerRC_Eat;
		}
	}

	[_private setIsHandlingUserInput:NO];
	return 0;
}

- (void)lateDraw
{
	if ([_private drawPending])
	{
		[self redraw:MADF_DRAWUPDATE];
	}
}

- (void)lateRedraw
{
	[self redraw:MADF_DRAWOBJECT];
}

- (void)invalidated:(BOOL)force
{
	if (![_private drawPending] || force || [_private isHandlingUserInput])
	{
		[_private setDrawPendingWithSchedule:!force];

		if (force)
		{
			[[OBRunLoop mainRunLoop] performSelector:@selector(lateRedraw) target:self];
		}
	}
}

- (void)scrollToX:(int)sx y:(int)sy
{
	[_private setScrollX:sx y:sy];
	[[_private scrollingDelegate] webView:self scrolledToLeft:sx top:sy];

	// don't use draw scheduling - we want to have scrolling as fast as possible
	if (![_private drawPending] || [_private drawPendingWithSchedule])
	{
		[_private setDrawPendingWithSchedule:NO];
		[[OBRunLoop mainRunLoop] performSelector:@selector(lateDraw) target:self];
	}
}

- (void)setDocumentWidth:(int)width height:(int)height
{
	[_private setDocumentWidth:width height:height];
	if (![_private printingState])
	{
		[[_private scrollingDelegate] webView:self changedContentsSizeToWidth:width height:height];
	}
}

- (void)setBackForwardListDelegate:(id<WkWebViewBackForwardListDelegate>)delegate
{
	[_private setBackForwardDelegate:delegate];
}

- (void)setDebugConsoleDelegate:(id<WkWebViewDebugConsoleDelegate>)delegate
{
	[_private setConsoleDelegate:delegate];
}

- (void)setMediaDelegate:(id<WkWebViewMediaDelegate>)delegate
{
	[_private setMediaDelegate:delegate];
}

- (void)setAllRequestsHandlerDelegate:(id<WkWebViewAllRequestsHandlerDelegate>)delegate
{
	[_private setAllRequestsHandler:delegate];
}

- (void)setEditorDelegate:(id<WkWebViewEditorDelegate>)delegate
{
	[_private setEditorDelegate:delegate];
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

- (void)setProgressDelegate:(id<WkWebViewProgressDelegate>)delegate
{
	[_private setProgressDelegate:delegate];
}

- (void)setContextMenuDelegate:(id<WkWebViewContextMenuDelegate>)delegate
{
	[_private setContextMenuDelegate:delegate];
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
	if ([_private printingState])
	{
		[_private setSavedPageZoom:pageFactor textZoom:textFactor];
	}
	else
	{
		auto webPage = [_private page];
		webPage->setPageAndTextZoomFactors(pageFactor, textFactor);
		[_private callOverlayCallback];
	}
}

- (void)internalSetPageZoomFactor:(float)pageFactor textZoomFactor:(float)textFactor
{
	// used by WkPrinting!
	auto webPage = [_private page];
	webPage->setPageAndTextZoomFactors(pageFactor, textFactor);
}

- (int)pageWidth
{
	return [_private documentWidth];
}

- (int)pageHeight
{
	return [_private documentHeight];
}

- (int)visibleWidth
{
	auto webPage = [_private page];
	return webPage->size().width();
}

- (int)visibleHeight
{
	auto webPage = [_private page];
	return webPage->size().height();
}

- (void)primeLayoutForWidth:(int)width height:(int)height
{
	auto webPage = [_private page];
	webPage->setVisibleSize(width, height);
	WebKit::WebProcess::singleton().handleSignals(0); // needed or we'll paint all black
}

- (BOOL)screenShotRectAtX:(int)x y:(int)y intoRastPort:(struct RastPort *)rp withWidth:(ULONG)width height:(ULONG)height
{
	auto webPage = [_private page];
	return webPage->drawRect(x, y, width, height, rp);
}

- (BOOL)searchFor:(OBString *)string direction:(BOOL)forward caseSensitive:(BOOL)caseFlag wrap:(BOOL)wrapFlag startInSelection:(BOOL)startInSelection
{
    if (![string length])
        return NO;
	auto webPage = [_private page];
	WebCore::FindOptions options;

	if (!forward)
		options.add(WebCore::FindOptionFlag::Backwards);
	if (!caseFlag)
		options.add(WebCore::FindOptionFlag::CaseInsensitive);
	if (wrapFlag)
		options.add(WebCore::FindOptionFlag::WrapAround);
	if (startInSelection)
		options.add(WebCore::FindOptionFlag::StartInSelection);

	bool outWrapped = false;
	return webPage->search(WTF::String::fromUTF8([string cString]), options, outWrapped);
}

- (void)beganPrinting
{
	if ([_private printingState])
	{
		[self becomeInactive];
		[[_private scrollingDelegate] webView:self changedContentsSizeToWidth:1 height:1];
		[self redraw:MADF_DRAWUPDATE];
	}
}

- (WkPrintingState *)beginPrinting
{
	return [self beginPrintingWithSettings:nil];
}

- (WkPrintingState *)beginPrintingWithSettings:(OBDictionary *)settings
{
	if (nil == [_private printingState])
	{
		[_private beingPrintingWithWebView:self settings:settings];
		if ([_private printingState])
			[[OBRunLoop mainRunLoop] performSelector:@selector(beganPrinting) target:self];
	}

	return [_private printingState];
}

- (void)spoolToFile:(OBString *)file withDelegate:(id<WkPrintingStateDelegate>)delegate
{
	auto webPage = [_private page];
	WkPrintingStatePrivate *state = [_private printingState];
	WkPrintingPage *page = [state pageWithMarginsApplied];

	if (state && webPage && page)
	{
		if ([[state profile] isPDFFilePrinter])
		{
			webPage->pdfStart([page contentWidth] * 72.f, [page contentHeight] * 72.f, [state landscape],
				[state pagesPerSheet], [state printMargins], [state context], [state shouldPrintBackgrounds], [file nativeCString]);
		}
		else
		{
			if (0 == [file length])
			{
				OBMutableString *path = [OBMutableString stringWithCapacity:256];
				[path appendString:@"PRINTER:"];
				[path appendFormat:@"PROFILE=\"%@\" COPIES=%ld FORMAT=PS", [[state profile] name], [state copies]];
				if ([[state profile] canSelectPageFormat])
					[path appendFormat:@" SIZE=\"%@\"", [[[state profile] selectedPageFormat] key]];
				file = path;
			}
			webPage->printStart([page contentWidth] * 72.f, [page contentHeight] * 72.f, [state landscape],
				[state pagesPerSheet], [state printMargins], [state context], [[state profile] psLevel], [state shouldPrintBackgrounds], [file nativeCString]);
		}

		BOOL doOdd = [state parity] != WkPrintingState_Parity_EvenSheets;
		BOOL doEven = [state parity] != WkPrintingState_Parity_OddSheets;
		
		float progress = 0;
		[delegate printingState:state updatedProgress:progress];

		WkPrintingRange *range = [state printingRange];
		LONG printed = 0;
		for (LONG i = [range pageStart]; i <= [range pageEnd]; i++)
		{
			if (((i & 1) == 1) && doOdd)
				webPage->printSpool([state context], i - 1);
			else if (((i & 1) == 0) && doEven)
				webPage->printSpool([state context], i - 1);

			printed++;
			progress = float(printed) / float([range count]);
			[delegate printingState:state updatedProgress:progress];
		}
		
		webPage->printingFinished();
	}
}

- (BOOL)isPrinting
{
	return [_private printingState] != nil;
}

- (WkPrintingState *)printingState
{
	return [_private printingState];
}

- (void)endPrinting
{
	[_private endPrinting];
	[[_private scrollingDelegate] webView:self changedContentsSizeToWidth:[_private documentWidth]
		height:[_private documentHeight]];
	[[_private scrollingDelegate] webView:self scrolledToLeft:[_private scrollX] top:[_private scrollY]];
	[self redraw:MADF_DRAWOBJECT];
}

- (void)updatePrinting
{
	if ([_private printingState])
	{
		auto webPage = [_private page];
		webPage->printingFinished();
		[self redraw:MADF_DRAWOBJECT];
		[[_private scrollingDelegate] webView:self changedContentsSizeToShowPrintingSheets:[[_private printingState] sheets]];
		[[_private scrollingDelegate] webView:self scrolledToSheet:[[_private printingState] previevedSheet]];
	}
}

- (void)updatePrintPreviewSheet
{
	if ([_private printingState])
	{
		auto webPage = [_private page];
		webPage->printingFinished();
		[self redraw:MADF_DRAWOBJECT];
		[[_private scrollingDelegate] webView:self scrolledToSheet:[[_private printingState] previevedSheet]];
	}
}

- (void)recalculatePrinting
{
	[[_private printingState] recalculatePages];
}

- (BOOL)canUndo
{
	auto webPage = [_private page];
	if (webPage)
		return webPage->canUndo();
	return NO;
}

- (void)undo
{
	auto webPage = [_private page];
	if (webPage)
		webPage->undo();
}

- (BOOL)canRedo
{
	auto webPage = [_private page];
	if (webPage)
		return webPage->canRedo();
	return NO;
}

- (void)redo
{
	auto webPage = [_private page];
	if (webPage)
		webPage->redo();
}

@end
