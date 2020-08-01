#include "config.h"
#include "Cursor.h"
#include <intuition/pointerclass.h>

namespace WebCore {

void Cursor::ensurePlatformCursor() const
{
    if (m_platformCursor || m_type == Cursor::Pointer)
        return;

    switch (m_type) {
    case Cursor::Pointer:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_NORMAL);
        break;
    case Cursor::Cross:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_AIMING);
        break;
    case Cursor::Hand:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_NORMAL);
        break;
    case Cursor::IBeam:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_SELECTTEXT);
        break;
    case Cursor::Wait:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_BUSY);
        break;
    case Cursor::Help:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_HELP);
        break;
    case Cursor::Move:
    case Cursor::MiddlePanning:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_MOVE);
        break;
    case Cursor::EastResize:
    case Cursor::EastPanning:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_HORIZONTALRESIZE);
        break;
    case Cursor::NorthResize:
    case Cursor::NorthPanning:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_VERTICALRESIZE);
        break;
    case Cursor::NorthEastResize:
    case Cursor::NorthEastPanning:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_DIAGONALRESIZE1);
        break;
    case Cursor::NorthWestResize:
    case Cursor::NorthWestPanning:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_DIAGONALRESIZE2);
        break;
    case Cursor::SouthResize:
    case Cursor::SouthPanning:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_VERTICALRESIZE);
        break;
    case Cursor::SouthEastResize:
    case Cursor::SouthEastPanning:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_DIAGONALRESIZE1);
        break;
    case Cursor::SouthWestResize:
    case Cursor::SouthWestPanning:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_DIAGONALRESIZE2);
        break;
    case Cursor::WestResize:
    case Cursor::WestPanning:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_HORIZONTALRESIZE);
        break;
    case Cursor::NorthSouthResize:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_VERTICALRESIZE);
        break;
    case Cursor::EastWestResize:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_HORIZONTALRESIZE);
        break;
    case Cursor::NorthEastSouthWestResize:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_DIAGONALRESIZE1);
        break;
    case Cursor::NorthWestSouthEastResize:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_DIAGONALRESIZE1);
        break;
    case Cursor::ColumnResize:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_HORIZONTALRESIZE);
        break;
    case Cursor::RowResize:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_HORIZONTALRESIZE);
        break;
    case Cursor::VerticalText:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_SELECTTEXT);
        break;
    case Cursor::Cell:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_SELECTTEXT);
        break;
    case Cursor::ContextMenu:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_NORMAL);
        break;
    case Cursor::Alias:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_NORMAL);
        break;
    case Cursor::Progress:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_WORKING);
        break;
    case Cursor::NoDrop:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_NOTAVAILABLE);
        break;
    case Cursor::NotAllowed:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_NOTAVAILABLE);
        break;
    case Cursor::Copy:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_NORMAL);
        break;
    case Cursor::None:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_DOT);
        break;
    case Cursor::ZoomIn:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_NORMAL);
        break;
    case Cursor::ZoomOut:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_NORMAL);
        break;
    case Cursor::Grab:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_NORMAL);
        break;
    case Cursor::Grabbing:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_MOVE);
        break;
    case Cursor::Custom:
        m_platformCursor = WebCore::PlatformCursor(POINTERTYPE_NORMAL);
        break;
    }
}

}

