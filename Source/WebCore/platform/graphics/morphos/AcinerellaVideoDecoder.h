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
	AcinerellaVideoDecoder(AcinerellaDecoderClient* client, RefPtr<AcinerellaPointer> acinerella, RefPtr<AcinerellaMuxedBuffer> buffer, int index, const ac_stream_info &info, bool isLive);
public:
	static RefPtr<AcinerellaVideoDecoder> create(AcinerellaDecoderClient* client, RefPtr<AcinerellaPointer> acinerella, RefPtr<AcinerellaMuxedBuffer> buffer, int index, const ac_stream_info &info, bool isLive)
	{
		return WTF::adoptRef(*new AcinerellaVideoDecoder(client, acinerella, buffer, index, info, isLive));
	}

	~AcinerellaVideoDecoder();
	
	void setFakeDecode(bool fakeDecode) { m_fakeDecode = fakeDecode; }

	bool isAudio() const override { return false; }
	bool isVideo() const override { return true; }
	bool isText() const override { return false; }
	
	double readAheadTime() const override { return 1.5f; }
	
	double framesPerSecond() const { return m_fps; }

	bool isReadyToPlay() const override;

	bool isPlaying() const override;
	double position() const override;
	double bufferSize() const override { return m_bufferedSeconds; }
	void paint(GraphicsContext&, const FloatRect&) override;

	void setOverlayWindowCoords(struct ::Window *w, int scrollx, int scrolly, int mleft, int mtop, int mright, int mbottom, int width, int height);

	int frameWidth() const { return m_frameWidth; }
	int frameHeight() const { return m_frameHeight; }
	
	void setAudioPresentationTime(double apts);
	void setCanDropKeyFrames(bool canDropKeyFrames) { m_canDropKeyFrames = canDropKeyFrames; }

protected:
	void startPlaying() override;
	void stopPlaying() override;
	void onGetReadyToPlay() override;

	void onThreadShutdown() override;
	void onFrameDecoded(const AcinerellaDecodedFrame &frame) override;
	void onDecoderChanged(RefPtr<AcinerellaPointer> acinerella) override;
	void flush() override;
	void onCoolDown() override;

	void pullThreadEntryPoint();
	void blitFrameLocked();
	
	void updateOverlayCoords();

	bool getAudioPresentationTime(double &time);

protected:
	RefPtr<Thread>  m_pullThread;
	BinarySemaphore m_pullEvent;
	BinarySemaphore m_frameEvent;
	
	uint32_t        m_bufferedSamples = 0;
	volatile float  m_bufferedSeconds = 0.f;
	volatile bool   m_playing = false;
	bool            m_waitingToPlay = false;
	int             m_frameWidth;
	int             m_frameHeight;
	double          m_position = 0.f;
	double          m_fps;
	double          m_frameDuration;

	Lock            m_audioLock;
	double          m_audioPosition = 0.0;
	MonotonicTime   m_audioPositionRealTime;
	bool            m_hasAudioPosition = false;
	
	Seconds         m_accumulatedDecodingTime;
	int             m_accumulatedDecodingCount = 0;
	
	Seconds         m_accumulatedCairoTime;
	int             m_accumulatedCairoCount = 0;
	
	bool            m_fakeDecode = false;
	bool            m_canDropKeyFrames = false;
	
	int             m_paintX, m_paintY, m_paintX2 = 0, m_paintY2;
	int             m_outerX, m_outerY, m_outerX2 = 0, m_outerY2;
	int             m_windowWidth, m_windowHeight;
	int             m_visibleWidth, m_visibleHeight;
	
	uint32_t               m_overlayFillColor = 0;
	struct ::VLayerHandle *m_overlayHandle = nullptr;
	struct ::Window       *m_overlayWindow = nullptr;
};

}
}

#endif

