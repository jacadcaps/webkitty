#include "MediaSourcePrivateMorphOS.h"

#if ENABLE(VIDEO) && ENABLE(MEDIA_SOURCE)

#include "ContentType.h"
#include "MediaSourcePrivateClient.h"
#include "MediaPlayerPrivateMorphOS.h"

#define D(x) x

namespace WebCore {

Ref<MediaSourcePrivateMorphOS> MediaSourcePrivateMorphOS::create(MediaPlayerPrivateMorphOS& parent, MediaSourcePrivateClient& client)
{
    auto source = adoptRef(*new MediaSourcePrivateMorphOS(parent, client));
    client.setPrivateAndOpen(source.copyRef());
    return source;
}

MediaSourcePrivateMorphOS::MediaSourcePrivateMorphOS(MediaPlayerPrivateMorphOS& parent, MediaSourcePrivateClient& client)
    : m_player(parent)
    , m_client(client)
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
}

MediaSourcePrivateMorphOS::~MediaSourcePrivateMorphOS()
{
	D(dprintf("%s: bye!\n", __PRETTY_FUNCTION__));
    for (auto& sourceBufferPrivate : m_sourceBuffers)
        sourceBufferPrivate->clearMediaSource();
}

MediaSourcePrivate::AddStatus MediaSourcePrivateMorphOS::addSourceBuffer(const ContentType& contentType, RefPtr<SourceBufferPrivate>& buffer)
{
	D(dprintf("%s: '%s'\n", __PRETTY_FUNCTION__, contentType.raw().utf8().data()));
	buffer = MediaSourceBufferPrivateMorphOS::create(this);
	RefPtr<MediaSourceBufferPrivateMorphOS> sourceBufferPrivate = static_cast<MediaSourceBufferPrivateMorphOS*>(buffer.get());
	m_sourceBuffers.add(sourceBufferPrivate);
	return Ok;
}

void MediaSourcePrivateMorphOS::durationChanged()
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
}

void MediaSourcePrivateMorphOS::markEndOfStream(EndOfStreamStatus)
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
}

void MediaSourcePrivateMorphOS::unmarkEndOfStream()
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
}

MediaPlayer::ReadyState MediaSourcePrivateMorphOS::readyState() const
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
	return m_player.readyState();
}

void MediaSourcePrivateMorphOS::setReadyState(MediaPlayer::ReadyState rs)
{
	D(dprintf("%s: %d\n", __PRETTY_FUNCTION__, int(rs)));
	m_player.accSetReadyState(rs);
}

void MediaSourcePrivateMorphOS::waitForSeekCompleted()
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
}

void MediaSourcePrivateMorphOS::seekCompleted()
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
}

void MediaSourcePrivateMorphOS::warmUp()
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
	for (auto& sourceBufferPrivate : m_sourceBuffers)
		sourceBufferPrivate->warmUp();
	
	m_player.accSetReadyState(WebCore::MediaPlayerEnums::ReadyState::HaveEnoughData);
	m_player.accSetReadyState(WebCore::MediaPlayerEnums::ReadyState::HaveFutureData);
}

void MediaSourcePrivateMorphOS::onSourceBufferInitialized(RefPtr<MediaSourceBufferPrivateMorphOS> &source)
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
// TMP: needs to pass this to player
	warmUp();
}

}


#endif
