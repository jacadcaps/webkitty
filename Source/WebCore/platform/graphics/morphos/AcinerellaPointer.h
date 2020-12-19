#pragma once

#include "config.h"

#if ENABLE(VIDEO)

#include <wtf/ThreadSafeRefCounted.h>
#include "acinerella.h"

namespace WebCore {
namespace Acinerella {

class AcinerellaPointer : public ThreadSafeRefCounted<AcinerellaPointer>
{
template<typename T>
using deleted_unique_ptr = std::unique_ptr<T,std::function<void(T*)>>;
	AcinerellaPointer(ac_instance *instance = nullptr);
public:
	~AcinerellaPointer();
	
	static RefPtr<AcinerellaPointer> create(ac_instance *instance = nullptr);

	void setInstance(ac_instance *instance);
	ac_instance *instance() { return m_instance.get(); }

	void setAudioDecoder(ac_decoder *);
	ac_decoder *audioDecoder() { return m_audioDecoder.get(); };

	void setVideoDecoder(ac_decoder *);
	ac_decoder *videoDecoder() { return m_videoDecoder.get(); };

protected:
	deleted_unique_ptr<ac_instance> m_instance;
	deleted_unique_ptr<ac_decoder>  m_audioDecoder;
	deleted_unique_ptr<ac_decoder>  m_videoDecoder;
};

}
}

#endif
