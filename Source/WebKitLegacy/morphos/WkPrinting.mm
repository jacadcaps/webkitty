#import "WkPrinting_private.h"
#import <proto/dos.h>
#import <proto/obframework.h>
#import <proto/ppd.h>
#import <proto/exec.h>
#import <mui/MUIFramework.h>
#import "WkWebView.h"

extern "C" { void dprintf(const char *,...); }

@interface WkWebView ()
- (void)updatePrinting;
- (void)internalSetPageZoomFactor:(float)pageFactor textZoomFactor:(float)textFactor;
@end

template <class T> class ForListNodes {
public:
	ForListNodes(struct ::List *list) {
		_node = list ? reinterpret_cast<struct ::MinNode *>(list->lh_Head) : nullptr;
	}
	ForListNodes(struct ::MinList *list) {
		_node = list ? reinterpret_cast<struct ::MinNode *>(list->mlh_Head) : nullptr;
	}
	ForListNodes(struct ::List &list) {
		_node = reinterpret_cast<struct ::MinNode *>(list.lh_Head);
	}
	ForListNodes(struct ::MinList &list) {
		_node = reinterpret_cast<struct ::MinNode *>(list.mlh_Head);
	}
	T *nextNode() {
		if (_node && _node->mln_Succ) {
			auto t = reinterpret_cast<T *>(_node);
			_node = _node->mln_Succ;
			return t;
		}
		return nullptr;
	}
protected:
	struct ::MinNode *_node;
};

@implementation WkPrintingPagePrivate

- (id)initWithName:(OBString *)name key:(OBString *)key width:(float)width height:(float)height
	marginLeft:(float)mleft marginRight:(float)mright marginTop:(float)mtop marginBottom:(float)mbottom
{
	if ((self = [super init]))
	{
		_name = [name retain];
		_key = [key retain];
		_width = width;
		_height = height;
		_marginLeft = mleft;
		_marginRight = mright;
		_marginTop = mtop;
		_marginBottom = mbottom;
	}
	
	return self;
}

- (void)dealloc
{
	[_name release];
	[_key release];
	[super dealloc];
}

+ (WkPrintingPagePrivate *)pageWithName:(OBString *)name key:(OBString *)key width:(float)width height:(float)height
	marginLeft:(float)mleft marginRight:(float)mright marginTop:(float)mtop marginBottom:(float)mbottom
{
	return [[[self alloc] initWithName:name key:key width:width height:height marginLeft:mleft marginRight:mright marginTop:mtop marginBottom:mbottom] autorelease];
}

- (OBString *)name
{
	return _name;
}

- (OBString *)key
{
	return _key;
}

- (float)width
{
	return _width;
}

- (float)height
{
	return _height;
}

- (float)contentWidth
{
	return _width - (_marginLeft + _marginRight);
}

- (float)contentHeight
{
	return _height - (_marginTop + _marginBottom);
}

- (float)marginLeft
{
	return _marginLeft;
}

- (float)marginRight
{
	return _marginRight;
}

- (float)marginTop
{
	return _marginTop;
}

- (float)marginBottom
{
	return _marginBottom;
}

@end

@implementation WkPrintingProfilePrivate

- (id)initWithProfile:(OBString *)profile
{
	if ((self = [super init]))
	{
		_profile = [profile retain];

		struct Library *PPDBase = _ppdBase = OpenLibrary("ppd.library", 50);
		if (PPDBase)
		{
			PPD_ERROR err;
			const char *path = [[OBString stringWithFormat:@"SYS:Prefs/Printers/Profiles/%@", _profile] nativeCString];
			_ppd = OpenPPDFromIFF((STRPTR)path, &err);
		}
	}

	return self;
}

- (void)dealloc
{
	struct Library *PPDBase = _ppdBase;
	if (_ppd)
		ClosePPD(_ppd);
	if (_ppdBase)
		CloseLibrary(_ppdBase);
	[_profile release];
	[super dealloc];
}

