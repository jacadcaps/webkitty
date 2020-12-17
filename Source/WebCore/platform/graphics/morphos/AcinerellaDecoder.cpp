#include "AcinerellaDecoder.h"

#if ENABLE(VIDEO)
#include "acinerella.h"
#include "AcinerellaContainer.h"

#define D(x) x

namespace WebCore {
namespace Acinerella {

AcinerellaDecoder::AcinerellaDecoder(Acinerella *parent, RefPtr<AcinerellaMuxedBuffer> buffer, int index, const ac_stream_info &info)
	: m_parent(parent)
	, m_muxer(buffer)
{
	m_decoder = deleted_unique_ptr<ac_decoder>(ac_create_decoder(parent->ac(), index), [](ac_decoder*decoder){ ac_free_decoder(decoder); });

	D(dprintf("%s: %p valid %d ac %p %d decoder %p\n", __func__, this, isValid(), parent->ac(), index, m_decoder.get()));

	if (isValid())
	{
		m_duration = std::max(ac_get_stream_duration(parent->ac(), index), double(parent->ac()->info.duration)/1000.0);
		m_bitrate = parent->ac()->info.bitrate;
		D(dprintf("%s: %p starting thread; duration %lld br %ld\n", __func__, this, parent->ac()->info.duration, parent->ac()->info.bitrate));
		m_thread = Thread::create("Acinerella Decoder", [this] {
			threadEntryPoint();
		});
	}
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

void AcinerellaDecoder::play()
{
	D(dprintf("%s: %p\n", __func__, this));
	dispatch([this](){ startPlaying(); });
}

void AcinerellaDecoder::pause()
{
	dispatch([this](){ stopPlaying(); });
}

bool AcinerellaDecoder::decodeNextFrame()
{
	D(dprintf("%s: get package..\n", __func__));
	AcinerellaPackage buffer;
	if (m_muxer->nextPackage(*this, buffer))
	{
// TODO: handle flushing here!!
		AcinerellaDecodedFrame frame;
		
		{
			auto lock = holdLock(m_lock);
			if (!m_freeFrames.empty())
			{
				// move-assign (copies the pointer and puts a nullptr on the freeFrames queue's front obj)
				frame = WTFMove(m_freeFrames.front());
				// D(dprintf("reused frame %p, move ok? %p\n", frame.frame(), m_freeFrames.front().frame()));
				m_freeFrames.pop();
			}
			else
			{
				frame = AcinerellaDecodedFrame(m_decoder.get());
			}
		}
		
		if (frame.frame())
		{
			D(dprintf("%s: got frame!\n", __func__));
			if (1 == ac_decode_package_ex(buffer.package(), m_decoder.get(), frame.frame()))
			{
				D(dprintf("%s: decoded frame\n", __func__));
				auto lock = holdLock(m_lock);
				onFrameDecoded(frame);
				m_decodedFrames.emplace(WTFMove(frame));
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
	float position = 0;

	{
		auto lock = holdLock(m_lock);
		if (!m_decodedFrames.empty())
			position = m_decodedFrames.front().frame()->timecode;
	}

	m_parent->onDecoderUpdatedPosition(*this, position);
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
