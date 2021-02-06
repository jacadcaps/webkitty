#pragma once

#include "config.h"

#if ENABLE(VIDEO)

#include "AcinerellaDecoder.h"
#include <devices/ahi.h>
#include <exec/libraries.h>

namespace WebCore {
namespace Acinerella {

class AcinerellaAudioDecoder : public AcinerellaDecoder
{
public:
	AcinerellaAudioDecoder(AcinerellaDecoderClient* client, RefPtr<AcinerellaMuxedBuffer> buffer, int index, const ac_stream_info &info, bool isLive);

	int rate() const { return m_audioRate; }
	int channels() const { return m_audioChannels; }
	int bits() const { return m_audioBits; }
	
	bool isAudio() const override { return true; }
	bool isVideo() const override { return false; }
	bool isText() const override { return false; }
	
	float readAheadTime() const override { return m_isLive ? 15.f : 5.f; }

	bool isReadyToPlay() const override;

	bool isPlaying() const override;
	float position() const override;
	float bufferSize() const override { return m_bufferedSeconds; }

protected:
	void startPlaying() override;
	void stopPlaying() override;
	void doSetVolume(float volume) override;

	void onThreadShutdown() override;
	void onFrameDecoded(const AcinerellaDecodedFrame &frame) override;
	void flush() override;
	bool initializeAudio();

	static void soundFunc();
	void fillBuffer(int index);
	void ahiThreadEntryPoint();

protected:
	Library        *m_ahiBase = nullptr;
	MsgPort        *m_ahiPort = nullptr;
	AHIRequest     *m_ahiIO = nullptr;
	AHIAudioCtrl   *m_ahiControl = nullptr;
	AHISampleInfo   m_ahiSample[2];
	float           m_ahiSampleTimestamp[2];
	uint32_t        m_ahiSampleLength; // *2 for bytes
	uint32_t        m_ahiSampleBeingPlayed;
	uint32_t        m_ahiFrameOffset; //
	BinarySemaphore m_ahiSampleConsumed;
	RefPtr<Thread>  m_ahiThread;
	bool            m_ahiThreadShuttingDown = false;

	uint32_t        m_bufferedSamples = 0;
	volatile float  m_bufferedSeconds = 0.f;
	volatile bool   m_playing = false;
	float           m_position = 0.f;
	int             m_audioRate;
	int             m_audioChannels;
	int             m_audioBits;
};

}
}

#endif
