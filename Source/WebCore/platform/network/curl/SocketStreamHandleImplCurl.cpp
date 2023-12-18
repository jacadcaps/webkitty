/*
 * Copyright (C) 2009 Brent Fulgham.  All rights reserved.
 * Copyright (C) 2009 Google Inc.  All rights reserved.
 * Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "SocketStreamHandleImpl.h"

#if USE(CURL)

#include "CurlStreamScheduler.h"
#include "DeprecatedGlobalSettings.h"
#include "CookieRequestHeaderFieldProxy.h"
#include "SharedBuffer.h"
#include "SocketStreamError.h"
#include "NetworkStorageSession.h"
#include "../../../../WebKitLegacy/WebCoreSupport/SocketStreamHandleClient.h"
#include "StorageSessionProvider.h"

namespace WebCore {

SocketStreamHandleImpl::SocketStreamHandleImpl(const URL& url, SocketStreamHandleClient& client, const StorageSessionProvider* provider)
    : SocketStreamHandle(url, client)
    , m_storageSessionProvider(provider)
    , m_scheduler(CurlContext::singleton().streamScheduler())
{
    // FIXME: Using DeprecatedGlobalSettings from here is a layering violation.
    if (m_url.protocolIs("wss"_s) && DeprecatedGlobalSettings::allowsAnySSLCertificate())
        CurlContext::singleton().sslHandle().setIgnoreSSLErrors(true);

    m_streamID = m_scheduler.createStream(m_url, *this);
}

SocketStreamHandleImpl::~SocketStreamHandleImpl()
{
    destructStream();
}

std::optional<size_t> SocketStreamHandleImpl::platformSendInternal(const uint8_t* data, size_t length)
{
    if (isStreamInvalidated())
        return std::nullopt;

    if (m_totalSendDataSize + length > maxBufferSize)
        return 0;
    m_totalSendDataSize += length;

    auto buffer = makeUniqueArray<uint8_t>(length);
    memcpy(buffer.get(), data, length);

    m_scheduler.send(m_streamID, WTFMove(buffer), length);
    return length;
}

void SocketStreamHandleImpl::platformClose()
{
    destructStream();

    if (m_state == Closed)
        return;
    m_state = Closed;

    m_client.didCloseSocketStream(*this);
}

void SocketStreamHandleImpl::didOpen(CurlStreamID)
{
    if (m_state != Connecting)
        return;
    m_state = Open;

    m_client.didOpenSocketStream(*this);
}

void SocketStreamHandleImpl::didSendData(CurlStreamID, size_t length)
{
    ASSERT(m_totalSendDataSize - length >= 0);

    m_totalSendDataSize -= length;
    sendPendingData();
}

void SocketStreamHandleImpl::didReceiveData(CurlStreamID, const SharedBuffer& data)
{
    if (m_state != Open)
        return;

    m_client.didReceiveSocketStreamData(*this, data.data(), data.size());
}

void SocketStreamHandleImpl::didFail(CurlStreamID, CURLcode errorCode)
{
    destructStream();

    if (m_state == Closed)
        return;

    if (errorCode == CURLE_RECV_ERROR)
        m_client.didFailToReceiveSocketStreamData(*this);
    else
        m_client.didFailSocketStream(*this, SocketStreamError(errorCode, m_url.string(), CurlHandle::errorDescription(errorCode)));
}

void SocketStreamHandleImpl::destructStream()
{
    if (isStreamInvalidated())
        return;

    m_scheduler.destroyStream(m_streamID);
    m_streamID = invalidCurlStreamID;
}

static size_t removeTerminationCharacters(const uint8_t* data, size_t dataLength)
{
#ifndef NDEBUG
    ASSERT(dataLength > 2);
    ASSERT(data[dataLength - 2] == '\r');
    ASSERT(data[dataLength - 1] == '\n');
#else
    UNUSED_PARAM(data);
#endif

    // Remove the terminating '\r\n'
    return dataLength - 2;
}

static std::optional<std::pair<Vector<uint8_t>, bool>> cookieDataForHandshake(const NetworkStorageSession* networkStorageSession, const CookieRequestHeaderFieldProxy& headerFieldProxy)
{
    if (!networkStorageSession)
        return std::nullopt;
    
    auto [cookieDataString, secureCookiesAccessed] = networkStorageSession->cookieRequestHeaderFieldValue(headerFieldProxy);
    if (cookieDataString.isEmpty())
        return std::pair<Vector<uint8_t>, bool> { { }, secureCookiesAccessed };

    CString cookieData = cookieDataString.utf8();

    Vector<uint8_t> data = { 'C', 'o', 'o', 'k', 'i', 'e', ':', ' ' };
    data.append(cookieData.data(), cookieData.length());
    data.appendVector(Vector<uint8_t>({ '\r', '\n', '\r', '\n' }));

    return std::pair<Vector<uint8_t>, bool> { data, secureCookiesAccessed };
}

void SocketStreamHandleImpl::platformSendHandshake(const uint8_t* data, size_t length, const std::optional<CookieRequestHeaderFieldProxy>& headerFieldProxy, Function<void(bool, bool)>&& completionHandler)
{
    Vector<uint8_t> cookieData;
    bool secureCookiesAccessed = false;

    if (headerFieldProxy) {
        auto cookieDataFromNetworkSession = cookieDataForHandshake(m_storageSessionProvider ? m_storageSessionProvider->storageSession() : nullptr, *headerFieldProxy);
        if (!cookieDataFromNetworkSession) {
            completionHandler(false, false);
            return;
        }

        std::tie(cookieData, secureCookiesAccessed) = *cookieDataFromNetworkSession;
        if (cookieData.size())
            length = removeTerminationCharacters(data, length);
    }

    if (!m_buffer.isEmpty()) {
        if (m_buffer.size() + length + cookieData.size() > maxBufferSize) {
            // FIXME: report error to indicate that buffer has no more space.
            return completionHandler(false, secureCookiesAccessed);
        }
        m_buffer.append(data, length);
        m_buffer.append(cookieData.data(), cookieData.size());
        m_client.didUpdateBufferedAmount(*this, bufferedAmount());
        return completionHandler(true, secureCookiesAccessed);
    }
    size_t bytesWritten = 0;
    if (m_state == Open) {
        // Unfortunately, we need to send the data in one buffer or else the handshake fails.
        Vector<uint8_t> sendData;
        sendData.reserveInitialCapacity(length + cookieData.size());
        sendData.append(data, length);
        sendData.append(cookieData.data(), cookieData.size());

        if (auto result = platformSendInternal(sendData.data(), sendData.size()))
            bytesWritten = result.value();
        else
            return completionHandler(false, secureCookiesAccessed);
    }
    if (m_buffer.size() + length + cookieData.size() - bytesWritten > maxBufferSize) {
        // FIXME: report error to indicate that buffer has no more space.
        return completionHandler(false, secureCookiesAccessed);
    }
    if (bytesWritten < length + cookieData.size()) {
        size_t cookieBytesWritten = 0;
        if (bytesWritten < length)
            m_buffer.append(data + bytesWritten, length - bytesWritten);
        else
            cookieBytesWritten = bytesWritten - length;
        m_buffer.append(cookieData.data() + cookieBytesWritten, cookieData.size() - cookieBytesWritten);
        m_client.didUpdateBufferedAmount(static_cast<SocketStreamHandle&>(*this), bufferedAmount());
    }
    return completionHandler(true, secureCookiesAccessed);
}

void SocketStreamHandleImpl::platformSend(const uint8_t* data, size_t length, Function<void(bool)>&& completionHandler)
{
    if (!m_buffer.isEmpty()) {
        if (m_buffer.size() + length > maxBufferSize) {
            // FIXME: report error to indicate that buffer has no more space.
            return completionHandler(false);
        }
        m_buffer.append(data, length);
        m_client.didUpdateBufferedAmount(*this, bufferedAmount());
        return completionHandler(true);
    }
    size_t bytesWritten = 0;
    if (m_state == Open) {
        if (auto result = platformSendInternal(data, length))
            bytesWritten = result.value();
        else
            return completionHandler(false);
    }
    if (m_buffer.size() + length - bytesWritten > maxBufferSize) {
        // FIXME: report error to indicate that buffer has no more space.
        return completionHandler(false);
    }
    if (bytesWritten < length) {
        m_buffer.append(data + bytesWritten, length - bytesWritten);
        m_client.didUpdateBufferedAmount(static_cast<SocketStreamHandle&>(*this), bufferedAmount());
    }
    return completionHandler(true);
}

bool SocketStreamHandleImpl::sendPendingData()
{
    if (m_state != Open && m_state != Closing)
        return false;
    if (m_buffer.isEmpty()) {
        if (m_state == Open)
            return false;
        if (m_state == Closing) {
            disconnect();
            return false;
        }
    }
    bool pending;
    do {
        auto result = platformSendInternal(m_buffer.firstBlockData(), m_buffer.firstBlockSize());
        if (!result)
            return false;
        size_t bytesWritten = result.value();
        if (!bytesWritten)
            return false;
        pending = bytesWritten != m_buffer.firstBlockSize();
        ASSERT(m_buffer.size() - bytesWritten <= maxBufferSize);
        m_buffer.consume(bytesWritten);
    } while (!pending && !m_buffer.isEmpty());
    m_client.didUpdateBufferedAmount(static_cast<SocketStreamHandle&>(*this), bufferedAmount());
    return true;
}

size_t SocketStreamHandleImpl::bufferedAmount()
{
    return m_buffer.size();
}

} // namespace WebCore

#endif
