/*
 * Copyright (C) 2013 Apple Inc.  All rights reserved.
 * Copyright (C) 2017 Sony Interactive Entertainment Inc.
 * Copyright (C) 2017 NAVER Corp.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "CurlDownload.h"

#if USE(CURL)

#include "HTTPParsers.h"
#include "ResourceRequest.h"
#include "ResourceResponse.h"
#include "SharedBuffer.h"
#include "SynchronousLoaderClient.h"
#include "ResourceHandle.h"
#include "ResourceHandleInternal.h"

namespace WebCore {

CurlDownload::~CurlDownload()
{
    if (m_curlRequest)
        m_curlRequest->invalidateClient();
	if (m_deleteTmpFile && m_curlRequest)
        FileSystem::deleteFile(m_curlRequest->getDownloadedFilePath());
}

void CurlDownload::init(CurlDownloadListener& listener, const URL& url)
{
    m_listener = &listener;
    m_request.setURL(url);
}

void CurlDownload::init(CurlDownloadListener& listener, ResourceHandle* handle, const ResourceRequest& request, const ResourceResponse&)
{
    m_listener = &listener;
    m_request = request.isolatedCopy();
	
	if (handle)
	{
		auto* internal = handle->getInternal();
		if (internal && internal->m_curlRequest)
		{
			m_user = internal->m_curlRequest->user();
			m_password = internal->m_curlRequest->password();
		}
	}
}

void CurlDownload::start()
{
    ASSERT(isMainThread());

	m_isCancelled = false;
    m_curlRequest = createCurlRequest(m_request);
    m_curlRequest->enableDownloadToFile();
    m_curlRequest->setDeletesDownloadFileOnCancelOrError(m_deleteTmpFile);
	m_curlRequest->setUserPass(m_user, m_password);
    m_curlRequest->start();
}

void CurlDownload::resume()
{
    ASSERT(isMainThread());
    ASSERT(m_curlRequest);

	if (m_curlRequest && m_curlRequest->isCompletedOrCancelled())
	{
		String oldPath = m_curlRequest->getDownloadedFilePath();
		m_isCancelled = false;
		m_curlRequest = createCurlRequest(m_request);
		m_curlRequest->resumeDownloadToFile(oldPath);
		m_curlRequest->setUserPass(m_user, m_password);
		m_curlRequest->setDeletesDownloadFileOnCancelOrError(m_deleteTmpFile);
		m_curlRequest->start();
	}
}

bool CurlDownload::cancel()
{
    m_isCancelled = true;

    if (m_curlRequest)
        m_curlRequest->cancel();

    return true;
}

void CurlDownload::setDeleteTmpFile(bool deleteTmpFile)
{
	m_deleteTmpFile = deleteTmpFile;
	if (m_curlRequest)
		m_curlRequest->setDeletesDownloadFileOnCancelOrError(m_deleteTmpFile);
}

Ref<CurlRequest> CurlDownload::createCurlRequest(ResourceRequest& request)
{
    // FIXME: Use a correct sessionID.
    auto curlRequest = CurlRequest::create(request, *this);
    return curlRequest;
}

void CurlDownload::curlDidReceiveResponse(CurlRequest& request, CurlResponse&& response)
{
    ASSERT(isMainThread());

    if (m_isCancelled)
        return;

    m_response = ResourceResponse(response);

    if (m_response.shouldRedirect()) {
        willSendRequest();
        return;
    }

    if (m_listener)
        m_listener->didReceiveResponse(m_response);

    request.completeDidReceiveResponse();
}


void CurlDownload::curlDidReceiveBuffer(CurlRequest&, Ref<SharedBuffer>&& buffer)
{
    ASSERT(isMainThread());

    if (m_isCancelled)
        return;

    if (m_listener)
        m_listener->didReceiveDataOfLength(buffer->size());
}

void CurlDownload::curlDidComplete(CurlRequest& request, NetworkLoadMetrics&&)
{
    ASSERT(isMainThread());

    if (m_isCancelled)
        return;

    if (!m_destination.isEmpty()) {
        if (!request.getDownloadedFilePath().isEmpty())
            FileSystem::moveFile(request.getDownloadedFilePath(), m_destination);
    }

    if (m_listener)
        m_listener->didFinish();
}

void CurlDownload::curlDidFailWithError(CurlRequest& request, ResourceError&&error, CertificateInfo&&)
{
    ASSERT(isMainThread());

    if (m_isCancelled)
        return;

    if (m_deletesFileUponFailure && !request.getDownloadedFilePath().isEmpty())
        FileSystem::deleteFile(request.getDownloadedFilePath());

    if (m_listener)
        m_listener->didFail(error);
}

bool CurlDownload::shouldRedirectAsGET(const ResourceRequest& request, bool crossOrigin)
{
    if ((request.httpMethod() == "GET") || (request.httpMethod() == "HEAD"))
        return false;

    if (!request.url().protocolIsInHTTPFamily())
        return true;

    if (m_response.isSeeOther())
        return true;

    if ((m_response.isMovedPermanently() || m_response.isFound()) && (request.httpMethod() == "POST"))
        return true;

    if (crossOrigin && (request.httpMethod() == "DELETE"))
        return true;

    return false;
}

void CurlDownload::willSendRequest()
{
    ASSERT(isMainThread());

    static const int maxRedirects = 20;

    if (m_redirectCount++ > maxRedirects) {
        if (m_listener)
            m_listener->didFail(ResourceError::httpError(CURLE_TOO_MANY_REDIRECTS, m_request.url()));
        return;
    }

    String location = m_response.httpHeaderField(HTTPHeaderName::Location);
    URL newURL = URL(m_response.url(), location);
    bool crossOrigin = !protocolHostAndPortAreEqual(m_request.url(), newURL);

    ResourceRequest newRequest = m_request;
    newRequest.setURL(newURL);

    if (shouldRedirectAsGET(newRequest, crossOrigin)) {
        newRequest.setHTTPMethod("GET");
        newRequest.setHTTPBody(nullptr);
        newRequest.clearHTTPContentType();
    }

    if (crossOrigin) {
        // If the network layer carries over authentication headers from the original request
        // in a cross-origin redirect, we want to clear those headers here. 
        newRequest.clearHTTPAuthorization();
        newRequest.clearHTTPOrigin();
    }

    m_curlRequest->cancel();

    m_curlRequest = createCurlRequest(newRequest);
    m_curlRequest->start();
}

}

#endif
