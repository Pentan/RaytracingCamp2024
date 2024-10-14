#ifndef PETALS_SPECTRUM_H
#define PETALS_SPECTRUM_H

#include <vector>
#include "types.h"

namespace Petals {
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