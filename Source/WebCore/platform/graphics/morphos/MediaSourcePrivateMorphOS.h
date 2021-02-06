#pragma once

#include "config.h"

#if ENABLE(VIDEO) && ENABLE(MEDIA_SOURCE)

#include "MediaSourcePrivate.h"
#include "MediaSourceBufferPrivateMorphOS.h"
#include <wtf/MediaTime.h>

namespace WebCore {

class MediaPlayerPrivateMorphOS;

class MediaSourcePrivateMorphOS final : public MediaSourcePrivate {
public:
    static Ref<MediaSourcePrivateMorphOS> create(MediaPlayerPrivateMorphOS&, MediaSourcePrivateClient&);
    virtual ~MediaSourcePrivateMorphOS();

private:
    MediaSourcePrivateMorphOS(MediaPlayerPrivateMorphOS&, MediaSourcePrivateClient&);

public:
    // MediaSourcePrivate Overrides
    AddStatus addSourceBuffer(const ContentType&, RefPtr<SourceBufferPrivate>&) override;
    void durationChanged() override;
    void markEndOfStream(EndOfStreamStatus) override;
    void unmarkEndOfStream() override;
    MediaPlayer::ReadyState readyState() const override;
    void setReadyState(MediaPlayer::ReadyState) override;
    void waitForSeekCompleted() override;
    void seekCompleted() override;
	
	void onSourceBufferInitialized(RefPtr<MediaSourceBufferPrivateMorphOS>&);

    void orphan() { m_orphaned = true; }
    WeakPtr<MediaPlayerPrivateMorphOS> player() { return makeWeakPtr(m_player); }
    void warmUp();

private:
    MediaPlayerPrivateMorphOS&                       m_player;
    Ref<MediaSourcePrivateClient>                    m_client;
	HashSet<RefPtr<MediaSourceBufferPrivateMorphOS>> m_sourceBuffers;
    bool                                             m_orphaned = false;
};

}

#endif
