#include "MediaSourcePrivateMorphOS.h"

#if ENABLE(VIDEO) && ENABLE(MEDIA_SOURCE)

#include "ContentType.h"
#include "MediaSourcePrivateClient.h"
#include "MediaPlayerPrivateMorphOS.h"

#define D(x) 
// #pragma GCC optimize ("O0")

namespace WebCore {

Ref<MediaSourcePrivateMorphOS> MediaSourcePrivateMorphOS::create(MediaPlayerPrivateMorphOS& parent, MediaSourcePrivateClient& client, const String &url)
{
    auto source = adoptRef(*new MediaSourcePrivateMorphOS(parent, client, url));
    client.setPrivateAndOpen(source.copyRef());
    return source;
}

MediaSourcePrivateMorphOS::MediaSourcePrivateMorphOS(MediaPlayerPrivateMorphOS& parent, MediaSourcePrivateClient& client, const String &url)
    : m_player(makeWeakPtr(parent))
    , m_client(client)
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
	m_url = url.substring(5);
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

    MediaEngineSupportParameters parameters;
    parameters.isMediaSource = true;
    parameters.type = contentType;
    if (MediaPlayerPrivateMorphOS::extendedSupportsType(parameters, MediaPlayer::SupportsType::MayBeSupported) == MediaPlayer::SupportsType::IsNotSupported)
        return NotSupported;

	buffer = MediaSourceBufferPrivateMorphOS::create(this);
	RefPtr<MediaSourceBufferPrivateMorphOS> sourceBufferPrivate = static_cast<MediaSourceBufferPrivateMorphOS*>(buffer.get());
	m_sourceBuffers.add(sourceBufferPrivate);
	return Ok;
}

void MediaSourcePrivateMorphOS::onSourceBufferRemoved(RefPtr<MediaSourceBufferPrivateMorphOS>& buffer)
{
	if (m_paintingBuffer == buffer)
		m_paintingBuffer = nullptr;
	m_sourceBuffers.remove(buffer);
	m_activeSourceBuffers.remove(buffer);
	buffer->clearMediaSource();
	if (m_player)
		m_player->notifyActiveSourceBuffersChanged();
}

void MediaSourcePrivateMorphOS::durationChanged()
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
    if (m_player)
		m_player->accSetDuration(duration().toDouble());
}

void MediaSourcePrivateMorphOS::markEndOfStream(EndOfStreamStatus status)
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
//    if (status == EosNoError)
 //       m_player.setNetworkState(MediaPlayer::NetworkState::Loaded);
}

void MediaSourcePrivateMorphOS::unmarkEndOfStream()
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
}

MediaPlayer::ReadyState MediaSourcePrivateMorphOS::readyState() const
{
//	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
	if (m_player)
		return m_player->readyState();
    return m_readyState;
}

void MediaSourcePrivateMorphOS::setReadyState(MediaPlayer::ReadyState rs)
{
//	D(dprintf("%s: %d\n", __PRETTY_FUNCTION__, int(rs)));
	m_readyState = rs;
	if (m_player)
		m_player->accSetReadyState(rs);
}

void MediaSourcePrivateMorphOS::waitForSeekCompleted()
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
	warmUp();
}

void MediaSourcePrivateMorphOS::onSourceBufferLoadingProgressed()
{
	WTF::callOnMainThread([this, protect = makeRef(*this)]() {
		D(dprintf("[MS]onSourceBufferLoadingProgressed: \n"));
		setReadyState(MediaPlayer::ReadyState::HaveCurrentData);

		if (m_player)
			m_player->setLoadingProgresssed(true);
	});
}

MediaTime MediaSourcePrivateMorphOS::duration()
{
	return m_client->duration();
}

std::unique_ptr<PlatformTimeRanges> MediaSourcePrivateMorphOS::buffered()
{
	return m_client->buffered();
}

void MediaSourcePrivateMorphOS::setVolume(double vol)
{
    if (vol != m_volume)
    {
        m_volume = vol;
        
        for (auto& sourceBufferPrivate : m_activeSourceBuffers)
            sourceBufferPrivate->setVolume(vol);
    }
}

