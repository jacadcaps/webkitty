#include "AcinerellaDecoder.h"

#if ENABLE(VIDEO)
#include "acinerella.h"
#include "AcinerellaContainer.h"
#include "MediaPlayerMorphOS.h"
#include <proto/exec.h>

#define D(x) 
#define DNF(x)
#define DI(x)
#define DBF(x) 

// #pragma GCC optimize ("O0")

namespace WebCore {
namespace Acinerella {

AcinerellaDecoder::AcinerellaDecoder(AcinerellaDecoderClient *client, RefPtr<AcinerellaPointer> acinerella, RefPtr<AcinerellaMuxedBuffer> buffer, int index, const ac_stream_info &info, bool isLiveStream)
	: m_client(client)
	, m_muxer(buffer)
	, m_index(index)
	, m_isLive(isLiveStream)
{
	auto ac = acinerella->instance();
	m_duration = std::max(ac_get_stream_duration(ac, index), double(ac->info.duration)/1000.0);

	// simulated duration of 3 chunks
	if (m_isLive)
		m_duration = 15.f;

	m_bitrate = ac->info.bitrate;
	m_lastDecoder = acinerella->decoder(m_index);
}

AcinerellaDecoder::~AcinerellaDecoder()
{
	DI(dprintf("%s: %p\n", __func__, this));
}

void AcinerellaDecoder::warmUp()
{
	if (!m_terminating && !m_thread)
	{
		DI(dprintf("%s: %p starting thread\n", __func__, this));
		m_thread = Thread::create(isAudio() ? "Acinerella Audio Decoder" : "Acinerella Video Decoder", [this] {
			threadEntryPoint();
		});
	}

	D(dprintf("[%s]%s: %p\n", isAudio() ? "A":"V",__func__, this));
	dispatch([this] {
		decodeUntilBufferFull();
		m_client->onDecoderWarmedUp(makeRef(*this));
	});
}

void AcinerellaDecoder::coolDown()
{
	D(dprintf("[%s]%s: %p\n", isAudio() ? "A":"V",__func__, this));
	dispatch([this] {
		stopPlaying();
		onCoolDown();
	});
}

void AcinerellaDecoder::prePlay()
{
	D(dprintf("[%s]%s: %p\n", isAudio() ? "A":"V",__func__, this));
	dispatch([this](){
		decodeUntilBufferFull();
		onGetReadyToPlay();
		if (isReadyToPlay()) {
			onReadyToPlay();
		}
	});
}

void AcinerellaDecoder::play()
{
	D(dprintf("[%s]%s: %p\n", isAudio() ? "A":"V",__func__, this));
	dispatch([this](){
		decodeUntilBufferFull();
		onGetReadyToPlay();
		if (isReadyToPlay())
		{
			startPlaying();
		}
	});
}

void AcinerellaDecoder::onReadyToPlay()
{
	m_client->onDecoderReadyToPlay(makeRef(*this));
}

void AcinerellaDecoder::pause(bool willSeek)
{
	dispatch([this, willSeek](){
		stopPlaying();
		if (willSeek)
			flush();
	});
}

void AcinerellaDecoder::setVolume(float volume)
{
	if (isAudio())
		dispatch([this, volume](){ doSetVolume(volume); });
}

bool AcinerellaDecoder::decodeNextFrame()
{
	EP_SCOPE(DNF);
	RefPtr<AcinerellaPackage> buffer;

	DNF(dprintf("[%s]%s: this %p\n", isAudio() ? "A":"V", __func__, this));

	if (m_terminating)
		return false;

	if ((buffer = m_muxer->nextPackage(*this)))
	{
		AcinerellaDecodedFrame frame;
		auto acinerella = buffer->acinerella();
		auto *decoder = !!acinerella ? acinerella->decoder(m_index) : nullptr;

		if (!decoder)
			return false;

		if (m_lastDecoder != decoder)
		{
			m_lastDecoder = decoder;
			onDecoderChanged(acinerella);
		}

		// used both if acinerella sends us stuff AND in case of discontinuity
		// either way, we must flush caches here!
		if (buffer->isFlushPackage())
		{
			DNF(dprintf("[%s]%s: got flush packet! (live %d)\n", isAudio() ? "A":"V", __func__, m_isLive));
			
			if (!m_isLive)
			{
				ac_flush_buffers(decoder);
				flush();
			}

			return true;
		}

		if (buffer->package())
		{
			double pts = ac_get_package_pts(acinerella->instance(), buffer->package());

			if (m_droppingFrames)
			{
dprintf(">> pts %f drop %f\n", float(pts), float(m_dropToPTS));
				if (pts < m_dropToPTS)
				{
					return true;
				}
				else
				{
					m_droppingUntilKeyFrame = true;
					m_droppingFrames = false;
					return true;
				}
			}
			else if (m_droppingUntilKeyFrame)
			{
dprintf("waitkf %f\n", float(pts));
				if (ac_get_package_keyframe(buffer->package()))
				{
dprintf("got Kf! %f\n", float(pts));
					m_droppingUntilKeyFrame = false;
				}
				else
				{
					return true;
				}
			}
		}

		auto rcPush = ac_push_package(decoder, buffer->package());
		if (rcPush != PUSH_PACKAGE_SUCCESS)
			return false;

		for (;;)
		{
			AcinerellaDecodedFrame frame = AcinerellaDecodedFrame(acinerella, decoder);
			auto rcFrame = ac_receive_frame(decoder, frame.frame());
			
			switch (rcFrame)
			{
			case RECEIVE_FRAME_SUCCESS:
				{
#if defined(EP_PROFILING) && EP_PROFILING
					{
						char buffer[128];
						sprintf(buffer, "frame TS %f", float(frame.frame()->timecode));
						EP_EVENTSTR(buffer);
					}
#endif
					auto lock = holdLock(m_lock);
					onFrameDecoded(frame);
					DNF(dprintf("[%s]%s: decoded frame @ %f\n", isAudio() ? "A":"V", __func__, float(frame.frame()->timecode)));
					m_decodedFrames.emplace(WTFMove(frame));
				}
				break;
			case RECEIVE_FRAME_NEED_PACKET:
				// we'll have to call decodeNextFrame again
				return true;
			case RECEIVE_FRAME_ERROR:
				return false;
			case RECEIVE_FRAME_EOF:
				m_decoderEOF = true;
				return false;
			}
		}
	}

	return false;
}

void AcinerellaDecoder::decodeUntilBufferFull()
{
	EP_SCOPE(untilBufferFull);

	DBF(dprintf("[%s]%s: %p - start!\n", isAudio() ? "A":"V", __func__, this));

	do
	{
		if (!decodeNextFrame())
			break;
	} while (bufferSize() < readAheadTime());

	DBF(dprintf("[%s]%s: %p - buffer full\n", isAudio() ? "A":"V", __func__, this));
}

void AcinerellaDecoder::dropUntilPTS(double pts)
{
	EP_SCOPE(untilBufferFull);

	DBF(dprintf("[%s]%s: %p - start!\n", isAudio() ? "A":"V", __func__, this));

	m_droppingFrames = true;
	m_droppingUntilKeyFrame = false;
	m_dropToPTS = pts;

	while (!m_terminating && (m_droppingFrames || m_droppingUntilKeyFrame))
		decodeNextFrame();

	decodeUntilBufferFull();
}

void AcinerellaDecoder::flush()
{
	auto lock = holdLock(m_lock);

	while (!m_decodedFrames.empty())
		m_decodedFrames.pop();
}

void AcinerellaDecoder::onPositionChanged()
{
#if defined(EP_PROFILING) && EP_PROFILING
	{
		char buffer[128];
		sprintf(buffer, "position %f", float(position()));
		EP_EVENTSTR(buffer);
	}
#endif
	D(dprintf("[%s]%s: %p to %f\n", isAudio() ? "A":"V", __func__, this, position()));
	m_client->onDecoderUpdatedPosition(makeRef(*this), position());
}

void AcinerellaDecoder::onDurationChanged()
{
	D(dprintf("[%s]%s: %p to %f\n", isAudio() ? "A":"V", __func__, this, duration()));
	m_client->onDecoderUpdatedDuration(makeRef(*this), duration());
}

void AcinerellaDecoder::onEnded()
{
	EP_EVENT(ended);
	D(dprintf("[%s]%s: %p\n", isAudio() ? "A":"V", __func__, this));
	m_client->onDecoderEnded(makeRef(*this));
}

void AcinerellaDecoder::terminate()
{
	EP_SCOPE(terminate);

	DI(dprintf("[%s]%s: %p\n", isAudio() ? "A":"V", __func__, this));
	m_terminating = true;
	if (!m_thread)
		return;

	DI(dprintf("[%s]%s: %p disp..\n", isAudio() ? "A":"V", __func__, this));
	m_queue.append(makeUnique<Function<void ()>>([this] {
		performTerminate();
	}));
	m_thread->waitForCompletion();

	DI(dprintf("[%s]%s: %p completed\n", isAudio() ? "A":"V", __func__, this));
	ASSERT(m_queue.killed());
	m_thread = nullptr;
	m_client = nullptr;
	m_muxer = nullptr;

	DI(dprintf("[%s]%s: %p done\n", isAudio() ? "A":"V", __func__, this));
}

void AcinerellaDecoder::threadEntryPoint()
{
	SetTaskPri(FindTask(0), 3);

	RefPtr<AcinerellaDecoder> refSelf = WTF::makeRef(*this);

	DI(dprintf("[%s]%s: %p\n", isAudio() ? "A":"V", __func__, this));
	if (!onThreadInitialize())
	{
		// TODO: signal failure to parent
	}
	
	while (auto function = m_queue.waitForMessage())
	{
		(*function)();
	}
	
	DI(dprintf("[%s]%s: %p .. shutting down...\n", isAudio() ? "A":"V", __func__, this));
	onThreadShutdown();
	DI(dprintf("[%s]%s: %p onThreadShutdown done\n", isAudio() ? "A":"V", __func__, this));
}

void AcinerellaDecoder::dispatch(Function<void ()>&& function)
{
	ASSERT(!m_queue.killed() && m_thread);
	m_queue.append(makeUnique<Function<void ()>>(WTFMove(function)));
}

void AcinerellaDecoder::performTerminate()
{
	DI(dprintf("[%s]%s: %p\n", isAudio() ? "A":"V", __func__, this));
	ASSERT(!isMainThread());
	m_queue.kill();
}

}
}

#undef D
#endif
