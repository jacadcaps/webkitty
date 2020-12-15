#pragma once

#if ENABLE(VIDEO)

namespace WebCore {

class NetworkingContext;

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
};

}

#endif

