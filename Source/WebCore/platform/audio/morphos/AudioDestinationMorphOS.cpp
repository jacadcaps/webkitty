/*
 * Copyright (C) 2020 Jacek Piszczek
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

#if ENABLE(WEB_AUDIO)
#include "AudioDestinationMorphOS.h"
#include "AudioChannel.h"
#include "AudioSourceProvider.h"
#include "NotImplemented.h"

namespace WebCore {

static const unsigned framesToPull = 128;

float AudioDestination::hardwareSampleRate()
{
	return 44100.f;
}

unsigned long AudioDestination::AudioDestination::maxChannelCount()
{
	return 0; // stereo
}

std::unique_ptr<AudioDestination> AudioDestination::create(AudioIOCallback& callback, const String&, unsigned numberOfInputChannels, unsigned numberOfOutputChannels, float sampleRate)
{
    return makeUnique<AudioDestinationMorphOS>(callback, sampleRate);
}

AudioDestinationMorphOS::AudioDestinationMorphOS(AudioIOCallback&callback, float sampleRate)
	: m_callback(callback)
	, m_renderBus(AudioBus::create(2, framesToPull, false))
	, m_sampleRate(sampleRate)
	, m_isPlaying(false)
{
	notImplemented();
}

AudioDestinationMorphOS::~AudioDestinationMorphOS()
{

}

void AudioDestinationMorphOS::start()
{
	notImplemented();
}

void AudioDestinationMorphOS::stop()
{

}

unsigned AudioDestinationMorphOS::framesPerBuffer() const
{
	return 1;
}

}

#endif