- (WkPrintingPage *)pageForNode:(PAGE_SIZE_NODE *)node
{
	if (!node)
		return nil;

	float w = node->Width;
	float h = node->Height;
	float margins[4];
	w /= 72.f;
	h /= 72.f;
	margins[0] = node->Left_Margin;
	margins[1] = node->Right_Margin;
	margins[2] = node->Top_Margin;
	margins[3] = node->Bottom_Margin;
	margins[0] = margins[0] / 72.f;
	margins[1] = margins[1] / 72.f;
	margins[2] = margins[2] / 72.f;
	margins[3] = margins[3] / 72.f;
	margins[1] = w - margins[1];
	margins[2] = h - margins[2]; // wut da fuq?

	return [WkPrintingPagePrivate pageWithName:[OBString stringWithUTF8String:node->Full_Name] key:[OBString stringWithUTF8String:node->Name] width:w height:h
		marginLeft:margins[0] marginRight:margins[1] marginTop:margins[2] marginBottom:margins[3]];
}

- (OBArray*)pageFormats
{
	OBMutableArray *out = [OBMutableArray array];
	
	if (_ppd)
	{
		ForListNodes<PAGE_SIZE_NODE> fn(_ppd->PageSizes);
		PAGE_SIZE_NODE *node;

		while ((node = fn.nextNode()))
		{
			[out addObject:[self pageForNode:node]];
		}
	}
	else
	{
		[out addObject:[WkPrintingPagePrivate pageWithName:@"A4" key:@"A4" width:8.3f height:11.7f marginLeft:0.2 marginRight:0.2 marginTop:0.2 marginBottom:0.2]];
	}

	return out;
}

- (WkPrintingPage *)defaultPageFormat
{
	if (_ppd)
	{
		return [self pageForNode:_ppd->DefaultPageSize];
	}

	return nil;
}

- (WkPrintingPage *)selectedPageFormat
{
	if (_ppd)
	{
		WkPrintingPage *info = [self pageForNode:_ppd->SelectedPageSize];
		if (info)
			return info;
	}

	WkPrintingPage *def = [self defaultPageFormat];
	if (def)
		return def;
	
	return [[self pageFormats] firstObject];
}

- (OBString *)printerModel
{
	if (_ppd)
		return [OBString stringWithUTF8String:_ppd->Description.Model_Name];
	return nil;
}

- (OBString *)manufacturer
{
	if (_ppd)
		return [OBString stringWithUTF8String:_ppd->Description.Manufacturer];
	return nil;
}

- (LONG)psLevel
{
	if (_ppd)
		return _ppd->Parameters.PS_Level;
	return 3;
}

- (OBString *)name
{
	return _profile;
}

@end

@interface WkPrintingPDFProfile : WkPrintingProfilePrivate
{
	WkPrintingStatePrivate *_state; // WEAK!
	WkPrintingPage         *_page;
	OBArray                *_formats;
}
@end

@implementation WkPrintingPDFProfile

- (id)initWithState:(WkPrintingStatePrivate *)state
{
	if ((self = [super init]))
	{
		_state = state; // CAUTION
	}

	return self;
}

- (void)dealloc
{
	[_page release];
	[_formats release];
	[super dealloc];
}

- (void)clearState
{
	_state = nil;
}

- (BOOL)isPDFFilePrinter
{
	return YES;
}

- (OBString *)printerModel
{
	return @"PDF";
}

- (OBString *)manufacturer
{
	return @"MorphOS Team";
}

- (OBArray*)pageFormats
{
	if (!_formats)
	{
		OBMutableArray *out = [OBMutableArray array];
		[out addObject:[WkPrintingPagePrivate pageWithName:@"A4" key:@"A4" width:8.3f height:11.7f
			marginLeft:0.2 marginRight:0.2 marginTop:0.2 marginBottom:0.2]];
		[out addObject:[WkPrintingPagePrivate pageWithName:@"A3" key:@"A3" width:11.7f height:16.5f
			marginLeft:0.2 marginRight:0.2 marginTop:0.2 marginBottom:0.2]];
		[out addObject:[WkPrintingPagePrivate pageWithName:@"A2" key:@"A2" width:16.5f height:23.4f
			marginLeft:0.2 marginRight:0.2 marginTop:0.2 marginBottom:0.2]];
		[out addObject:[WkPrintingPagePrivate pageWithName:@"US Letter" key:@"USLetter" width:8.5f height:11.f
			marginLeft:0.2 marginRight:0.2 marginTop:0.2 marginBottom:0.2]];
		[out addObject:[WkPrintingPagePrivate pageWithName:@"US Legal" key:@"USLetter" width:8.5f height:14.f
			marginLeft:0.2 marginRight:0.2 marginTop:0.2 marginBottom:0.2]];
		_formats = [out retain];
	}
	
	return _formats;
}

