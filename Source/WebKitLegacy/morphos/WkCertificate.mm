#import <ob/OBFramework.h>
#import <proto/obframework.h>
#import "WkCertificate.h"

#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/x509_vfy.h>
#include <openssl/bio.h>
#include <openssl/pem.h>

extern "C" { void dprintf(const char *,...); }

#define SHALEN 32

static X509_STORE     *_store;

@interface _WkCertificate : WkCertificate
{
	OBData       *_data;
	BOOL          _decoded;
	BOOL          _valid;
	BOOL          _selfSigned;
	OBDictionary *_subjectName;
	OBDictionary *_issuerName;
	OBData       *_sha;
	OBString     *_serial;
	OBString     *_algorithm;
	OBString     *_validNotBefore;
	OBString     *_validNotAfter;
	int           _version;
}
@end

@interface _WkCertificateChain : WkCertificateChain
{
	OBArray *_certificates;
}
@end

@implementation _WkCertificate

- (id)initWithData:(OBData *)data
{
	if ((self = [super init]))
	{
		_data = [data copy];
	}

	return self;
}

- (void)dealloc
{
	[_data release];
	[_subjectName release];
	[_issuerName release];
	[_sha release];
	[_serial release];
	[_algorithm release];
	[_validNotBefore release];
	[_validNotAfter release];
	[super dealloc];
}

- (OBData *)certificate
{
	return _data;
}

+ (void)initialize
{
	_store = X509_STORE_new();
	if (_store)
	{
		if (1 != X509_STORE_load_locations(_store, "MOSSYS:Data/SSL/curl-ca-bundle.crt", nullptr))
		{
			X509_STORE_free(_store);
			_store = nullptr;
		}
	}
}

+ (void)shutdown
{
	if (_store)
		X509_STORE_free(_store);
}

- (OBDictionary *)_dnToDictionary:(const char *)dnString
{
	if (dnString)
	{
		OBMutableDictionary *out = [OBMutableDictionary dictionaryWithCapacity:16];
		OBArray *pairs = [[OBString stringWithUTF8String:dnString + 1] componentsSeparatedByString:@"/"];
		OBEnumerator *e = [pairs objectEnumerator];
		OBString *pair;
		while ((pair = [e nextObject]))
		{
			OBArray *kv = [pair componentsSeparatedByString:@"="];
			if ([kv count] == 2)
				[out setObject:[kv lastObject] forKey:[kv firstObject]];
		}

		return out;
	}

	return nil;
}

- (OBString *)_serialFromCert:(X509 *)cert
{
	ASN1_INTEGER *serial = X509_get_serialNumber(cert);
	BIGNUM *bn = ASN1_INTEGER_to_BN(serial, NULL);
	if (!bn)
		return nil;

	char *tmp = BN_bn2dec(bn);
	if (tmp)
	{
		OBString *out = [OBString stringWithUTF8String:tmp];
		BN_free(bn);
		OPENSSL_free(tmp);
		return out;
	}

	BN_free(bn);
	return nil;
}

- (OBString *)_convertASN1TIME:(ASN1_TIME *)t
{
	int rc;
	char date[128];
	BIO *b = BIO_new(BIO_s_mem());

	if (b)
	{
		rc = ASN1_TIME_print(b, t);
		if (rc <= 0)
		{
			BIO_free(b);
			return nil;
		}
		
		rc = BIO_gets(b, date, 128);
		BIO_free(b);

		if (rc > 0)
		{
			return [OBString stringWithUTF8String:date];
		}
	}
	
	return nil;
}

- (X509 *)_cert
{
	if (_data)
	{
		const unsigned char *bytes = (const unsigned char *)[_data bytes];
		BIO* certBio = BIO_new(BIO_s_mem());
		if (certBio)
		{
			BIO_write(certBio, bytes, int([_data length]));
			X509* cert = PEM_read_bio_X509(certBio, NULL, NULL, NULL);
			BIO_free(certBio);
			return cert;
		}
	}
	
	return nullptr;
}

