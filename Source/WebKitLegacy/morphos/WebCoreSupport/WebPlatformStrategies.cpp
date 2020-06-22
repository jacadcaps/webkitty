/*
 * Copyright (C) 2010-2017 Apple Inc. All rights reserved.
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

#include "WebPlatformStrategies.h"

#include "WebFrameNetworkingContext.h"
#include "WebResourceLoadScheduler.h"
#include <WebCore/BlobRegistryImpl.h>
#include <WebCore/FrameLoader.h>
#include <WebCore/NetworkStorageSession.h>
#include <WebCore/Page.h>
#include <WebCore/PageGroup.h>

using namespace WebCore;

void WebPlatformStrategies::initialize()
{
    static NeverDestroyed<WebPlatformStrategies> platformStrategies;
}

WebPlatformStrategies::WebPlatformStrategies()
{
    setPlatformStrategies(this);
}

LoaderStrategy* WebPlatformStrategies::createLoaderStrategy()
{
    return new WebResourceLoadScheduler;
}

PasteboardStrategy* WebPlatformStrategies::createPasteboardStrategy()
{
    return nullptr;
}

BlobRegistry* WebPlatformStrategies::createBlobRegistry()
{
    using namespace WebCore;
    class EmptyBlobRegistry : public WebCore::BlobRegistry {
        void registerFileBlobURL(const URL&, Ref<BlobDataFileReference>&&, const String& contentType) final { ASSERT_NOT_REACHED(); }
        void registerBlobURL(const URL&, Vector<BlobPart>&&, const String& contentType) final { ASSERT_NOT_REACHED(); }
        void registerBlobURL(const URL&, const URL& srcURL) final { ASSERT_NOT_REACHED(); }
        void registerBlobURLOptionallyFileBacked(const URL&, const URL& srcURL, RefPtr<BlobDataFileReference>&&, const String& contentType) final { ASSERT_NOT_REACHED(); }
        void registerBlobURLForSlice(const URL&, const URL& srcURL, long long start, long long end) final { ASSERT_NOT_REACHED(); }
        void unregisterBlobURL(const URL&) final { ASSERT_NOT_REACHED(); }
        unsigned long long blobSize(const URL&) final { ASSERT_NOT_REACHED(); return 0; }
        void writeBlobsToTemporaryFiles(const Vector<String>& blobURLs, CompletionHandler<void(Vector<String>&& filePaths)>&&) final { ASSERT_NOT_REACHED(); }
    };
    static NeverDestroyed<EmptyBlobRegistry> blobRegistry;
    return &blobRegistry.get();
}
