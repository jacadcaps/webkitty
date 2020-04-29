#pragma once
#include "WebKit.h"
#include <WebCore/PopupMenu.h>
#include <WebCore/PopupMenuClient.h>

namespace WebKit {

class WebPage;

class PopupMenuMorphOS : public WebCore::PopupMenu
{
public:
    PopupMenuMorphOS(WebCore::PopupMenuClient* client, WebPage* page);
    PopupMenuMorphOS();

    void show(const WebCore::IntRect&, WebCore::FrameView*, int index) override;
    void hide() override;
    void updateFromElement() override;
    void disconnectClient() override;

private:
	WebKit::WebPage*          m_page;
	WebCore::PopupMenuClient* m_client;
};

};
