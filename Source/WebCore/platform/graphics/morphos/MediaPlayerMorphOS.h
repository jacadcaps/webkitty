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
	float m_duration;
	int   m_frequency;
	int   m_bits;
	int   m_channels;
	int   m_width;
	int   m_height;
	bool  m_isLive;
};

struct MediaPlayerMorphOSSettings
{
public:
    static MediaPlayerMorphOSSettings &settings();

	bool m_enableVideo = false;
	bool m_enableAudio = false;
	bool m_decodeVideo = false;
	bool m_enableMediaSource = false;
	
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
