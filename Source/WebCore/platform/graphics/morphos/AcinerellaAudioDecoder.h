#pragma once

#include "config.h"

#if ENABLE(VIDEO)

#include "AcinerellaDecoder.h"

namespace WebCore {
namespace Acinerella {

class AcinerellaAudioDecoder : public AcinerellaDecoder
{
public:
	AcinerellaAudioDecoder(Acinerella &parent, RefPtr<AcinerellaMuxedBuffer> buffer, int index, const ac_stream_info &info);

	int rate() const { return m_audioRate; }
	int channels() const { return m_audioChannels; }
	int bits() const { return m_audioBits; }
	
	bool isAudio() const override { return true; }
	int warmUpQueueSize() const override { return 3; }

	void startPlaying() override;
	void stopPlaying() override;

	bool isPlaying() const override;
	Seconds position() const override;

protected:
	int     m_audioRate;
	int     m_audioChannels;
	int     m_audioBits;
};

}
}

#endif
