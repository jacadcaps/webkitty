#import <ob/OBString.h>
#import <ob/OBCopying.h>

@class OBURL, OBDictionary, OBMutableDictionary, OBData;

typedef enum {
   WkMutableNetworkRequestUseProtocolCachePolicy,
   WkMutableNetworkRequestReloadIgnoringCacheData,
   WkMutableNetworkRequestReturnCacheDataElseLoad,
   WkMutableNetworkRequestReturnCacheDataDontLoad
} WkMutableNetworkRequestCachePolicy;

// Wraps WebKits network requests and allows their customization
@interface WkMutableNetworkRequest : OBObject<OBCopying, OBMutableCopying>

+ (id)requestWithURL:(OBURL *)url;
+ (id)requestWithURL:(OBURL *)url cachePolicy:(WkMutableNetworkRequestCachePolicy)cachePolicy timeoutInterval:(float)timeout;

- (OBURL *)URL;
- (void)setURL:(OBURL *)value;

- (WkMutableNetworkRequestCachePolicy)cachePolicy;
- (void)setCachePolicy:(WkMutableNetworkRequestCachePolicy)cachePolicy;

- (float)timeoutInterval;
- (void)setTimeoutInterval:(float)interval;

- (OBString *)HTTPMethod;
- (void)setHTTPMethod:(OBString *)value;

- (OBData *)HTTPBody;
- (void)setHTTPBody:(OBData *)body;

- (OBDictionary *)allHTTPHeaderFields;
- (OBString *)valueForHTTPHeaderField:(OBString *)field;
- (void)setAllHTTPHeaderFields:(OBDictionary *)allValues;
- (void)setValue:(OBString *)value forHTTPHeaderField:(OBString *)field;

- (OBURL *)mainDocumentURL;
- (void)setMainDocumentURL:(OBURL *)url;

- (BOOL)HTTPShouldHandleCookies;
- (void)setHTTPShouldHandleCookies:(BOOL)handleCookies;

- (BOOL)allowsAnyHTTPSClientCertificate;
- (void)setAllowsAnyHTTPSClientCertificate:(BOOL)allowsany;

- (OBString *)clientCertificate;
- (void)setClientCertificate:(OBString *)certificate;

@end
