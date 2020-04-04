#pragma once
#include "WebKit.h"
#include <WebCore/BackForwardClient.h>

namespace WebKit {

class WebPage;

class BackForwardClientMorphOS : public WebCore::BackForwardClient
{
public:
	BackForwardClientMorphOS(WebPage *view);

    static Ref<BackForwardClientMorphOS> create(WebPage *view)
    {
        return WTF::adoptRef(*new BackForwardClientMorphOS(view));
    }
	
	void addItem(Ref<WebCore::HistoryItem>&& item) final;

    void goToItem(WebCore::HistoryItem& item) final;
	
    RefPtr<WebCore::HistoryItem> itemAtIndex(int) final;
    unsigned backListCount() const final;
    unsigned forwardListCount() const final;

    void close() final;
};

}

