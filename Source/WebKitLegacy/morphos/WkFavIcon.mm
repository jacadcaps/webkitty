#undef __OBJC__
#import "WebKit.h"
#import <WebCore/SharedBuffer.h>
#import <WebCore/BitmapImage.h>
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

- (BOOL)loadSharedData:(WebCore::SharedBuffer *)data
{
#if 1
    auto image = WebCore::BitmapImage::create();
    if (image->setData(data, true) < WebCore::EncodedDataStatus::SizeAvailable)
        return NO;
	
	auto native = image->nativeImageForCurrentFrame();

	unsigned char *imgdata;
	int width, height, stride;

	cairo_surface_flush (native.get());

	imgdata = cairo_image_surface_get_data (native.get());
	width = cairo_image_surface_get_width (native.get());
	height = cairo_image_surface_get_height (native.get());
	stride = cairo_image_surface_get_stride (native.get());

	UBYTE *bytes = (UBYTE *)MediaAllocVec(width * height * 4);

	if (bytes == nullptr)
		return NO;

	for (int i = 0; i < height; i++)
	{
		memcpy(bytes + (i * width * 4), imgdata + (i * stride), width * 4);
	}

	@synchronized (self) {
		if (_dataPrescaled)
			MediaFreeVec(_dataPrescaled);
		_dataPrescaled = bytes;
		_widthPrescaled = width;
		_heightPrescaled = height;
		_useAlpha = YES; //!
	}

	return YES;
#else // no .icu files :/

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
		LONG alpha;
		
		w = MediaGetPort(image, 0, MMA_Video_Width);
		h = MediaGetPort(image, 0, MMA_Video_Height);
		alpha = MediaGetPort(image, 0, MMA_Video_UseAlpha);

		data = MediaAllocVec(w * h * 4);
		if (data)
		{
			if (DoMethod(image, MMM_Pull, 0, (IPTR)data, w * h * 4))
			{
				@synchronized (self) {
					if (_dataPrescaled)
						MediaFreeVec(_dataPrescaled);
					_dataPrescaled = (UBYTE *)data;
					_widthPrescaled = w;
					_heightPrescaled = h;
					_useAlpha = alpha;
					DisposeObject(image);
					return YES;
				}
			}

			MediaFreeVec(data);
		}
		
		DisposeObject(image);
	}
	return NO;
#endif
}

- (WkFavIconPrivate *)initWithSharedData:(WebCore::SharedBuffer *)data forHost:(OBString *)host
{
	if ((self = [super init]))
	{
		_host = WTF::String::fromUTF8([[host lowercaseString] cString]);
		
		if (![self loadSharedData:data])
		{
			[self release];
			return nil;
		}
	}
	
	return self;
}

- (void)askMinMax:(struct MUI_MinMax *)minmaxinfo
{
	minmaxinfo->MinWidth += 12;
	minmaxinfo->MinHeight += 12;
	
	minmaxinfo->DefWidth += 18;
	minmaxinfo->DefHeight += 18;

	minmaxinfo->MaxWidth += 64;
	minmaxinfo->MaxHeight += 64;
}

- (void)onThreadDone
{
	[self redraw:MADF_DRAWUPDATE];
}

- (void)thread:(OBNumber *)targetHeight
{

	if (nullptr == _dataPrescaled)
	{
		RefPtr<WebCore::SharedBuffer> buffer = WebCore::SharedBuffer::createWithContentsOfFile(WebKit::generateFileNameForIcon(_host));
		if (buffer.get() && buffer->size())
		{
			[self loadSharedData:buffer.get()];
		}
	}

	if (_dataPrescaled)
	{
		UBYTE *data;
		LONG height = [targetHeight longValue];
		
		int xwidth = _widthPrescaled, xheight = _heightPrescaled, xstride = _widthPrescaled * 4;

		float ratio = ((float)height) / ((float)xheight);
		LONG width = floor(((float)xwidth) * ratio);
		
		data = (UBYTE *)malloc(width * height * 4);

		if (data)
		{
			kieroscale32(_dataPrescaled, xwidth, xheight, width, height, xstride, width*4, data);
			MediaFreeVec(_dataPrescaled);
			_dataPrescaled = NULL;
			
			@synchronized (self) {
				if (_data)
					free(_data);
				_data = data;
				_width = width;
				_height = height;
			}
			
			[[OBRunLoop mainRunLoop] performSelector:@selector(onThreadDone) target:self];
		}
	}
}

- (void)onShowTimer
{
	[_loadResizeTimer invalidate];
	[_loadResizeTimer release];
	_loadResizeTimer = nil;
	
	[OBThread startWithObject:self selector:@selector(thread:) argument:[OBNumber numberWithLong:[self innerHeight]]];
}

- (BOOL)show:(struct LongRect *)clip
{
	if ([super show:clip])
	{
		if (_loadResizeTimer)
		{
			[_loadResizeTimer invalidate];
			[_loadResizeTimer release];
		}
		
		if ([self innerHeight] != _height || !_data)
		{
			_loadResizeTimer = [[OBScheduledTimer scheduledTimerWithInterval:1.0 perform:[OBPerform performSelector:@selector(onShowTimer) target:self] repeats:NO] retain];
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
	if (_loadResizeTimer)
	{
		[_loadResizeTimer invalidate];
		[_loadResizeTimer release];
	}
	[super dealloc];
}

- (BOOL)draw:(ULONG)flags
{
	[super draw:flags];

	@synchronized (self) {

		if (_data && nullptr == _dataPrescaled)
		{
			if (_useAlpha)
				WritePixelArrayAlpha(_data, 0, 0, _width * 4, [self rastPort],
					[self left] + (([self innerWidth] - _width) / 2), [self top], _width, _height, 0xFFFFFFFF);
			else
				WritePixelArray(_data, 0, 0, _width * 4, [self rastPort],
					[self left] + (([self innerWidth] - _width) / 2), [self top], _width, _height, RECTFMT_ARGB);
		}
	}

	return YES;
}

+ (WkFavIconPrivate *)cacheIconWithData:(WebCore::SharedBuffer *)data forHost:(OBString *)host
{
	if (data && data->size())
	{
		return [[[self alloc] initWithSharedData:data forHost:host] autorelease];
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
		return [[[WkFavIconPrivate alloc] initWithSharedData:buffer.get() forHost:host] autorelease];
	}
	
	return nil;
}

@end
