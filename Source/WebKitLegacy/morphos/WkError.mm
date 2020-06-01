#import "WkError_private.h"
#import "WkCertificate.h"
#import <ob/OBFramework.h>
#undef __OBJC__
#import <WebCore/ResourceError.h>
#import <WebCore/CertificateInfo.h>
#define __OBJC__

@interface WkErrorPrivate : WkError
{
	OBURL *_url;
	OBString *_domain;
	WkCertificateChain *_certificates;
	WkErrorType _type;
	int _code;
}
@end

@implementation WkErrorPrivate

- (id)initWithResourceError:(const WebCore::ResourceError &)error
{
	if ((self = [super init]))
	{
		_code = error.errorCode();

		auto udomain = error.domain().utf8();
		_domain = [[OBString stringWithUTF8String:udomain.data()] retain];

		auto uurl = error.failingURL().string().utf8();
		_url = [[OBURL URLWithString:[OBString stringWithUTF8String:uurl.data()]] retain];

		const auto& certInfo = error.certificateInfo();

		if (certInfo)
		{
			const auto& chain = certInfo->certificateChain();
			if (chain.size())
			{
				OBMutableArray *certArray = [OBMutableArray arrayWithCapacity:chain.size()];
				// NOTE: we want root > intermediate > client cert order
				for (int i = chain.size() - 1; i >= 0; i--)
				{
					const auto &cert = chain[i];
					[certArray addObject:[WkCertificate certificateWithData:(const char*)cert.data() length:cert.size()]];
				}

				_certificates = [[WkCertificateChain certificateChainWithCertificates:certArray] retain];
			}
		}
		
		if (error.isSSLCertVerificationError())
			_type = WkErrorType_SSLCertification;
		else if (error.isSSLConnectError())
			_type = WkErrorType_SSLConnection;
		else switch (error.type())
		{
		case WebCore::ResourceErrorBase::Type::Null: _type = WkErrorType_Null; break;
		case WebCore::ResourceErrorBase::Type::General: _type = WkErrorType_General; break;
		case WebCore::ResourceErrorBase::Type::AccessControl: _type = WkErrorType_AccessControl; break;
		case WebCore::ResourceErrorBase::Type::Cancellation: _type = WkErrorType_Cancellation; break;
		case WebCore::ResourceErrorBase::Type::Timeout: _type = WkErrorType_Timeout; break;
		}
	}

	return self;
}

- (void)dealloc
{
	[_url release];
	[_domain release];
	[_certificates release];
	[super dealloc];
}

- (OBString *)domain
{
	return _domain;
}

- (OBURL *)URL
{
	return _url;
}

- (int)errorCode
{
	return _code;
}

- (WkErrorType)type
{
	return _type;
}

- (WkCertificateChain *)certificates
{
	return _certificates;
}

@end

@implementation WkError

+ (WkError *)errorWithResourceError:(const WebCore::ResourceError &)error
{
	return [[[WkErrorPrivate alloc] initWithResourceError:error] autorelease];
}

- (OBString *)domain
{
	return nil;
}

- (OBURL *)URL
{
	return nil;
}

- (int)errorCode
{
	return -1;
}

- (WkErrorType)type
{
	return WkErrorType_Null;
}

- (WkCertificateChain *)certificates
{
	return nil;
}

@end
