#import <ob/OBString.h>

@class WkWebView;

@protocol WkWebViewVideoTrack <OBObject>

- (OBString *)codec;
- (int)width;
- (int)height;
- (int)bandwidth;

@end

@protocol WkWebViewAudioTrack <OBObject>

- (OBString *)codec;
- (int)frequency;
- (int)channels;
- (int)bits;

@end

@protocol WkWebViewMediaDelegate <OBObject>

typedef enum {
    WkWebViewMediaDelegateQuery_MediaPlayback = 1,
    WkWebViewMediaDelegateQuery_MediaSource = 2,
    WkWebViewMediaDelegateQuery_VP9 = 3,
    WkWebViewMediaDelegateQuery_HLS = 4,
    WkWebViewMediaDelegateQuery_HVC1 = 5,
} WkWebViewMediaDelegateQuery;

typedef enum {
	WkWebViewMediaDelegateMediaType_File,
	WkWebViewMediaDelegateMediaType_HLS,
	WkWebViewMediaDelegateMediaType_MediaSource,
} WkWebViewMediaDelegateMediaType;

typedef ULONG WkWebViewMediaDelegateStreamIdentifier;

- (BOOL)webView:(WkWebView *)view queriedForSupportOf:(WkWebViewMediaDelegateQuery)mode withSettingsValue:(BOOL)allow;
- (void)webView:(WkWebView *)view loadedStream:(WkWebViewMediaDelegateStreamIdentifier)identifier type:(WkWebViewMediaDelegateMediaType)type
	withAudioTrack:(id<WkWebViewAudioTrack>)audioTrack videoTrack:(id<WkWebViewVideoTrack>)videoTrack;
- (void)webView:(WkWebView *)view unloadedStream:(WkWebViewMediaDelegateStreamIdentifier)identifier;
- (void)webView:(WkWebView *)view playingStream:(WkWebViewMediaDelegateStreamIdentifier)identifier;
- (void)webView:(WkWebView *)view pausedStream:(WkWebViewMediaDelegateStreamIdentifier)identifier;

@end
