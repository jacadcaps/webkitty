#include "AcinerellaHLS.h"

#if ENABLE(VIDEO)

#include <proto/exec.h>

namespace WebCore {
namespace Acinerella {

#define D(x) 

static const String rnReplace("\r\n");
static const String rnReplacement("\n");

class HLSMasterPlaylistParser
{
public:
	HLSMasterPlaylistParser(const String &baseURL, const String &sdata)
	{
		Vector<String> lines = String(sdata).replace(rnReplace, rnReplacement).split('\n');

		// Must contain at the very least:
		// #EXTM3U
		// #EXT-X-VERSION:*
		// #EXT-X-STREAM-INF
		// url
		//
		// So 4 lines at minimum, otherwise it won't be a valid playlist
		if (lines.size() >= 4 && equalIgnoringASCIICase(lines[0], "#extm3u"))
		{
			// Format kida-verified. Look for Streams
			HLSStreamInfo info;
			bool hopingForM3U8 = false;
			bool foundChunks = false;

			for (size_t i = 2; i < lines.size(); i++)
			{
				String& line = lines[i];

				if (startsWithLettersIgnoringASCIICase(line, "#ext-x-stream-inf:"))
				{
					info.m_url = "";
					info.m_codecs.clear();
					info.m_bandwidth = 0;

// TODO: codecs, bandwidth, etc

					// next line should be our link...
					hopingForM3U8 = true;
				}
				else if (hopingForM3U8)
				{
					if (line.contains("m3u8"))
					{
						if (line.contains(':'))
						{
							info.m_url = line;
						}
						else
						{
							auto pos = baseURL.reverseFind('/');
							if (pos != WTF::notFound)
							{
								info.m_url = baseURL.substring(0, pos + 1);
								info.m_url.append(line);
							}
						}
						m_streams.append(info);
						info.m_url = "";
						hopingForM3U8 = false;
					}
				}
				else if (startsWithLettersIgnoringASCIICase(line, "#extinf:"))
				{
					foundChunks = true;
				}
			}
			
			if (info.m_url.length())
			{
				m_streams.append(info);
			}
			
			// this isn't master but a direct HLS link!
			if (m_streams.size() == 0 && foundChunks)
			{
				info.m_url = baseURL;
				m_streams.append(info);
				D(dprintf("%s: this is no master HLS %s\n", __PRETTY_FUNCTION__, baseURL.utf8().data()));
			}
		}
	}
	
