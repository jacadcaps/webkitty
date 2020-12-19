#include "AcinerellaAudioDecoder.h"

#if ENABLE(VIDEO)
#include <proto/ahi.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

namespace WebCore {
namespace Acinerella {

#undef AHI_BASE_NAME
#define AHI_BASE_NAME m_ahiBase

#define D(x) 

AcinerellaAudioDecoder::AcinerellaAudioDecoder(Acinerella* parent, RefPtr<AcinerellaMuxedBuffer> buffer, int index, const ac_stream_info &info)
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
	if (m_ahiControl)
	{
		AHI_ControlAudio(m_ahiControl, AHIC_Play, TRUE, TAG_DONE);
		m_playing = true;
	}
}

void AcinerellaAudioDecoder::stopPlaying()
{
	if (m_ahiControl)
	{
		AHI_ControlAudio(m_ahiControl, AHIC_Play, FALSE, TAG_DONE);
		m_playing = false;
	}
}

void AcinerellaAudioDecoder::doSetVolume(float volume)
{
	if (m_ahiControl)
	{
		AHI_SetVol(0, (LONG) (float(0x10000L) * volume),
			0x8000L, m_ahiControl, AHISF_IMM);
	}
}

bool AcinerellaAudioDecoder::isReadyToPlay() const
{
	return true;
}

bool AcinerellaAudioDecoder::onThreadInitialize()
{
	D(dprintf("%s:\n", __func__));
	if ((m_ahiPort = CreateMsgPort()))
	{
		if ((m_ahiIO = reinterpret_cast<AHIRequest *>(CreateIORequest(m_ahiPort, sizeof(AHIRequest)))))
		{
			m_ahiIO->ahir_Version = 4;
			if (0 == OpenDevice(AHINAME, AHI_NO_UNIT, reinterpret_cast<IORequest *>(m_ahiIO), 0))
			{
				m_ahiBase = reinterpret_cast<Library *>(m_ahiIO->ahir_Std.io_Device);

				D(dprintf("%s: ahiBase %p\n", __func__, m_ahiBase));

				static struct EmulLibEntry GATE_SoundFunc = {
					TRAP_LIB, 0, (void (*)(void))AcinerellaAudioDecoder::soundFunc
				};

				static struct Hook __soundHook = {
					{NULL,NULL},
					(ULONG (*)()) &GATE_SoundFunc,
					NULL, NULL,
				};

				if ((m_ahiControl = AHI_AllocAudio(
					AHIA_UserData, reinterpret_cast<IPTR>(this),
					AHIA_AudioID, AHI_DEFAULT_ID,
					AHIA_Sounds, 2, AHIA_Channels, 1,
					AHIA_MixFreq, m_audioRate,
					AHIA_SoundFunc, reinterpret_cast<IPTR>(&__soundHook),
					TAG_DONE)))
				{
					ULONG maxSamples = 0, mixFreq = 0;

					AHI_GetAudioAttrs(AHI_INVALID_ID, m_ahiControl,
						AHIDB_MaxPlaySamples, reinterpret_cast<IPTR>(&maxSamples),
						TAG_DONE);
					AHI_ControlAudio(m_ahiControl,
						AHIC_MixFreq_Query, reinterpret_cast<IPTR>(&mixFreq),
						TAG_DONE);
					
					D(dprintf("%s: control allocated\n", __func__));
					
					if (maxSamples && mixFreq)
					{
						m_ahiSampleLength = maxSamples * m_audioRate / mixFreq;
						
						m_ahiSampleLength = std::max(m_ahiSampleLength, (m_audioRate / m_ahiSampleLength) * m_ahiSampleLength);
						
						D(dprintf("%s: sample length: %d\n", __func__, m_ahiSampleLength));

						for (int i = 0; i < 2; i++)
						{
							m_ahiSample[i].ahisi_Type = AHIST_S16S;
							m_ahiSample[i].ahisi_Length = m_ahiSampleLength;
							m_ahiSample[i].ahisi_Address = malloc(m_ahiSampleLength * 4);
						}
						
						if (m_ahiSample[0].ahisi_Address && m_ahiSample[1].ahisi_Address)
						{
							bzero(m_ahiSample[0].ahisi_Address, m_ahiSampleLength * 4);
							bzero(m_ahiSample[1].ahisi_Address, m_ahiSampleLength * 4);
							
							m_ahiSampleBeingPlayed = 0;
							m_ahiThreadShuttingDown = false;
							m_ahiFrameOffset = 0;
							
							fillBuffer(0);
							
							m_ahiThread = Thread::create("Acinerella AHI Pump", [this] {
								ahiThreadEntryPoint();
							});
							m_ahiThread->changePriority(10);
							
							if (0 == AHI_LoadSound(0, AHIST_DYNAMICSAMPLE, reinterpret_cast<APTR>(&m_ahiSample[0]), m_ahiControl)
								&& 0 == AHI_LoadSound(1, AHIST_DYNAMICSAMPLE, reinterpret_cast<APTR>(&m_ahiSample[1]), m_ahiControl))
							{
								D(dprintf("%s: samples loaded - samplelen %d\n", __func__, m_ahiSampleLength));
								if (0 == AHI_ControlAudio(m_ahiControl, AHIC_Play, TRUE, TAG_DONE))
								{
									D(dprintf("%s: calling AHI_Play!\n", __func__));
									AHI_Play(m_ahiControl,
										AHIP_BeginChannel, 0,
										AHIP_Freq, m_audioRate,
										AHIP_Vol, 0x10000L, AHIP_Pan, 0x8000L,
										AHIP_Sound, 0, AHIP_Offset, 0, AHIP_Length, 0,
										AHIP_EndChannel, 0,
										TAG_DONE);
									m_playing = true;
								}
								else
								{
									D(dprintf("%s: AHI_ControlAudio failed\n", __func__));
								}
							}
							
							return true;
						}
						
						for (int i = 0; i < 2; i++)
						{
							free(m_ahiSample[i].ahisi_Address);
							m_ahiSample[i].ahisi_Address = nullptr;
						}
					}

					AHI_FreeAudio(m_ahiControl);
					m_ahiControl = 0;
				}
			}
			
			DeleteIORequest(reinterpret_cast<IORequest *>(m_ahiIO));
			m_ahiIO = nullptr;
			m_ahiBase = nullptr;
		}
		
		DeleteMsgPort(m_ahiPort);
		m_ahiPort = nullptr;
	}
	
	return false;
}

void AcinerellaAudioDecoder::onThreadShutdown()
{
	D(dprintf("%s:\n", __func__));

	if (m_ahiControl)
	{
		AHI_ControlAudio(m_ahiControl, AHIC_Play, FALSE, TAG_DONE);
	}

	if (m_ahiThread)
	{
		m_ahiThreadShuttingDown = true;
		m_ahiSampleConsumed.signal();
		m_ahiThread->waitForCompletion();
		D(dprintf("%s: AHI thread shut down... \n", __func__));
	}

	if (m_ahiControl)
	{
		AHI_FreeAudio(m_ahiControl);
		m_ahiControl = nullptr;
		D(dprintf("%s: AHI control disposed\n", __func__));
	}

	for (int i = 0; i < 2; i++)
	{
		free(m_ahiSample[i].ahisi_Address);
		m_ahiSample[i].ahisi_Address = nullptr;
	}

	if (m_ahiIO)
	{
		CloseDevice(reinterpret_cast<IORequest *>(m_ahiIO));
		DeleteIORequest(reinterpret_cast<IORequest *>(m_ahiIO));
		m_ahiIO = nullptr;
		m_ahiBase = nullptr;
	}

	if (m_ahiPort)
	{
		DeleteMsgPort(m_ahiPort);
		m_ahiPort = nullptr;
	}

	D(dprintf("%s: done\n", __func__));
}

void AcinerellaAudioDecoder::onFrameDecoded(const AcinerellaDecodedFrame &frame)
{
	m_bufferedSamples += frame.frame()->buffer_size / 4; // 16bitStereo = 4BPF
	m_bufferedSeconds = float(m_bufferedSamples) / float(m_audioRate);
}

bool AcinerellaAudioDecoder::isPlaying() const
{
	return m_playing;
}

float AcinerellaAudioDecoder::position() const
{
	return 0.f;
}

#undef AHI_BASE_NAME
#define AHI_BASE_NAME me->m_ahiBase
void AcinerellaAudioDecoder::soundFunc()
{
	AHIAudioCtrl *ahiCtrl = reinterpret_cast<AHIAudioCtrl *>(REG_A2);
	AcinerellaAudioDecoder *me = reinterpret_cast<AcinerellaAudioDecoder *>(ahiCtrl->ahiac_UserData);
	
	me->m_ahiSampleBeingPlayed ++;
	AHI_SetSound(0, me->m_ahiSampleBeingPlayed % 2, 0, 0, me->m_ahiControl, 0);

	// D(dprintf("%s: setSound %d (%d)\n", __func__, me->m_ahiSampleBeingPlayed % 2, me->m_ahiSampleBeingPlayed));

	me->m_ahiSampleConsumed.signal();

}

void AcinerellaAudioDecoder::fillBuffer(int index)
{
	uint32_t offset = 0;
	uint32_t bytesLeft = m_ahiSampleLength * 4;
	bool didPopFrames = false;

	// D(dprintf("%s: sample %d, next index: %d\n", __func__, m_ahiSampleBeingPlayed, index));

	{
		auto lock = holdLock(m_lock);
		while (!m_decodedFrames.empty() && bytesLeft)
		{
			const auto *frame = m_decodedFrames.front().frame();
			size_t copyBytes = std::min(frame->buffer_size - m_ahiFrameOffset, bytesLeft);
			
			// D(dprintf("%s: cb %d of %d afo %d bs %d\n", __func__, copyBytes, offset, m_ahiFrameOffset, frame->buffer_size));
			
			memcpy(reinterpret_cast<uint8_t*>(m_ahiSample[index].ahisi_Address) + offset, frame->pBuffer + m_ahiFrameOffset, copyBytes);
			
			m_ahiFrameOffset += copyBytes;
			offset += copyBytes;
			bytesLeft -= copyBytes;

			if (frame->buffer_size == m_ahiFrameOffset)
			{
//				D(dprintf("%s: pop frame sized %d\n", __func__, frame->buffer_size));

				m_bufferedSamples -= frame->buffer_size / 4; // 16bitStereo = 4BPF
				m_bufferedSeconds = float(m_bufferedSamples) / float(m_audioRate);
				m_freeFrames.emplace(WTFMove(m_decodedFrames.front()));
				m_decodedFrames.pop();
				m_ahiFrameOffset = 0;
				didPopFrames = true;
			}
		}
	}

	// Clear remaining buffer in case of an underrun
	bzero(reinterpret_cast<uint8_t*>(m_ahiSample[index].ahisi_Address) + offset, bytesLeft);

#if 0
				BPTR f = Open("sys:out.raw", MODE_READWRITE);
				if (f)
				{
					Seek(f, 0, OFFSET_END);
					Write(f, m_ahiSample[index].ahisi_Address, m_ahiSampleLength * 4);
					Close(f);
				}
#endif



	// D(dprintf("%s: done, bleft %d offset %d\n", __func__, bytesLeft, offset));

	if (didPopFrames)
		dispatch([this, protectedThis(makeRef(*this))]() {
			if (!m_ahiThreadShuttingDown)
				decodeUntilBufferFull();
		});
}

void AcinerellaAudioDecoder::ahiThreadEntryPoint()
{
	SetTaskPri(FindTask(0), 50);
	while (!m_ahiThreadShuttingDown)
	{
		m_ahiSampleConsumed.wait();
		uint32_t index = m_ahiSampleBeingPlayed % 2; // this sample will play next
		fillBuffer(index);
	}
}


}
}

#undef D
#endif