- (void)decode
{
	if (_data && !_decoded)
	{
		X509* cert = [self _cert];

#if 0
	dprintf("%s: %d ceert %p len %d bytes %p %c%c%c%c\n", __PRETTY_FUNCTION__, __LINE__, cert, [_data length], bytes, bytes[0], bytes[1], bytes[2], bytes[3]);
#endif

		_decoded = YES;

		if (cert)
		{
			char nameBuffer[4096];

			if (X509_NAME_oneline(X509_get_subject_name(cert), nameBuffer, sizeof(nameBuffer)))
			{
				_subjectName = [[self _dnToDictionary:nameBuffer] retain];
			}

			if (X509_NAME_oneline(X509_get_issuer_name(cert), nameBuffer, sizeof(nameBuffer)))
			{
				_issuerName = [[self _dnToDictionary:nameBuffer] retain];
			}

			char *sha = static_cast<char *>(OBAlloc(SHALEN));
			if (sha)
			{
				const EVP_MD *digest = EVP_sha256();
				if (X509_digest(cert, digest, (unsigned char*) sha, NULL))
				{
					_sha = [[OBData dataWithBytesNoCopy:sha length:SHALEN freeWhenDone:YES] retain];
				}
				else
				{
					OBFree(sha);
				}
			}

			_version = ((int) X509_get_version(cert)) + 1;
			_serial = [[self _serialFromCert:cert] retain];

			int pkey_nid = X509_get_signature_nid(cert);
			if (pkey_nid != NID_undef)
			{
				const char* nidname = OBJ_nid2ln(pkey_nid);
				if (nidname && *nidname)
					_algorithm = [[OBString stringWithUTF8String:nidname] retain];
			}
			
			_validNotAfter = [[self _convertASN1TIME:X509_get_notAfter(cert)] retain];
			_validNotBefore = [[self _convertASN1TIME:X509_get_notBefore(cert)] retain];

			_valid = X509_check_ca(cert) > 0;
			_selfSigned = X509_check_issued(cert, cert) == X509_V_OK;

			X509_free(cert);
			
#if 0
			dprintf("cert %p valid %d self %d\nsn %s\nin %s\n%s>%s\n", self, _valid, _selfSigned, [[_issuerName description] cString], [[_subjectName description] cString],
				[_validNotBefore cString], [_validNotAfter cString]);
#endif
		}
	}
}

- (BOOL)isRoot
{
	[self decode];
	return _valid && _selfSigned;
}

- (BOOL)isValid
{
	[self decode];
	return _valid;
}

- (void)setValid:(BOOL)valid
{
	_valid = valid;
}

- (BOOL)isSelfSigned
{
	[self decode];
	return _selfSigned;
}

- (OBDictionary *)subjectName
{
	[self decode];
	return _subjectName;
}

- (OBDictionary *)issuerName
{
	[self decode];
	return _issuerName;
}

- (int)version
{
	[self decode];
	return _version;
}

- (OBString *)serialNumber
{
	[self decode];
	return _serial;
}

- (OBData *)sha256
{
	[self decode];
	return _sha;
}

static inline void hex_encode(const unsigned char* readbuf, void *writebuf, size_t len)
{
	for(size_t i=0; i < len; i++) {
		char *l = (char*) (2*i + ((intptr_t) writebuf));
		sprintf(l, "%02x", readbuf[i]);
	}
}

- (OBString *)sha256Hex
{
	OBData *sha = [self sha256];
	if ([sha length] == SHALEN)
	{
		char buffer[SHALEN * 2];
		hex_encode(static_cast<const unsigned char*>([sha bytes]), buffer, SHALEN);
		return [OBString stringWithCString:buffer length:(SHALEN * 2) encoding:MIBENUM_UTF_8];
	}
	
	return nil;
}

- (OBString *)algorithm
{
	[self decode];
	return _algorithm;
}

- (OBString *)notValidBefore
{
	[self decode];
	return _validNotBefore;
}

- (OBString *)notValidAfter
{
	[self decode];
	return _validNotAfter;
}

@end

@implementation WkCertificate

+ (WkCertificate *)certificateWithData:(OBData *)data
{
	return [[[_WkCertificate alloc] initWithData:data] autorelease];
}

