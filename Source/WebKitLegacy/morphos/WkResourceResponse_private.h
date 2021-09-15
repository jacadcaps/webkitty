#import "WkResourceResponse.h"

namespace WebCore { class ResourceResponse; }

@interface WkResourceResponsePrivate : WkResourceResponse
{
	OBURL        *_url;
	OBString     *_textEncodingName;
	OBString     *_mimeType;
	OBString     *_httpStatusText;
	OBDictionary *_httpHeaders;
	ULONG         _httpStatusCode;
	QUAD          _expectedContentLength;
}

+ (WkResourceResponsePrivate *)responseWithResponse:(const WebCore::ResourceResponse &)response;

@end
