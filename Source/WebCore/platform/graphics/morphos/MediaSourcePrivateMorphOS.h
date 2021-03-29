#pragma once

#include "config.h"

#if ENABLE(VIDEO) && ENABLE(MEDIA_SOURCE)

#include "MediaSourcePrivate.h"
#include "MediaSourceBufferPrivateMorphOS.h"
#include <wtf/MediaTime.h>

struct Window;

namespace WebCore {

class GraphicsContext;
class FloatRect;
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
	void onSourceBufferReadyToPaint(RefPtr<MediaSourceBufferPrivateMorphOS>&);
	void onSourceBufferRemoved(RefPtr<MediaSourceBufferPrivateMorphOS>&);

	void onAudioSourceBufferUpdatedPosition(RefPtr<MediaSourceBufferPrivateMorphOS>&, double);

	bool paused() const { return m_paused; }
	bool ended() const { return m_ended; }
	bool isSeeking() const { return m_seeking; }

	void seek(double time);

    void orphan();
    WeakPtr<MediaPlayerPrivateMorphOS> player() { return makeWeakPtr(m_player); }
    void warmUp();
    void coolDown();

	void play();
	void pause();

	void paint(GraphicsContext&, const FloatRect&);
	void setOverlayWindowCoords(struct ::Window *w, int scrollx, int scrolly, int mleft, int mtop, int mright, int mbottom, int width, int height);

private:
    MediaPlayerPrivateMorphOS&                       m_player;
    Ref<MediaSourcePrivateClient>                    m_client;
	HashSet<RefPtr<MediaSourceBufferPrivateMorphOS>> m_sourceBuffers;
	RefPtr<MediaSourceBufferPrivateMorphOS>          m_paintingBuffer;
    bool                                             m_orphaned = false;
	bool                                             m_paused = true;
	bool                                             m_ended = false;
	bool                                             m_seeking = false;
};

}

#endif
