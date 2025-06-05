#if ENABLE(VIDEO) && ENABLE(MEDIA_SOURCE)

#include "AcinerellaPointer.h"
#include "AcinerellaBuffer.h"
#include "AcinerellaMuxer.h"
#include "AcinerellaDecoder.h"
#include "MediaPlayerMorphOS.h"
#include "AudioTrackPrivateMorphOS.h"
#include "VideoTrackPrivateMorphOS.h"

#include "SourceBufferPrivate.h"
#include "SourceBufferPrivateClient.h"
#include "MediaSample.h"

#include <wtf/Function.h>
#include <wtf/MessageQueue.h>
#include <wtf/Threading.h>
#include <wtf/StdList.h>
#include <wtf/text/WTFString.h>
#include <wtf/ThreadSafeRefCounted.h>
#include <wtf/RunLoop.h>
#include <wtf/NativePromise.h>

#include <dos/dos.h>

namespace WebCore {

class MediaSourceChunkReaderTrackFactory
{
public:
    virtual ~MediaSourceChunkReaderTrackFactory() = default;
    virtual RefPtr<VideoTrackPrivateMorphOS> videoTrack(int index) const = 0;
    virtual RefPtr<AudioTrackPrivateMorphOS> audioTrack(int index) const = 0;
};

class MediaSourceChunkReaderDataProvider
{
public:
    MediaSourceChunkReaderDataProvider() = default;
    ~MediaSourceChunkReaderDataProvider() = default;

    void initialize(Function<void(void)>&& underrun);

    // any thread
    enum class ChunkType { Data, ReInitialization };
    void push(Ref<SharedBuffer>&&, ChunkType = ChunkType::Data);
    void terminate();

    // io thread
    int pull(uint8_t *buf, int size);

protected:
    struct Entry { RefPtr<SharedBuffer> buffer; ChunkType chunkType; };
    Vector<Entry>   m_queue;
	BinarySemaphore m_event;
    Lock            m_lock;
    int             m_bufferPosition = 0;
    Atomic<bool>    m_terminated = false;
    Function<void(void)> m_onDataUnderrun;
};

class MediaSourceChunkReader : public WTF::ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr<MediaSourceChunkReader>
{
protected:
	MediaSourceChunkReader(MediaSourceChunkReaderTrackFactory& trackFactory);

public:
	virtual ~MediaSourceChunkReader();

	typedef WTF::StdList<WTF::RefPtr<WebCore::MediaSample>> MediaSamplesList;

    static RefPtr<MediaSourceChunkReader> create(MediaSourceChunkReaderTrackFactory& trackFactory) {
		return adoptRef(*new MediaSourceChunkReader(trackFactory));
	}

    enum class DecodeResult { InitialInitialize, Reinitialize, Samples };

    using DecodePromise = NativePromise<DecodeResult, void>;
    Ref<DecodePromise> decodeAsync(Ref<SharedBuffer>&&);

	void getSamples(MediaSamplesList& outSamples);
    WebCore::SourceBufferPrivateClient::InitializationSegment getInitializationSegment();
    const MediaPlayerMorphOSInfo& getInfo() const { return m_info; }
	void terminate();

	int numDecoders() const { return m_numDecoders; }
	double highestPTS() const { return m_highestPTS; }

	RefPtr<Acinerella::AcinerellaPointer>& acinerella() { return m_acinerella; }

protected:
	bool initialize();
	bool decodeAllMediaSamples();
	bool keepDecoding();
    void updateMetadata();

	int read(uint8_t *buf, int size);
   
	static int acReadCallback(void *me, uint8_t *buf, int size);

    int analyzeHeader(Ref<SharedBuffer>&, ac_initialization_segment_stream* streams, size_t streamsMax);

protected:
	RefPtr<Acinerella::AcinerellaPointer> m_acinerella;
    MediaSourceChunkReaderTrackFactory&   m_trackFactory;
	uint32_t                              m_audioDecoderMask = 0;
	uint32_t                              m_videoDecoderMask = 0;

	MediaSamplesList                      m_samples;
    Lock                                  m_lock;

    MediaPlayerMorphOSInfo                m_info;

    // For promises
    RefPtr<WorkQueue>                     m_workQueue;

    // I/O Thread
    RefPtr<Thread>                        m_thread;
    MessageQueue<Function<void ()>>       m_queue;
    MediaSourceChunkReaderDataProvider    m_dataProvider;
    BinarySemaphore                       m_underrunEvent;

    Atomic<bool>                          m_underrunSignalled = false;
	bool                                  m_terminating = false;
    bool                                  m_initializationDone = false;
	bool                                  m_signalComplete = true;

	double                                m_highestPTS = 0.0;

	int                                   m_decodeCount = 0;
	int                                   m_decodeAppendCount = 0;
	int                                   m_numDecoders = 0;

    ac_initialization_segment_stream      m_streamInfo[Acinerella::AcinerellaMuxedBuffer::maxDecoders];
    int                                   m_numStreamInfo = 0;

	BPTR                                  m_debugFile = 0;
};

}

#endif
