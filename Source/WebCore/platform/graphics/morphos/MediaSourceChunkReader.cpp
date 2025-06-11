#include "config.h"
#include "MediaSourcePrivateMorphOS.h"
#include "ContentType.h"

#if ENABLE(VIDEO) && ENABLE(MEDIA_SOURCE)

#include "MediaSourceBufferPrivateMorphOS.h"
#include "SourceBufferPrivateClient.h"
#include "MediaPlayerPrivateMorphOS.h"
#include "MediaSourcePrivateMorphOS.h"
#include "MediaDescriptionMorphOS.h"
#include "InbandTextTrackPrivate.h"
#include "AcinerellaAudioDecoder.h"
#include "AcinerellaVideoDecoder.h"
#include "AcinerellaMuxer.h"
#include "AudioTrackPrivateMorphOS.h"
#include "VideoTrackPrivateMorphOS.h"
#include "MediaSampleMorphOS.h"
#include "WebMHeaderParser.h"
#include <libavutil/common.h>
#include <libavutil/error.h>

#include <proto/dos.h>
#include <proto/exec.h>
#include <dos/dos.h>

#define DLIFETIME(x) 
#define DIO(x)
#define DM(x) 
#define DSAMPLES(x)
#define DINIT(x) 
#define DNERR(x)
#define DN 0
#define DNVIDEOONLY 0
#define DPROVIDER(x)

// #pragma GCC optimize ("O0")
// #define DEBUG_FILE

