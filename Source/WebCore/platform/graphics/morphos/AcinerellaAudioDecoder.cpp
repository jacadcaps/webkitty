#include "AcinerellaAudioDecoder.h"

#if ENABLE(VIDEO)
namespace WebCore {
namespace Acinerella {

#define D(x) x

AcinerellaAudioDecoder::AcinerellaAudioDecoder(Acinerella &parent, RefPtr<AcinerellaMuxedBuffer> buffer, int index, const ac_stream_info &info)
	: AcinerellaDecoder(parent, buffer, index, info)
	, m_audioRate(info.additional_info.audio_info.samples_per_second)
	, m_audioChannels(info.additional_info.audio_info.channel_count)
	, m_audioBits(info.additional_info.audio_info.bit_depth)
{
	D(dprintf("%s: %p\n", __func__, this));
}

void AcinerellaAudioDecoder::startPlaying()
{
	D(dprintf("%s:\n", __func__));
	RefPtr<AcinerellaMuxedBuffer> muxer(m_muxer);
	if (muxer)
	{
		D(dprintf("%s: calling nextPackage...\n", __func__));
		AcinerellaPackage buffer;
		if (muxer->nextPackage(*this, buffer))
		{
	D(dprintf("%s: got package!\n", __func__));

		}
	}
}

void AcinerellaAudioDecoder::stopPlaying()
{

}

bool AcinerellaAudioDecoder::isPlaying() const
{
	return false;
}

Seconds AcinerellaAudioDecoder::position() const
{
	return 0_s;
}

}
}

#undef D
#endif
