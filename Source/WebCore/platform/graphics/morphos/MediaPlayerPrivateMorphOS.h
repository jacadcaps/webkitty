#pragma once

#include "config.h"

#if ENABLE(VIDEO)

#include "MediaPlayerPrivate.h"
#include "PlatformLayer.h"

namespace WebCore {

struct MediaPlayerPrivateMorphOSSettings
{
	MediaPlayerPrivateMorphOSSettings()
		: m_enableVideo(true)
		, m_enableAudio(true)
		, m_enableOgg(true)
		, m_enableFlv(true)
		, m_enableWebm(true)
	{
	}
	bool m_enableVideo;
	bool m_enableAudio;
	bool m_enableOgg;
	bool m_enableFlv;
	bool m_enableWebm;
};

class MediaPlayerPrivateMorphOS : public MediaPlayerPrivateInterface, public CanMakeWeakPtr<MediaPlayerPrivateMorphOS, WeakPtrFactoryInitialization::Eager>
{
    WTF_MAKE_FAST_ALLOCATED;
public:
    MediaPlayerPrivateMorphOS(MediaPlayer*);
    virtual ~MediaPlayerPrivateMorphOS();

    static void registerMediaEngine(MediaEngineRegistrar);
    static MediaPlayer::SupportsType extendedSupportsType(const MediaEngineSupportParameters&, MediaPlayer::SupportsType);
    static bool supportsKeySystem(const String& keySystem, const String& mimeType);
	
    static MediaPlayerPrivateMorphOSSettings &settings();

    void cancelLoad() final;
	void play() final;
    void pause() final;
    FloatSize naturalSize() const final;

    bool hasVideo() const final;
    bool hasAudio() const final;

    void setVisible(bool) final;
    bool seeking() const final;
    bool paused() const final;
	
    MediaPlayer::NetworkState networkState() const final;
    MediaPlayer::ReadyState readyState() const final;
    std::unique_ptr<PlatformTimeRanges> buffered() const final;
    void paint(GraphicsContext&, const FloatRect&) final;
    bool didLoadingProgress() const final;
	
protected:
	MediaPlayer* m_player;
};

};

#endif

