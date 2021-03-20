#include "MediaSourcePrivateMorphOS.h"

#if ENABLE(VIDEO) && ENABLE(MEDIA_SOURCE)

#include "ContentType.h"
#include "MediaSourcePrivateClient.h"
#include "MediaPlayerPrivateMorphOS.h"

#define D(x) x
// #pragma GCC optimize ("O0")

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
//	D(dprintf("%s: %d\n", __PRETTY_FUNCTION__, int(rs)));
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

void MediaSourcePrivateMorphOS::orphan()
{
	m_orphaned = true;
	m_paintingBuffer = nullptr;
}

void MediaSourcePrivateMorphOS::warmUp()
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
	for (auto& sourceBufferPrivate : m_sourceBuffers)
		sourceBufferPrivate->warmUp();
	
	m_player.accSetReadyState(WebCore::MediaPlayerEnums::ReadyState::HaveEnoughData);
	m_player.accSetReadyState(WebCore::MediaPlayerEnums::ReadyState::HaveFutureData);
}

void MediaSourcePrivateMorphOS::play()
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
	for (auto& sourceBufferPrivate : m_sourceBuffers)
		sourceBufferPrivate->play();
	m_paused = false;
}

// TODO locks for painting buffer access

void MediaSourcePrivateMorphOS::pause()
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
	for (auto& sourceBufferPrivate : m_sourceBuffers)
		sourceBufferPrivate->pause();
	m_paused = true;
}

void MediaSourcePrivateMorphOS::seek(double time)
{
	D(dprintf("%s: %f\n", __PRETTY_FUNCTION__, float(time)));
	if (m_seeking)
		return;
	
	m_seeking = true;

    for (auto& sourceBufferPrivate : m_sourceBuffers)
        sourceBufferPrivate->willSeek(time);

	m_client->seekToTime(MediaTime::createWithDouble(time));
}

void MediaSourcePrivateMorphOS::paint(GraphicsContext& gc, const FloatRect& rect)
{
	if (!!m_paintingBuffer)
		m_paintingBuffer->paint(gc, rect);
}

void MediaSourcePrivateMorphOS::setOverlayWindowCoords(struct ::Window *w, int scrollx, int scrolly, int mleft, int mtop, int mright, int mbottom)
{
	if (!!m_paintingBuffer)
		m_paintingBuffer->setOverlayWindowCoords(w, scrollx, scrolly, mleft, mtop, mright, mbottom);
}

void MediaSourcePrivateMorphOS::onSourceBufferInitialized(RefPtr<MediaSourceBufferPrivateMorphOS> &source)
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
// TMP: needs to pass this to player
	warmUp();
}

void MediaSourcePrivateMorphOS::onSourceBufferReadyToPaint(RefPtr<MediaSourceBufferPrivateMorphOS>& buffer)
{
	m_paintingBuffer = buffer;
	m_player.accNextFrameReady();

	m_seeking = false;
}

void MediaSourcePrivateMorphOS::onSourceBufferRemoved(RefPtr<MediaSourceBufferPrivateMorphOS>& buffer)
{
	if (m_paintingBuffer == buffer)
		m_paintingBuffer = nullptr;
	m_sourceBuffers.remove(buffer);
	buffer->clearMediaSource();
}

void MediaSourcePrivateMorphOS::onAudioSourceBufferUpdatedPosition(RefPtr<MediaSourceBufferPrivateMorphOS>&, double position)
{
	if (m_paintingBuffer)
		m_paintingBuffer->setAudioPresentationTime(position);
		
	m_seeking = false;
}

}


#endif
