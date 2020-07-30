#import "WkDownload_private.h"
#import "WkNetworkRequestMutable.h"

#undef __OBJC__
#include "WebKit.h"
#include <wtf/WallTime.h>
#include <wtf/text/WTFString.h>
#include <WebCore/CurlDownload.h>
#include <WebCore/ResourceResponse.h>
#include <WebCore/TextEncoding.h>
#include <wtf/FileSystem.h>
#define __OBJC__

extern "C" { void dprintf(const char *, ...); }

#define D(x) x

@class _WkDownload;

class WebDownload final : public WebCore::CurlDownloadListener
{
public:
	void initialize(_WkDownload *outer, OBURL *url);
	void initialize(_WkDownload *outer, WebCore::ResourceHandle* handle, const WebCore::ResourceRequest& request, const WebCore::ResourceResponse& response);
	bool start();
	bool cancel();

	bool cancelForResume();
	bool resume();
	
	void deleteTmpFile();

    void didReceiveResponse(const WebCore::ResourceResponse& response) override;
    void didReceiveDataOfLength(int size) override;
    void didFinish() override;
    void didFail() override;

	size_t size() const { return m_size; }
	size_t downloadedSize() const { return m_receivedSize; }

private:
	_WkDownload                  *m_outerObject; // weak
    RefPtr<WebCore::CurlDownload> m_download;
    size_t                        m_size { 0 };
    size_t                        m_receivedSize { 0 };
};

@interface _WkDownload : WkDownload
{
	WkMutableNetworkRequest *_request;
	WebDownload              _download;
	OBURL                   *_url;
	OBString                *_filename;
	id<WkDownloadDelegate>   _delegate;
	bool                     _selfretained;
	bool                     _isPending;
	bool                     _isFailed;
	bool                     _isFinished;
}

- (id<WkDownloadDelegate>)delegate;
- (void)setFilename:(OBString *)f;
- (void)selfrelease;
- (void)setPending:(BOOL)pending;
- (void)setFailed:(BOOL)failed;
- (void)setFinished:(BOOL)fini;

@end

void WebDownload::initialize(_WkDownload *outer, OBURL *url)
{
	m_outerObject = outer;
	m_download = adoptRef(new WebCore::CurlDownload());
	if (m_download)
	{
		WTF::URL wurl(WTF::URL(), [[url absoluteString] cString]);
		m_download->init(*this, wurl);
	}
}

void WebDownload::initialize(_WkDownload *outer, WebCore::ResourceHandle*handle, const WebCore::ResourceRequest&request, const WebCore::ResourceResponse& response)
{
	m_outerObject = outer;
	m_download = adoptRef(new WebCore::CurlDownload());
	if (m_download)
	{
		auto uurl = response.url().string().utf8();
		auto umime = response.mimeType().utf8();
		auto uname = response.suggestedFilename().utf8();

		if (0 == uname.length())
			uname = WebCore::decodeURLEscapeSequences(response.url().lastPathComponent()).utf8();
		
		[m_outerObject setFilename:[OBString stringWithUTF8String:uname.data()]];
		m_size = response.expectedContentLength();
	
		m_download->init(*this, handle, request, response);
	}
}

bool WebDownload::start()
{
	if (!m_download)
		return false;
	m_download->start();
	
	[[m_outerObject delegate] downloadDidBegin:m_outerObject];
	
	return true;
}

bool WebDownload::cancel()
{
	D(dprintf("%s: dl %p\n", __PRETTY_FUNCTION__, m_download));
	if (!m_download)
		return true;
	m_download->setDeleteTmpFile(true);
	if (!m_download->cancel())
		return false;
	m_download->setListener(nullptr);
	m_download = nullptr;
	return true;
}

#pragma GCC push_options
#pragma GCC optimize ("O0") // or crash when resuming :|

