#include "config.h"

#include "VideoTrackPrivateMorphOS.h"

#define D(x) x

#if ENABLE(VIDEO) && ENABLE(VIDEO_TRACK)

namespace WebCore {

VideoTrackPrivateMorphOS::VideoTrackPrivateMorphOS(WeakPtr<MediaPlayerPrivateMorphOS> player, int index)
	: m_index(index)
	, m_player(player)
{
	m_id = "A" + String::number(index);
	D(dprintf("%s(%p)\n", __PRETTY_FUNCTION__, this));
}

VideoTrackPrivate::Kind VideoTrackPrivateMorphOS::kind() const
{
	return VideoTrackPrivate::Kind();
}

void VideoTrackPrivateMorphOS::disconnect()
{
	D(dprintf("%s(%p)\n", __PRETTY_FUNCTION__, this));
	m_player = nullptr;
}

void VideoTrackPrivateMorphOS::setEnabled(bool)
{
	D(dprintf("%s(%p)\n", __PRETTY_FUNCTION__, this));
}

void VideoTrackPrivateMorphOS::markAsActive()
{
	D(dprintf("%s(%p)\n", __PRETTY_FUNCTION__, this));
}

}

#endif
