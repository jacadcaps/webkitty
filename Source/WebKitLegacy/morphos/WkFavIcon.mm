#undef __OBJC__
#import "WebKit.h"
#import <WebCore/SharedBuffer.h>
#define __OBJC__
#import "WkFavIcon_private.h"
#import <proto/graphics.h>
#import <proto/intuition.h>
#import <graphics/gfx.h>
#import <cybergraphx/cybergraphics.h>
#import <proto/cybergraphics.h>
#import <cairo.h>

#include <proto/multimedia.h>
#include <classes/multimedia/multimedia.h>
#include <classes/multimedia/video.h>

static inline void getbilinearpixel32(unsigned char *data,int rowbytes,int px,int py ,unsigned char *dest)
{
	int px0,py0;
	unsigned int dx,dy;
	unsigned int dx1,dy1;

	unsigned int w;

	unsigned int p1, p2;
	unsigned int pp1,pp2;
	unsigned int ww=(1<<7);
	px0 = px >> 16;                         /* round down */
	py0 = py >> 16;
	dx = px & 0x0000ffff;					/* get the fraction */
	dy = py & 0x0000ffff;

	dx1 = 65535 - dx;
	dy1 = 65535 - dy;

	data = data + py0 * rowbytes + px0 * 4;

	/* apply weights */

	w = dx1 *dy1 >> 25;
	p1 = *(unsigned int*)data;
	p2 = p1 & 0x00ff00ff;
	p1 = (p1 & 0xff00ff00)>>8;
	pp1 = p1 * w;
	pp2 = p2 * w;
	ww-=w;

	data += 4;
	w = dx * dy1 >> 25;
	p1 = *(unsigned int*)data;
	p2 = p1 & 0x00ff00ff;
	p1 = (p1 & 0xff00ff00)>>8;
	pp1 += p1 * w;
	pp2 += p2 * w;
	ww-=w;

	data += rowbytes - 4;
	w = dx1 *dy >> 25;
	p1 = *(unsigned int*)data;
	p2 = p1 & 0x00ff00ff;
	p1 = (p1 & 0xff00ff00)>>8;
	pp1 += p1 * w;
	pp2 += p2 * w;
	ww-=w;

	data += 4;
	w = ww;
	p1 = *(unsigned int*)data;
	p2 = p1 & 0x00ff00ff;
	p1 = (p1 & 0xff00ff00)>>8;
	pp1 += p1 * w;
	pp2 += p2 * w;

	pp1 = (pp1 << 1) & 0xff00ff00;
	pp2 = (pp2 >> 7) & 0x00ff00ff;

	*(unsigned int*)dest = pp1 | pp2;
}

static void kieroscale32(UBYTE *src, int srcwidth, int srcheight, int destwidth, int destheight, int rowbytes, int destrowbytes, UBYTE *buffermem)
{
	int py = 0;
	int ystep = (srcheight - 1) * 65536 / destheight;
	int cx,cy;

	for (cy = 0; cy < destheight; cy++)
	{
		int xstep = (srcwidth - 1) * 65536 / destwidth;
		int px = 0;
		UBYTE *dst = buffermem + (cy * destrowbytes);

		for (cx = 0; cx < destwidth; cx++)
		{
			getbilinearpixel32(src, rowbytes, px, py, dst);
			dst += 4;
			px += xstep;
		}
		py += ystep;
	}
}

namespace WebKit {
	String generateFileNameForIcon(const WTF::String &inHost);
}

@implementation WkFavIconPrivate

