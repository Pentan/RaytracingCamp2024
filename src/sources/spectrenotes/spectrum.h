#ifndef SPECTRENOTES_SPECTRUM_H
#define SPECTRENOTES_SPECTRUM_H

#include <vector>
#include "types.h"

namespace Spectrenotes {
	class Spectrum {
	public:
		static RTFloat validMinLambda();
		static RTFloat validMaxLambda();
	};

	class SpectrumSample {
	public:
		SpectrumSample(int binCount);
		~SpectrumSample();

		void addSample(RTFloat wavelength, RTFloat sample);
		RTFloat X();
		RTFloat Y();
		RTFloat Z();

		XYZColor toXYZ() const;

	private:
		std::vector<RTSpectrumType> sampleBin;
		std::vector<int> sampleCounts;
	};
}

#endif