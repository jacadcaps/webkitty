#pragma once

#if ENABLE(VIDEO)

#include "AudioTrackPrivate.h"
#include <wtf/WeakPtr.h>

namespace WebCore {

class MediaPlayerPrivateMorphOS;
class MediaSourceBufferPrivateMorphOS;

class AudioTrackPrivateMorphOS : public AudioTrackPrivate
{
public:

    static RefPtr<AudioTrackPrivateMorphOS> create(ThreadSafeWeakPtr<MediaPlayerPrivateMorphOS> player, int index)
    {
        return adoptRef(*new AudioTrackPrivateMorphOS(player, index));
    }

    Kind kind() const final;

    virtual void disconnect();

    void setEnabled(bool) override;

    int trackIndex() const override { return m_index; }

    TrackID id() const override { return TrackID(m_index); }
    String label() const override { return m_label; }
    String language() const override { return m_language; }

protected:
    AudioTrackPrivateMorphOS(ThreadSafeWeakPtr<MediaPlayerPrivateMorphOS>, int index);

	int m_index;
    String m_label;
    String m_language;
    ThreadSafeWeakPtr<MediaPlayerPrivateMorphOS> m_player;
};

#if ENABLE(MEDIA_SOURCE)
class AudioTrackPrivateMorphOSMS : public AudioTrackPrivateMorphOS
{
public:

    static RefPtr<AudioTrackPrivateMorphOSMS> create(MediaSourceBufferPrivateMorphOS *source, int index)
    {
        return adoptRef(*new AudioTrackPrivateMorphOSMS(source, index));
    }

    void disconnect() override;
    void setEnabled(bool) override;

protected:
    AudioTrackPrivateMorphOSMS(MediaSourceBufferPrivateMorphOS*, int index);
    MediaSourceBufferPrivateMorphOS *m_source;
};
#endif

}

#endif
