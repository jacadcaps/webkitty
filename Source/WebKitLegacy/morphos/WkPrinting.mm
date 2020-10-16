#import "WkPrinting_private.h"
#import <proto/dos.h>
#import <proto/obframework.h>
#import <proto/ppd.h>
#import <proto/exec.h>

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

	return [self defaultPageFormat];
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
	return [[[WkPrintingProfilePrivate alloc] initWithProfile:profile] autorelease];
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
