#pragma once

#include "config.h"

#if ENABLE(VIDEO)

#include "AcinerellaDecoder.h"
struct VLayerHandle;
struct Window;

namespace WebCore {
namespace Acinerella {

class AcinerellaVideoDecoder : public AcinerellaDecoder
{
	AcinerellaVideoDecoder(AcinerellaDecoderClient* client, RefPtr<AcinerellaMuxedBuffer> buffer, int index, const ac_stream_info &info, bool isLive);
public:
	static RefPtr<AcinerellaVideoDecoder> create(AcinerellaDecoderClient* client, RefPtr<AcinerellaMuxedBuffer> buffer, int index, const ac_stream_info &info, bool isLive)
	{
		return WTF::adoptRef(*new AcinerellaVideoDecoder(client, buffer, index, info, isLive));
	}

	~AcinerellaVideoDecoder();
	
	void setFakeDecode(bool fakeDecode) { m_fakeDecode = fakeDecode; }

	bool isAudio() const override { return false; }
	bool isVideo() const override { return true; }
	bool isText() const override { return false; }
	
	float readAheadTime() const override { return -1.f; }
	
	float framesPerSecond() const { return m_fps; }

	bool isReadyToPlay() const override;

	bool isPlaying() const override;
	float position() const override;
	float bufferSize() const override { return m_bufferedSeconds; }
	void paint(GraphicsContext&, const FloatRect&) override;

	void setOverlayWindowCoords(struct ::Window *w, int scrollx, int scrolly, int mleft, int mtop, int mright, int mbottom);

	int frameWidth() const { return m_frameWidth; }
	int frameHeight() const { return m_frameHeight; }
	
	void setAudioPresentationTime(float apts) { m_audioPosition = apts; }

protected:
	void startPlaying() override;
	void stopPlaying() override;

	void onThreadShutdown() override;
	void onFrameDecoded(const AcinerellaDecodedFrame &frame) override;
	void flush() override;

	void pullThreadEntryPoint();
	void blitFrameLocked();
	
	void updateOverlayCoords();

protected:
	RefPtr<Thread>  m_pullThread;
	BinarySemaphore m_pullEvent;
	
	uint32_t        m_bufferedSamples = 0;
	volatile float  m_bufferedSeconds = 0.f;
	volatile bool   m_playing = false;
	int             m_frameWidth;
	int             m_frameHeight;
	float           m_position = 0.f;
	float           m_fps;
	float           m_frameDuration;
	float           m_audioPosition = 0.f;
	
	Seconds         m_accumulatedDecodingTime;
	int             m_accumulatedDecodingCount = 0;
	
	Seconds         m_accumulatedCairoTime;
	int             m_accumulatedCairoCount = 0;
	
	bool            m_fakeDecode = false;
	
	int             m_paintX, m_paintY, m_paintX2 = 0, m_paintY2;
	int             m_outerX, m_outerY, m_outerX2 = 0, m_outerY2;
	
	uint32_t               m_overlayFillColor = 0;
	struct ::VLayerHandle *m_overlayHandle = nullptr;
	struct ::Window       *m_overlayWindow = nullptr;
};

}
}

#endif

