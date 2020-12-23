#pragma once

#if ENABLE(VIDEO)

#include <wtf/Function.h>

namespace WebCore {

class NetworkingContext;
class Page;

struct MediaPlayerMorphOSSettings
{
public:
    static MediaPlayerMorphOSSettings &settings();

	bool m_enableVideo = false;
	bool m_enableAudio = false;
	bool m_decodeVideo = false;
	bool m_enableOgg = true;
	bool m_enableFlv = true;
	bool m_enableWebm = true;
	
	NetworkingContext *m_networkingContextForRequests = nullptr;

	Function<void(void *player, const String &url, WebCore::Page *page, Function<void(bool doLoad)>)> m_preloadCheck;
	Function<void(void *player)> m_loadCancelled;
};

}

#endif

