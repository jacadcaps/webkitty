#include "MediaSampleMorphOS.h"
#include "acinerella.h"

#if ENABLE(VIDEO)

#define D(x) 

namespace WebCore {

MediaSampleMorphOS::MediaSampleMorphOS(RefPtr<Acinerella::AcinerellaPackage>& sample, const FloatSize& presentationSize, const String& trackId)
    : m_pts(MediaTime::zeroTime())
    , m_dts(MediaTime::zeroTime())
    , m_duration(MediaTime::zeroTime())
	, m_trackId(trackId)
    , m_sample(sample)
	, m_presentationSize(presentationSize)
{
	if (sample.get())
	{
		m_pts = MediaTime::createWithDouble(ac_get_package_pts(sample->acinerella()->instance(), sample->package()));
		m_dts = MediaTime::createWithDouble(ac_get_package_dts(sample->acinerella()->instance(), sample->package()));
		m_duration = MediaTime::createWithDouble(ac_get_package_duration(sample->acinerella()->instance(), sample->package()));
		
		if (ac_get_package_duration(sample->acinerella()->instance(), sample->package()) <= 0.0f)
			m_duration = MediaTime::createWithDouble(0.02);
		
		D(dprintf("MediaSample: pts %f dts %f duration %f\n", m_pts.toFloat(), m_dts.toFloat(), m_duration.toFloat()));
	}
}

Ref<MediaSample> MediaSampleMorphOS::createNonDisplayingCopy() const
{
	RefPtr<Acinerella::AcinerellaPackage> ptr(m_sample.get());
	return MediaSampleMorphOS::create(ptr, m_presentationSize, m_trackId);
}

void MediaSampleMorphOS::offsetTimestampsBy(const MediaTime& timestampOffset)
{
    if (!timestampOffset)
        return;
    m_pts += timestampOffset;
    m_dts += timestampOffset;
    
    // TODO: poke into acinerella buffers? gtk appears to be doing this
}

PlatformSample MediaSampleMorphOS::platformSample()
{
	PlatformSample sample = { PlatformSample::MorphOSSampleType, { .mosSample = m_sample.get() } };
	return sample;
}

}

#endif