namespace WebCore {

/*****************************************/

void MediaSourceChunkReaderDataProvider::initialize(Function<void(void)>&& underrun)
{
    m_onDataUnderrun = WTFMove(underrun);
}

void MediaSourceChunkReaderDataProvider::push(Ref<SharedBuffer>&& buffer, MediaSourceChunkReaderDataProvider::ChunkType type)
{
    DPROVIDER(dprintf("[MSDP][%p]%s: %ld bytes type %s\n", this, __func__, buffer->size(), type == ChunkType::ReInitialization?"init":"plain"));
    auto lock = Locker(m_lock);
    m_queue.append({std::move(buffer), type});
    m_event.signal();
}

void MediaSourceChunkReaderDataProvider::terminate()
{
    DPROVIDER(dprintf("[MSDP][%p]%s: \n", this, __func__));
    m_terminated.store(true);
    m_event.signal();
}

int MediaSourceChunkReaderDataProvider::pull(uint8_t *buf, int size)
{
    DPROVIDER(dprintf("[MSDP][%p]%s: %ld\n", this, __func__, size));
    int readTotal = 0;

    while (size > 0 && !m_terminated.loadRelaxed())
    {
        bool needsToWait = false;

        do
        {
            auto lock = Locker(m_lock);

            if (m_queue.size() == 0) {
                DPROVIDER(dprintf("[MSDP][%p]%s: empty queue\n", this, __func__));
                needsToWait = true;
                break;
            }

            int canRead = m_queue[0].buffer->size() - m_bufferPosition;
            DPROVIDER(dprintf("[MSDP][%p]%s: canread %ld bpos %ld qsize %ld q1st %ld\n", this, __func__, canRead, m_bufferPosition, m_queue.size(), m_queue[0].buffer->size()));

            if (canRead >= size)
            {
                m_queue[0].buffer->copyTo(std::span(buf + readTotal, size), m_bufferPosition);
                m_bufferPosition += size;
                readTotal += size;
                size = 0;

                if (m_bufferPosition == m_queue[0].buffer->size() && ChunkType::Data == m_queue[0].chunkType)
                {
                    DPROVIDER(dprintf("[MSDP][%p]%s: removed read chunk\n", this, __func__));
                    m_queue.remove(0);
                    m_bufferPosition = 0;
                }
            }
            else if (m_queue.size() > 1 && ChunkType::ReInitialization == m_queue[1].chunkType)
            {
                // 1st time we get here, return the bytes we can still read
                if (canRead > 0)
                {
                    DPROVIDER(dprintf("[MSDP][%p]%s: reading till EOF\n", this, __func__));
                    int readActual = std::min(size, canRead);
                    if (readActual > 0)
                    {
                        m_queue[0].buffer->copyTo(std::span(buf + readTotal, readActual), m_bufferPosition);
                        m_bufferPosition += readActual;
                        readTotal += readActual;
                        size -= readActual;
                    }
                    return readTotal ? readTotal : AVERROR_EOF;
                }
                // 2nd time we get here, return an EOF - the next chunk requires reinitialization before resuming read ops
                else
                {
                    DPROVIDER(dprintf("[MSDP][%p]%s: chunk EOF due to reinitialization\n", this, __func__));
                    m_queue.remove(0);
                    m_bufferPosition = 0;
                    m_onDataUnderrun(); // wake up the client
                    return AVERROR_EOF;
                }
            
            }
            else
            {
                int readActual = std::min(size, canRead);
                if (readActual > 0)
                {
                    m_queue[0].buffer->copyTo(std::span(buf + readTotal, readActual), m_bufferPosition);
                    readTotal += readActual;
                    size -= readActual;
                }

                m_queue.remove(0);
                m_bufferPosition = 0;
                DPROVIDER(dprintf("[MSDP][%p]%s: removed read chunk, readTotal %ld\n", this, __func__, readTotal));
            }
        }
        while (0);

        if (needsToWait)
        {
            DPROVIDER(dprintf("[MSDP][%p]%s: underrun, rt %ld\n", this, __func__, readTotal));
            m_onDataUnderrun();
            m_event.waitFor(10_s);
        }
    }

    if (m_terminated.loadRelaxed())
        return AVERROR_EXIT;
    return readTotal;
}

/*****************************************/

MediaSourceChunkReader::MediaSourceChunkReader(MediaSourceChunkReaderTrackFactory& trackFactory)
	: m_trackFactory(trackFactory)
{
	DLIFETIME(dprintf("%s(%p):\n", __PRETTY_FUNCTION__, this));
    m_workQueue = WorkQueue::create("MediaSourceChunkReader"_s);

    m_dataProvider.initialize([this](){
        m_underrunSignalled.store(true);
        m_underrunEvent.signal();
    });

	m_thread = Thread::create("MediaSourceChunkReader_IO"_s, [this] {
        while (auto function = m_queue.waitForMessage())
        {
            (*function)();
        }
    });
}

MediaSourceChunkReader::~MediaSourceChunkReader()
{
	DLIFETIME(dprintf("%s(%p): \n", __PRETTY_FUNCTION__, this));
	terminate();
}

void MediaSourceChunkReader::terminate()
{
	DLIFETIME(dprintf("%s(%p): \n", __PRETTY_FUNCTION__, this));
	if (m_terminating)
		return;

	m_terminating = true;
    m_dataProvider.terminate();

	m_workQueue->shutdown();
	m_workQueue = nullptr;

	m_queue.append(makeUnique<Function<void ()>>([this] {
        m_queue.kill();
	}));

	m_thread->waitForCompletion();
	m_thread = nullptr;
}

int MediaSourceChunkReader::analyzeHeader(Ref<SharedBuffer>& buffer, ac_initialization_segment_stream* streams, size_t streamsMax)
{
    unsigned char tmp[1024];
    buffer->copyTo(std::span(tmp, std::min(sizeof(tmp), buffer->size())));

    int tracks = 0;
    if (WebMParser::isWebM(tmp, sizeof(tmp)))
    {
        WebMParser parser;
        tracks = parser.parse(tmp, sizeof(tmp), streams, streamsMax);
    }
    else
    {
        tracks = ac_is_initialization_segment(tmp, std::min(sizeof(tmp), buffer->size()), streams, streamsMax);
    }

    DSAMPLES(dprintf("%s: tracks %d webm %d\n", __PRETTY_FUNCTION__, tracks, WebMParser::isWebM(tmp, sizeof(tmp))));

    return tracks;
}

Ref<MediaSourceChunkReader::DecodePromise> MediaSourceChunkReader::decodeAsync(Ref<SharedBuffer>&& buffer)
{
    DSAMPLES(dprintf("%s:\n", __PRETTY_FUNCTION__));

    if (m_terminating)
        return MediaSourceChunkReader::DecodePromise::createAndReject();

    WorkQueue& q = *m_workQueue.get();
    return invokeAsync(q, [buffer = WTFMove(buffer), protectedThis = Ref{*this}, this] () mutable {

        m_decodeAppendCount ++;


#if 0
{
    char name[128];
    sprintf(name, "ram:%p-%d.mp4", this, m_decodeAppendCount);
    BPTR f = Open(name, MODE_NEWFILE);
    Vector<uint8_t> v = buffer->copyData();
    Write(f, v.data(), v.size());
    Close(f);
}
#endif

        if (m_terminating)
            return MediaSourceChunkReader::DecodePromise::createAndReject();

        if (!m_initializationDone)
        {
            DSAMPLES(dprintf("%s: received initialization buffer\n", __PRETTY_FUNCTION__));

            // initial initialization
            int tracks = analyzeHeader(buffer, m_streamInfo, sizeof(m_streamInfo) / sizeof(ac_initialization_segment_stream));

            if (tracks > 0)
            {
                m_dataProvider.push(WTFMove(buffer));
                m_numStreamInfo = tracks;
                m_initializationDone = true;
                updateMetadata();
                return MediaSourceChunkReader::DecodePromise::createAndResolve(DecodeResult::InitialInitialize);
            }
            else
            {
                m_numStreamInfo = 0;
                return MediaSourceChunkReader::DecodePromise::createAndReject();
            }
        }
        else
        {
            ac_initialization_segment_stream streamInfo[Acinerella::AcinerellaMuxedBuffer::maxDecoders];
            int tracks = analyzeHeader(buffer, streamInfo, sizeof(streamInfo) / sizeof(ac_initialization_segment_stream));
            if (tracks > 0)
            {
                DSAMPLES(dprintf("%s: received reinitialization!!!\n", __PRETTY_FUNCTION__));
                // this is re-initialization
                m_numStreamInfo = std::min(tracks, int(Acinerella::AcinerellaMuxedBuffer::maxDecoders));
                memcpy(m_streamInfo, streamInfo, m_numStreamInfo * sizeof(ac_initialization_segment_stream));
                updateMetadata();
                if (m_acinerella)
                {
                    m_dataProvider.push(SharedBuffer::create()); // used to fool read code in case there's no pending buffers
                    m_dataProvider.push(WTFMove(buffer), MediaSourceChunkReaderDataProvider::ChunkType::ReInitialization);
                    decodeAllMediaSamples(); // pull all the pending data until previous acinerella gets an EOF
                    DSAMPLES(dprintf("%s: decoded until the end of previous package\n", __PRETTY_FUNCTION__));
                    m_acinerella = nullptr;
                    DSAMPLES(dprintf("%s: killed old acinerella\n", __PRETTY_FUNCTION__));
                    return MediaSourceChunkReader::DecodePromise::createAndResolve(DecodeResult::Reinitialize);
                }
            }

            DSAMPLES(dprintf("%s: received buffer size %ld\n", __PRETTY_FUNCTION__, buffer->size()));
            m_dataProvider.push(WTFMove(buffer));
            if (initialize() && decodeAllMediaSamples())
                return MediaSourceChunkReader::DecodePromise::createAndResolve(DecodeResult::Samples);
            return MediaSourceChunkReader::DecodePromise::createAndReject();
        }
    });
}

void MediaSourceChunkReader::getSamples(MediaSamplesList& outSamples)
{
	auto lock = Locker(m_lock);
	std::swap(outSamples, m_samples);
	DSAMPLES(dprintf("[MS]%s: %d samples\n", __func__, outSamples.size()));
}

WebCore::SourceBufferPrivateClient::InitializationSegment MediaSourceChunkReader::getInitializationSegment()
{
    SourceBufferPrivateClient::InitializationSegment initializationSegment;
    double duration = 0;

    for (int i = 0; i < m_numStreamInfo; i++)
    {
        ac_initialization_segment_stream& info = m_streamInfo[i];
        duration = std::max(duration, info.duration/1000.0);

        DM(dprintf("%s: index %d st %d\n", __func__, i, info.type));

        switch (info.type)
        {
        case AC_STREAM_TYPE_VIDEO:
            {
                WebCore::SourceBufferPrivateClient::InitializationSegment::VideoTrackInformation videoTrackInformation;
                videoTrackInformation.track = m_trackFactory.videoTrack(i);
                videoTrackInformation.description = MediaDescriptionMorphOS::createVideoWithCodec(String::fromUTF8(info.codecName));
                initializationSegment.videoTracks.append(WTFMove(videoTrackInformation));
            }
            break;
            
        case AC_STREAM_TYPE_AUDIO:
            {
                WebCore::SourceBufferPrivateClient::InitializationSegment::AudioTrackInformation audioTrackInformation;
                audioTrackInformation.track = m_trackFactory.audioTrack(i);
                audioTrackInformation.description = MediaDescriptionMorphOS::createAudioWithCodec(String::fromUTF8(info.codecName));
                initializationSegment.audioTracks.append(WTFMove(audioTrackInformation));
            }
            break;

        default:
            break;
        }
    }

    initializationSegment.duration = MediaTime::createWithDouble(duration);
    return initializationSegment;
}

bool MediaSourceChunkReader::initialize()
{
    if (m_acinerella)
        return true;

	DINIT(dprintf("[MS]%s\n", __func__));
	EP_SCOPE(initialize);

	m_acinerella = Acinerella::AcinerellaPointer::create();

	if (m_acinerella)
	{
		DINIT(dprintf("[MS] ac_open()... \n"));

		if (-1 == ac_open(m_acinerella->instance(), static_cast<void *>(this), nullptr, &acReadCallback, nullptr, nullptr, nullptr))
		{
            DINIT(dprintf("[MS] ac_open() failure!\n"));
			return false;
		}
		
		DINIT(dprintf("[MS] ac_open() success!\n"));
		m_audioDecoderMask = 0;
		m_videoDecoderMask = 0;
		
		for (int i = 0; i < std::min(Acinerella::AcinerellaMuxedBuffer::maxDecoders, m_acinerella->instance()->stream_count); i++)
		{
			ac_stream_info info;
			ac_get_stream_info(m_acinerella->instance(), i, &info);

			switch (info.stream_type)
			{
			case AC_STREAM_TYPE_VIDEO:
				m_videoDecoderMask |= (1UL << i);
				m_numDecoders = std::max(m_numDecoders, i+1);
				break;

			case AC_STREAM_TYPE_AUDIO:
				m_audioDecoderMask |= (1UL << i);
				m_numDecoders = std::max(m_numDecoders, i+1);
				break;
			
			default:
				break;
			}
		}
  
        updateMetadata();
		
		return true;
	}
		
	return false;
}

void MediaSourceChunkReader::updateMetadata()
{
    double duration = 0;
    m_info.m_width = 0;
    m_info.m_isLive = false;
    m_info.m_channels = 0;
    m_info.m_isDownloadable = false;

    for (int i = 0; i < m_numStreamInfo; i++)
    {
        ac_initialization_segment_stream& info = m_streamInfo[i];
        duration = std::max(duration, info.duration/1000.0);

        DM(dprintf("%s: index %d st %d\n", __func__, i, info.type));

        switch (info.type)
        {
        case AC_STREAM_TYPE_VIDEO:
            {
                DM(dprintf("%s: video %d %f codec %s\n", __func__, i, float(duration), info.codecName));
                m_info.m_width = info.typeData.video.width;
                m_info.m_height = info.typeData.video.height;
                m_info.m_videoCodec = String::fromUTF8(info.codecName);
                m_info.m_bitRate = info.bitrate;
            }
            break;
            
        case AC_STREAM_TYPE_AUDIO:
            {
                DM(dprintf("%s: audio %d %f codec %s channels %d\n", __func__, i, float(duration), info.codecName, info.typeData.audio.channels));
                m_info.m_channels = info.typeData.audio.channels;
                m_info.m_frequency = info.typeData.audio.frequency;
                m_info.m_bits = info.typeData.audio.bits;
                m_info.m_audioCodec = String::fromUTF8(info.codecName);
            }
            break;

        default:
            break;
        }
    }

    m_info.m_duration = duration;
}

bool MediaSourceChunkReader::keepDecoding()
{
	auto lock = Locker(m_lock);

//if (m_videoDecoderMask != 0)
//dprintf("[MS]%s: term %d bs %d bp %d reof %d lover %d\n", __func__, m_terminating, m_buffer.size(), m_bufferPosition, m_readEOF, m_leftOver.size());

	if (m_terminating || m_underrunSignalled.loadRelaxed() || !m_acinerella)
		return false;

	return true;
}

bool MediaSourceChunkReader::decodeAllMediaSamples()
{
	DSAMPLES(dprintf("[MS]%s\n", __func__));
    m_underrunSignalled.store(false);

	m_queue.append(makeUnique<Function<void ()>>([this] {
        while (keepDecoding())
        {
            RefPtr<Acinerella::AcinerellaPackage> package = Acinerella::AcinerellaPackage::create(m_acinerella, ac_read_package(m_acinerella->instance()));
            if (package.get() && package->package())
            {
                TrackID trackID = package->index();
                if ((m_audioDecoderMask & (1uL << package->index())) || (m_videoDecoderMask & (1uL << package->index())))
                {
                    RefPtr<MediaSampleMorphOS> mediaSample = MediaSampleMorphOS::create(package, FloatSize(320, 240), trackID);

                    if (mediaSample->presentationTime().toDouble() - 1.0 > m_highestPTS)
                    {
                        m_highestPTS = mediaSample->presentationTime().toDouble();
                    }
    #if DN
                    m_decodeCount++;
                    #if DNVIDEOONLY
                    if (m_videoDecoderMask & (1uL << package->index()))
                    #endif
                        if (0 == (m_decodeCount % 15)) dprintf("%s(%p): %s sample created (PTS %f)\n", __func__, this, (m_audioDecoderMask & (1uLL << package->index())) ? "audio" : "video",
                            mediaSample->presentationTime().toFloat());
    #endif
                    auto lock = Locker(m_lock);
                    m_samples.emplace_back(mediaSample);
                }
                else
                {
                    DNERR(dprintf("%s: invalid packet\n", __func__));
                    // reject unknown packets completely
                    continue;
                }
            }
            else
            {
                break;
            }
        }
    }));

    while (!m_underrunSignalled.loadRelaxed()) {
        m_underrunEvent.waitFor(10_s);
    }

    return true;
}

int MediaSourceChunkReader::read(uint8_t *buf, int size)
{
    return m_dataProvider.pull(buf, size);
}

int MediaSourceChunkReader::acReadCallback(void *me, uint8_t *buf, int size)
{
	return static_cast<MediaSourceChunkReader *>(me)->read(buf, size);
}

}

#endif
