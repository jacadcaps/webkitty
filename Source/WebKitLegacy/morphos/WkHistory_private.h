#undef __OBJC__
#import "WebKit.h"
#import "BackForwardList.h"
#define __OBJC__

#import "WkHistory.h"
#import <ob/OBArrayMutable.h>

@interface WkBackForwardListItemPrivate : WkBackForwardListItem
{
	OBString *_title;
	OBURL    *_url;
	OBURL    *_initialURL;
	WTF::RefPtr<WebCore::HistoryItem> _item;
}
- (WebCore::HistoryItem &)item;
@end

@interface WkBackForwardListPrivate : WkBackForwardList
{
	WTF::RefPtr<WebKit::BackForwardList> _client;
}

+ (id)backForwardListPrivate:(WTF::RefPtr<WebKit::BackForwardList>)bf;
- (WTF::RefPtr<WebKit::BackForwardList>)client;

@end
