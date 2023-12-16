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

#pragma once
#include "INet.h"
#include "StateConverter.h"

namespace TrainingCell
{
	/// <summary>
	/// Abstract implementation of a neural net with state conversion mechanism.
	/// </summary>
	class NetWithConverterAbstract : public INet
	{
	protected:
		/// <summary>
		/// Access to the net.
		/// </summary>
		virtual DeepLearning::Net<DeepLearning::CpuDC>& net() = 0;

		/// <summary>
		/// Access to the net.
		/// </summary>
		virtual const DeepLearning::Net<DeepLearning::CpuDC>& net() const = 0;

		/// <summary>
		/// Access to the converter.
		/// </summary>
		virtual const StateConverter& converter() const = 0;

	public:
		/// <summary>
		/// See summary of the base class.
		/// </summary>
		void calc_gradient_and_value(const DeepLearning::CpuDC::tensor_t& state, const DeepLearning::CpuDC::tensor_t& target_value,
			const DeepLearning::CostFunctionId& cost_func_id, std::vector<DeepLearning::LayerGradient<DeepLearning::CpuDC>>& out_gradient,
			DeepLearning::CpuDC::tensor_t& out_value, DeepLearning::Net<DeepLearning::CpuDC>::Context& context) const override;

		/// <summary>
		/// See summary of the base class.
		/// </summary>
		double evaluate(const std::vector<int>& state, DeepLearning::CpuDC::tensor_t& out_state_converted,
			DeepLearning::Net<DeepLearning::CpuDC>::Context& comp_context) const override;

		/// <summary>
		/// See summary of the base class.
		/// </summary>
		void update(const std::vector<DeepLearning::LayerGradient<DeepLearning::CpuDC>>& gradient,
			const double learning_rate, const double& lambda) override;

		/// <summary>
		/// See summary of the base class.
		/// </summary>
		bool validate_net_input_size(const std::size_t state_size) const override;

		/// <summary>
		/// Returns size of the input neural net layer deduced from the given size of state and the given converter.
		/// </summary>
		static std::size_t calc_input_net_size(const std::size_t state_size, const StateConverter& converter);
	};

}
