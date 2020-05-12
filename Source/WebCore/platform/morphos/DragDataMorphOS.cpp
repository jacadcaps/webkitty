#include "config.h"
#include "DragData.h"
#include "SelectionData.h"

namespace WebCore {

bool DragData::canSmartReplace() const
{
    return false;
}

bool DragData::containsColor() const
{
    return false;
}

bool DragData::containsFiles() const
{
    return m_platformDragData->hasFilenames();
}

unsigned DragData::numberOfFiles() const
{
    return m_platformDragData->filenames().size();
}

Vector<String> DragData::asFilenames() const
{
    return m_platformDragData->filenames();
}

bool DragData::containsPlainText() const
{
    return m_platformDragData->hasText();
}

String DragData::asPlainText() const
{
    return m_platformDragData->text();
}

Color DragData::asColor() const
{
    return Color();
}

bool DragData::containsCompatibleContent(DraggingPurpose) const
{
// TODO: dragging data
    return false; // containsPlainText() || containsURL() || m_platformDragData->hasMarkup() || containsColor() || containsFiles();
}

bool DragData::containsURL(FilenameConversionPolicy filenamePolicy) const
{
    return !asURL(filenamePolicy).isEmpty();
}

String DragData::asURL(FilenameConversionPolicy filenamePolicy, String* title) const
{
    if (!m_platformDragData->hasURL())
        return String();
    if (filenamePolicy != ConvertFilenames) {
        URL url(URL(), m_platformDragData->url());
        if (url.isLocalFile())
            return String();
    }

    String url(m_platformDragData->url());
    if (title)
        *title = m_platformDragData->urlLabel();
    return url;
}

}

