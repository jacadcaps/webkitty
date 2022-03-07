#pragma once
#include <cstddef>
#include <sys/types.h>

namespace WebCore {

class Altivec
{
public:
	static void muxFloatAudioChannelsToInterleavedInt16(int16_t *out, const float *channelA, const float *channelB, size_t samples);

	// in 8-floats
	static void clamp(float* outputVector, float minimum, float maximum, const float* inputVector, size_t numberOfElementsToProcess);
	static void multiplyByScalarThenAddToOutput(const float* inputVector, float scalar, float* outputVector, size_t numberOfElementsToProcess);

	// in 4-floats
	static void multiplyByScalar(const float* inputVector, float scalar, float* outputVector, size_t numberOfElementsToProcess);
	static void addScalar(const float* inputVector, float scalar, float* outputVector, size_t numberOfElementsToProcess);
	static void add(const float* inputVector1, const float* inputVector2, float* outputVector, size_t numberOfElementsToProcess);
	static void multiply(const float* inputVector1, const float* inputVector2, float* outputVector, size_t numberOfElementsToProcess);
};

}