	Vector<HLSStreamInfo> &streams() { return m_streams; }

protected:
	Vector<HLSStreamInfo> m_streams;
};

HLSStream::HLSStream(const String &baseURL, const String &sdata)
{
	Vector<String> lines = String(sdata).replace(rnReplace, rnReplacement).split('\n');

	if (lines.size() >= 2 && equalIgnoringASCIICase(lines[0], "#EXTM3U"))
	{
		bool hopingForURL = false;
		double duration = 0.f;
		m_mediaSequence = -1;

		for (size_t i = 1; i < lines.size(); i++)
		{
			auto &line = lines[i];

			if (startsWithLettersIgnoringASCIICase(line, "#ext-x-media-sequence"))
			{
				m_mediaSequence = line.substring(22).toUInt64();
				D(dprintf("mediaseq: %llu\n", m_mediaSequence));
				m_mediaSequence--; //! we want 1st added chunk to have the right sequence!
			}
			else if (startsWithLettersIgnoringASCIICase(line, "#extinf:"))
			{
				duration = line.substring(8).toDouble();
				hopingForURL = true;
			}
			else if (hopingForURL && !startsWithLettersIgnoringASCIICase(line,"#"))
			{
				HLSChunk chunk;
				chunk.m_mediaSequence = ++m_mediaSequence;
				chunk.m_duration = duration;

				if (line.contains(':'))
				{
					chunk.m_url = line;
				}
				else
				{
					auto pos = baseURL.reverseFind('/');
					if (pos != WTF::notFound)
					{
						chunk.m_url = baseURL.substring(0, pos + 1);
						chunk.m_url.append(line);
					}
				}

				m_chunks.emplace(WTFMove(chunk));
			}
			// dprintf("parse l '%s' (got %d)\n", line.utf8().data(), m_chunks.size());
		}
	}
}

HLSStream& HLSStream::operator+=(HLSStream& append)
{
	while (!append.m_chunks.empty())
	{
		if (append.m_chunks.front().m_mediaSequence > m_mediaSequence || (m_chunks.empty() && m_mediaSequence == -1))
		{
			m_mediaSequence = append.m_chunks.front().m_mediaSequence;
			m_chunks.emplace(WTFMove(append.m_chunks.front()));
		}

		append.m_chunks.pop();
	}

	return *this;
}

AcinerellaNetworkBufferHLS::AcinerellaNetworkBufferHLS(AcinerellaNetworkBufferResourceLoaderProvider *resourceProvider, const String &url, size_t readAhead)
	: AcinerellaNetworkBuffer(resourceProvider, url, readAhead)
	, m_playlistRefreshTimer(RunLoop::current(), this, &AcinerellaNetworkBufferHLS::refreshTimerFired)
{
	D(dprintf("%s(%p) - url %s\n", __func__, this, url.utf8().data()));
}

AcinerellaNetworkBufferHLS::~AcinerellaNetworkBufferHLS()
{
	D(dprintf("%s(%p) \n", __func__, this));
	stop();
	if (m_chunkRequestInRead)
		m_chunkRequestInRead->die();
}

// main thread
void AcinerellaNetworkBufferHLS::start(uint64_t from)
{
	D(dprintf("%s(%p) - from %llu\n", __func__, this, from));
	(void)from;

	if (!m_hasMasterList)
	{
		m_stopping = false;
		m_stream.clear();
		m_hasMasterList = false;

		m_hlsRequest = AcinerellaNetworkFileRequest::create(m_url, [this](bool succ) { masterPlaylistReceived(succ); });
	}
	else
	{
		D(dprintf("%s(%p) - restart?!?\n", __func__, this));
	}
}

// main thread
void AcinerellaNetworkBufferHLS::stop()
{
	D(dprintf("%s(%p) \n", __func__, this));
	m_stopping = true;

	if (m_hlsRequest)
		m_hlsRequest->cancel();
	m_hlsRequest = nullptr;
	
	if (m_chunkRequest)
		m_chunkRequest->stop();

	{
		auto lock = holdLock(m_lock);
		m_chunkRequest = nullptr;
		
		if (m_chunkRequestInRead)
			m_chunkRequestInRead->die();
	}

	m_event.signal();

	D(dprintf("%s(%p) killing old chunks\n", __func__, this));
	auto lock = holdLock(m_lock);
	while (!m_chunksRequestPreviouslyRead.empty())
	{
		m_chunksRequestPreviouslyRead.front()->die();
		m_chunksRequestPreviouslyRead.pop();
	}
}

// main thread
void AcinerellaNetworkBufferHLS::masterPlaylistReceived(bool succ)
{
	D(dprintf("%s(%p) \n", __func__, this));
	if (succ && m_hlsRequest)
	{
		auto buffer = m_hlsRequest->buffer();
		m_hlsRequest->cancel();
		if (buffer && buffer->size())
		{
			HLSMasterPlaylistParser parser(m_url, String::fromUTF8(buffer->data(), buffer->size()));

			m_hasMasterList = true;
			m_streams = WTFMove(parser.streams());

			if (m_streams.size())
			{
				m_hlsRequest = AcinerellaNetworkFileRequest::create(m_streams[0].m_url, [this](bool succ) { childPlaylistReceived(succ); });
				return;
			}
		}
	}
	
	if (m_hlsRequest)
		m_hlsRequest->cancel();
	m_hlsRequest = nullptr;
}

// main thread
void AcinerellaNetworkBufferHLS::childPlaylistReceived(bool succ)
{
	D(dprintf("%s(%p) \n", __func__, this));
	if (succ && m_hlsRequest)
	{
		auto buffer = m_hlsRequest->buffer();
		m_hlsRequest->cancel();

		if (buffer && buffer->size())
		{
			HLSStream stream(m_url, String::fromUTF8(buffer->data(), buffer->size()));
			m_stream += stream; // append and merge :)
			
			D(dprintf("%s(%p) queue %d mediaseq %llu %d cr %p\n", __func__, this, m_stream.size(), m_stream.mediaSequence(), m_stream.empty(), m_chunkRequest.get()));
			// start loading chunks!
			if (!m_stream.empty() && !m_chunkRequest)
			{
				D(dprintf("%s(%p) chunk '%s' duration %f\n", __func__, this, m_stream.current().m_url.utf8().data(), m_stream.current().m_duration));
				auto chunkRequest = AcinerellaNetworkBuffer::create(m_provider, m_stream.current().m_url);
				m_stream.pop();
				
				{
					auto lock = holdLock(m_lock);
					m_chunkRequest = chunkRequest;
				}
				
				chunkRequest->start();
				
				// wake up the ::read
				m_event.signal();
			}
		}
	}

	double duration = m_stream.targetDuration();
	if (duration <= 1.0 && !m_stream.empty())
		duration = std::min(m_stream.current().m_duration, duration);
	if (duration <= 1.0)
		duration = 3.0;
	duration *= 0.5;
	D(dprintf("%s(%p): refresh in %f \n", __func__, this, duration));
	m_playlistRefreshTimer.startOneShot(Seconds(duration));

	if (m_hlsRequest)
		m_hlsRequest->cancel();
	m_hlsRequest = nullptr;
}

// main thread
void AcinerellaNetworkBufferHLS::refreshTimerFired()
{
	D(dprintf("%s(%p) \n", __func__, this));
	if (m_hlsRequest)
		m_hlsRequest->cancel();
	m_hlsRequest = AcinerellaNetworkFileRequest::create(m_streams[0].m_url, [this](bool succ) { childPlaylistReceived(succ); });
}

// main thread
void AcinerellaNetworkBufferHLS::chunkSwallowed()
{
	D(dprintf("%s(%p): cr %p streams %d\n", __func__, this, m_chunkRequest.get(), m_stream.size()));

	{
		auto lock = holdLock(m_lock);
		while (!m_chunksRequestPreviouslyRead.empty())
		{
			m_chunksRequestPreviouslyRead.front()->stop();
			m_chunksRequestPreviouslyRead.pop();
		}
	}

	if (!m_stream.empty())
	{
		D(dprintf("%s(%p): load '%s' queue %d\n", __func__, this, m_stream.current().m_url.utf8().data(), m_stream.size()));
		auto chunkRequest = AcinerellaNetworkBuffer::create(m_provider, m_stream.current().m_url);
		m_stream.pop();
		
		{
			auto lock = holdLock(m_lock);
			m_chunkRequest = chunkRequest;
		}
		
		chunkRequest->start();
		
		// wake up the ::read
		m_event.signal();
	}
	else
	{
		D(dprintf("%s:(%p): ran out of chunks!!\n", __func__, this));
	}
}

int64_t AcinerellaNetworkBufferHLS::length()
{
	auto lock = holdLock(m_lock);
	if (m_chunkRequestInRead)
		return m_chunkRequestInRead->length();
	return 0;
}

int64_t AcinerellaNetworkBufferHLS::position()
{
	auto lock = holdLock(m_lock);
	if (m_chunkRequestInRead)
		return m_chunkRequestInRead->position();
	return 0;
}

// acinerella decoder thread
int AcinerellaNetworkBufferHLS::read(uint8_t *outBuffer, int size, int64_t readPosition)
{
	D(dprintf("%s(%p): requested %ld inread %p\n", __PRETTY_FUNCTION__, this, size, m_chunkRequestInRead.get()));

	while (!m_stopping)
	{
		bool needsToWait = false;

		if (m_chunkRequestInRead)
		{
			int read = m_chunkRequestInRead->read(outBuffer, size, readPosition);

			if (0 == read)
			{
				{
					auto lock = holdLock(m_lock);
					m_chunksRequestPreviouslyRead.emplace(m_chunkRequestInRead);
					m_chunkRequestInRead = nullptr;
					D(dprintf("%s(%p): discontinuity! \n", __PRETTY_FUNCTION__, this));
				}

				return eRead_Discontinuity;
			}
			else
			{
				D(dprintf("%s(%p): read %d from chunk %p bytesleft %lld\n", __PRETTY_FUNCTION__, this, read, m_chunkRequestInRead.get(),
					m_chunkRequestInRead->length() - m_chunkRequestInRead->position()));
				return read;
			}
		}

		{
			auto lock = holdLock(m_lock);
			m_chunkRequestInRead = m_chunkRequest;
			m_chunkRequest = nullptr;
			needsToWait = m_chunkRequestInRead.get() == nullptr;
			D(dprintf("%s(%p): chunk swapped to %p needswait %d\n", __PRETTY_FUNCTION__, this, m_chunkRequestInRead.get(), needsToWait));
		}

		WTF::callOnMainThread([this, protect = makeRef(*this)]() {
			chunkSwallowed();
		});

		if (needsToWait)
		{
			D(dprintf("%s: read stall!\n", __PRETTY_FUNCTION__));
			m_event.waitFor(10_s);
		}
	}
	
	if (m_stopping && m_chunkRequestInRead)
	{
		auto lock = holdLock(m_lock);
		m_chunksRequestPreviouslyRead.emplace(m_chunkRequestInRead);
		m_chunkRequestInRead = nullptr;
	}

	D(dprintf("%s(%p): failing out (%d)\n", __PRETTY_FUNCTION__, this, m_stopping));
	return -1;
}

}
}

#undef D
#endif
