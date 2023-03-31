#include "nbl/asset/filters/kernels/CConvolutionWeightFunction.h"

namespace nbl::asset::impl
{

template <int32_t derivative>
float convolution_weight_function_helper<CWeightFunction1D<SBoxFunction>, CWeightFunction1D<SBoxFunction>>::operator_impl(const CConvolutionWeightFunction1D<CWeightFunction1D<SBoxFunction>, CWeightFunction1D<SBoxFunction>>& _this, const float x, const uint32_t channel, const uint32_t sampleCount)
{
	assert(false);

	const auto [minIntegrationLimit, maxIntegrationLimit] = _this.getIntegrationDomain(x);

	// const float kernelAWidth = getKernelWidth(m_kernelA);
	// const float kernelBWidth = getKernelWidth(m_kernelB);

	// const auto& kernelNarrow = kernelAWidth < kernelBWidth ? m_kernelA : m_kernelB;
	// const auto& kernelWide = kernelAWidth > kernelBWidth ? m_kernelA : m_kernelB;

	// We assume that the wider kernel is stationary (not shifting as `x` changes) while the narrower kernel is the one which shifts, such that it is always centered at x.
	// return (maxIntegrationLimit - minIntegrationLimit) * kernelWide.weight(x, channel) * kernelNarrow.weight(0.f, channel);
	return 0.f;
}

template <int32_t derivative>
float convolution_weight_function_helper<CWeightFunction1D<SGaussianFunction>, CWeightFunction1D<SGaussianFunction>>::operator_impl(const CConvolutionWeightFunction1D<CWeightFunction1D<SGaussianFunction>, CWeightFunction1D<SGaussianFunction>>& _this, const float x, const uint32_t channel, const uint32_t sampleCount)
{
	assert(false);

#if 0
	const float kernelA_stddev = m_kernelA.m_multipliedScale[channel];
	const float kernelB_stddev = m_kernelB.m_multipliedScale[channel];
	const float convolution_stddev = core::sqrt(kernelA_stddev * kernelA_stddev + kernelB_stddev * kernelB_stddev);

	const auto stretchFactor = core::vectorSIMDf(convolution_stddev, 1.f, 1.f, 1.f);
	auto convolutionKernel = asset::CGaussianImageFilterKernel();
	convolutionKernel.stretchAndScale(stretchFactor);

	return convolutionKernel(x, channel);
#endif
	return 0.f;
}

template <int32_t derivative>
float convolution_weight_function_helper<CWeightFunction1D<SKaiserFunction>, CWeightFunction1D<SKaiserFunction>>::operator_impl(const CConvolutionWeightFunction1D<CWeightFunction1D<SKaiserFunction>, CWeightFunction1D<SKaiserFunction>>& _this, const float x, const uint32_t channel, const uint32_t sampleCount)
{
	// return getKernelWidth(m_kernelA) > getKernelWidth(m_kernelB) ? m_kernelA.weight(x, channel) : m_kernelB.weight(x, channel);
	return 0.f;
}

} // end namespace nbl::asset