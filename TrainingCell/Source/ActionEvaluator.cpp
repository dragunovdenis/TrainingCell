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

#include "../Headers/ActionEvaluator.h"

namespace TrainingCell
{
	ActionEvaluator::ActionEvaluator(const IState* const state_ptr, const std::vector<Move>* const actions_ptr)
		: _state_ptr(state_ptr), _actions_ptr(actions_ptr)
	{
		if (!_state_ptr || !_actions_ptr)
			throw std::exception("Invalid input");
	}

	int ActionEvaluator::get_actions_count() const
	{
		return static_cast<int>(_actions_ptr->size());
	}

	DeepLearning::Tensor ActionEvaluator::evaluate(const int action_id) const
	{
		return _state_ptr->get_state((*_actions_ptr)[action_id]);
	}

	DeepLearning::Tensor ActionEvaluator::evaluate() const
	{
		return _state_ptr->to_tensor();
	}

	double ActionEvaluator::calc_reward(const DeepLearning::Tensor& prev_after_state,
		const DeepLearning::Tensor& next_after_state) const
	{
		return _state_ptr->calc_reward(prev_after_state, next_after_state);
	}

	const IStateSeed& ActionEvaluator::current_state_seed() const
	{
		return *_state_ptr;
	}

	const std::vector<Move>& ActionEvaluator::actions() const
	{
		return *_actions_ptr;
	}

	std::vector<int> ActionEvaluator::to_std_vector() const
	{
		return _state_ptr->to_std_vector();
	}

	std::vector<int> ActionEvaluator::get_inverted_std() const
	{
		return _state_ptr->get_inverted_std();
	}
}
