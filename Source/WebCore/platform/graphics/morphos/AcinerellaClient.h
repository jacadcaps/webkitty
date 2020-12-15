#pragma once
#include "MediaPlayerEnums.h"

namespace WebCore {
namespace Acinerella {

class AcinerellaClient
{
public:
	virtual bool accEnableAudio() const = 0;
	virtual bool accEnableVideo() const = 0;
	virtual void accSetNetworkState(WebCore::MediaPlayerEnums::NetworkState state) = 0;
	virtual void accSetReadyState(WebCore::MediaPlayerEnums::ReadyState state) = 0;
};

}
}
