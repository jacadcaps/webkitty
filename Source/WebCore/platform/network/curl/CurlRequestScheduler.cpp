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
#include "CurlRequestScheduler.h"

#if OS(MORPHOS)
extern "C" {
LONG WaitSelect(LONG nfds, fd_set *readfds, fd_set *writefds, fd_set *exeptfds,
                struct timeval *timeout, ULONG *maskp);
}
#include <unistd.h> /* for usleep */
#endif

#if USE(CURL)

#include "CurlRequestSchedulerClient.h"

namespace WebCore {

CurlRequestScheduler::CurlRequestScheduler(long maxConnects, long maxTotalConnections, long maxHostConnections)
    : m_maxConnects(maxConnects)
    , m_maxTotalConnections(maxTotalConnections)
    , m_maxHostConnections(maxHostConnections)
{
}

bool CurlRequestScheduler::add(CurlRequestSchedulerClient* client)
{
    ASSERT(isMainThread());

    if (!client)
        return false;

    startTransfer(client);
    startThreadIfNeeded();

    return true;
}

void CurlRequestScheduler::cancel(CurlRequestSchedulerClient* client)
{
    ASSERT(isMainThread());

    if (!client)
        return;

    cancelTransfer(client);
}

void CurlRequestScheduler::callOnWorkerThread(WTF::Function<void()>&& task)
{
    {
        auto locker = holdLock(m_mutex);
        m_taskQueue.append(WTFMove(task));
    }

    startThreadIfNeeded();
}

void CurlRequestScheduler::startThreadIfNeeded()
{
    ASSERT(isMainThread());

    {
        auto locker = holdLock(m_mutex);
        if (m_runThread)
            return;
    }

    if (m_thread)
        m_thread->waitForCompletion();

    {
        auto locker = holdLock(m_mutex);
        m_runThread = true;
    }

    m_thread = Thread::create("curlThread", [this] {
        workerThread();

        auto locker = holdLock(m_mutex);
        m_runThread = false;
    });
}

void CurlRequestScheduler::stopThreadIfNoMoreJobRunning()
{
    ASSERT(!isMainThread());

    auto locker = holdLock(m_mutex);
    if (m_activeJobs.size() || m_taskQueue.size())
        return;

    m_runThread = false;
}

void CurlRequestScheduler::stopThread()
{
    {
        auto locker = holdLock(m_mutex);
        m_runThread = false;
    }

    if (m_thread) {
        m_thread->waitForCompletion();
        m_thread = nullptr;
    }
}

void CurlRequestScheduler::executeTasks()
{
    ASSERT(!isMainThread());

    Vector<WTF::Function<void()>> taskQueue;

    {
        auto locker = holdLock(m_mutex);
        taskQueue = WTFMove(m_taskQueue);
    }

    for (auto& task : taskQueue)
        task();
}

void CurlRequestScheduler::workerThread()
{
    ASSERT(!isMainThread());

    m_curlMultiHandle = makeUnique<CurlMultiHandle>();
    m_curlMultiHandle->setMaxConnects(m_maxConnects);
    m_curlMultiHandle->setMaxTotalConnections(m_maxTotalConnections);
    m_curlMultiHandle->setMaxHostConnections(m_maxHostConnections);

    while (true) {
        {
            auto locker = holdLock(m_mutex);
            if (!m_runThread)
                break;
        }

#if OS(MORPHOS)
        struct timeval starttime;
        gettimeofday(&starttime, NULL);
#endif

        executeTasks();

        // Retry 'select' if it was interrupted by a process signal.
        int rc = 0;
        do {
            fd_set fdread;
            fd_set fdwrite;
            fd_set fdexcep;
            int maxfd = 0;

#if OS(MORPHOS)
            const int selectTimeoutMS = 100;
#else
            const int selectTimeoutMS = 5;
#endif

            struct timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = selectTimeoutMS * 1000; // select waits microseconds

            m_curlMultiHandle->getFdSet(fdread, fdwrite, fdexcep, maxfd);

            // When the 3 file descriptors are empty, winsock will return -1
            // and bail out, stopping the file download. So make sure we
            // have valid file descriptors before calling select.
            if (maxfd >= 0)
            {
#if OS(MORPHOS)
				rc = WaitSelect(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout, nullptr);
#else
                rc = ::select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
#endif
			}
#if OS(MORPHOS)
			else
			{
				rc = WaitSelect(0, nullptr, nullptr, nullptr, &timeout, nullptr);
			}
		} while (0);
#else
        } while (rc == -1 && errno == EINTR);
#endif

#if OS(MORPHOS)
        int64_t previoustotaltransfers;
        {
            auto locker = holdLock(m_mutex);
            previoustotaltransfers = m_totaltransfers;
            m_totaltransfers = 0; // reset transfers counter
        }
#endif

        int activeCount = 0;
        while (m_curlMultiHandle->perform(activeCount) == CURLM_CALL_MULTI_PERFORM) { }

        // check the curl messages indicating completed transfers
        // and free their resources
        while (true) {
            int messagesInQueue = 0;
            CURLMsg* msg = m_curlMultiHandle->readInfo(messagesInQueue);
            if (!msg)
                break;

            ASSERT(msg->msg == CURLMSG_DONE);
            if (auto client = m_clientMaps.inlineGet(msg->easy_handle))
                completeTransfer(client, msg->data.result);
        }

#if OS(MORPHOS)
        int64_t transfersdelta;
        {
            auto locker = holdLock(m_mutex);
            transfersdelta = m_totaltransfers - previoustotaltransfers;
        }
        // transfers counter unchanged from the last perform call?
        if (transfersdelta == 0)
        {
            // sleep for maximum 5ms. if we already spent that much time
            // doing something else, don't sleep more
            const long sleeptime = 5 * 1000;
            struct timeval stoptime;
            gettimeofday(&stoptime, NULL);
            timersub(&stoptime, &starttime, &stoptime);
            if (stoptime.tv_sec == 0 && stoptime.tv_usec < sleeptime)
                usleep(sleeptime - stoptime.tv_usec);
        }
#endif

        stopThreadIfNoMoreJobRunning();
    }

