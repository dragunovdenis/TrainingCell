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
#include "../../DeepLearning/DeepLearning/Math/Tensor.h"
#include "IStateSeed.h"
#include "Move.h"

namespace TrainingCell
{
	/// <summary>
	/// A minimal interface providing an agent with all the needed access to an environment's state
	/// </summary>
	class IMinActionEvaluator
	{
	public:
		/// <summary>
		/// Virtual destructor
		/// </summary>
		virtual ~IMinActionEvaluator() = default;

		/// <summary>
		/// Returns number of available actions to take in the current state of environment.
		/// It is assumed that integer values from "0" to "actions count" - 1 represent ID's of the available actions.
		/// </summary>
		[[nodiscard]] virtual int get_actions_count() const = 0;

		/// <summary>
		/// Returns tensor representation of the current state after an action with the given ID was "taken"
		/// </summary>
		[[nodiscard]] virtual DeepLearning::Tensor evaluate(const int action_id) const = 0;

		/// <summary>
		/// Returns tensor representation of the current state (with no action "taken")
		/// </summary>
		[[nodiscard]] virtual DeepLearning::Tensor evaluate() const = 0;

		/// <summary>
		/// For the given pair of previous and next after-states represented with "raw" tensors,
		/// calculates reward "suggested" by the difference between the states
		///</summary>
		[[nodiscard]] virtual double calc_reward(const DeepLearning::Tensor& prev_after_state,
			const DeepLearning::Tensor& next_after_state) const = 0;
	};

	/// <summary>
	/// An interface providing an agent with all the needed access to an environment's state
	/// </summary>
	class IActionEvaluator : public IMinActionEvaluator
	{
	public:
		/// <summary>
		/// Read-only access to the current state's seed
		/// </summary>
		[[nodiscard]] virtual const IStateSeed& current_state_seed() const = 0;

		/// <summary>
		/// Read-only access to the collection of available actions
		/// </summary>
		[[nodiscard]] virtual const std::vector<Move>& actions() const = 0;

		/// <summary>
		/// Returns int vector representation of the current state
		/// </summary>
		[[nodiscard]] virtual std::vector<int> to_std_vector() const = 0;

		/// <summary>
		/// Returns an "inverted" representation of the current state, i.e. a state that it is seen by the opponent (an agent playing "anti" pieces)
		/// in the form of integer vector
		/// </summary>
		[[nodiscard]] virtual std::vector<int> get_inverted_std() const = 0;
	};
}