+ (WkCertificate *)certificateWithData:(const char *)data length:(ULONG)length
{
	return [[[_WkCertificate alloc] initWithData:[OBData dataWithBytes:data length:length]] autorelease];
}

+ (void)shutdown
{
	[_WkCertificate shutdown];
}

- (OBData *)certificate
{
	return nil;
}

- (BOOL)isRoot
{
	return NO;
}

- (BOOL)isValid
{
	return YES;
}

- (void)setValid:(BOOL)valid
{
	
}

- (BOOL)isSelfSigned
{
	return YES;
}

- (OBDictionary *)subjectName
{
	return nil;
}

- (OBDictionary *)issuerName
{
	return nil;
}

- (int)version
{
	return 0;
}

- (OBString *)serialNumber
{
	return nil;
}

- (OBData *)sha256
{
	return nil;
}

- (OBString *)sha256Hex
{
	return nil;
}

- (OBString *)algorithm
{
	return nil;
}

- (OBString *)notValidBefore
{
	return nil;
}

- (OBString *)notValidAfter
{
	return nil;
}

@end

@implementation _WkCertificateChain

- (id)initWithCertificates:(OBArray *)certs
{
	if ((self = [super init]))
	{
		if ([certs count])
			_certificates = [certs retain];
	}
	
	return self;
}

- (void)dealloc
{
	[_certificates release];
	[super dealloc];
}

- (OBArray *)certificates
{
	return _certificates;
}

- (BOOL)verify:(OBError **)outerror
{
	ULONG chainCount = [_certificates count];

	if (chainCount > 1)
	{
		STACK_OF(X509)* x509_ca_stack = sk_X509_new_null();
		if (x509_ca_stack)
		{
			X509 *xcert;
			BOOL ok = YES;

			for (ULONG i = 0; i < [_certificates count]; i++)
			{
				_WkCertificate *wkcert = [_certificates objectAtIndex:i];
				xcert = [wkcert _cert];

				if (!xcert)
				{
					// failed
					ok = NO;
					if (outerror)
						*outerror = [OBError errorWithDomain:@"SSLVerification" code:X509_V_ERR_UNSPECIFIED userInfo:nil];
					break;
				}

				if (i < [_certificates count] - 1)
				{
					sk_X509_push(x509_ca_stack, xcert);
				}
			}

			if (ok)
			{
				X509_STORE_CTX *ctx = X509_STORE_CTX_new();
				if (ctx)
				{
					if (1 == X509_STORE_CTX_init(ctx, _store, xcert, x509_ca_stack))
					{
						int rc = X509_verify_cert(ctx);
						ok = rc == 1;
						if (!ok && outerror)
							*outerror = [OBError errorWithDomain:@"SSLVerification" code:X509_STORE_CTX_get_error(ctx) userInfo:nil];
						if (ok)
						{
							for (ULONG i = 0; i < [_certificates count]; i++)
							{
								_WkCertificate *wkcert = [_certificates objectAtIndex:i];
								[wkcert setValid:YES];
							}
						}
					}
					else
					{
						ok = NO;
						if (outerror)
							*outerror = [OBError errorWithDomain:@"SSLVerification" code:X509_V_ERR_OUT_OF_MEM userInfo:nil];
					}

					X509_STORE_CTX_free(ctx);
				}
				else
				{
					ok = NO;
					if (outerror)
						*outerror = [OBError errorWithDomain:@"SSLVerification" code:X509_V_ERR_OUT_OF_MEM userInfo:nil];
				}
			}
			
			if (xcert)
				X509_free(xcert);
			sk_X509_pop_free(x509_ca_stack, X509_free);
			return ok;
		}
	}
	
	return NO;
}

@end

@implementation WkCertificateChain : OBObject

+ (WkCertificateChain *)certificateChainWithCertificates:(OBArray *)certificates
{
	return [[[_WkCertificateChain alloc] initWithCertificates:certificates] autorelease];
}

+ (WkCertificateChain *)certificateChainWithCertificate:(WkCertificate *)cert
{
	return [[[_WkCertificateChain alloc] initWithCertificates:[OBArray arrayWithObject:cert]] autorelease];
}

