#pragma once

#include "config.h"

#if ENABLE(VIDEO)

#include "MediaPlayerPrivate.h"
#include "PlatformLayer.h"
#include "MediaPlayerMorphOS.h"
#include "AcinerellaClient.h"

namespace WebCore {

namespace Acinerella {
	class Acinerella;
}

template<typename T>
using deleted_unique_ptr = std::unique_ptr<T,std::function<void(T*)>>;

class MediaPlayerPrivateMorphOS : public MediaPlayerPrivateInterface, public Acinerella::AcinerellaClient, public CanMakeWeakPtr<MediaPlayerPrivateMorphOS, WeakPtrFactoryInitialization::Eager>
{
    WTF_MAKE_FAST_ALLOCATED;
public:
    MediaPlayerPrivateMorphOS(MediaPlayer*);
    virtual ~MediaPlayerPrivateMorphOS();

    static void registerMediaEngine(MediaEngineRegistrar);
    static MediaPlayer::SupportsType extendedSupportsType(const MediaEngineSupportParameters&, MediaPlayer::SupportsType);
    static bool supportsKeySystem(const String& keySystem, const String& mimeType);
	
    void load(const String&) final;
    void cancelLoad() final;
    void prepareToPlay() final;
    bool canSaveMediaData() const final;

	void play() final;
    void pause() final;
    FloatSize naturalSize() const final;

    float duration() const final { return m_duration; }
    float currentTime() const final { return m_currentTime; }

    bool hasVideo() const final;
    bool hasAudio() const final;

    void setVisible(bool) final;
    bool seeking() const final;
    bool paused() const final;

    void setVolume(float) final;
    void setMuted(bool) final;

    MediaPlayer::NetworkState networkState() const final;
    MediaPlayer::ReadyState readyState() const final;
    std::unique_ptr<PlatformTimeRanges> buffered() const final;
    void paint(GraphicsContext&, const FloatRect&) final;
    bool didLoadingProgress() const final;
	
	bool accEnableAudio() const override;
	bool accEnableVideo() const override;
	void accSetNetworkState(WebCore::MediaPlayerEnums::NetworkState state) override;
	void accSetReadyState(WebCore::MediaPlayerEnums::ReadyState state) override;
	void accSetBufferLength(float buffer) override;
	void accSetPosition(float buffer) override;
	void accSetDuration(float buffer) override;

protected:
	MediaPlayer* m_player;
	RefPtr<Acinerella::Acinerella> m_acinerella;
	MediaPlayer::NetworkState m_networkState = { MediaPlayer::NetworkState::Empty };
	MediaPlayer::ReadyState m_readyState = { MediaPlayer::ReadyState::HaveNothing };
	float m_duration = 0.f;
	float m_currentTime = 0.f;

friend class Acinerella::Acinerella;
};

};

#endif

