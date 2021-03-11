#pragma once

#if ENABLE(VIDEO) && ENABLE(VIDEO_TRACK)

#include "VideoTrackPrivate.h"
#include "MediaPlayerPrivateMorphOS.h"
#include <wtf/WeakPtr.h>

namespace WebCore {

class VideoTrackPrivateMorphOS final : public VideoTrackPrivate
{
public:

    static RefPtr<VideoTrackPrivateMorphOS> create(WeakPtr<MediaPlayerPrivateMorphOS> player, int index)
    {
        return adoptRef(*new VideoTrackPrivateMorphOS(player, index));
    }

    Kind kind() const final;

    void disconnect();

    void setEnabled(bool);
    void markAsActive();

    int trackIndex() const override { return m_index; }

    AtomString id() const override { return AtomString(m_id); }
    AtomString label() const override { return AtomString(m_label); }
    AtomString language() const override { return AtomString(m_language); }

private:
    VideoTrackPrivateMorphOS(WeakPtr<MediaPlayerPrivateMorphOS>, int index);

	int m_index;
    String m_id;
    String m_label;
    String m_language;
    WeakPtr<MediaPlayerPrivateMorphOS> m_player;
};

}

#endif
