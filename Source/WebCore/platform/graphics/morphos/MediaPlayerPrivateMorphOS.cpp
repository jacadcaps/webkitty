#include "MediaPlayerPrivateMorphOS.h"

#if ENABLE(VIDEO)

#include "MediaPlayer.h"
#include "MediaSourcePrivateClient.h"
#include "MockMediaSourcePrivate.h"
#include "NotImplemented.h"
#include <wtf/MainThread.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/text/WTFString.h>

#define D(x)

namespace WebCore {

static MediaPlayerPrivateMorphOSSettings m_playerSettings;

class MediaPlayerFactoryMediaSourceMorphOS final : public MediaPlayerFactory {
public:
    MediaPlayerEnums::MediaEngineIdentifier identifier() const final { return MediaPlayerEnums::MediaEngineIdentifier::MorphOS; };

    std::unique_ptr<MediaPlayerPrivateInterface> createMediaEnginePlayer(MediaPlayer* player) const final { return makeUnique<MediaPlayerPrivateMorphOS>(player); }

    static void s_getSupportedTypes(HashSet<String, ASCIICaseInsensitiveHash>& types)
    {
    	if (m_playerSettings.m_enableAudio)
    	{
			types.add(String("audio/aac"));
			types.add(String("audio/basic"));
			types.add(String("audio/mp4"));
			types.add(String("audio/flac"));
			types.add(String("audio/mpeg"));
			types.add(String("audio/vnd.wave"));
			types.add(String("audio/wav"));
			types.add(String("audio/wave"));

			types.add(String("audio/x-aiff"));
			types.add(String("audio/x-flac"));
			types.add(String("audio/x-m4a"));
			types.add(String("audio/x-pn-wav"));
			types.add(String("audio/x-wav"));

			if (m_playerSettings.m_enableOgg)
				types.add(String("audio/ogg"));
			if (m_playerSettings.m_enableWebm)
				types.add(String("audio/webm"));
		}
		
		if (m_playerSettings.m_enableVideo)
		{
			types.add(String("video/avi"));
			types.add(String("video/flv"));
			types.add(String("video/mp4"));
			types.add(String("video/vnd.objectvideo"));
			types.add(String("video/x-flv"));

			if (m_playerSettings.m_enableOgg)
			{
				types.add(String("video/ogg"));
				types.add(String("video/x-theora+ogg"));
			}
			if (m_playerSettings.m_enableWebm)
				types.add(String("video/webm"));
		}
    }

	void getSupportedTypes(HashSet<String, ASCIICaseInsensitiveHash>& types) const final
	{
		s_getSupportedTypes(types);
	}

    static MediaPlayer::SupportsType s_supportsTypeAndCodecs(const MediaEngineSupportParameters& parameters)
    {
		D(dprintf("%s: url '%s' content '%s' ctype '%s' isource %d istream %d\n", __PRETTY_FUNCTION__,
			parameters.url.string().utf8().data(), parameters.type.raw().utf8().data(), parameters.type.containerType().utf8().data(), parameters.isMediaSource, parameters.isMediaStream));
       	auto containerType = parameters.type.containerType();
		if (containerType.isEmpty())
			return MediaPlayer::SupportsType::MayBeSupported;
		HashSet<String, ASCIICaseInsensitiveHash> types;
		s_getSupportedTypes(types);
		D(dprintf("%s: contains? %d\n", __PRETTY_FUNCTION__, types.contains(containerType)));
		if (types.contains(containerType))
			return parameters.type.codecs().isEmpty() ? MediaPlayer::SupportsType::MayBeSupported : MediaPlayer::SupportsType::IsSupported;
		D(dprintf("%s: not supported %s\n", __PRETTY_FUNCTION__, parameters.type.raw().utf8().data()));
        return MediaPlayer::SupportsType::IsNotSupported;
    }
	
