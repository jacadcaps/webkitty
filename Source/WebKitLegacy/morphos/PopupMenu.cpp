#include "PopupMenu.h"

using namespace WebCore;
using namespace WTF;

namespace WebKit {

PopupMenuMorphOS::PopupMenuMorphOS(WebCore::PopupMenuClient* client, WebPage* page)
	: m_page(page)
	, m_client(client)
{

}

PopupMenuMorphOS::PopupMenuMorphOS()
{

}

void PopupMenuMorphOS::show(const IntRect&, FrameView*, int index)
{

}

void PopupMenuMorphOS::hide()
{

}

void PopupMenuMorphOS::updateFromElement()
{

}

void PopupMenuMorphOS::disconnectClient()
{
	m_client = nullptr;
}

}