- (OBArray *)certificates
{
	return nil;
}

- (BOOL)verify:(OBError **)outerror
{
	if (outerror)
		*outerror = nil;
	return NO;
}

+ (OBString *)validationErrorStringForErrorCode:(int)e
{
	switch ((int) e)
	{
		case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
			return @"ERR_UNABLE_TO_GET_ISSUER_CERT";
		case X509_V_ERR_UNABLE_TO_GET_CRL:
			return @"ERR_UNABLE_TO_GET_CRL";
		case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
			return @"ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE";
		case X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
			return @"ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE";
		case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
			return @"ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY";
		case X509_V_ERR_CERT_SIGNATURE_FAILURE:
			return @"ERR_CERT_SIGNATURE_FAILURE";
		case X509_V_ERR_CRL_SIGNATURE_FAILURE:
			return @"ERR_CRL_SIGNATURE_FAILURE";
		case X509_V_ERR_CERT_NOT_YET_VALID:
			return @"ERR_CERT_NOT_YET_VALID";
		case X509_V_ERR_CERT_HAS_EXPIRED:
			return @"ERR_CERT_HAS_EXPIRED";
		case X509_V_ERR_CRL_NOT_YET_VALID:
			return @"ERR_CRL_NOT_YET_VALID";
		case X509_V_ERR_CRL_HAS_EXPIRED:
			return @"ERR_CRL_HAS_EXPIRED";
		case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
			return @"ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD";
		case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
			return @"ERR_ERROR_IN_CERT_NOT_AFTER_FIELD";
		case X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD:
			return @"ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD";
		case X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD:
			return @"ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD";
		case X509_V_ERR_OUT_OF_MEM:
			return @"ERR_OUT_OF_MEM";
		case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
			return @"ERR_DEPTH_ZERO_SELF_SIGNED_CERT";
		case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
			return @"ERR_SELF_SIGNED_CERT_IN_CHAIN";
		case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
			return @"ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY";
		case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
			return @"ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE";
		case X509_V_ERR_CERT_CHAIN_TOO_LONG:
			return @"ERR_CERT_CHAIN_TOO_LONG";
		case X509_V_ERR_CERT_REVOKED:
			return @"ERR_CERT_REVOKED";
		case X509_V_ERR_INVALID_CA:
			return @"ERR_INVALID_CA";
		case X509_V_ERR_PATH_LENGTH_EXCEEDED:
			return @"ERR_PATH_LENGTH_EXCEEDED";
		case X509_V_ERR_INVALID_PURPOSE:
			return @"ERR_INVALID_PURPOSE";
		case X509_V_ERR_CERT_UNTRUSTED:
			return @"ERR_CERT_UNTRUSTED";
		case X509_V_ERR_CERT_REJECTED:
			return @"ERR_CERT_REJECTED";
		case X509_V_ERR_SUBJECT_ISSUER_MISMATCH:
			return @"ERR_SUBJECT_ISSUER_MISMATCH";
		case X509_V_ERR_AKID_SKID_MISMATCH:
			return @"ERR_AKID_SKID_MISMATCH";
		case X509_V_ERR_AKID_ISSUER_SERIAL_MISMATCH:
			return @"ERR_AKID_ISSUER_SERIAL_MISMATCH";
		case X509_V_ERR_KEYUSAGE_NO_CERTSIGN:
			return @"ERR_KEYUSAGE_NO_CERTSIGN";
		case X509_V_ERR_INVALID_EXTENSION:
			return @"ERR_INVALID_EXTENSION";
		case X509_V_ERR_INVALID_POLICY_EXTENSION:
			return @"ERR_INVALID_POLICY_EXTENSION";
		case X509_V_ERR_NO_EXPLICIT_POLICY:
			return @"ERR_NO_EXPLICIT_POLICY";
		case X509_V_ERR_APPLICATION_VERIFICATION:
			return @"ERR_APPLICATION_VERIFICATION";
		case X509_V_ERR_INVALID_CALL:
			return @"ERR_INVALID_CALL";
		default:
			return @"ERR_UNKNOWN";
	}
}

@end
