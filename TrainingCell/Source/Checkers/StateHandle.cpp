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

#include "../../Headers/Checkers/StateHandle.h"
#include "../../Headers/Checkers/State.h"

namespace TrainingCell::Checkers
{
	StateHandle::StateHandle(State state) : _state(std::move(state))
	{
		_actions = _state.get_moves();
	}

	int StateHandle::get_moves_count() const
	{
		return static_cast<int>(_actions.size());
	}

	std::vector<int> StateHandle::evaluate(const int move_id) const
	{
		return _state.get_vector(_actions[move_id]);
	}

	std::vector<int> StateHandle::evaluate() const
	{
		return _state.to_vector();
	}

	double StateHandle::calc_reward(const std::vector<int>& prev_state,
	                                const std::vector<int>& next_state) const
	{
		return State::calc_reward(prev_state, next_state);
	}

	const IStateSeed& StateHandle::current_state_seed() const
	{
		return _state;
	}

	const std::vector<Move> StateHandle::get_all_moves() const
	{
		return _actions;
	}

	std::vector<int> StateHandle::evaluate_inverted() const
	{
		return _state.get_vector_inverted();
	}

	std::vector<int> StateHandle::evaluate_inverted(const int move_id) const
	{
		return _state.get_vector_inverted(_actions[move_id]);
	}

	bool StateHandle::is_capture_action(const int action_id) const
	{
		return _actions[action_id].sub_moves[0].capture.is_valid();
	}

	bool StateHandle::is_inverted() const
	{
		return _state.is_inverted();
	}

	void StateHandle::move_invert_reset(const int action_id)
	{
		_state.make_move(_actions[action_id]);
		_state.invert();
		_actions = _state.get_moves();
	}

	State StateHandle::get_state() const
	{
		return _state;
	}
}
