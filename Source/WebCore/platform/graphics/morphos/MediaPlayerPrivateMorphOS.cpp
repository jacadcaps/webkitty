#include "MediaPlayerPrivateMorphOS.h"

#if ENABLE(VIDEO)

#include "MediaPlayer.h"
#include "MediaSourcePrivateClient.h"
#include "NotImplemented.h"
#include "AcinerellaContainer.h"

#include "HTMLMediaElement.h"
#include "Frame.h"

#define D(x) x
#define DM(x) x

namespace WebCore {



MediaPlayerMorphOSSettings &MediaPlayerMorphOSSettings::settings()
{
	static MediaPlayerMorphOSSettings m_playerSettings;
	return m_playerSettings;
}

class MediaPlayerFactoryMediaSourceMorphOS final : public MediaPlayerFactory {
public:
    MediaPlayerEnums::MediaEngineIdentifier identifier() const final { return MediaPlayerEnums::MediaEngineIdentifier::MorphOS; };

    std::unique_ptr<MediaPlayerPrivateInterface> createMediaEnginePlayer(MediaPlayer* player) const final { return makeUnique<MediaPlayerPrivateMorphOS>(player); }

    static void s_getSupportedTypes(HashSet<String, ASCIICaseInsensitiveHash>& types)
    {
    	if (MediaPlayerMorphOSSettings::settings().m_enableAudio)
    	{
			types.add(String("audio/aac"));
			types.add(String("audio/basic"));
			types.add(String("audio/mp3"));
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

			types.add(String("audio/ogg"));
			types.add(String("audio/webm"));

			types.add(String("audio/x-mpegurl"));
			types.add(String("audio/x-scpls"));
			types.add(String("audio/mpa"));
			types.add(String("audio/mpa-robust"));
		}
		
		if (MediaPlayerMorphOSSettings::settings().m_enableVideo)
		{
			types.add(String("video/avi"));
			types.add(String("video/flv"));
			types.add(String("video/mp4"));
			types.add(String("video/vnd.objectvideo"));
			types.add(String("video/x-flv"));

			types.add(String("video/ogg"));
			types.add(String("video/x-theora+ogg"));
			types.add(String("video/webm"));
		}
		
		// HLS
		if (MediaPlayerMorphOSSettings::settings().m_enableAudio || MediaPlayerMorphOSSettings::settings().m_enableVideo)
		{
			types.add(String("application/x-mpegurl"));
			types.add(String("application/vnd.apple.mpegurl"));
		}
    }

	void getSupportedTypes(HashSet<String, ASCIICaseInsensitiveHash>& types) const final
	{
		s_getSupportedTypes(types);
	}

    static MediaPlayer::SupportsType s_supportsTypeAndCodecs(const MediaEngineSupportParameters& parameters)
    {
    	if (startsWithLettersIgnoringASCIICase(parameters.type.raw(), "image/"))
    		return MediaPlayer::SupportsType::IsNotSupported;
    	if (startsWithLettersIgnoringASCIICase(parameters.url.string(), "data:"))
    		return MediaPlayer::SupportsType::IsNotSupported;

#if ENABLE(MEDIA_SOURCE)
    	if (startsWithLettersIgnoringASCIICase(parameters.url.string(), "blob:"))
    		return MediaPlayer::SupportsType::MayBeSupported;
#else
    	if (startsWithLettersIgnoringASCIICase(parameters.url.string(), "blob:"))
    		return MediaPlayer::SupportsType::IsNotSupported;
#endif

		DM(dprintf("%s: url '%s' content '%s' ctype '%s' isource %d istream %d profiles %d\n", __func__,
			parameters.url.string().utf8().data(), parameters.type.raw().utf8().data(), parameters.type.containerType().utf8().data(), parameters.isMediaSource, parameters.isMediaStream,
			parameters.type.profiles().size()));
       	auto containerType = parameters.type.containerType();
		if (containerType.isEmpty())
		{
			DM(dprintf("%s: container empty, assume 'maybe'\n", __func__));
			return MediaPlayer::SupportsType::MayBeSupported;
		}
		HashSet<String, ASCIICaseInsensitiveHash> types;
		s_getSupportedTypes(types);
		DM(dprintf("%s: '%s' contained in list? %d\n", __func__, parameters.type.containerType().utf8().data(), types.contains(containerType)));
		if (types.contains(containerType))
		{
			auto codecs = parameters.type.codecs();
			if (codecs.isEmpty())
			{
				DM(dprintf("%s: codecs empty, assume 'maybe'\n", __func__));
				return MediaPlayer::SupportsType::MayBeSupported;
			}

			DM(dprintf("%s: lists %d codecs\n", __func__, codecs.size()));
			for (size_t i = 0; i < codecs.size(); i++)
			{
				auto &codec = codecs.at(i);
				if (startsWithLettersIgnoringASCIICase(codec, "av01")) // requires ffmpeg 4.0!
				{
					DM(dprintf("%s: rejecting unsupported codec %s\n", __func__, codec.utf8().data()));
					return MediaPlayer::SupportsType::IsNotSupported;
				}
				else
				{
					DM(dprintf("%s: we should be OK with codec %s\n", __func__, codec.utf8().data()));
				}
			}

			return  MediaPlayer::SupportsType::IsSupported;
		}
		DM(dprintf("%s: not supported!\n", __func__));
        return MediaPlayer::SupportsType::IsNotSupported;
    }
	