bool WebDownload::cancelForResume()
{
	D(dprintf("%s: dl %p\n", __PRETTY_FUNCTION__, m_download));
	if (!m_download)
		return true;
	m_download->setDeleteTmpFile(false);
	if (!m_download->cancel())
		return false;
	D(dprintf("%s: dl %p done\n", __PRETTY_FUNCTION__, m_download));
	return true;
}

bool WebDownload::resume()
{
	if (m_download && m_download->isCancelled())
		m_download->resume();
}

#pragma GCC pop_options

void WebDownload::didReceiveResponse(const WebCore::ResourceResponse& response)
{
	[m_outerObject retain];
	if (m_download)
	{
		// (add received size to handle resuming)
		// try to keep m_size if already set! (see the case in which we dl from a pending response)
		// the response here is often bogus in that case :|
		if (0 == m_size || m_receivedSize)
			m_size = m_receivedSize + response.expectedContentLength();
		[[m_outerObject delegate] didReceiveResponse:m_outerObject];

		OBString *path = [m_outerObject filename];

		if (0 == [path length])
		{
			String suggestedFilename = response.suggestedFilename();
			if (suggestedFilename.isEmpty())
				suggestedFilename = response.url().lastPathComponent();
			suggestedFilename = WebCore::decodeURLEscapeSequences(suggestedFilename);
			
			auto usuggestedFilename = suggestedFilename.utf8();
			path = [[m_outerObject delegate] decideFilenameForDownload:m_outerObject withSuggestedName:[OBString stringWithUTF8String:usuggestedFilename.data()]];
		}
		else
		{
			path = [[m_outerObject delegate] decideFilenameForDownload:m_outerObject withSuggestedName:path];
		}
		
		if (path)
		{
			[m_outerObject setFilename:path];
			m_download->setDestination(WTF::String::fromUTF8([path nativeCString]));
		}
		else
		{
			[m_outerObject cancel];
		}
	}
	[m_outerObject autorelease];
}

void WebDownload::didReceiveDataOfLength(int size)
{
	m_receivedSize += size;
	[[m_outerObject delegate] download:m_outerObject didReceiveBytes:size];
}

void WebDownload::didFinish()
{
	m_download->setDeleteTmpFile(false);
	m_download->setListener(nullptr);
	m_download = nullptr;

	[m_outerObject setFinished:YES];
	[m_outerObject setPending:NO];
	[[m_outerObject delegate] downloadDidFinish:m_outerObject];
	[m_outerObject selfrelease];
}

void WebDownload::didFail()
{
	[m_outerObject setPending:NO];
	[m_outerObject setFailed:YES];
	
	if (m_download)
		m_download->setDeleteTmpFile(true);

	[[m_outerObject delegate] download:m_outerObject didFailWithError:nil];
	[m_outerObject selfrelease];
}

@implementation _WkDownload

- (WkDownload *)initWithURL:(OBURL *)url withDelegate:(id<WkDownloadDelegate>)delegate
{
	if ((self = [super init]))
	{
		_url = [url retain];
		_delegate = [delegate retain];
		_isPending = false;
		_isFailed = false;
		_download.initialize(self, _url);
	}
	
	return self;
}

- (WkDownload *)initWithRequest:(WkMutableNetworkRequest *)request withDelegate:(id<WkDownloadDelegate>)delegate
{
	if ((self = [super init]))
	{
		_delegate = [delegate retain];
		_request = [request retain];
		_url = [[request URL] retain];
		_isPending = false;
		_isFailed = false;
		_download.initialize(self, _url);
	}
	
	return self;
}

- (WkDownload *)initWithHandle:(WebCore::ResourceHandle*)handle request:(const WebCore::ResourceRequest&)request response:(const WebCore::ResourceResponse&)response withDelegate:(id<WkDownloadDelegate>)delegate
{
	if ((self = [super init]))
	{
		auto uurl = response.url().string().utf8();
		_url = [[OBURL URLWithString:[OBString stringWithUTF8String:uurl.data()]] retain];
		_delegate = [delegate retain];
		_isPending = false;
		_isFailed = false;
		_download.initialize(self, handle, request, response);
	}
	return self;
}

