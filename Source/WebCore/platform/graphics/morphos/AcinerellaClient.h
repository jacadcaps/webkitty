#pragma once
#include "MediaPlayerEnums.h"
#include "MediaPlayerMorphOS.h"

namespace WebCore {
namespace Acinerella {

#include <wtf/Seconds.h>

class AcinerellaClient
{
public:
	virtual void accInitialized(MediaPlayerMorphOSInfo info) = 0;
	virtual bool accEnableAudio() const = 0;
	virtual bool accEnableVideo() const = 0;
	virtual void accSetNetworkState(WebCore::MediaPlayerEnums::NetworkState state) = 0;
	virtual void accSetReadyState(WebCore::MediaPlayerEnums::ReadyState state) = 0;
	virtual void accSetBufferLength(float buffer) = 0;
	virtual void accSetPosition(float position) = 0;
	virtual void accSetDuration(float duration) = 0;
	virtual void accEnded() = 0;
};

}
}
