#pragma once

#if ENABLE(VIDEO)

#include "MediaSample.h"
#include "FloatSize.h"
#include "AcinerellaPointer.h"
#include "AcinerellaMuxer.h"
#include <wtf/text/AtomString.h>

namespace WebCore {

class MediaSampleMorphOS final : public MediaSample {
	MediaSampleMorphOS(AcinerellaPointer&& sample, const FloatSize& presentationSize, const AtomString& trackId);

public:
    static Ref<MediaSampleMorphOS> create(AcinerellaPointer&& sample, const FloatSize& presentationSize, const AtomString& trackId)
    {
        return adoptRef(*new MediaSampleMorphOS(WTFMove(sample), presentationSize, trackId));
    }

    MediaTime presentationTime() const override { return m_pts; }
    MediaTime decodeTime() const override { return m_dts; }
    MediaTime duration() const override { return m_duration; }
    AtomString trackID() const override { return m_trackId; }
    void setTrackID(const String& trackId) override { m_trackId = trackId; }
    size_t sizeInBytes() const override { return m_size; }
    FloatSize presentationSize() const override { return m_presentationSize; }
    void offsetTimestampsBy(const MediaTime&) override;
    void setTimestamps(const MediaTime&, const MediaTime&) { }
    bool isDivisable() const override { return false; }
    std::pair<RefPtr<MediaSample>, RefPtr<MediaSample>> divide(const MediaTime& presentationTime) override { return { nullptr, nullptr }; }
    Ref<MediaSample> createNonDisplayingCopy() const override;

    virtual SampleFlags flags() const { return m_flags; }
    virtual PlatformSample platformSample() { return &m_sample; }

protected:
    MediaTime m_pts;
    MediaTime m_dts;
    MediaTime m_duration;
    AtomString m_trackId;
    size_t m_size { 0 };
    AcinerellaPackage m_sample;
    FloatSize m_presentationSize;
    MediaSample::SampleFlags m_flags { MediaSample::IsSync };
};

}

#endif
