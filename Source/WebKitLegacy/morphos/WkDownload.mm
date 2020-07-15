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
	void setDeletesFileOnFailure(bool deletes);

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
}

- (id<WkDownloadDelegate>)delegate;
- (void)setFilename:(OBString *)f;
- (void)selfrelease;
- (void)setPending:(BOOL)pending;
- (void)setFailed:(BOOL)failed;

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
	if (!m_download->cancel())
		return false;
	m_download->setListener(nullptr);
	m_download = nullptr;
	return true;
}

bool WebDownload::cancelForResume()
{
	D(dprintf("%s: dl %p\n", __PRETTY_FUNCTION__, m_download));
	if (!m_download)
		return true;
	if (!m_download->cancel())
		return false;
	m_download->setListener(nullptr);
	m_download = nullptr;
	// TODO ?
	return true;
}

void WebDownload::didReceiveResponse(const WebCore::ResourceResponse& response)
{
	[m_outerObject retain];
	if (m_download)
	{
		m_size = response.expectedContentLength();
		[[m_outerObject delegate] didReceiveResponse:m_outerObject];
		
		String suggestedFilename = response.suggestedFilename();
		if (suggestedFilename.isEmpty())
			suggestedFilename = FileSystem::pathGetFileName(response.url().string());
		suggestedFilename = WebCore::decodeURLEscapeSequences(suggestedFilename);
		
		auto usuggestedFilename = suggestedFilename.utf8();
		OBString *path = [[m_outerObject delegate] decideFilenameForDownload:m_outerObject withSuggestedName:[OBString stringWithUTF8String:usuggestedFilename.data()]];
		
		if (path)
		{
			[m_outerObject setFilename:[path copy]];
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
	[m_outerObject setPending:NO];
	[[m_outerObject delegate] downloadDidFinish:m_outerObject];
	[m_outerObject selfrelease];
}

void WebDownload::didFail()
{
	[m_outerObject setPending:NO];
	[m_outerObject setFailed:YES];

	[[m_outerObject delegate] download:m_outerObject didFailWithError:nil];
	[m_outerObject selfrelease];
}

void WebDownload::setDeletesFileOnFailure(bool deletes)
{
	if (m_download)
		m_download->setDeletesFileUponFailure(deletes);
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
// https://curl.haxx.se/libcurl/c/CURLOPT_RESUME_FROM_LARGE.html
	return 0;
}

- (void)setDeletesFilesUponFailure:(BOOL)deleteOnFailure
{
	_download.setDeletesFileOnFailure(deleteOnFailure);
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

- (void)setDeletesFilesUponFailure:(BOOL)deleteOnFailure
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

- (OBURL *)url
{
	return nil;
}

- (OBString *)filename
{
	return nil;
}

@end