- (BOOL)canSelectPageFormat
{
	return YES;
}

- (void)setSelectedPageFormat:(WkPrintingPage *)page
{
	if (page)
	{
		[_page autorelease];
		_page = [page retain];
		// force refresh on page change
		[_state setProfile:[_state profile]];
	}
}

- (WkPrintingPage *)selectedPageFormat
{
	if (_page)
		return _page;
	
	// force array generation
	return [[self pageFormats] firstObject];
}

- (OBString *)name
{
	return @"PDF";
}

@end

@implementation WkPrintingProfile

+ (OBArray /* OBString */ *)allProfiles
{
	BPTR lock = Lock("SYS:Prefs/Printers/Profiles", ACCESS_READ);
	OBMutableArray *out = nil;

	if (lock)
	{
		APTR buffer = OBAlloc(4096);
		struct ExAllControl *eac = (struct ExAllControl *)(buffer ? AllocDosObject(DOS_EXALLCONTROL, 0) : NULL);
		
		if (eac)
		{
			BOOL more;
			eac->eac_LastKey = 0;

			out = [OBMutableArray arrayWithCapacity:128];
			
			do
			{
				more = ExAll(lock, (struct ExAllData *)buffer, 4096, ED_TYPE, eac);
				if ((!more) && (IoErr() != ERROR_NO_MORE_ENTRIES)) {
					out = nil;
					break;
				}
				if (eac->eac_Entries == 0) {
					continue;
				}
				struct ExAllData *ead = (struct ExAllData *)buffer;
				do {
					if (ead->ed_Type < 0)
						[out addObject:[OBString stringWithCString:CONST_STRPTR(ead->ed_Name) encoding:MIBENUM_SYSTEM]];
					ead = ead->ed_Next;
				} while (ead);
			} while (more);
			
			FreeDosObject(DOS_EXALLCONTROL, eac);
		}
		
		if (buffer) OBFree(buffer);
		UnLock(lock);
	}
	
	return out;
}

+ (OBString *)defaultProfile
{
	OBString *defaultProfile = nil;
	char ppdPath[1024];

	if (-1 != GetVar("DefaultPrinter", ppdPath, sizeof(ppdPath), 0))
	{
		const char *name = PathPart(ppdPath);
		if (name)
			defaultProfile = [OBString stringWithCString:name encoding:MIBENUM_SYSTEM];
	}
	
	return defaultProfile;
}

+ (WkPrintingProfile *)spoolInfoForProfile:(OBString *)profile
{
	if (profile)
		return [[[WkPrintingProfilePrivate alloc] initWithProfile:profile] autorelease];
	return nil;
}

+ (WkPrintingProfile *)pdfProfileWithState:(WkPrintingStatePrivate *)state
{
	return [[[WkPrintingPDFProfile alloc] initWithState:state] autorelease];
}

- (BOOL)canSelectPageFormat
{
	return NO;
}

- (void)setSelectedPageFormat:(WkPrintingPage *)page
{
	(void)page;
}

- (OBArray /* WkPrintingPage */*)pageFormats
{
	return nil;
}

- (WkPrintingPage *)defaultPageFormat
{
	return nil;
}

- (WkPrintingPage *)selectedPageFormat
{
	return nil;
}

- (OBString *)printerModel
{
	return nil;
}

- (OBString *)manufacturer
{
	return nil;
}

- (LONG)psLevel
{
	return 2;
}

- (BOOL)isPDFFilePrinter
{
	return NO;
}

- (OBString *)name
{
	return @"?";
}

@end

@implementation WkPrintingPage

- (OBString *)name
{
	return @"";
}

- (OBString *)key
{
	return @"";
}

- (float)width
{
	return 1.0f;
}

- (float)height
{
	return 1.0f;
}

- (float)contentWidth
{
	return 1.0f;
}

- (float)contentHeight
{
	return 1.0f;
}

- (float)marginLeft
{
	return 1.0f;
}

- (float)marginRight
{
	return 1.0f;
}

