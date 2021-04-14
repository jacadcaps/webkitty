#pragma once

#if ENABLE(VIDEO)

#include <wtf/Function.h>

#define EP_PROFILING 0
#include <libeventprofiler.h>

namespace WebCore {

class NetworkingContext;
class MediaPlayer;

struct MediaPlayerMorphOSInfo
{
	float m_duration = 0;
	int   m_frequency = 0;
	int   m_bits;
	int   m_channels = 0;
	int   m_width = 0;
	int   m_height;
	bool  m_isLive = false;
    bool  m_isDownloadable = false;
};

struct MediaPlayerMorphOSSettings
{
public:
    static MediaPlayerMorphOSSettings &settings();

	bool m_enableVideo = true;
	bool m_enableAudio = true;
	bool m_decodeVideo = true;
	bool m_enableMediaSource = false;
    bool m_enableVP9 = false;
    
    // NOTE: keep in sync with WkGlobalSettings_LoopFilter
    enum class SkipLoopFilter {
        Default,
        NonRef,
        BiDirectional,
        NonIntra,
        NonKey,
        All
    };

    SkipLoopFilter m_loopFilter = SkipLoopFilter::All;
 
	NetworkingContext *m_networkingContextForRequests = nullptr;

	Function<bool(WebCore::MediaPlayer *player, const String &url)> m_preloadCheck;
	Function<void(WebCore::MediaPlayer *player, const String &url, MediaPlayerMorphOSInfo &info,
		Function<void(bool doLoad)>&&, Function<void()> &&yieldFunc)> m_loadCheck;
	Function<void(WebCore::MediaPlayer *player)> m_loadCancelled;
	Function<void(WebCore::MediaPlayer *player)> m_willPlay;

	Function<void(WebCore::MediaPlayer *player,
		Function<void(void *windowPtr, int scrollX, int scrollY, int left, int top, int right, int bottom, int width, int height)>&&)> m_overlayRequest;
	Function<void(WebCore::MediaPlayer *player)> m_overlayUpdate;
};

}

#endif
