#import "WkFavIcon.h"

namespace WebCore {
	class SharedBuffer;
};

struct BitMap;
@class OBString;

@interface WkFavIconPrivate : WkFavIcon
{
	UBYTE  *_data;
	UBYTE  *_dataPrescaled;
	LONG    _width;
	LONG    _height;
	BOOL    _useAlpha;
}

+ (WkFavIconPrivate *)cacheIconWithData:(WebCore::SharedBuffer *)data;

@end
