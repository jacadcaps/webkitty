/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
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

#include "config.h"
#include "WebGPUPipelineDescriptorBase.h"

#if ENABLE(GPU_PROCESS)

#include "WebGPUConvertFromBackingContext.h"
#include "WebGPUConvertToBackingContext.h"
#include <pal/graphics/WebGPU/WebGPUPipelineDescriptorBase.h>

namespace WebKit::WebGPU {

std::optional<PipelineDescriptorBase> ConvertToBackingContext::convertToBacking(const PAL::WebGPU::PipelineDescriptorBase& pipelineDescriptorBase)
{
    auto base = convertToBacking(static_cast<const PAL::WebGPU::ObjectDescriptorBase&>(pipelineDescriptorBase));
    if (!base)
        return std::nullopt;

    WebGPUIdentifier layout;
    if (pipelineDescriptorBase.layout) {
        layout = convertToBacking(*pipelineDescriptorBase.layout);
        if (!layout)
            return std::nullopt;
    }

    return { { WTFMove(*base), layout } };
}

std::optional<PAL::WebGPU::PipelineDescriptorBase> ConvertFromBackingContext::convertFromBacking(const PipelineDescriptorBase& pipelineDescriptorBase)
{
    auto base = convertFromBacking(static_cast<const ObjectDescriptorBase&>(pipelineDescriptorBase));
    if (!base)
        return std::nullopt;

    PAL::WebGPU::PipelineLayout* layout = nullptr;
    if (pipelineDescriptorBase.layout) {
        layout = convertPipelineLayoutFromBacking(pipelineDescriptorBase.layout);
        if (!layout)
            return std::nullopt;
    }

    return { { WTFMove(*base), layout } };
}

} // namespace WebKit

#endif // ENABLE(GPU_PROCESS)
