#include "config.h"

#if ENABLE(WEB_AUDIO)

#include "AudioFileReader.h"
#include "AudioBus.h"
#include "NotImplemented.h"

extern "C" {void dprintf(const char *,...);}

namespace WebCore {

RefPtr<AudioBus> createBusFromInMemoryAudioFile(const void* data, size_t dataSize, bool mixToMono, float sampleRate)
{
	notImplemented();
	return nullptr;
}

RefPtr<AudioBus> createBusFromAudioFile(const char* filePath, bool mixToMono, float sampleRate)
{
	notImplemented();
	return nullptr;
}

}

#endif

