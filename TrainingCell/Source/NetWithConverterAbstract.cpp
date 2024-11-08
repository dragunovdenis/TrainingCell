//Copyright (c) 2023 Denys Dragunov, dragunovdenis@gmail.com
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files(the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
//copies of the Software, and to permit persons to whom the Software is furnished
//to do so, subject to the following conditions :

//The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
//INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
//PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
//HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "../Headers/NetWithConverterAbstract.h"

namespace TrainingCell
{
	void NetWithConverterAbstract::calc_gradient_and_value(const DeepLearning::CpuDC::tensor_t& state,
		const DeepLearning::CpuDC::tensor_t& target_value, const DeepLearning::CostFunctionId& cost_func_id,
		std::vector<DeepLearning::LayerGradient<DeepLearning::CpuDC>>& out_gradient,
		DeepLearning::CpuDC::tensor_t& out_value, const double gradient_scale_factor,
		DeepLearning::Net<DeepLearning::CpuDC>::Context& context) const
	{
		net().calc_gradient_and_value(state,
			target_value, cost_func_id,
			out_gradient,
			out_value, static_cast<DeepLearning::Real>(gradient_scale_factor), context);
	}

	double NetWithConverterAbstract::evaluate(const std::vector<int>& state, DeepLearning::CpuDC::tensor_t& out_state_converted,
		DeepLearning::Net<DeepLearning::CpuDC>::Context& comp_context) const
	{
		converter().convert(state, out_state_converted);
		net().act(out_state_converted, comp_context);
		return comp_context.get_out()(0, 0, 0);
	}

	void NetWithConverterAbstract::update(const std::vector<DeepLearning::LayerGradient<DeepLearning::CpuDC>>& gradient,
		const double learning_rate, const double& lambda)
	{
		net().update(gradient, static_cast<DeepLearning::Real>(learning_rate), static_cast<DeepLearning::Real>(lambda));
	}

	bool NetWithConverterAbstract::validate_net_input_size(const std::size_t state_size) const
	{
		return static_cast<long long>(calc_input_net_size(state_size, converter())) == net().in_size().coord_prod();
	}

	std::size_t NetWithConverterAbstract::calc_input_net_size(const std::size_t state_size, const StateConverter& converter)
	{
		return state_size * converter.get_expansion_factor();
	}

	void NetWithConverterAbstract::allocate(std::vector<DeepLearning::LayerGradient<DeepLearning::CpuDC>>& gradient,
		const bool assign_zero) const
	{
		net().allocate(gradient, assign_zero);
	}
}
