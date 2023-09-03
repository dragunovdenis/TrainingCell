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
#include "IActionEvaluator.h"
#include "IState.h"

namespace TrainingCell
{
	/// <summary>
	/// Implementation of action evaluator interface for the case of checkerboard games
	/// </summary>
	class ActionEvaluator : public IActionEvaluator
	{
		/// <summary>
		/// Pointer to a state
		/// </summary>
		const IState* const _state_ptr{};

		/// <summary>
		/// Pointer to a collection of actions
		/// </summary>
		const std::vector<Move>* const _actions_ptr{};

	public:
		/// <summary>
		/// Constructor
		/// </summary>
		ActionEvaluator(const IState* const state_ptr, const std::vector<Move>* const actions_ptr);

		/// <summary>
		/// See documentation os the base class
		/// </summary>
		[[nodiscard]] int get_actions_count() const override;

		/// <summary>
		/// See documentation os the base class
		/// </summary>
		[[nodiscard]] DeepLearning::Tensor evaluate(const int action_id) const override;

		/// <summary>
		/// See documentation os the base class
		/// </summary>
		[[nodiscard]] DeepLearning::Tensor evaluate() const override;

		/// <summary>
		/// See documentation os the base class
		/// </summary>
		[[nodiscard]] double calc_reward(const DeepLearning::Tensor& prev_after_state,
			const DeepLearning::Tensor& next_after_state) const override;

		/// <summary>
		/// See documentation os the base class
		/// </summary>
		[[nodiscard]] const IStateSeed& current_state_seed() const override;

		/// <summary>
		/// See documentation os the base class
		/// </summary>
		[[nodiscard]] const std::vector<Move>& actions() const override;

		/// <summary>
		/// See documentation os the base class
		/// </summary>
		[[nodiscard]] std::vector<int> to_std_vector() const override;

		/// <summary>
		/// See documentation os the base class
		/// </summary>
		[[nodiscard]] std::vector<int> get_inverted_std() const override;

	};
}
