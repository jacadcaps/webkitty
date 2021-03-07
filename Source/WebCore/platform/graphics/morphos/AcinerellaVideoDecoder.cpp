#include "AcinerellaVideoDecoder.h"
#include "AcinerellaContainer.h"

#if ENABLE(VIDEO)

#include <WebCore/GraphicsContext.h>
#include <WebCore/PlatformContextCairo.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include <cairo.h>

#include <proto/intuition.h>
#include <intuition/intuition.h>
#include <proto/cybergraphics.h>
#include <cybergraphx/cybergraphics.h>
#include <cybergraphx/cgxvideo.h>
#include <proto/cgxvideo.h>
#include <graphics/rpattr.h>
#include <proto/graphics.h>

#define D(x) x
#define DSYNC(x) x

// #pragma GCC optimize ("O0")


namespace WebCore {
namespace Acinerella {

AcinerellaVideoDecoder::AcinerellaVideoDecoder(AcinerellaDecoderClient* client, RefPtr<AcinerellaMuxedBuffer> buffer, int index, const ac_stream_info &info, bool isLiveStream)
	: AcinerellaDecoder(client, buffer, index, info, isLiveStream)
{
	m_fps = info.additional_info.video_info.frames_per_second;
	m_frameDuration = 1.f / m_fps;
	m_frameWidth = info.additional_info.video_info.frame_width;
	m_frameHeight = info.additional_info.video_info.frame_height;
	D(dprintf("%s: %p fps %f\n", __func__, this, m_fps));
	
	auto decoder = client->acinerellaPointer()->decoder(index);
	ac_set_output_format(decoder, AC_OUTPUT_YUV420P);
	
	m_pullThread = Thread::create("Acinerella Video Pump", [this] {
		pullThreadEntryPoint();
	});
	
}

AcinerellaVideoDecoder::~AcinerellaVideoDecoder()
{
	D(dprintf("%s: %p\n", __func__, this));
	
	if (!!m_pullThread)
	{
		m_pullEvent.signal();
		m_pullThread->waitForCompletion();
		m_pullThread = nullptr;
	}

	if (m_overlayHandle)
	{
		DetachVLayer(m_overlayHandle);
		DeleteVLayerHandle(m_overlayHandle);
	}
}

bool AcinerellaVideoDecoder::isReadyToPlay() const
{
	return true;
}

bool AcinerellaVideoDecoder::isPlaying() const
{
	return m_playing;
}

float AcinerellaVideoDecoder::position() const
{
	return 0.5f;
}

void AcinerellaVideoDecoder::startPlaying()
{
	D(dprintf("%s: %p\n", __func__, this));
	m_playing = true;
	onPositionChanged();
	m_pullEvent.signal();
	m_client->onDecoderReadyToPaint(makeRef(*this));
}

void AcinerellaVideoDecoder::stopPlaying()
{
	D(dprintf("%s: %p\n", __func__, this));
	m_playing = false;
	m_pullEvent.signal();
}

void AcinerellaVideoDecoder::onThreadShutdown()
{
	D(dprintf("%s: %p\n", __func__, this));
	m_pullEvent.signal();
	m_pullEvent.signal();
	m_pullThread->waitForCompletion();
	m_pullThread = nullptr;
	D(dprintf("%s: %p done\n", __func__, this));
}

void AcinerellaVideoDecoder::onFrameDecoded(const AcinerellaDecodedFrame &frame)
{
	D(dprintf("%s: %p\n", __func__, this));
	m_bufferedSeconds += m_frameDuration;
}

void AcinerellaVideoDecoder::flush()
{

}

void AcinerellaVideoDecoder::setOverlayWindowCoords(struct ::Window *w, int scrollx, int scrolly, int mleft, int mtop, int mright, int mbottom)
{
	{
		auto lock = holdLock(m_lock);
		
		D(dprintf("%s: window %p\n", __func__, w));
		
		if (m_overlayWindow != w)
		{
			m_overlayWindow = w;
			
			if (m_overlayHandle)
			{
				DetachVLayer(m_overlayHandle);
				DeleteVLayerHandle(m_overlayHandle);
				m_overlayHandle = nullptr;
			}
			
			if (m_overlayWindow && !m_terminating)
			{
				m_overlayHandle = CreateVLayerHandleTags(m_overlayWindow->WScreen,
					VOA_SrcType, SRCFMT_YCbCr420,
					VOA_UseColorKey, TRUE, VOA_UseBackfill, FALSE, VOA_SrcWidth, m_frameWidth,
					VOA_SrcHeight, m_frameHeight, VOA_DoubleBuffer, TRUE,
					TAG_DONE);
			
				if (m_overlayHandle)
				{
					AttachVLayerTags(m_overlayHandle, m_overlayWindow, VOA_ColorKeyFill, FALSE, TAG_DONE);
					m_overlayFillColor = GetVLayerAttr(m_overlayHandle, VOA_ColorKey);
					D(dprintf("%s: fill %08lx\n", __func__, m_overlayFillColor));
					m_pullEvent.signal();
				}
				else
				{
					D(dprintf("%s: failed creating vlayer for size %d %d\n", __func__, m_overlayFillColor, m_frameWidth, m_frameHeight));
				}
			}
		}
	}

	m_outerX = mleft;
	m_outerY = mtop;
	m_outerX2 = mright;
	m_outerY2 = mbottom;

	if (m_overlayHandle && m_paintX2 > 0)
	{
		updateOverlayCoords();
	}
}

void AcinerellaVideoDecoder::updateOverlayCoords()
{
	int visibleWidth = m_paintX2 - m_paintX + 1;
	int visibleHeight = m_paintY2 - m_paintY + 1;
	int offsetX = 0;
	int offsetY = 0;

	float frameRatio = float(m_frameWidth) / float(m_frameHeight);
	float frameRevRatio = float(m_frameHeight) / float(m_frameWidth);
	float visibleRatio = float(visibleWidth) / float(visibleHeight);

	if (frameRatio < visibleRatio)
	{
		offsetX = visibleWidth - (float(visibleHeight) * frameRatio);
		offsetX /= 2;
	}
	else
	{
		offsetY = visibleHeight - (float(visibleWidth) * frameRevRatio);
		offsetY /= 2;
	}

	auto lock = holdLock(m_lock);
	if (m_overlayHandle)
	{
		SetVLayerAttrTags(m_overlayHandle,
			VOA_LeftIndent, m_outerX + m_paintX + offsetX,
			VOA_TopIndent, m_outerY + m_paintY + offsetY,
			VOA_RightIndent, m_overlayWindow->Width - ((m_paintX2 - offsetX) + m_outerX),
			VOA_BottomIndent, m_overlayWindow->Height - ((m_paintY2 - offsetY) + m_outerY),
			TAG_DONE);
		D(dprintf("%s: setting vlayer bounds %d %d %d %d\n", __func__, m_outerX + m_paintX, m_outerY + m_paintY,
			m_overlayWindow->Width - (m_paintX2 + m_outerX), m_overlayWindow->Height - (m_paintY2 + m_outerY)));
	}
}

void AcinerellaVideoDecoder::paint(GraphicsContext& gc, const FloatRect& rect)
{
	WebCore::PlatformContextCairo *context = gc.platformContext();
	cairo_t* cr = context->cr();
	cairo_save(cr);
	cairo_set_source_rgb(cr, double((m_overlayFillColor >> 16) & 0xff) / 255.0, double((m_overlayFillColor >> 8) & 0xff) / 255.0,
		double(m_overlayFillColor & 0xff) / 255.0);
	cairo_rectangle(cr, rect.x(), rect.y(), rect.width(), rect.height());
	cairo_fill(cr);
	cairo_restore(cr);

	bool needsToSetCoords = m_paintX != rect.x() || m_paintY != rect.y() || m_paintX2 != ((rect.x() + rect.width()) - 1) || m_paintY2 != ((rect.y() + rect.height()) - 1);

	m_paintX = rect.x();
	m_paintY = rect.y();
	m_paintX2 = (rect.x() + rect.width()) - 1;
	m_paintY2 = (rect.y() + rect.height()) - 1;

	if (needsToSetCoords)
		updateOverlayCoords();

#if 0
// ?!? WE cannot paint since the data is in planar yuv or some other cgxvideo format

	EP_SCOPE(paint);

	auto lock = holdLock(m_lock);
	if (m_decodedFrames.size())
	{
		MonotonicTime mtStart = MonotonicTime::now();
	
		const auto *frame = m_decodedFrames.front().frame();
		WebCore::PlatformContextCairo *context = gc.platformContext();
		cairo_t* cr = context->cr();
		auto *avFrame = ac_get_frame(m_decodedFrames.front().pointer()->decoder(m_index));
		// CAIRO_FORMAT_RGB24 is actually 00RRGGBB on BigEndian

		if (rect.width() == m_frameWidth && rect.height() == m_frameHeight)
		{
			auto surface = cairo_image_surface_create_for_data(avFrame->data[0], CAIRO_FORMAT_RGB24, m_frameWidth, m_frameHeight, avFrame->linesize[0]);
			if (surface)
			{
				cairo_save(cr);
				cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
				cairo_translate(cr, rect.x(), rect.y());
				cairo_rectangle(cr, 0, 0, rect.width(), rect.height());
				cairo_clip(cr);
				cairo_set_source_surface(cr, surface, 0, 0);
				cairo_paint(cr);
				cairo_restore(cr);
				cairo_surface_destroy(surface);
			}
		}
		else
		{
			auto surface = cairo_image_surface_create_for_data(avFrame->data[0], CAIRO_FORMAT_RGB24, m_frameWidth, m_frameHeight, avFrame->linesize[0]);
			if (surface)
			{
				cairo_save(cr);
				cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
				cairo_translate(cr, rect.x(), rect.y());
				cairo_rectangle(cr, 0, 0, rect.width(), rect.height());
				cairo_pattern_t *pattern = cairo_pattern_create_for_surface(surface);
				if (pattern)
				{
					cairo_matrix_t matrix;
					cairo_matrix_init_scale(&matrix, float(m_frameWidth) / rect.width(), float(m_frameHeight) / rect.height());
					cairo_pattern_set_matrix(pattern, &matrix);
					cairo_pattern_set_filter(pattern, CAIRO_FILTER_FAST);
					cairo_set_source(cr, pattern);
					cairo_clip(cr);
					cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
					cairo_paint(cr);
					cairo_pattern_destroy(pattern);
				}
				cairo_restore(cr);
				cairo_surface_destroy(surface);
			}
		}
		
		MonotonicTime mtEnd = MonotonicTime::now();
		Seconds decodingTime = (mtEnd - mtStart);

		m_accumulatedCairoCount ++;
		m_accumulatedCairoTime += decodingTime;

		if (m_accumulatedCairoCount % int(m_fps))
			D(dprintf("%s: paint time %f, avg %f\n", __func__, float(decodingTime.value()),
				float(m_accumulatedCairoTime.value() / double(m_accumulatedCairoCount))));
				
	}
#endif
}

void AcinerellaVideoDecoder::blitFrameLocked()
{
	if (m_overlayHandle && m_decodedFrames.size())
	{
		const auto *frame = m_decodedFrames.front().frame();
		auto *avFrame = ac_get_frame(m_decodedFrames.front().pointer()->decoder(m_index));

		if (avFrame && avFrame->data[0] && avFrame->data[1] && avFrame->data[2])
		{
			int w = m_frameWidth & -8;
			int h = m_frameHeight & -2;
			int x = 0;
			int y = 0;
		
			UBYTE *pY, *pCb, *pCr;
			UBYTE *sY, *sCb, *sCr;
			ULONG ptY, stY, ptCb, stCb, ptCr, stCr;
			ULONG w2 = w >> 1;

			h &= -2;
			w &= -2;
	
			sY   = avFrame->data[0];
			sCb  = avFrame->data[1];
			sCr  = avFrame->data[2];
			stY  = avFrame->linesize[0];
			stCb = avFrame->linesize[1];
			stCr = avFrame->linesize[2];
			
			LockVLayer(m_overlayHandle);

			ptY = GetVLayerAttr(m_overlayHandle, VOA_Modulo) >> 1;
			ptCr = ptCb = ptY >> 1;
			pY = (UBYTE *)GetVLayerAttr(m_overlayHandle, VOA_BaseAddress);
			pCb = pY + (ptY * m_frameHeight);
			pCr = pCb + ((ptCb * m_frameHeight) >> 1);
			pY += (y * ptY) + x;
			pCb += ((y * ptCb) >> 1) + (x >> 1);
			pCr += ((y * ptCr) >> 1) + (x >> 1);

			if (stY == ptY && w == m_frameWidth)
			{
				memcpy(pY, sY, ptY * h);
				memcpy(pCb, sCb, (ptCb * h) >> 1);
				memcpy(pCr, sCr, (ptCr * h) >> 1);
			}
			else do
			{
				memcpy(pY, sY, w);

				pY += ptY;
				sY += stY;

				memcpy(pY, sY, w);
				memcpy(pCb, sCb, w2);
				memcpy(pCr, sCr, w2);

				sY += stY;
				sCb += stCb;
				sCr += stCr;

				pY += ptY;
				pCb += ptCb;
				pCr += ptCr;

				h -= 2;
			} while (h > 0);

			UnlockVLayer(m_overlayHandle);
		}
	}
}

void AcinerellaVideoDecoder::pullThreadEntryPoint()
{
	D(dprintf("%s: %p\n", __func__, this));

	while (!m_terminating)
	{
		m_pullEvent.waitFor(5_s);

		if (m_playing && !m_terminating && m_overlayHandle)
		{
			while (m_playing)
			{
				Seconds sleepFor = 0_s;
				float pts;

				// Grab time point (disregarding time it takes to swap vlayer buffers)
				auto timeDisplayed = MonotonicTime::now();

				{
					auto lock = holdLock(m_lock);

					// Show previous frame
					SwapVLayerBuffer(m_overlayHandle);

					if (m_decodedFrames.size())
					{
						// Store current frame's pts
						pts = m_decodedFrames.front().pts();
						// Blit the frame into overlay backbuffer
						blitFrameLocked();
						// Pop the frame
						m_decodedFrames.pop();
					}
					else
					{
						break;
					}
				}

				decodeUntilBufferFull();
				// Get next presentation time
				float nextPts = m_decodedFrames.front().pts();
				// Sleep for the duration of last frame - time we've already spent
				sleepFor = Seconds(nextPts - pts);
				sleepFor -= MonotonicTime::now() - timeDisplayed;

				DSYNC(dprintf("[V]%s: pts %f next frame in %f diff pts %f\n", __func__, pts, float(sleepFor.value()), nextPts - pts));

				if (sleepFor.value() > 0.0 && pts >= m_audioPosition)
					m_pullEvent.waitFor(sleepFor);
			}
		}

#if 0

		while (m_playing && !m_terminating && m_overlayHandle)
		{
			MonotonicTime mtStart = MonotonicTime::now();
			{
				EP_SCOPE(pullFrame);
				auto lock = holdLock(m_lock);
				if (m_decodedFrames.size())
				{
					const auto *frame = m_decodedFrames.front().frame();
					auto *avFrame = ac_get_frame(m_decodedFrames.front().pointer()->decoder(m_index));
					presentationTime = frame->timecode;

//					auto surface = cairo_image_surface_create_for_data(avFrame->data[0], CAIRO_FORMAT_RGB24, m_frameWidth, m_frameHeight, avFrame->linesize[0]);

//					WritePixelArray(avFrame->data[0], 0, 0, avFrame->linesize[0], win->RPort, 0, 0, m_frameWidth, m_frameHeight, RECTFMT_ARGB);

					blitFrameLocked();

					SwapVLayerBuffer(m_overlayHandle);

					m_decodedFrames.pop();
					m_bufferedSeconds -= m_frameDuration;
				}
			}
//			m_client->onDecoderReadyToPaint(makeRef(*this));

			decodeUntilBufferFull();
			MonotonicTime mtEnd = MonotonicTime::now();
			Seconds decodingTime = (mtEnd - mtStart);
			m_accumulatedDecodingCount ++;
			m_accumulatedDecodingTime += decodingTime;
			Seconds sleepFor = Seconds(m_frameDuration) - (decodingTime);
			if (sleepFor.value() > 0.0)
			{
				EP_SCOPE(sleep);
				m_pullEvent.waitFor(sleepFor);
			}
			if (m_accumulatedDecodingCount % int(m_fps))
				DSYNC(dprintf("[V]%s: pts %f decode time %f, avg %f\n", __func__, presentationTime, float(decodingTime.value()),
					float(m_accumulatedDecodingTime.value() / double(m_accumulatedDecodingCount))));
		}
#endif
	}
}

}
}

#endif