    m_curlMultiHandle = nullptr;
}

#if OS(MORPHOS)
int CurlRequestScheduler::progressCallback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    CurlRequestScheduler *scheduler = (CurlRequestScheduler *) clientp;
    auto locker = holdLock(scheduler->m_mutex);
    // account any kind of download or upload to total counter
    scheduler->m_totaltransfers += dlnow + ulnow;
    return 0;
}
#endif

void CurlRequestScheduler::startTransfer(CurlRequestSchedulerClient* client)
{
    client->retain();

    auto task = [this, client]() {
        CURL* handle = client->setupTransfer();
        if (!handle) {
            completeTransfer(client, CURLE_FAILED_INIT);
            return;
        }

        m_curlMultiHandle->addHandle(handle);

#if OS(MORPHOS)
        curl_easy_setopt(handle, CURLOPT_XFERINFOFUNCTION, progressCallback);
        curl_easy_setopt(handle, CURLOPT_XFERINFODATA, this);
        curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0L);
#endif
        ASSERT(!m_clientMaps.contains(handle));
        m_clientMaps.set(handle, client);
    };

    auto locker = holdLock(m_mutex);
    m_activeJobs.add(client);
    m_taskQueue.append(WTFMove(task));
}

void CurlRequestScheduler::completeTransfer(CurlRequestSchedulerClient* client, CURLcode result)
{
    finalizeTransfer(client, [client, result]() {
        client->didCompleteTransfer(result);
    });
}

void CurlRequestScheduler::cancelTransfer(CurlRequestSchedulerClient* client)
{
    finalizeTransfer(client, [client]() {
        client->didCancelTransfer();
    });
}

void CurlRequestScheduler::finalizeTransfer(CurlRequestSchedulerClient* client, Function<void()> completionHandler)
{
    auto locker = holdLock(m_mutex);

    if (!m_activeJobs.contains(client))
        return;

    m_activeJobs.remove(client);

    auto task = [this, client, completionHandler = WTFMove(completionHandler)]() {
        if (client->handle()) {
            ASSERT(m_clientMaps.contains(client->handle()));
            m_clientMaps.remove(client->handle());
            m_curlMultiHandle->removeHandle(client->handle());
        }

        completionHandler();

        callOnMainThread([client]() {
            client->release();
        });
    };

    m_taskQueue.append(WTFMove(task));
}

}

#endif