- (float)marginTop
{
	return 1.0f;
}

- (float)marginBottom
{
	return 1.0f;
}

@end

@implementation WkPrintingStatePrivate

- (id)initWithWebView:(WkWebView *)view frame:(WebCore::Frame *)frame
{
	if ((self = [super init]))
	{
		_webView = view;
		_context = new WebCore::PrintContext(frame);
		OBArray *profileNames = [WkPrintingProfile allProfiles];
		_profiles = [[OBMutableArray arrayWithCapacity:[profileNames count] + 1] retain];
		_scale = 1.0f;

		OBString *defaultProfile = [WkPrintingProfile defaultProfile];
		OBEnumerator *eNames = [profileNames objectEnumerator];
		OBString *name;

		[_profiles addObject:[WkPrintingProfilePrivate pdfProfileWithState:self]];

		while ((name = [eNames nextObject]))
		{
			WkPrintingProfile *profile = [WkPrintingProfile spoolInfoForProfile:name];
			if (profile)
			{
				[_profiles addObject:profile];
				if ([name isEqualToString:defaultProfile])
					[self setProfile:_profile];
			}
		}

		if (!_profile)
			[self setProfile:[_profiles firstObject]];
	}

	return self;
}

- (void)dealloc
{
	delete _context;
	OBEnumerator *e = [_profiles objectEnumerator];
	WkPrintingPDFProfile *profile;
	while ((profile = [e nextObject]))
	{
		if ([profile isKindOfClass:[WkPrintingPDFProfile class]])
			[(id)profile clearState];
	}
	[_profile release];
	[_profiles release];
	[super dealloc];
}

- (WkWebView *)webView
{
	return _webView;
}

- (WkPrintingProfile *)profile
{
	return _profile;
}

- (OBArray * /* WkPrintingProfile */)allProfiles
{
	return _profiles;
}

- (void)setProfile:(WkPrintingProfile *)profile
{
	[_profile autorelease];
	_profile = [profile retain];

	WkPrintingPage *page = [_profile selectedPageFormat];

	if (_context && page)
	{
		_context->end();

		_defaultMargins = YES;
		
		float contentWidth = [page contentWidth];
		float contentHeight = [page contentHeight];
		
		_marginLeft = [page marginLeft];
		_marginTop = [page marginTop];
		_marginRight = [page marginRight];
		_marginBottom = [page marginBottom];

		if (_landscape)
		{
			contentHeight = [page contentWidth];
			contentWidth = [page contentHeight];

			_marginLeft = [page marginTop];
			_marginTop = [page marginLeft];
			_marginRight = [page marginBottom];
			_marginBottom = [page marginRight];
		}

		// works, so meh
		[_webView internalSetPageZoomFactor:1.0f textZoomFactor:_scale];

		// RectEdges(U&& top, U&& right, U&& bottom, U&& left)
		WebCore::FloatBoxExtent margins(_marginTop * 72.f, _marginRight * 72.f, _marginBottom * 72.f, _marginLeft * 72.f);
		auto computedPageSize = _context->computedPageSize(WebCore::FloatSize(contentWidth * 72.f, contentHeight * 72.f), margins);

		_context->begin(computedPageSize.width(), computedPageSize.height());

		float fullPageHeight;
		_context->computePageRects(WebCore::FloatRect(0, 0, computedPageSize.width(), computedPageSize.height()), 0, 0,
			1.0f, fullPageHeight, true);
	}
	
	[self needsRedraw];
}

- (void)invalidate
{
	if (_context)
		delete _context;
	_context = nullptr;
	_webView = nil;
}

- (void)needsRedraw
{
	[_webView updatePrinting];
}

- (LONG)pages
{
	if (_context)
		return _context->pageCount();
	return 1;
}

- (WebCore::PrintContext *)context
{
	return _context;
}

- (float)userScalingFactor
{
	return _scale;
}

- (void)setUserScalingFactor:(float)scaling
{
	if (_scale != scaling)
	{
		_scale = scaling;
		[_webView internalSetPageZoomFactor:1.0f textZoomFactor:_scale];
		[self setProfile:_profile];
	}
}

- (float)marginLeft
{
	return _marginLeft;
}

- (float)marginTop
{
	return _marginTop;
}

