#include "AcinerellaDecoder.h"

#if ENABLE(VIDEO)
#include "acinerella.h"
#include "AcinerellaContainer.h"
#include <proto/exec.h>

#define D(x) 

namespace WebCore {
namespace Acinerella {

AcinerellaDecoder::AcinerellaDecoder(Acinerella *parent, RefPtr<AcinerellaMuxedBuffer> buffer, int index, const ac_stream_info &, bool isLiveStream)
	: m_parent(parent)
	, m_muxer(buffer)
	, m_isLive(isLiveStream)
{
	m_duration = std::max(ac_get_stream_duration(parent->ac(), index), double(parent->ac()->info.duration)/1000.0);
	m_bitrate = parent->ac()->info.bitrate;
	D(dprintf("%s: %p starting thread; duration %lld br %ld\n", __func__, this, parent->ac()->info.duration, parent->ac()->info.bitrate));
	m_thread = Thread::create("Acinerella Decoder", [this] {
		threadEntryPoint();
	});
}

AcinerellaDecoder::~AcinerellaDecoder()
{
	D(dprintf("%s: %p\n", __func__, this));
}

void AcinerellaDecoder::warmUp()
{
	D(dprintf("%s: %p\n", __func__, this));
	dispatch([this]{ decodeUntilBufferFull(); });
}

void AcinerellaDecoder::flushAndWarmUp()
{
	D(dprintf("%s: %p\n", __func__, this));
	dispatch([this]{ flush(); decodeUntilBufferFull(); });
}

void AcinerellaDecoder::play()
{
	D(dprintf("%s: %p\n", __func__, this));
	dispatch([this](){ startPlaying(); });
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
	AcinerellaPackage buffer;
	if (m_muxer->nextPackage(*this, buffer))
	{
		AcinerellaDecodedFrame frame;
		auto *decoder = buffer.acinerella() ? (isAudio() ? buffer.acinerella()->audioDecoder() : buffer.acinerella()->videoDecoder()) : nullptr;

		// used both if acinerella sends us stuff AND in case of discontinuity
		// either way, we must flush caches here!
		if (ac_flush_packet() == buffer.package())
		{
			D(dprintf("%s: got flush packet!\n", __func__));
			
			if (!m_isLive)
			{
				ac_flush_buffers(decoder);
				
				while (!m_decodedFrames.empty())
				{
					m_decodedFrames.pop();
				}
			}

			return true;
		}

		frame = AcinerellaDecodedFrame(buffer.acinerella(), decoder);

		D(dprintf("%s: frame %p package %p decoder %p ac %p frameptr %p buffer %p size %d\n", __func__,
			frame.frame(), buffer.package(), decoder, buffer.acinerella().get(), frame.pointer().get(), frame.frame()->pBuffer, frame.frame()->buffer_size));

		if (buffer.package() && buffer.acinerella() && frame.frame() && decoder)
		{
			if (1 == ac_decode_package_ex(buffer.package(), decoder, frame.frame()))
			{
				auto lock = holdLock(m_lock);
				onFrameDecoded(frame);
				m_decodedFrames.emplace(WTFMove(frame));
//				D(dprintf("%s: decoded frames %d\n", __func__, m_decodedFrames.size()));
				return true;
			}
		}
		else
		{
			return false;
		}
		
		return true;
	}
	
	return false;
}

void AcinerellaDecoder::decodeUntilBufferFull()
{
	while (bufferSize() < readAheadTime())
	{
		if (!decodeNextFrame())
			break;
	}
	
	m_parent->onDecoderReadyToPlay(*this);
}

void AcinerellaDecoder::flush()
{
	auto lock = holdLock(m_lock);

	while (!m_decodedFrames.empty())
		m_decodedFrames.pop();
}

void AcinerellaDecoder::onPositionChanged()
{
	D(dprintf("%s: %p\n", __func__, this));
	m_parent->onDecoderUpdatedPosition(*this, position());
}

void AcinerellaDecoder::terminate()
{
	D(dprintf("%s: %p\n", __func__, this));
	m_terminating = true;
	if (!m_thread)
		return;
	ASSERT(isMainThread());
	ASSERT(!m_queue.killed() && m_thread);

	if (!m_thread)
		return;

	D(dprintf("%s: %p disp..\n", __func__, this));
	m_queue.append(makeUnique<Function<void ()>>([this] {
		performTerminate();
	}));
	m_thread->waitForCompletion();

	D(dprintf("%s: %p completed\n", __func__, this));
	ASSERT(m_queue.killed());
	m_thread = nullptr;
	m_parent = nullptr;
	m_muxer = nullptr;

	D(dprintf("%s: %p done\n", __func__, this));
}

void AcinerellaDecoder::threadEntryPoint()
{
	SetTaskPri(FindTask(0), 5);

	D(dprintf("%s: %p\n", __func__, this));
	if (!onThreadInitialize())
	{
		// TODO: signal failure to parent
	}
	
	while (auto function = m_queue.waitForMessage())
	{
		(*function)();
	}
	
	onThreadShutdown();
}

void AcinerellaDecoder::dispatch(Function<void ()>&& function)
{
	ASSERT(!m_queue.killed() && m_thread);
	m_queue.append(makeUnique<Function<void ()>>(WTFMove(function)));
}

void AcinerellaDecoder::performTerminate()
{
	D(dprintf("%s: %p\n", __func__, this));
	ASSERT(!isMainThread());
	m_queue.kill();
}

}
}

#undef D
#endif
