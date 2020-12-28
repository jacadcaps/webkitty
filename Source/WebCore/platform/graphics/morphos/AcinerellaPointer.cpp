#include "AcinerellaPointer.h"

#if ENABLE(VIDEO)

#include <wtf/RefPtr.h>
#include <proto/exec.h>

#define D(x) 

namespace WebCore {
namespace Acinerella {

RefPtr<AcinerellaPointer> AcinerellaPointer::create(ac_instance *instance)
{
	RefPtr<AcinerellaPointer> pointer = WTF::adoptRef(*new AcinerellaPointer(instance ? instance : ac_init()));
	if (pointer && !pointer->instance())
		return nullptr;
	return pointer;
}

AcinerellaPointer::AcinerellaPointer(ac_instance *instance)
	: m_instance(deleted_unique_ptr<ac_instance>(instance, [](ac_instance*instance){ ac_free(instance); }))
{
	D(dprintf("%s(%p): created %p\n", __PRETTY_FUNCTION__, this, m_instance.get()));
}

AcinerellaPointer::~AcinerellaPointer()
{
	D(dprintf("%s(%p): killing %p, audiodec %p\n", __PRETTY_FUNCTION__, this, m_instance.get(), m_audioDecoder.get()));
	m_audioDecoder.reset();
	m_videoDecoder.reset();
	m_instance.reset();
}

void AcinerellaPointer::setInstance(ac_instance *instance)
{
	m_audioDecoder.reset();
	m_videoDecoder.reset();
	m_instance.reset();
	
	m_instance = deleted_unique_ptr<ac_instance>(instance, [](ac_instance*instance){ ac_free(instance); });
	D(dprintf("%s(%p): set %p\n", __PRETTY_FUNCTION__, this, m_instance.get()));
}

void AcinerellaPointer::setAudioDecoder(ac_decoder *decoder)
{
	m_audioDecoder.reset();
	m_audioDecoder = deleted_unique_ptr<ac_decoder>(decoder, [](ac_decoder* decoder){ ac_free_decoder(decoder); });
}

void AcinerellaPointer::setVideoDecoder(ac_decoder *decoder)
{
	m_videoDecoder.reset();
	m_videoDecoder = deleted_unique_ptr<ac_decoder>(decoder, [](ac_decoder* decoder){ ac_free_decoder(decoder); });
}

}
}

#endif
