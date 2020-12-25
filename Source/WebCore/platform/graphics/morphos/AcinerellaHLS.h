#pragma once

#include "config.h"

#if ENABLE(VIDEO)

#include "AcinerellaBuffer.h"
#include <wtf/RunLoop.h>

namespace WebCore {
namespace Acinerella {

struct HLSStreamInfo
{
	String         m_url;
	Vector<String> m_codecs;
	int            m_bandwidth = 0;
};

struct HLSChunk
{
	String    m_url;
	int64_t   m_mediaSequence;
	float     m_duration;
};

class HLSStream
{
public:
	HLSStream() = default;
	HLSStream(const String &baseURL, const String &data);
	HLSStream& operator+=(HLSStream& append);

	const HLSChunk &current() const { static HLSChunk empty; return m_chunks.empty() ? empty: m_chunks.front(); }
	void pop() { if (!m_chunks.empty()) m_chunks.pop(); }
	bool ended() const { return m_ended; }
	bool empty() const { return m_chunks.empty(); }
	size_t size() const { return m_chunks.size(); }
	void clear() { while (!empty()) pop(); m_mediaSequence = -1; }
	int targetDuration() const { return m_targetDuration; }
	int64_t mediaSequence() const { return m_mediaSequence; }

protected:
	std::queue<HLSChunk> m_chunks;
	int64_t              m_mediaSequence = -1;
	int                  m_targetDuration = 0;
	bool                 m_ended = false;
};

class AcinerellaNetworkBufferHLS : public AcinerellaNetworkBuffer
{
public:
	AcinerellaNetworkBufferHLS(const String &url, size_t readAhead);
	virtual ~AcinerellaNetworkBufferHLS();

	void start(uint64_t from = 0) override;
	void stop() override;

	int read(uint8_t *outBuffer, int size, int64_t ignore) override;

	int64_t length() override;
	int64_t position() override;

protected:

	void masterPlaylistReceived(bool succ);
	void childPlaylistReceived(bool succ);

	void refreshTimerFired();
	void chunkSwallowed();

protected:
	RunLoop::Timer<AcinerellaNetworkBufferHLS>  m_playlistRefreshTimer;
	RefPtr<AcinerellaNetworkBuffer>             m_chunkRequest;
	RefPtr<AcinerellaNetworkBuffer>             m_chunkRequestInRead;
	std::queue<RefPtr<AcinerellaNetworkBuffer>> m_chunksRequestPreviouslyRead;
	RefPtr<AcinerellaNetworkFileRequest>        m_hlsRequest;
	Vector<HLSStreamInfo>                       m_streams;
	BinarySemaphore                             m_event;
	HLSStream                                   m_stream;
	Lock                                        m_lock;
	bool                                        m_hasMasterList = false;
	bool                                        m_stopping = false;
};

}
}

#endif
