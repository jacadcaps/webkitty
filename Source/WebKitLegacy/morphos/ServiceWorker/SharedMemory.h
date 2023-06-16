/*
 * Copyright (C) 2010-2019 Apple Inc. All rights reserved.
 * Copyright (C) 2017 Sony Interactive Entertainment Inc.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "WebKit.h"
#include <wtf/Forward.h>
#include <wtf/Noncopyable.h>
#include <wtf/ThreadSafeRefCounted.h>

namespace WebCore {
class SharedBuffer;
class FragmentedSharedBuffer;
}

namespace WebKit {

class SharedMemory : public ThreadSafeRefCounted<SharedMemory> {
    SharedMemory() = default;
public:
    enum class Protection {
        ReadOnly,
        ReadWrite
    };

    // NOTE: WebKit uses the handle in order to pass the shared memory between processes via IPC
    // We don't have to do this in WebKitLegacy!
    struct Handle {
        RefPtr<SharedMemory> _ptr;
    };

    static RefPtr<SharedMemory> allocate(size_t);
    static RefPtr<SharedMemory> copyBuffer(const WebCore::FragmentedSharedBuffer&);
    static RefPtr<SharedMemory> map(const Handle& handle, Protection) { return const_cast<SharedMemory*>(handle._ptr.get()); }

    ~SharedMemory();

    size_t size() const { return m_size; }
    void* data() const
    {
        ASSERT(m_data);
        return m_data;
    }

    static unsigned systemPageSize() { return 512; }
    bool createHandle(Handle& handle, Protection) { handle._ptr = this; return true; }

    Ref<WebCore::SharedBuffer> createSharedBuffer(size_t) const;

private:
    size_t m_size;
    void* m_data = nullptr;
};

};