    MediaPlayer::SupportsType supportsTypeAndCodecs(const MediaEngineSupportParameters& parameters) const final
    {
    	return s_supportsTypeAndCodecs(parameters);
	}
};

MediaPlayerPrivateMorphOSSettings &MediaPlayerPrivateMorphOS::settings()
{
	return m_playerSettings;
}

MediaPlayerPrivateMorphOS::MediaPlayerPrivateMorphOS(MediaPlayer* player)
	: m_player(player)
{
	notImplemented();
}

MediaPlayerPrivateMorphOS::~MediaPlayerPrivateMorphOS()
{

}

void MediaPlayerPrivateMorphOS::registerMediaEngine(MediaEngineRegistrar registrar)
{
	registrar(makeUnique<MediaPlayerFactoryMediaSourceMorphOS>());
}

MediaPlayer::SupportsType MediaPlayerPrivateMorphOS::extendedSupportsType(const MediaEngineSupportParameters& parameters, MediaPlayer::SupportsType type)
{
	return MediaPlayerFactoryMediaSourceMorphOS::s_supportsTypeAndCodecs(parameters);
}

bool MediaPlayerPrivateMorphOS::supportsKeySystem(const String& keySystem, const String& mimeType)
{
	// this encrypted media support, which we don't have
	return false;
}

void MediaPlayerPrivateMorphOS::load(const String& url)
{
	D(dprintf("%s: %s\n", __PRETTY_FUNCTION__, url.utf8().data()));
    m_networkState = MediaPlayer::NetworkState::Loading;
    m_player->networkStateChanged();
    m_readyState = MediaPlayer::ReadyState::HaveNothing;
    m_player->readyStateChanged();
}

void MediaPlayerPrivateMorphOS::cancelLoad()
{
	D(dprintf("%s:\n", __PRETTY_FUNCTION__));
	notImplemented();

}

void MediaPlayerPrivateMorphOS::prepareToPlay()
{
	D(dprintf("%s:\n", __PRETTY_FUNCTION__));
}

bool MediaPlayerPrivateMorphOS::canSaveMediaData() const
{
	D(dprintf("%s:\n", __PRETTY_FUNCTION__));
	return false;
}

void MediaPlayerPrivateMorphOS::play()
{
	notImplemented();
	D(dprintf("%s:\n", __PRETTY_FUNCTION__));
}

void MediaPlayerPrivateMorphOS::pause()
{
	notImplemented();
	D(dprintf("%s:\n", __PRETTY_FUNCTION__));
}

void MediaPlayerPrivateMorphOS::setVolume(float volume)
{
	D(dprintf("%s: vol %f\n", __PRETTY_FUNCTION__, volume));
}

void MediaPlayerPrivateMorphOS::setMuted(bool muted)
{
	D(dprintf("%s: %d\n", __PRETTY_FUNCTION__, muted));
}

FloatSize MediaPlayerPrivateMorphOS::naturalSize() const
{
	return { 320, 240 };
}

bool MediaPlayerPrivateMorphOS::hasVideo() const
{
	return false;
}

bool MediaPlayerPrivateMorphOS::hasAudio() const
{
	return false;
}

void MediaPlayerPrivateMorphOS::setVisible(bool visible)
{
	D(dprintf("%s: visible %d\n", __PRETTY_FUNCTION__, visible));
}

bool MediaPlayerPrivateMorphOS::seeking() const
{
	return false;
}

bool MediaPlayerPrivateMorphOS::paused() const
{
	return true;
}

MediaPlayer::NetworkState MediaPlayerPrivateMorphOS::networkState() const
{
	return m_networkState;
}

MediaPlayer::ReadyState MediaPlayerPrivateMorphOS::readyState() const
{
	return m_readyState;
}

std::unique_ptr<PlatformTimeRanges> MediaPlayerPrivateMorphOS::buffered() const
{
	return makeUnique<PlatformTimeRanges>();
}

void MediaPlayerPrivateMorphOS::paint(GraphicsContext&, const FloatRect&)
{

}

bool MediaPlayerPrivateMorphOS::didLoadingProgress() const
{
	return false;
}

}

#undef D
#endif
