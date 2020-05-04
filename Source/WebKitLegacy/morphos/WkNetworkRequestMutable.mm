#import "WkNetworkRequestMutable.h"
#import <ob/OBFramework.h>

@interface WkMutableNetworkRequestPrivate : WkMutableNetworkRequest
{
	OBURL *_url;
	OBURL *_mainDocumentURL;
	WkMutableNetworkRequestCachePolicy _cachePolicy;
	float _timeout;
	OBString *_httpMethod;
	OBString *_clientCertificate;
	OBData *_httpBody;
	OBMutableDictionary *_headers;
	BOOL _shouldHandleCoookies;
	BOOL _allowsAnyClientCertificate;
}
@end

@implementation WkMutableNetworkRequestPrivate

- (id)initWithURL:(OBURL *)url
{
	return [self initWithURL:url cachePolicy:WkMutableNetworkRequestUseProtocolCachePolicy timeoutInterval:60.0];
}

- (id)initWithURL:(OBURL *)url cachePolicy:(WkMutableNetworkRequestCachePolicy)cachePolicy timeoutInterval:(float)timeout
{
	if ((self = [super init]))
	{
		_url = [url copy];
		_cachePolicy = cachePolicy;
		_timeout = timeout;
		_httpMethod = @"GET";
	}
	
	return self;
}

- (void)dealloc
{
	[_url release];
	[_mainDocumentURL release];
	[_httpMethod release];
	[_clientCertificate release];
	[_httpBody release];
	[_headers release];
	[super dealloc];
}

- (id)mutableCopy
{
	WkMutableNetworkRequest *copy = [[WkMutableNetworkRequestPrivate alloc] initWithURL:_url cachePolicy:_cachePolicy timeoutInterval:_timeout];

	if (copy)
	{
		[copy setHTTPMethod:_httpMethod];
		[copy setHTTPBody:_httpBody];
		[copy setAllHTTPHeaderFields:_headers];
		[copy setMainDocumentURL:_mainDocumentURL];
		[copy setHTTPShouldHandleCookies:_shouldHandleCoookies];
		[copy setAllowsAnyHTTPSClientCertificate:_allowsAnyClientCertificate];
		[copy setClientCertificate:_clientCertificate];
	}
	
	return copy;
}

- (OBURL *)URL
{
	return _url;
}

- (void)setURL:(OBURL *)value
{
	[_url autorelease];
	_url = [value copy];
}

- (WkMutableNetworkRequestCachePolicy)cachePolicy
{
	return _cachePolicy;
}

- (void)setCachePolicy:(WkMutableNetworkRequestCachePolicy)cachePolicy
{
	_cachePolicy = cachePolicy;
}

- (float)timeoutInterval
{
	return _timeout;
}

- (void)setTimeoutInterval:(float)interval
{
	_timeout = interval;
}

- (OBString *)HTTPMethod
{
	return _httpMethod;
}

- (void)setHTTPMethod:(OBString *)value
{
	[_httpMethod autorelease];
	_httpMethod = [value copy];
}

- (OBData *)HTTPBody
{
	return _httpBody;
}

- (void)setHTTPBody:(OBData *)body
{
	[_httpBody autorelease];
	_httpBody = [body copy];
}

- (OBDictionary *)allHTTPHeaderFields
{
	return _headers;
}

- (OBString *)valueForHTTPHeaderField:(OBString *)field
{
	return [_headers objectForKey:field];
}

- (void)setAllHTTPHeaderFields:(OBDictionary *)allValues
{
	[_headers autorelease];
	_headers = [allValues copy];
}

- (void)setValue:(OBString *)value forHTTPHeaderField:(OBString *)field
{
	if (nil == _headers)
		_headers = [OBMutableDictionary new];
	[_headers setObject:value forKey:field];
}

- (OBURL *)mainDocumentURL
{
	return _mainDocumentURL;
}

- (void)setMainDocumentURL:(OBURL *)url
{
	[_mainDocumentURL autorelease];
	_mainDocumentURL = [url copy];
}

- (BOOL)HTTPShouldHandleCookies
{
	return _shouldHandleCoookies;
}

- (void)setHTTPShouldHandleCookies:(BOOL)handleCookies
{
	_shouldHandleCoookies = handleCookies;
}

- (BOOL)allowsAnyHTTPSClientCertificate
{
	return _allowsAnyClientCertificate;
}

- (void)setAllowsAnyHTTPSClientCertificate:(BOOL)allowsany
{
	_allowsAnyClientCertificate = allowsany;
}

- (OBString *)clientCertificate
{
	return _clientCertificate;
}

- (void)setClientCertificate:(OBString *)certificate
{
	[_clientCertificate autorelease];
	_clientCertificate = [certificate copy];
}

@end

@implementation WkMutableNetworkRequest

+ (id)requestWithURL:(OBURL *)url
{
	return [[[WkMutableNetworkRequestPrivate alloc] initWithURL:url] autorelease];
}

+ (id)requestWithURL:(OBURL *)url cachePolicy:(WkMutableNetworkRequestCachePolicy)cachePolicy timeoutInterval:(float)timeout
{
	return [[[WkMutableNetworkRequestPrivate alloc] initWithURL:url cachePolicy:cachePolicy timeoutInterval:timeout] autorelease];
}

- (id)copy
{
	return [self retain];
}

- (id)mutableCopy
{
	return nil;
}

- (OBURL *)URL
{
	return nil;
}

- (void)setURL:(OBURL *)value
{
}

- (WkMutableNetworkRequestCachePolicy)cachePolicy
{
	return WkMutableNetworkRequestCachePolicy::WkMutableNetworkRequestUseProtocolCachePolicy;
}

- (void)setCachePolicy:(WkMutableNetworkRequestCachePolicy)cachePolicy
{
}

- (float)timeoutInterval
{
	return 60.0f;
}

- (void)setTimeoutInterval:(float)interval
{
}

- (OBString *)HTTPMethod
{
	return nil;
}

- (void)setHTTPMethod:(OBString *)value
{
}

- (OBData *)HTTPBody
{
	return nil;
}

- (void)setHTTPBody:(OBData *)body
{
}

- (OBDictionary *)allHTTPHeaderFields
{
	return nil;
}

- (OBString *)valueForHTTPHeaderField:(OBString *)field
{
	return nil;
}

- (void)setAllHTTPHeaderFields:(OBDictionary *)allValues
{
}

- (void)setValue:(OBString *)value forHTTPHeaderField:(OBString *)field
{
}

- (OBURL *)mainDocumentURL
{
	return nil;
}

- (void)setMainDocumentURL:(OBURL *)url
{
}

- (BOOL)HTTPShouldHandleCookies
{
	return NO;
}

- (void)setHTTPShouldHandleCookies:(BOOL)handleCookies
{
}

- (BOOL)allowsAnyHTTPSClientCertificate
{
	return NO;
}

- (void)setAllowsAnyHTTPSClientCertificate:(BOOL)allowsany
{
}

- (OBString *)clientCertificate
{
	return nil;
}

- (void)setClientCertificate:(OBString *)certificate
{
}

@end
