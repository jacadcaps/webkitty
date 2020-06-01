#include "WkCertificateViewer.h"
#include <mui/MUIFramework.h>

extern "C" { void dprintf(const char *,...); }

@interface _WkCertificateListEntry : OBObject<MUIListtreeEntry>
{
	WkCertificate *_certificate;
	BOOL           _branch;
}
@end

@implementation _WkCertificateListEntry

- (id)initWithCertificate:(WkCertificate *)cert asBranch:(BOOL)branch
{
	if ((self = [super init]))
	{
		_certificate = [cert retain];
		_branch = branch;
	}
	
	return self;
}

- (void)dealloc
{
	[_certificate release];
	[super dealloc];
}

+ (_WkCertificateListEntry *)entryWithCertificate:(WkCertificate *)cert asBranch:(BOOL)branch
{
	return [[[self alloc] initWithCertificate:cert asBranch:branch] autorelease];
}

- (OBArray *)listtreeDisplay
{
	return [OBArray arrayWithObject:[[_certificate subjectName] objectForKey:@"CN"]];
}

- (BOOL)listtreeBranch
{
	return _branch;
}

- (WkCertificate *)certificate
{
	return _certificate;
}

@end

@interface _WkListTree : MCCListtree
@end

@implementation _WkListTree

- (Boopsiobject *)instantiateTagList:(struct TagItem *)tags
{
	struct TagItem atags[] = { MUIA_List_AdjustHeight, TRUE, TAG_MORE, (IPTR)tags };
	return [super instantiateTagList:atags];
}

@end

@implementation WkCertificateVerifier

- (void)certificateSelected
{
	WkCertificate *certificate = [((_WkCertificateListEntry *)[_tree active]) certificate];
	if (certificate)
	{
		[_name setContents:[[certificate subjectName] objectForKey:@"CN"]];
		[_issuedBy setContents:[[certificate issuerName] objectForKey:@"CN"]];
		[_expires setContents:[certificate notValidAfter]];
		[_valid setContents:[certificate isValid] ? OBL(@"This vertificate is valid.", @"Certificate validity msg") : OBL(@"This vertificate is NOT valid.", @"Certificate validity msg")];
	}
}

- (id)initWithCertificateChain:(WkCertificateChain *)chain
{
	if ((self = [super initWithObjects:
		_tree = [[_WkListTree new] autorelease],
		[MUIGroup groupWithColumns:2 objects:
			[MUILabel label:OBL(@"Name:", @"Certificate Name")],
			_name = [MUIText textWithContents:nil],
			[MUILabel label:OBL(@"Issued by:", @"Certificate Name")],
			_issuedBy = [MUIText textWithContents:nil],
			[MUILabel label:OBL(@"Expires:", @"Certificate Name")],
			_expires = [MUIText textWithContents:nil],
			[MUIRectangle rectangleWithWeight:0],
			_valid = [MUIText textWithContents:nil],
			nil],
		nil]))
	{
		_certificateChain = [chain retain];
		
		OBArray *certificates = [chain certificates];
		ULONG certs = [certificates count];
		_WkCertificateListEntry *last = nil;
		for (ULONG i = 0; i < certs; i++)
		{
			WkCertificate *certificate = [certificates objectAtIndex:i];
			_WkCertificateListEntry *next;
			if (last)
				[_tree insert:next = [_WkCertificateListEntry entryWithCertificate:certificate asBranch:i + 1 < certs] intoList:last flags:kMCCListtreeInsertFlag_Open];
			else
				[_tree insert:next = [_WkCertificateListEntry entryWithCertificate:certificate asBranch:i + 1 < certs] flags:kMCCListtreeInsertFlag_Open];
			last = next;
		}
		
		[_tree notify:@selector(active) performSelector:@selector(certificateSelected) withTarget:self];
		[_tree setActive:last];
	}
	
	return self;
}

- (void)dealloc
{
	[_certificateChain release];
	[super dealloc];
}

+ (WkCertificateVerifier *)verifierForCertificateChain:(WkCertificateChain *)chain
{
	return [[[self alloc] initWithCertificateChain:chain] autorelease];
}

- (WkCertificateChain *)certificateChain
{
	return _certificateChain;
}

@end