- (void)dealloc
{
	[self cancel];
	[_url release];
	[_request release];
	[_delegate release];
	[_filename release];
	[super dealloc];
}

- (id<WkDownloadDelegate>)delegate
{
	return _delegate;
}

- (void)start
{
	if (_download.start())
	{
		_isPending = true;
		@synchronized (self) {
			if (!_selfretained)
			{
				[self retain];
				_selfretained = YES;
			}
		}
	}
}

- (void)selfrelease
{
	@synchronized (self) {
		if (_selfretained)
		{
			[self autorelease];
			_selfretained = NO;
		}
	}
}

- (void)cancel
{
	_isPending = false;
	_download.cancel();
}

- (void)cancelForResume
{
	_isPending = false;
	_download.cancelForResume();
}

- (BOOL)canResumeDownload
{
	return !_isPending && !_isFinished;
}

- (void)resume
{
	if (_download.resume())
	{
		_isPending = YES;
		_isFinished = NO;
		_isFailed = NO;
		@synchronized (self) {
			if (!_selfretained)
			{
				[self retain];
				_selfretained = YES;
			}
		}
	}
}

- (OBURL *)url
{
	return _url;
}

- (OBString *)filename
{
	return _filename;
}

- (void)setFilename:(OBString *)filename
{
	[_filename autorelease];
	_filename = [filename retain];
}

- (size_t)size
{
	return _download.size();
}

- (size_t)downloadedSize
{
	return _download.downloadedSize();
}

- (BOOL)isPending
{
	return _isPending;
}

- (BOOL)isFailed
{
	return _isFailed;
}

- (void)setPending:(BOOL)pending
{
	_isPending = pending;
}

- (void)setFailed:(BOOL)failed
{
	_isFailed = failed;
}

- (void)setFinished:(BOOL)fini
{
	_isFinished = fini;
}

- (BOOL)isFinished
{
	_isFinished;
}

@end

@implementation WkDownload

+ (WkDownload *)download:(OBURL *)url withDelegate:(id<WkDownloadDelegate>)delegate
{
	return [[[_WkDownload alloc] initWithURL:url withDelegate:delegate] autorelease];
}

+ (WkDownload *)downloadRequest:(WkMutableNetworkRequest *)request withDelegate:(id<WkDownloadDelegate>)delegate
{
	return [[[_WkDownload alloc] initWithRequest:request withDelegate:delegate] autorelease];
}

+ (WkDownload *)downloadWithHandle:(WebCore::ResourceHandle*)handle request:(const WebCore::ResourceRequest&)request response:(const WebCore::ResourceResponse&)response withDelegate:(id<WkDownloadDelegate>)delegate
{
	return [[[_WkDownload alloc] initWithHandle:handle request:request response:response withDelegate:delegate] autorelease];
}

+ (void)setDownloadPath:(OBString *)path
{
	const char *cpath = [path nativeCString];
	WebCore::CurlRequest::SetDownloadPath(WTF::String(cpath, strlen(cpath), MIBENUM_SYSTEM));
	WTF::FileSystemImpl::setTemporaryFilePathForPrefix(cpath, "download");
}

- (void)start
{

}

- (void)cancel
{

}

- (void)cancelForResume
{

}

- (BOOL)canResumeDownload
{
	return 0;
}

- (void)resume
{

}

- (size_t)size
{
	return 0;
}

- (size_t)downloadedSize
{
	return 0;
}

- (BOOL)isPending
{
	return 0;
}

- (BOOL)isFailed
{
	return 0;
}

- (BOOL)isFinished
{
	return 0;
}

- (OBURL *)url
{
	return nil;
}

- (OBString *)filename
{
	return nil;
}

@end
