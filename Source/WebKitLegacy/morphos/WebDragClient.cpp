#include "WebDragClient.h"
#include "WebPage.h"

extern "C" { void dprintf(const char *,...); }

namespace WebKit {
using namespace WebCore;

void WebDragClient::willPerformDragDestinationAction(DragDestinationAction action, const DragData&)
{
//	dprintf("%s\n", __PRETTY_FUNCTION__);
//    if (action == DragDestinationActionLoad)
//        m_page->willPerformLoadDragDestinationAction();
//    else
//        m_page->mayPerformUploadDragDestinationAction(); // Upload can happen from a drop event handler, so we should prepare early.
}

void WebDragClient::willPerformDragSourceAction(DragSourceAction, const IntPoint&, DataTransfer&)
{
//	dprintf("%s\n", __PRETTY_FUNCTION__);
}

DragSourceAction WebDragClient::dragSourceActionMaskForPoint(const IntPoint&)
{
//	dprintf("%s\n", __PRETTY_FUNCTION__);
    return WebCore::DragSourceActionAny; //m_page->allowedDragSourceActions();
}

void WebDragClient::startDrag(DragItem, DataTransfer&, Frame&)
{
//	dprintf("%s\n", __PRETTY_FUNCTION__);
}

void WebDragClient::didConcludeEditDrag()
{
}

} // namespace WebKit
