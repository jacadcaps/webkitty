#import "WkHistory_private.h"
#import <ob/OBFramework.h>

@implementation WkBackForwardListItemPrivate

- (id)initWithURL:(OBURL *)url initialURL:(OBURL *)initialurl title:(OBString *)title
{
	if ((self = [super init]))
	{
		_url = [url copy];
		_initialURL = [initialurl copy];
		_title = [title copy];
	}
	
	return self;
}

- (void)dealloc
{
	[_url release];
	[_initialURL release];
	[_title release];
	[super dealloc];
}

- (OBURL *)URL
{
	return _url;
}

- (OBURL *)initialURL
{
	return _initialURL;
}

- (OBString *)title
{
	return _title;
}

@end

@implementation WkBackForwardListItem

+ (id)itemWithURL:(OBURL *)url initialURL:(OBURL *)initialURL title:(OBString *)title
{
	return [[[WkBackForwardListItemPrivate alloc] initWithURL:url initialURL:initialURL title:title] autorelease];
}

- (OBURL *)URL
{
	return nil;
}

- (OBURL *)initialURL
{
	return nil;
}

- (OBString *)title
{
	return nil;
}

- (BOOL)isEqual:(id)otherObject
{
	if ([otherObject isKindOfClass:[WkBackForwardListItem class]])
	{
		return [[self URL] isEqual:[otherObject URL]] && [[self initialURL] isEqual:[otherObject initialURL]] && [[self title] isEqual:[otherObject title]];
	}
	
	return NO;
}

@end

@implementation WkBackForwardListPrivate

- (id)initWithClient:(WTF::RefPtr<WebKit::BackForwardClientMorphOS>)bf
{
	if ((self = [super init]))
	{
		_client = bf;
	}
	
	return self;
}

+ (id)backForwardListPrivate:(WTF::RefPtr<WebKit::BackForwardClientMorphOS>)bf
{
	return [[[self alloc] initWithClient:bf] autorelease];
}

- (WkBackForwardListItem *)backItem
{
	return nil;
}

- (WkBackForwardListItem *)forwardItem
{
	return nil;
}

- (WkBackForwardListItem *)currentItem
{
	return nil;
}

- (OBArray *)backList
{
	return nil;
}

- (OBArray *)forwardList
{
	return nil;
}

@end

@implementation WkBackForwardList

- (WkBackForwardListItem *)backItem
{
	return 0;
}

- (WkBackForwardListItem *)forwardItem
{
	return 0;
}

- (WkBackForwardListItem *)currentItem
{
	return 0;
}

- (OBArray __wkListType *)backList
{
	return 0;
}

- (OBArray __wkListType *)forwardList
{
	return 0;
}

@end
