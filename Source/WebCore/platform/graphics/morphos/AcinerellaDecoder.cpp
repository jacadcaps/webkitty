#include "config.h"
#include "AcinerellaDecoder.h"

#if ENABLE(VIDEO)
#include "acinerella.h"
#include "AcinerellaContainer.h"
#include "MediaPlayerMorphOS.h"
#include <proto/exec.h>

#define D(x) //x
#define DNF(x) //if (!isAudio()) {x;}
#define DI(x)
#define DBF(x)
#define DPOS(x)
#define DLIFETIME(x) 

// #pragma GCC optimize ("O0")

namespace WebCore {
namespace Acinerella {

AcinerellaDecoder::AcinerellaDecoder(AcinerellaDecoderClient *client, RefPtr<AcinerellaPointer> acinerella, RefPtr<AcinerellaMuxedBuffer> buffer, int index, const ac_stream_info &info, bool isLiveStream, bool isHLS)
	: m_client(client)
	, m_muxer(buffer)
	, m_index(index)
	, m_isLive(isLiveStream)
	, m_isHLS(isHLS)
{
	DLIFETIME(dprintf("%s: %p ++\033[0m\n", __func__, this));
	auto ac = acinerella->instance();
	m_duration = std::max(ac_get_stream_duration(ac, index), double(ac->info.duration)/1000.0);

	(void)info;

	// simulated duration of 3 chunks
	if (m_isLive)
		m_duration = 15.f;

	m_bitrate = ac->info.bitrate;
	m_lastDecoder = acinerella->decoder(m_index);
	m_codec = String::fromUTF8(ac_codec_name(acinerella->instance(), index));
}

AcinerellaDecoder::~AcinerellaDecoder()
{
	DLIFETIME(dprintf("%s: %p --\033[0m\n", __func__, this));
    if (!m_terminating)
    {
        dprintf("Wayfarer's media decoder shut down incorrectly, please send the following log to wayfarer@wayfarer.icu\n");
        DumpTaskState(FindTask(0));
    }
}

uint32_t AcinerellaDecoder::maxCompressedBufferSize() const
{
	// Reserve a few read-ahead windows worth of compressed data. m_bitrate is the whole-container
	// bitrate (bits/s); for video (which dominates the bitrate) that's a good upper bound, for audio
	// it over-estimates harmlessly (audio packets are tiny, so the gate just rarely trips).
	const double seconds = std::max(2.0, readAheadTime() * 4.0);
	double bytesPerSecond = double(m_bitrate) / 8.0;
	if (bytesPerSecond < 16384.0)
		bytesPerSecond = 16384.0; // bitrate unknown/bogus - stay generous so we never starve

	uint64_t bytes = uint64_t(bytesPerSecond * seconds);

	// Floor so a low-bitrate track always has room to fill its read-ahead; ceiling so we never
	// exceed the old hard cap even for very high bitrate streams.
	const uint64_t floorBytes = 1024 * 1024;
	const uint64_t ceilBytes = 28521267;
	bytes = std::max(bytes, floorBytes);
	bytes = std::min(bytes, ceilBytes);
	return uint32_t(bytes);
}

void AcinerellaDecoder::warmUp()
{
	if (!m_terminating && !m_thread)
	{
		DI(dprintf("%s: %p starting thread\033[0m\n", __func__, this));
		m_thread = Thread::create(isAudio() ? "Acinerella Audio Decoder"_s : "Acinerella Video Decoder"_s, [this] {
			threadEntryPoint();
		});
	}

	D(dprintf("[%s]%s: %p\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV",__func__, this));
	dispatch([this] {
		m_warminUp = true;
		decodeUntilBufferFull();
	});
}

void AcinerellaDecoder::coolDown()
{
	D(dprintf("[%s]%s: %p\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV",__func__, this));
	dispatch([this] {
		m_readying = false;
		stopPlaying();
		onCoolDown();
	});
}

void AcinerellaDecoder::prePlay()
{
	D(dprintf("[%s]%s: %p\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV",__func__, this));
	dispatch([this](){
		decodeUntilBufferFull();
		onGetReadyToPlay();
		D(dprintf("[%s]prePlay: %p ready %d\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV",__func__, isReadyToPlay()));
		m_readying = true; // force onReadyToPlay() if ready
		if (isReadyToPlay())
		{
			onReadyToPlay();
		}
	});
}

void AcinerellaDecoder::play()
{
	D(dprintf("[%s]%s: %p\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV",__func__, this));
	dispatch([this](){
		decodeUntilBufferFull();
		onGetReadyToPlay();
		if (isReadyToPlay())
		{
			startPlaying();
		}
		else
		{
			D(dprintf("[%s]%s: %p not ready to play just yet bs %f rahs %f\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV",__func__, this, float(bufferSize()), float(readAheadTime())));
		}
	});
}

void AcinerellaDecoder::onReadyToPlay()
{
	D(dprintf("[%s]%s: %p readying %d\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV",__func__, this, m_readying));
	if (m_readying)
	{
		m_readying = false;
		if (m_client)
			m_client->onDecoderReadyToPlay(Ref{*this});
	}
}

void AcinerellaDecoder::pause(bool willSeek)
{
	D(dprintf("[%s]%s: %p\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV",__func__, this));

	stopPlayingQuick(); // let VideoDecoder stop pumping frames immediately

	dispatch([this, willSeek](){
		m_readying = false;
		stopPlaying();
		if (willSeek)
			flush(true);
	});
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

	DNF(dprintf("[%s]%s: this %p term %d decEOF %d %d %d buffer %f\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV", __func__, this, m_terminating, m_decoderEOF,
		m_droppingFrames, m_droppingUntilKeyFrame, double(bufferSize())));

	if (m_terminating)
		return false;

	if ((buffer = m_muxer->nextPackage(*this)))
	{
		AcinerellaDecodedFrame frame;
		auto acinerella = buffer->acinerella();
		auto *decoder = !!acinerella ? acinerella->decoder(m_index) : nullptr;

		if (!decoder)
			return false;

		if (m_lastDecoder != decoder)
		{
			DNF(dprintf("[%s]%s: changing decoder from %p to %p\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV", __func__, m_lastDecoder, decoder));
			m_lastDecoder = decoder;
			onDecoderChanged(acinerella);
		}

		// used both if acinerella sends us stuff AND in case of discontinuity
		// either way, we must flush caches here!
		if (buffer->isFlushPackage())
		{
			DNF(dprintf("[%s]%s: got flush packet! (live %d hls %d)\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV", __func__, m_isLive, m_isHLS));
			
			if (!m_isHLS)
			{
				ac_flush_buffers(decoder);
				flush(false);
			}

			return true;
		}

		if (buffer->package())
		{
			double pts = ac_get_package_pts(acinerella->instance(), buffer->package());

            if (!acceptPackage(buffer, pts))
                return true;

			if (m_droppingFrames)
			{
				if (pts < m_dropToPTS)
				{
					// Flush the codec exactly once, on entry to the drop. The old code re-flushed on
					// every dropped packet, which re-acquired a keyframe each time and burned the CPU
					// for seconds on a deep backlog while producing no frames (the frozen-picture stall).
					// Dropped packets are never pushed, so a single flush leaves the codec empty and
					// ready for the next keyframe; subsequent packets are just skipped cheaply.
					if (!m_needsKF)
					{
						m_needsKF = true; // dropped frames - we'll need a keyframe!
						ac_flush_buffers(decoder);
					}
					return true;
				}
				else if (m_needsKF)
				{
					m_droppingUntilKeyFrame = true;
					m_droppingFrames = false;
					m_needsKF = false;
					return true;
				}
				else
				{
					m_droppingFrames = false;
				}
			}
			else if (m_droppingUntilKeyFrame)
			{
				if (ac_get_package_keyframe(buffer->package()))
				{
					m_droppingUntilKeyFrame = false;
				}
				else
				{
					return true;
				}
			}
		}

		DNF(dprintf("[%s]%s: package %p ts %f\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV", __func__, buffer->package(), float(ac_get_package_pts(acinerella->instance(), buffer->package()))));

		auto rcPush = ac_push_package(decoder, buffer->package());
		if (rcPush != PUSH_PACKAGE_SUCCESS)
		{
			DNF(dprintf("[%s]%s: failed ac_push_package %d\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV", __func__, rcPush));
			return true; // don't fail decoding - keep going instead!
		}

		for (;;)
		{
			AcinerellaDecodedFrame frame = AcinerellaDecodedFrame(acinerella, decoder);
			auto rcFrame = ac_receive_frame(decoder, frame.frame());
			
			switch (rcFrame)
			{
			case RECEIVE_FRAME_SUCCESS:
				{
#if defined(EP_PROFILING) && EP_PROFILING
					{
						char buffer[128];
						sprintf(buffer, "frame TS %f", float(frame.frame()->timecode));
						EP_EVENTSTR(buffer);
					}
#endif
					auto lock = Locker(m_lock);
					onFrameDecoded(frame);
					DNF(dprintf("[%s]%s: decoded frame @ %f\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV", __func__, float(frame.frame()->timecode)));
					m_decodedFrames.append(WTFMove(frame));
					m_decoderEOF = false;
					m_decodedSinceDump++;
					m_totalDecodedFrames++;
				}
				break;
			case RECEIVE_FRAME_NEED_PACKET:
				// we'll have to call decodeNextFrame again
//				DNF(dprintf("[%s]%s: NEED_PACKET\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV", __func__));
				return true;
			case RECEIVE_FRAME_ERROR:
				DNF(dprintf("[%s]%s: FRAME_ERROR\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV", __func__));
				return false;
			case RECEIVE_FRAME_EOF:
				DNF(dprintf("[%s]%s: FRAME_EOF\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV", __func__));
				m_decoderEOF = true;
				return false;
			}
		}
	}
	else if (m_muxer->isEOS())
	{
		m_decoderEOF = true;
	}

	return false;
}

void AcinerellaDecoder::requestDecodeUntilBufferFull()
{
	if (m_terminating)
		return;
	// Coalesce: only queue a refill if one isn't already pending. The pending flag is cleared at the
	// top of decodeUntilBufferFull(), so a request arriving while a refill runs still queues a fresh one.
	if (!m_refillRequested.exchange(true))
	{
		dispatch([this] {
			decodeUntilBufferFull();
		});
	}
}

void AcinerellaDecoder::decodeUntilBufferFull()
{
	EP_SCOPE(untilBufferFull);

	m_refillRequested = false;
	m_decoding = true;

	DBF(dprintf("[%s]%s: %p - start! wmup %d prep %d\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV", __func__, this, m_warminUp, m_readying));

	// Decode in a bounded slice; if we run out of budget while still behind, re-arm (coalesced) and
	// return so the message queue and other threads get a turn. Easing off the catch-up like this
	// avoids the CPU/lock-contention spike that otherwise follows an underrun.
	//
	// Crucially the budget counts frames we actually *produce*, not packets consumed: when catching
	// up after falling behind, decodeNextFrame() drops/skips packets cheaply without emitting a frame
	// (m_droppingFrames / m_droppingUntilKeyFrame), and throttling that would make re-sync crawl. A
	// large packet cap is the only backstop there, so we still yield during a pathologically long drop.
	const unsigned producedAtStart = m_totalDecodedFrames;
	int packetCap = decodeSliceFrames * 32;
	bool moreToDo = false;

	while (bufferSize() < readAheadTime())
	{
		if (!decodeNextFrame())
			break;

		if ((m_totalDecodedFrames - producedAtStart) >= unsigned(decodeSliceFrames) || --packetCap <= 0)
		{
			moreToDo = bufferSize() < readAheadTime();
			break;
		}
	}

	m_decoding = false;

	if (m_warminUp && bufferSize() >= readAheadTime())
	{
		m_warminUp = false;
		if (m_client)
			m_client->onDecoderWarmedUp(Ref{*this});
	}

	if (isReadyToPlay() && m_readying)
	{
		onReadyToPlay();
	}

	if (moreToDo)
		requestDecodeUntilBufferFull();

	DBF(dprintf("[%s]%s: %p - buffer full (%f s)\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV", __func__, this, float(bufferSize())));
}

void AcinerellaDecoder::dropUntilPTS(double pts)
{
	DBF(dprintf("[%s]%s: %p - start!\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV", __func__, this));

	m_dropToPTS = pts;
	m_droppingFrames = true;
	m_droppingUntilKeyFrame = false;
}

void AcinerellaDecoder::flush(bool willSeek)
{
	D(dprintf("[%s]%s: islive %d\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV", __func__, this, m_isLive));
	auto lock = Locker(m_lock);

    m_decodedFrames.clear();
		
	m_decoderEOF = false;
    m_droppingFrames = false;
    m_droppingUntilKeyFrame = false;
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
	DPOS(dprintf("[%s]%s: %p to %f\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV", __func__, this, position()));
	if (m_client)
		m_client->onDecoderUpdatedPosition(Ref{*this}, position());
}

void AcinerellaDecoder::onDurationChanged()
{
	D(dprintf("[%s]%s: %p to %f\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV", __func__, this, duration()));
	if (m_client)
		m_client->onDecoderUpdatedDuration(Ref{*this}, duration());
}

void AcinerellaDecoder::onEnded()
{
	EP_EVENT(ended);
	D(dprintf("[%s]%s: %p\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV", __func__, this));
	if (m_client)
		m_client->onDecoderEnded(Ref{*this});
}

void AcinerellaDecoder::terminate()
{
	EP_SCOPE(terminate);

	DLIFETIME(dprintf("[%s]%s: %p\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV", __func__, this));
	m_terminating = true;

	// The decoder thread may be parked inside a blocking m_muxer->nextPackage(); wake it so it
	// observes isTerminating() and returns instead of waiting for data that will never arrive.
	if (m_muxer)
		m_muxer->interrupt(m_index);

	if (!m_thread)
		return;

	onTerminate();

	DLIFETIME(dprintf("[%s]%s: %p disp..\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV", __func__, this));
	m_queue.append(makeUnique<Function<void ()>>([this] {
		performTerminate();
	}));
	m_thread->waitForCompletion();

	DLIFETIME(dprintf("[%s]%s: %p completed\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV", __func__, this));
	ASSERT(m_queue.killed());
	m_thread = nullptr;
	m_client = nullptr;
	m_muxer = nullptr;
    m_decodedFrames.clear();

	DLIFETIME(dprintf("[%s]%s: %p done\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV", __func__, this));
}

void AcinerellaDecoder::threadEntryPoint()
{
    if (isAudio()) // don't set it for video - we'll cause issues with main thread when getting close to 100% cpu usage
        SetTaskPri(FindTask(0), 3);

	RefPtr<AcinerellaDecoder> refSelf = WTF::Ref{*this};

	DI(dprintf("[%s]%s: %p\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV", __func__, this));
	if (!onThreadInitialize())
	{
		// TODO: signal failure to parent
	}
	
	while (auto function = m_queue.waitForMessage())
	{
		(*function)();
	}
	
	DI(dprintf("[%s]%s: %p .. shutting down...\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV", __func__, this));
	onThreadShutdown();
	DI(dprintf("[%s]%s: %p onThreadShutdown done\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV", __func__, this));
}

void AcinerellaDecoder::dispatch(Function<void ()>&& function)
{
	ASSERT(!m_queue.killed());
	if (m_terminating)
		return;
	m_queue.append(makeUnique<Function<void ()>>(WTFMove(function)));
}

void AcinerellaDecoder::performTerminate()
{
	DLIFETIME(dprintf("[%s]%s: %p\033[0m\n", isAudio() ? "\033[33mA":"\033[35mV", __func__, this));
	ASSERT(!isMainThread());
	m_queue.kill();
}

}
}

#undef D
#endif