- (WkFavIconPrivate *)initWithSharedData:(WebCore::SharedBuffer *)data
{
	if ((self = [super init]))
	{
		UQUAD size = data->size();
		struct TagItem imageTags[] = {
			{ MMA_StreamType, (IPTR)"memory.stream" },
			{ MMA_StreamHandle, (IPTR)data->data() },
			{ MMA_StreamLength, (IPTR)&size },
			{ MMA_MediaType, MMT_PICTURE },
			{ MMA_Video_UseAlpha, TRUE },
			{ TAG_DONE, 0 }
		};

		Boopsiobject *image = MediaNewObjectTagList(imageTags);

		if (image)
		{
			APTR data;
			LONG w, h;
			
			w = MediaGetPort(image, 0, MMA_Video_Width);
			h = MediaGetPort(image, 0, MMA_Video_Height);
			_useAlpha = MediaGetPort(image, 0, MMA_Video_UseAlpha);

			data = MediaAllocVec(w * h * 4);
			if (data)
			{
				if (DoMethod(image, MMM_Pull, 0, (IPTR)data, w * h * 4))
				{
					_dataPrescaled = (UBYTE *)data;
					_width = w;
					_height = h;
				}
			}
			
			DisposeObject(image);
		}
		
		if (NULL == _dataPrescaled)
		{
			[self release];
			return nil;
		}
	}
	
	return self;
}

- (void)askMinMax:(struct MUI_MinMax *)minmaxinfo
{
	if (_data)
	{
		minmaxinfo->MinWidth += _width;
		minmaxinfo->MinHeight += _height;

		minmaxinfo->DefWidth += _width;
		minmaxinfo->DefHeight += _height;

		minmaxinfo->MaxWidth += 32;
		minmaxinfo->MaxHeight += _height;
	}
	else
	{
		minmaxinfo->DefWidth += 18;
		minmaxinfo->DefHeight += 18;
		
		minmaxinfo->MaxWidth = 32;
		minmaxinfo->MaxHeight = 32;
	}
}

- (BOOL)show:(struct LongRect *)clip
{
	if ([super show:clip])
	{
		// The size is known at this time... so it's time to load/decode the imagery
		if (_dataPrescaled)
		{
			LONG height = [self innerHeight];
			
			int xwidth = _width, xheight = _height, xstride = _width * 4;

			float ratio = ((float)height) / ((float)xheight);
			LONG width = floor(((float)xwidth) * ratio);
			
			_data = (UBYTE *)malloc(width * height * 4);

			if (_data)
			{
				kieroscale32(_dataPrescaled, xwidth, xheight, width, height, xstride, width*4, _data);
				MediaFreeVec(_dataPrescaled);
				_dataPrescaled = NULL;
			}
			
			_width = width;
			_height = height;
		}
		return YES;
	}
	
	return NO;
}

- (void)dealloc
{
	if (_data)
		free(_data);
	if (_dataPrescaled)
		MediaFreeVec(_dataPrescaled);
	[super dealloc];
}

- (BOOL)draw:(ULONG)flags
{
	[super draw:flags];

	if (_data)
	{
		if (_useAlpha)
			WritePixelArrayAlpha(_data, 0, 0, _width * 4, [self rastPort],
				[self left] + (([self innerWidth] - _width) / 2), [self top], _width, _height, 0xFFFFFFFF);
		else
			WritePixelArray(_data, 0, 0, _width * 4, [self rastPort],
				[self left] + (([self innerWidth] - _width) / 2), [self top], _width, _height, RECTFMT_ARGB);
	}
	return YES;
}

+ (WkFavIconPrivate *)cacheIconWithData:(WebCore::SharedBuffer *)data
{
	if (data && data->size())
	{
		return [[[self alloc] initWithSharedData:data] autorelease];
	}
	
	return nil;
}

@end

@implementation WkFavIcon

+ (WkFavIcon *)favIconForHost:(OBString *)host
{
	RefPtr<WebCore::SharedBuffer> buffer = WebCore::SharedBuffer::createWithContentsOfFile(WebKit::generateFileNameForIcon(WTF::String::fromUTF8([host cString])));
	if (buffer.get() && buffer->size())
	{
		return [[[WkFavIconPrivate alloc] initWithSharedData:buffer.get()] autorelease];
	}
	
	return nil;
}

@end
