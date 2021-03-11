#include "AcinerellaDecoder.h"

#if ENABLE(VIDEO)
#include "acinerella.h"
#include "AcinerellaContainer.h"
#include "MediaPlayerMorphOS.h"
#include <proto/exec.h>

#define D(x)
#define DNF(x)
#define DI(x)
#define DBF(x) x

// #pragma GCC optimize ("O0")

namespace WebCore {
namespace Acinerella {

AcinerellaDecoder::AcinerellaDecoder(AcinerellaDecoderClient *client, RefPtr<AcinerellaMuxedBuffer> buffer, int index, const ac_stream_info &info, bool isLiveStream)
	: m_client(client)
	, m_muxer(buffer)
	, m_index(index)
	, m_isLive(isLiveStream)
{
	auto ac = client->acinerellaPointer()->instance();
	m_duration = std::max(ac_get_stream_duration(ac, index), double(ac->info.duration)/1000.0);

	// simulated duration of 3 chunks
	if (m_isLive)
		m_duration = 15.f;

	m_bitrate = ac->info.bitrate;
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
	dispatch([this]{ decodeUntilBufferFull(); });
}

void AcinerellaDecoder::play()
{
	D(dprintf("[%s]%s: %p\n", isAudio() ? "A":"V",__func__, this));
	dispatch([this](){
		decodeUntilBufferFull();
		startPlaying();
	});
}

void AcinerellaDecoder::pause()
{
	dispatch([this](){ stopPlaying(); });
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
		auto *decoder = buffer->acinerella() ? buffer->acinerella()->decoder(m_index) : nullptr;

		if (!decoder)
			return false;

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

		frame = AcinerellaDecodedFrame(buffer->acinerella(), decoder);

		DNF(dprintf("[%s]%s: frame %p package %p index %d decoder %p ac %p frameptr %p buffer %p size %d\n", isAudio() ? "A":"V", __func__,
			frame.frame(), buffer->package(), buffer->index(), decoder, buffer->acinerella().get(), frame.pointer().get(), frame.frame()->pBuffer, frame.frame()->buffer_size));

		if (buffer->package() && buffer->acinerella() && frame.frame() && decoder)
		{
			if (1 == ac_decode_package_ex(buffer->package(), decoder, frame.frame()))
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
				return true;
			}
			
			D(dprintf("[%s]%s: failed decoding frame\n", isAudio() ? "A":"V", __func__));
		}
		else
		{
			D(dprintf("[%s]%s: invalid input!\n", isAudio() ? "A":"V", __func__));
			return false;
		}
		
		return true;
	}
	
	return false;
}

void AcinerellaDecoder::decodeUntilBufferFull()
{
	EP_SCOPE(untilBufferFull);

	DBF(dprintf("[%s]%s: %p - start!\n", isAudio() ? "A":"V", __func__, this));

	if (readAheadTime() > 0.f)
	{
		do
		{
			if (!decodeNextFrame())
				break;
		} while (bufferSize() < readAheadTime());
	}
	else
	{
		decodeNextFrame();
	}

	DBF(dprintf("[%s]%s: %p - buffer full\n", isAudio() ? "A":"V", __func__, this));
	m_client->onDecoderReadyToPlay(makeRef(*this));
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
