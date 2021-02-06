#pragma once

#if ENABLE(VIDEO) && ENABLE(VIDEO_TRACK)

#include "AudioTrackPrivate.h"
#include "MediaPlayerPrivateMorphOS.h"
#include <wtf/WeakPtr.h>

namespace WebCore {

class AudioTrackPrivateMorphOS final : public AudioTrackPrivate
{
public:

    static RefPtr<AudioTrackPrivateMorphOS> create(WeakPtr<MediaPlayerPrivateMorphOS> player, int index)
    {
        return adoptRef(*new AudioTrackPrivateMorphOS(player, index));
    }

    Kind kind() const final;

    void disconnect();

    void setEnabled(bool) override;
    void markAsActive();

    int trackIndex() const override { return m_index; }

    AtomString id() const override { return m_id; }
    AtomString label() const override { return m_label; }
    AtomString language() const override { return m_language; }

private:
    AudioTrackPrivateMorphOS(WeakPtr<MediaPlayerPrivateMorphOS>, int index);

	int m_index;
    AtomString m_id;
    AtomString m_label;
    AtomString m_language;
    WeakPtr<MediaPlayerPrivateMorphOS> m_player;
};

}

#endif
