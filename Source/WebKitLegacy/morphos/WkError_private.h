#import "WkError.h"

namespace WebCore {
	class ResourceError;
}

@interface WkError (Private)

+ (WkError *)errorWithResourceError:(const WebCore::ResourceError &)error;

@end
