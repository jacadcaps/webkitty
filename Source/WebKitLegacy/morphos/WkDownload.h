#import <ob/OBURL.h>

@class WkMutableNetworkRequest, WkDownload, WkError;

@protocol WkDownloadDelegate <OBObject>

- (void)downloadDidBegin:(WkDownload *)download;

// Size should be known at the time of this callback, provided the server gave us one
- (void)didReceiveResponse:(WkDownload *)download;

// Respond with a string or nil to cancel download
- (OBString *)decideFilenameForDownload:(WkDownload *)download withSuggestedName:(OBString *)suggestedName;

- (void)download:(WkDownload *)download didReceiveBytes:(size_t)bytes;

- (void)downloadDidFinish:(WkDownload *)download;
- (void)download:(WkDownload *)download didFailWithError:(WkError *)error;
- (void)downloadNeedsAuthenticationCredentials:(WkDownload *)download;

@end

@interface WkDownload : OBObject

+ (WkDownload *)download:(OBURL *)url withDelegate:(id<WkDownloadDelegate>)delegate;
+ (WkDownload *)downloadRequest:(WkMutableNetworkRequest *)request withDelegate:(id<WkDownloadDelegate>)delegate;

- (void)start;
- (void)cancel;
- (void)cancelForResume;

- (BOOL)canResumeDownload;
- (void)resume;

- (OBURL *)url;
- (OBString *)filename;

- (void)setLogin:(OBString *)login password:(OBString *)password;

- (QUAD)size;
- (QUAD)downloadedSize;

- (BOOL)isPending;
- (BOOL)isFailed;
- (BOOL)isFinished;

@end
