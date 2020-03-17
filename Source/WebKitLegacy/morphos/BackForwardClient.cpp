#include "BackForwardClient.h"
#include <WebCore/HistoryItem.h>

BackForwardClientMorphOS::BackForwardClientMorphOS(WebView *view)
{

}

void BackForwardClientMorphOS::addItem(Ref<WebCore::HistoryItem>&& item)
{

}

void BackForwardClientMorphOS::goToItem(WebCore::HistoryItem& item)
{

}

RefPtr<WebCore::HistoryItem> BackForwardClientMorphOS::itemAtIndex(int index)
{
	return nullptr;
}

unsigned BackForwardClientMorphOS::backListCount() const
{
	return 0;
}

unsigned BackForwardClientMorphOS::forwardListCount() const
{
	return 0;
}

void BackForwardClientMorphOS::close()
{

}
