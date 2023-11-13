/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
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

#if PLATFORM(COCOA)

#include <pal/system/SleepDisabler.h>
#include <wtf/RefCounter.h>

namespace PAL {

#if PLATFORM(IOS_FAMILY)
enum ScreenSleepDisablerCounterType { };
typedef RefCounter<ScreenSleepDisablerCounterType> ScreenSleepDisablerCounter;
typedef ScreenSleepDisablerCounter::Token ScreenSleepDisablerCounterToken;
#endif

class SleepDisablerCocoa : public SleepDisabler {
public:
    explicit SleepDisablerCocoa(const String&, Type);
    virtual ~SleepDisablerCocoa();

#if PLATFORM(IOS_FAMILY)
    PAL_EXPORT static void setScreenWakeLockHandler(Function<bool(bool shouldKeepScreenAwake)>&&);
#endif

private:
    void takeScreenSleepDisablingAssertion(const String& reason);
    void takeSystemSleepDisablingAssertion(const String& reason);

    uint32_t m_sleepAssertion { 0 };
#if PLATFORM(IOS_FAMILY)
    ScreenSleepDisablerCounterToken m_screenSleepDisablerToken;
#endif
};

} // namespace PAL

#endif // PLATFORM(COCOA)
