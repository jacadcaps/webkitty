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
	
	NetworkingContext *m_networkingContextForRequests = nullptr;

	Function<void(void *player, const String &url, WebCore::Page *page, Function<void(bool doLoad)>)> m_preloadCheck;
	Function<void(void *player)> m_loadCancelled;
};

}

#endif
