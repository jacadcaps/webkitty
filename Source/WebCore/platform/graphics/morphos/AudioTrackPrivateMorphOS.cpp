#include "config.h"

#include "AudioTrackPrivateMorphOS.h"

#define D(x) x

#if ENABLE(VIDEO) && ENABLE(VIDEO_TRACK)

namespace WebCore {

AudioTrackPrivateMorphOS::AudioTrackPrivateMorphOS(WeakPtr<MediaPlayerPrivateMorphOS> player, int index)
	: m_index(index)
	, m_player(player)
{
	m_id = "A" + String::number(index);
	D(dprintf("%s(%p)\n", __PRETTY_FUNCTION__, this));
}

AudioTrackPrivate::Kind AudioTrackPrivateMorphOS::kind() const
{
	return AudioTrackPrivate::Kind();
}

void AudioTrackPrivateMorphOS::disconnect()
{
	D(dprintf("%s(%p)\n", __PRETTY_FUNCTION__, this));
	m_player = nullptr;
}

void AudioTrackPrivateMorphOS::setEnabled(bool enabled)
{
	D(dprintf("%s(%p): %d\n", __PRETTY_FUNCTION__, this, enabled));
	if (m_player)
		m_player->onTrackEnabled(m_index, enabled);
}

void AudioTrackPrivateMorphOS::markAsActive()
{
	D(dprintf("%s(%p)\n", __PRETTY_FUNCTION__, this));
}

}

#endif
