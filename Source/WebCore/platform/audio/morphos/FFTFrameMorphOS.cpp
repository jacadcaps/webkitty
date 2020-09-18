#include "config.h"

#include "FFTFrame.h"

#include "VectorMath.h"
#include <wtf/FastMalloc.h>
#include <wtf/StdLibExtras.h>

namespace {

size_t unpackedFFTDataSize(unsigned fftSize)
{
    return fftSize / 2 + 1;
}

} // anonymous namespace

namespace WebCore {

// Normal constructor: allocates for a given fftSize.
FFTFrame::FFTFrame(unsigned fftSize)
    : m_FFTSize(fftSize)
    , m_log2FFTSize(static_cast<unsigned>(log2(fftSize)))
    , m_realData(unpackedFFTDataSize(m_FFTSize))
    , m_imagData(unpackedFFTDataSize(m_FFTSize))
{
}

// Creates a blank/empty frame (interpolate() must later be called).
FFTFrame::FFTFrame()
    : m_FFTSize(0)
    , m_log2FFTSize(0)
{
}

// Copy constructor.
FFTFrame::FFTFrame(const FFTFrame& frame)
    : m_FFTSize(frame.m_FFTSize)
    , m_log2FFTSize(frame.m_log2FFTSize)
    , m_realData(unpackedFFTDataSize(frame.m_FFTSize))
    , m_imagData(unpackedFFTDataSize(frame.m_FFTSize))
{
}

void FFTFrame::initialize()
{
}

void FFTFrame::cleanup()
{
}

FFTFrame::~FFTFrame()
{
}

void FFTFrame::multiply(const FFTFrame& frame)
{
}

void FFTFrame::doFFT(const float* data)
{
}

void FFTFrame::doInverseFFT(float* data)
{
}

float* FFTFrame::realData() const
{
    return const_cast<float*>(m_realData.data());
}

float* FFTFrame::imagData() const
{
    return const_cast<float*>(m_imagData.data());
}

} // namespace WebCore
