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

- (id)init
{
	if ((self = [super init]))
	{
		[self setDragDropSort:NO];
		[self setFrame:MUIV_Frame_ReadList];
		[self setCycleChain:YES];
	}
	
	return self;
}

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
	[self onCertificateSelected:certificate];
}

- (id)initWithCertificateChain:(WkCertificateChain *)chain
{
	MUIGroup *left;
	if ((self = [super initWithObjects:
		_tree = [[_WkListTree new] autorelease],
		[MUIGroup horizontalGroupWithObjects:
			left = [MUIGroup groupWithObjects:
				[MUIDtpic dtpicWithFileName:@"PROGDIR:Resources/certificate.png"],
				nil],
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
			nil],
		nil]))
	{
		_certificateChain = [chain retain];
		
		[left setHorizWeight:0];
		
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

- (BOOL)setup
{
	if ([super setup])
	{
		[_tree setValue:MUIV_List_Active_Bottom forAttribute:MUIA_List_Active]; // hah
		return YES;
	}
	return NO;
}

- (WkCertificateChain *)certificateChain
{
	return _certificateChain;
}

- (void)onCertificateSelected:(WkCertificate *)certificate
{
	if (certificate)
	{
		[_name setContents:[[certificate subjectName] objectForKey:@"CN"]];
		[_issuedBy setContents:[[certificate issuerName] objectForKey:@"CN"]];
		[_expires setContents:[certificate notValidAfter]];
		[_valid setContents:[certificate isValid] ? OBL(@"This vertificate is valid.", @"Certificate validity msg") : OBL(@"This vertificate is NOT valid.", @"Certificate validity msg")];
	}
}

@end