void MediaSourcePrivateMorphOS::setMuted(bool muted)
{
    if (muted != m_muted)
    {
        m_muted = muted;
        
        for (auto& sourceBufferPrivate : m_activeSourceBuffers)
            sourceBufferPrivate->setVolume(muted ? 0 : m_volume);
    }
}

void MediaSourcePrivateMorphOS::seekCompleted()
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
	m_paused = false;
}

void MediaSourcePrivateMorphOS::orphan()
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
	m_orphaned = true;
	m_paintingBuffer = nullptr;
	m_player.clear();
}

void MediaSourcePrivateMorphOS::warmUp()
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
	for (auto& sourceBufferPrivate : m_activeSourceBuffers)
		sourceBufferPrivate->warmUp();

	setReadyState(MediaPlayer::ReadyState::HaveCurrentData);
	setReadyState(MediaPlayer::ReadyState::HaveFutureData);
}

void MediaSourcePrivateMorphOS::coolDown()
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
	for (auto& sourceBufferPrivate : m_sourceBuffers)
		sourceBufferPrivate->coolDown();
}

void MediaSourcePrivateMorphOS::play()
{
	m_paused = false;

	if (areDecodersReadyToPlay())
	{
		D(dprintf("%s: decoders ready!\n", __PRETTY_FUNCTION__));
		m_waitReady = false;

		for (auto& sourceBufferPrivate : m_activeSourceBuffers)
			sourceBufferPrivate->play();
	}
	else
	{
		D(dprintf("%s: prePlay...\n", __PRETTY_FUNCTION__));
		m_waitReady = true;
		for (auto& sourceBufferPrivate : m_activeSourceBuffers)
			sourceBufferPrivate->prePlay();
	}
}

void MediaSourcePrivateMorphOS::pause()
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
	for (auto& sourceBufferPrivate : m_activeSourceBuffers)
		sourceBufferPrivate->pause();
	m_paused = true;
	m_waitReady = false;
}

void MediaSourcePrivateMorphOS::seek(double time)
{
	D(dprintf("%s: %f\n", __PRETTY_FUNCTION__, float(time)));
	if (m_seeking)
		return;
	
	m_seeking = true;
	m_waitReady = true;

    for (auto& sourceBufferPrivate : m_activeSourceBuffers) {
        sourceBufferPrivate->willSeek(time);
	}

	m_client->seekToTime(MediaTime::createWithDouble(time));
}

void MediaSourcePrivateMorphOS::paint(GraphicsContext& gc, const FloatRect& rect)
{
	if (!!m_paintingBuffer)
		m_paintingBuffer->paint(gc, rect);
}

void MediaSourcePrivateMorphOS::setOverlayWindowCoords(struct ::Window *w, int scrollx, int scrolly, int mleft, int mtop, int mright, int mbottom, int width, int height)
{
	if (!!m_paintingBuffer)
		m_paintingBuffer->setOverlayWindowCoords(w, scrollx, scrolly, mleft, mtop, mright, mbottom, width, height);
}

void MediaSourcePrivateMorphOS::onSourceBufferInitialized(RefPtr<MediaSourceBufferPrivateMorphOS> &source)
{
	D(dprintf("%s: allinitialized %d\n", __PRETTY_FUNCTION__, areDecodersInitialized()));
	if (areDecodersInitialized())
	{
		WTF::callOnMainThread([this, protect = makeRef(*this)]() {

			if (!m_initialized)
			{
				MediaPlayerMorphOSInfo info;

				for (auto& sourceBufferPrivate : m_activeSourceBuffers) {
					auto &minfo = sourceBufferPrivate->info();

					if (minfo.m_width) {
						info.m_width = minfo.m_width;
						info.m_height = minfo.m_height;
                        m_hasVideo = true;
					}
					
					if (minfo.m_channels) {
						info.m_channels = minfo.m_channels;
						info.m_bits = minfo.m_bits;
						info.m_frequency = minfo.m_frequency;
                        m_hasAudio = true;
					}
					
					info.m_duration = duration().toFloat(); //! client provides us with the actual duration!
                    info.m_isDownloadable = false;
				}

				D(dprintf("%s: freq %d w %d h %d duration %f clientDuration %f\n", __PRETTY_FUNCTION__, info.m_frequency, info.m_width, info.m_height, float(info.m_duration), float(duration().toDouble())));
				m_initialized = true;
				if (m_player)
					m_player->accInitialized(info);
			}
		});
	}
}