- (float)marginRight
{
	return _marginRight;
}

- (float)marginBottom
{
	return _marginBottom;
}

- (void)setMarginLeft:(float)left top:(float)top right:(float)right bottom:(float)bottom
{
	_defaultMargins = NO;
	_marginLeft = left;
	_marginTop = top;
	_marginBottom = bottom;
	_marginRight = right;
	[self setProfile:_profile];
}

- (void)resetMarginsToPaperDefaults
{
	_defaultMargins = YES;
	_marginLeft = [[_profile selectedPageFormat] marginLeft];
	_marginTop = [[_profile selectedPageFormat] marginTop];
	_marginRight = [[_profile selectedPageFormat] marginRight];
	_marginBottom = [[_profile selectedPageFormat] marginBottom];
	[self setProfile:_profile];
}

- (void)setLandscape:(BOOL)landscape
{
	_landscape = landscape;
	[self setProfile:_profile];
	[self needsRedraw];
}

- (BOOL)landscape
{
	return _landscape;
}

- (LONG)pagesPerSheet
{
	return _pagesPerSheet;
}

- (void)setPagesPerSheet:(LONG)pps
{
	_pagesPerSheet = pps;
}

- (LONG)previevedPage
{
	return _previewedPage;
}

- (void)setPrevievedPage:(LONG)page
{
	if (_previewedPage != page && (_context && page < LONG(_context->pageCount())))
	{
		_previewedPage = page;
		[self needsRedraw];
	}
}

@end

@implementation WkPrintingState

- (WkWebView *)webView
{
	return nil;
}

- (WkPrintingProfile *)profile
{
	return nil;
}

- (void)setProfile:(WkPrintingProfile *)profile
{
	(void)profile;
}

- (OBArray * /* WkPrintingProfile */)allProfiles
{
	return nil;
}

- (LONG)pages
{
	return 1;
}

- (LONG)previevedPage
{
	return 0;
}

- (void)setPrevievedPage:(LONG)page
{
	(void)page;
}

- (float)userScalingFactor
{
	return 1.0f;
}

- (void)setUserScalingFactor:(float)scaling
{
	(void)scaling;
}

- (float)marginLeft
{
	return 0.2f;
}

- (float)marginTop
{
	return 0.2f;
}

- (float)marginRight
{
	return 0.2f;
}

- (float)marginBottom
{
	return 0.2f;
}

- (void)setMarginLeft:(float)left top:(float)top right:(float)right bottom:(float)bottom
{
	(void)left;
	(void)right;
	(void)top;
	(void)bottom;
}

- (void)resetMarginsToPaperDefaults
{

}

- (LONG)pagesPerSheet
{
	return 1;
}

- (void)setPagesPerSheet:(LONG)pps
{
	(void)pps;
}

- (void)setLandscape:(BOOL)landscape
{
	(void)landscape;
}

- (BOOL)landscape
{
	return NO;
}

@end

@interface WkPrintingRangePrivate : WkPrintingRange
{
	LONG _start;
	LONG _end;
}
@end

@implementation WkPrintingRangePrivate

- (id)initWithStart:(LONG)start end:(LONG)end
{
	if ((self = [super init]))
	{
		_start = start;
		_end = end;
	}
	return self;
}

- (LONG)pageStart
{
	return _start;
}

- (LONG)pageEnd
{
	return _end;
}

@end

@implementation WkPrintingRange

+ (WkPrintingRange *)rangeWithPage:(LONG)pageNo
{
	return [[[WkPrintingRangePrivate alloc] initWithStart:pageNo end:pageNo] autorelease];
}

+ (WkPrintingRange *)rangeFromPage:(LONG)pageStart toPage:(LONG)pageEnd
{
	return [[[WkPrintingRangePrivate alloc] initWithStart:pageStart end:pageEnd] autorelease];
}

+ (WkPrintingRange *)rangeFromPage:(LONG)pageStart count:(LONG)count
{
	return [[[WkPrintingRangePrivate alloc] initWithStart:pageStart end:pageStart + count - 1] autorelease];
}

- (LONG)pageStart
{
	return 1;
}

- (LONG)pageEnd
{
	return 1;
}

- (LONG)count
{
	return 1 + ([self pageEnd] - [self pageStart]);
}

@end

