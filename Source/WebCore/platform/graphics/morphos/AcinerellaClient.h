#pragma once
#include "MediaPlayerEnums.h"

namespace WebCore {
namespace Acinerella {

#include <wtf/Seconds.h>

class AcinerellaClient
{
public:
	virtual bool accEnableAudio() const = 0;
	virtual bool accEnableVideo() const = 0;
	virtual void accSetNetworkState(WebCore::MediaPlayerEnums::NetworkState state) = 0;
	virtual void accSetReadyState(WebCore::MediaPlayerEnums::ReadyState state) = 0;
	virtual void accSetBufferLength(float buffer) = 0;
	virtual void accSetPosition(float buffer) = 0;
	virtual void accSetDuration(float buffer) = 0;
};

}
}