void MediaSourcePrivateMorphOS::onSourceBufferReadyToPaint(RefPtr<MediaSourceBufferPrivateMorphOS>& buffer)
{
	m_paintingBuffer = buffer;
	if (m_player)
		m_player->accNextFrameReady();

	m_seeking = false;
}

void MediaSourcePrivateMorphOS::onSourceBuffersReadyToPlay()
{
	D(dprintf("%s: \n", __PRETTY_FUNCTION__));
	if (m_waitReady && areDecodersReadyToPlay())
	{
		play();
	}
}

void MediaSourcePrivateMorphOS::onSourceBufferFrameUpdate(RefPtr<MediaSourceBufferPrivateMorphOS>& buffer)
{
	if (m_paintingBuffer == buffer && m_player)
	{
		m_player->accFrameUpdateNeeded();
	}
}


void MediaSourcePrivateMorphOS::onAudioSourceBufferUpdatedPosition(RefPtr<MediaSourceBufferPrivateMorphOS>&, double position)
{
	if (m_orphaned)
		return;

	if (m_paintingBuffer)
		m_paintingBuffer->setAudioPresentationTime(position);

	if (position > (duration().toDouble() - 5.0))
	{
		// hack, we can only try and simulate an EOS like this :|
	    for (auto& sourceBufferPrivate : m_activeSourceBuffers)
		{
			sourceBufferPrivate->signalEOF();
		}
	}
	
	WTF::callOnMainThread([position, this, protect = makeRef(*this)]() {
		if (m_player)
			m_player->accSetPosition(position);
	});

	m_seeking = false;
}

bool MediaSourcePrivateMorphOS::areDecodersReadyToPlay()
{
    for (auto& sourceBufferPrivate : m_activeSourceBuffers)
	{
		if (!sourceBufferPrivate->areDecodersReadyToPlay())
			return false;
	}

	return true;
}

bool MediaSourcePrivateMorphOS::areDecodersInitialized()
{
    for (auto& sourceBufferPrivate : m_sourceBuffers)
	{
		if (!sourceBufferPrivate->isInitialized())
			return false;
	}

	return true;
}

void MediaSourcePrivateMorphOS::onSourceBufferDidChangeActiveState(RefPtr<MediaSourceBufferPrivateMorphOS>& buffer, bool active)
{
	D(dprintf("%s: source %p active %d total active %d total %d\n", __PRETTY_FUNCTION__, buffer.get(), active, m_activeSourceBuffers.size(), m_sourceBuffers.size()));
    if (active && !m_activeSourceBuffers.contains(buffer))
    {
        m_activeSourceBuffers.add(buffer);
        if (m_player)
			m_player->onActiveSourceBuffersChanged();
        durationChanged();
        if (!m_paused)
            buffer->play();
    }
    else if (!active && m_activeSourceBuffers.contains(buffer))
    {
		if (m_paintingBuffer == buffer)
		{
			m_paintingBuffer->setOverlayWindowCoords(nullptr, 0, 0, 0, 0, 0, 0, 0, 0);
			m_paintingBuffer = nullptr;
		}
    
        buffer->coolDown();
    
		m_activeSourceBuffers.remove(buffer);
        if (m_player)
			m_player->onActiveSourceBuffersChanged();
    }
}

void MediaSourcePrivateMorphOS::onSourceBufferEnded(RefPtr<MediaSourceBufferPrivateMorphOS>&)
{
	m_paused = true;
	m_ended = true;
	if (m_player)
		m_player->accEnded();
}

}


#endif