    MediaPlayer::SupportsType supportsTypeAndCodecs(const MediaEngineSupportParameters& parameters) const final
    {
    	return s_supportsTypeAndCodecs(parameters);
	}
};

MediaPlayerPrivateMorphOS::MediaPlayerPrivateMorphOS(MediaPlayer* player)
	: m_player(player)
{
	notImplemented();
}

MediaPlayerPrivateMorphOS::~MediaPlayerPrivateMorphOS()
{
	if (m_acinerella)
		m_acinerella->terminate();

	// remove all pending requests that could be referencing 'this'
	if (MediaPlayerMorphOSSettings::settings().m_loadCancelled)
		MediaPlayerMorphOSSettings::settings().m_loadCancelled(m_player);
}

void MediaPlayerPrivateMorphOS::registerMediaEngine(MediaEngineRegistrar registrar)
{
	registrar(makeUnique<MediaPlayerFactoryMediaSourceMorphOS>());
}

MediaPlayer::SupportsType MediaPlayerPrivateMorphOS::extendedSupportsType(const MediaEngineSupportParameters& parameters, MediaPlayer::SupportsType type)
{
	return MediaPlayerFactoryMediaSourceMorphOS::s_supportsTypeAndCodecs(parameters);
}

bool MediaPlayerPrivateMorphOS::supportsKeySystem(const String&, const String&)
{
	// this encrypted media support, which we don't have
	return false;
}

void MediaPlayerPrivateMorphOS::load(const String& url)
{
	D(dprintf("%s: %s\n", __PRETTY_FUNCTION__, url.utf8().data()));

	cancelLoad();

	if (startsWithLettersIgnoringASCIICase(url, "about:"))
		return;

	if (MediaPlayerMorphOSSettings::settings().m_preloadCheck)
	{
		if (!MediaPlayerMorphOSSettings::settings().m_preloadCheck(m_player, url))
		{
			m_networkState = WebCore::MediaPlayerEnums::NetworkState::FormatError;
			m_readyState = WebCore::MediaPlayerEnums::ReadyState::HaveNothing;
			m_player->networkStateChanged();
			m_player->readyStateChanged();
			return;
		}
	}

	m_networkState = MediaPlayer::NetworkState::Loading;
	m_player->networkStateChanged();
	m_readyState = MediaPlayer::ReadyState::HaveNothing;
	m_player->readyStateChanged();

	m_acinerella = Acinerella::Acinerella::create(this, url);
}

#if ENABLE(MEDIA_SOURCE)
void MediaPlayerPrivateMorphOS::load(const String& url, MediaSourcePrivateClient* client)
{
	D(dprintf("%s: %s\n", __PRETTY_FUNCTION__, url.utf8().data()));
	cancelLoad();

	if (startsWithLettersIgnoringASCIICase(url, "about:"))
		return;

	m_mediaSourcePrivate = MediaSourcePrivateMorphOS::create(*this, *client);


#if 0
	if (MediaPlayerMorphOSSettings::settings().m_preloadCheck)
	{
		if (!MediaPlayerMorphOSSettings::settings().m_preloadCheck(m_player, url))
		{
			m_networkState = WebCore::MediaPlayerEnums::NetworkState::FormatError;
			m_readyState = WebCore::MediaPlayerEnums::ReadyState::HaveNothing;
			m_player->networkStateChanged();
			m_player->readyStateChanged();
			return;
		}
	}

	m_networkState = MediaPlayer::NetworkState::Loading;
	m_player->networkStateChanged();
	m_readyState = MediaPlayer::ReadyState::HaveNothing;
	m_player->readyStateChanged();

	m_acinerella = Acinerella::Acinerella::create(this, url);
#endif
}
#endif

void MediaPlayerPrivateMorphOS::cancelLoad()
{
	D(dprintf("%s:\n", __PRETTY_FUNCTION__));

	if (MediaPlayerMorphOSSettings::settings().m_loadCancelled)
		MediaPlayerMorphOSSettings::settings().m_loadCancelled(m_player);
	
	if (m_mediaSourcePrivate)
		m_mediaSourcePrivate->orphan();
	m_mediaSourcePrivate = nullptr;
	
	m_prepareToPlay = m_acInitialized = false;
	pause();

	if (m_acinerella)
		m_acinerella->terminate();
}

void MediaPlayerPrivateMorphOS::prepareToPlay()
{
	D(dprintf("%s:\n", __PRETTY_FUNCTION__));
	m_prepareToPlay = true;
	if (m_mediaSourcePrivate)
		m_mediaSourcePrivate->warmUp();

	if (m_acinerella && m_acInitialized)
		m_acinerella->warmUp();
}

bool MediaPlayerPrivateMorphOS::canSaveMediaData() const
{
	D(dprintf("%s:\n", __PRETTY_FUNCTION__));
	return false;
}

void MediaPlayerPrivateMorphOS::play()
{
	if (m_acinerella)
		m_acinerella->play();
	D(dprintf("%s:\n", __PRETTY_FUNCTION__));
}

void MediaPlayerPrivateMorphOS::pause()
{
	if (m_acinerella)
		m_acinerella->pause();
	D(dprintf("%s:\n", __PRETTY_FUNCTION__));
}

void MediaPlayerPrivateMorphOS::setVolume(float volume)
{
	D(dprintf("%s: vol %f\n", __PRETTY_FUNCTION__, volume));
	if (m_acinerella)
		m_acinerella->setVolume(volume);
}

void MediaPlayerPrivateMorphOS::setMuted(bool muted)
{
	D(dprintf("%s: %d\n", __PRETTY_FUNCTION__, muted));
	if (m_acinerella)
		m_acinerella->setMuted(muted);
}

FloatSize MediaPlayerPrivateMorphOS::naturalSize() const
{
	return { 320, 240 };
}

bool MediaPlayerPrivateMorphOS::hasVideo() const
{
	if (m_acinerella)
		return m_acinerella->hasVideo();
	return false;
}

bool MediaPlayerPrivateMorphOS::hasAudio() const
{
	if (m_acinerella)
		return m_acinerella->hasAudio();
	return false;
}

void MediaPlayerPrivateMorphOS::setVisible(bool visible)
{
	(void)visible;
	D(dprintf("%s: visible %d\n", __PRETTY_FUNCTION__, visible));
}

bool MediaPlayerPrivateMorphOS::seeking() const
{
	D(dprintf("%s: %d\n", __PRETTY_FUNCTION__, m_acinerella?m_acinerella->isSeeking():false));
	if (m_acinerella)
		return m_acinerella->isSeeking();
	return false;
}

void MediaPlayerPrivateMorphOS::seek(float time)
{
	D(dprintf("%s: %f\n", __PRETTY_FUNCTION__, time));
	if (m_acinerella)
		return m_acinerella->seek(time);
}

bool MediaPlayerPrivateMorphOS::ended() const
{
	if (m_acinerella)
		return m_acinerella->ended();
	return true;
}

bool MediaPlayerPrivateMorphOS::paused() const
{
	if (m_acinerella)
		return m_acinerella->paused();
	return false;
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
	return makeUnique<PlatformTimeRanges>(MediaTime::createWithFloat(m_currentTime), MediaTime::createWithFloat(m_currentTime + 6));
}

void MediaPlayerPrivateMorphOS::paint(GraphicsContext&, const FloatRect&)
{

}

bool MediaPlayerPrivateMorphOS::didLoadingProgress() const
{
	return false;
}

MediaPlayer::MovieLoadType MediaPlayerPrivateMorphOS::movieLoadType() const
{
	if (m_acinerella)
		return m_acinerella->isLive() ? MediaPlayer::MovieLoadType::LiveStream : MediaPlayer::MovieLoadType::Download;
	return MediaPlayer::MovieLoadType::Download;
}

float MediaPlayerPrivateMorphOS::maxTimeSeekable() const
{
	if (m_acinerella && m_acinerella->canSeek())
		return m_duration;
	return 0.f;
}

void MediaPlayerPrivateMorphOS::accInitialized(MediaPlayerMorphOSInfo info)
{
	if (MediaPlayerMorphOSSettings::settings().m_loadCheck && m_acinerella)
	{
		MediaPlayerMorphOSSettings::settings().m_loadCheck(m_player, m_acinerella->url(), info, [this](bool doLoad) {
			if (doLoad)
			{
				m_acInitialized = true;
				m_player->characteristicChanged();
				if (m_prepareToPlay && m_acinerella)
					m_acinerella->warmUp();
			}
		});
	}
	else
	{
		m_acInitialized = true;
		m_player->characteristicChanged();
		if (m_prepareToPlay && m_acinerella)
			m_acinerella->warmUp();
	}
}

bool MediaPlayerPrivateMorphOS::accEnableAudio() const
{
	return MediaPlayerMorphOSSettings::settings().m_enableAudio;
}

bool MediaPlayerPrivateMorphOS::accEnableVideo() const
{
	return MediaPlayerMorphOSSettings::settings().m_enableVideo && MediaPlayerMorphOSSettings::settings().m_decodeVideo;
}

void MediaPlayerPrivateMorphOS::accSetNetworkState(WebCore::MediaPlayerEnums::NetworkState state)
{
	m_networkState = state;
	m_player->networkStateChanged();
}

void MediaPlayerPrivateMorphOS::accSetReadyState(WebCore::MediaPlayerEnums::ReadyState state)
{
	m_readyState = state;
	m_player->readyStateChanged();
}

void MediaPlayerPrivateMorphOS::accSetBufferLength(float buffer)
{
	(void)buffer;
	m_player->bufferedTimeRangesChanged();
	m_player->seekableTimeRangesChanged();
}

void MediaPlayerPrivateMorphOS::accSetPosition(float pos)
{
	D(dprintf("%s: timechanged to %f\n", __func__, this, pos));
	m_currentTime = pos;
	m_player->timeChanged();
}

void MediaPlayerPrivateMorphOS::accSetDuration(float dur)
{
	D(dprintf("%s: changed to %f\n", __func__, this, dur));
	m_duration = dur;
	m_player->durationChanged();
}

void MediaPlayerPrivateMorphOS::accEnded()
{
	m_currentTime = m_duration;
	m_player->timeChanged();
	m_player->characteristicChanged();
}

void MediaPlayerPrivateMorphOS::accFailed()
{
	m_networkState = WebCore::MediaPlayerEnums::NetworkState::FormatError;
	m_readyState = WebCore::MediaPlayerEnums::ReadyState::HaveNothing;
	m_player->networkStateChanged();
	m_player->readyStateChanged();
}

RefPtr<PlatformMediaResourceLoader> MediaPlayerPrivateMorphOS::accCreateResourceLoader()
{
	return m_player->createResourceLoader();
}

String MediaPlayerPrivateMorphOS::accReferrer()
{
	return m_player->referrer();
}

#if ENABLE(MEDIA_SOURCE)
void MediaPlayerPrivateMorphOS::onTrackEnabled(int index, bool enabled)
{
	// TODO: enable/disable track via source
}
#endif

}

#undef D
#undef DM
#endif
