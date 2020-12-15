#include "AcinerellaDecoder.h"

#if ENABLE(VIDEO)
#include "acinerella.h"
#include "AcinerellaContainer.h"

#define D(x) x

namespace WebCore {
namespace Acinerella {

AcinerellaDecoder::AcinerellaDecoder(Acinerella &parent, RefPtr<AcinerellaMuxedBuffer> buffer, int index, const ac_stream_info &info)
	: m_muxer(buffer)
{
	m_decoder = deleted_unique_ptr<ac_decoder>(ac_create_decoder(parent.ac(), index), [](ac_decoder*decoder){ ac_free_decoder(decoder); });

	D(dprintf("%s: %p valid %d ac %p %d decoder %p\n", __func__, this, isValid(), parent.ac(), index, m_decoder.get()));

	if (isValid())
	{
		D(dprintf("%s: %p starting thread\n", __func__, this));
		m_duration = Seconds(ac_get_stream_duration(parent.ac(), index));
		m_thread = Thread::create("Acinerella Decoder", [this] {
			threadEntryPoint();
		});
	}
}

void AcinerellaDecoder::warmUp()
{
	D(dprintf("%s: %p\n", __func__, this));

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

void AcinerellaDecoder::terminate()
{
	D(dprintf("%s: %p\n", __func__, this));
	m_terminating = true;
	m_muxer = nullptr;
	if (!m_thread)
		return;
	ASSERT(isMainThread());
	ASSERT(!m_queue.killed() && m_thread);

	if (!m_thread)
		return;

	m_queue.append(makeUnique<Function<void ()>>([this] {
		performTerminate();
	}));
	m_thread->waitForCompletion();
	ASSERT(m_queue.killed());
	m_thread = nullptr;

	D(dprintf("%s: %p done\n", __func__, this));
}

void AcinerellaDecoder::threadEntryPoint()
{
	D(dprintf("%s: %p\n", __func__, this));
	while (auto function = m_queue.waitForMessage())
	{
		(*function)();
	}
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
