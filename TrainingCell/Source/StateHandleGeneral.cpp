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

#include <algorithm>
#include "../Headers/StateHandleGeneral.h"
#include "../Headers/Checkers/CheckersState.h"
#include "../Headers/Chess/ChessState.h"

namespace TrainingCell
{
	template <class S>
	StateHandleGeneral<S>::StateHandleGeneral(S state) : _state(std::move(state))
	{
		_actions = _state.get_moves();
	}

	template <class S>
	int StateHandleGeneral<S>::get_moves_count() const
	{
		return static_cast<int>(_actions.size());
	}

	template <class S>
	std::vector<int> StateHandleGeneral<S>::evaluate(const int move_id) const
	{
		return _state.get_vector(_actions[move_id]);
	}

	template <class S>
	std::vector<int> StateHandleGeneral<S>::evaluate() const
	{
		return _state.to_vector();
	}

	template <class S>
	double StateHandleGeneral<S>::calc_reward(const std::vector<int>& prev_state,
		const std::vector<int>& next_state) const
	{
		return S::calc_reward(prev_state, next_state);
	}

	template <class S>
	const IStateSeed& StateHandleGeneral<S>::current_state_seed() const
	{
		return _state;
	}

	template <class S>
	const std::vector<Move> StateHandleGeneral<S>::get_all_moves() const
	{
		std::vector<Move> result(_actions.size());

		std::ranges::transform(_actions, result.begin(),
			[](const auto& chk_move) { return chk_move.to_move(); });

		return result;
	}
	template <class S>
	bool StateHandleGeneral<S>::is_capture_action(const int action_id) const
	{
		return _actions[action_id].is_capturing();
	}

	template <class S>
	bool StateHandleGeneral<S>::is_inverted() const
	{
		return _state.is_inverted();
	}

	template <class S>
	void StateHandleGeneral<S>::move_invert_reset(const int action_id)
	{
		_state.make_move(_actions[action_id]);
		_state.invert();
		_actions = _state.get_moves();
	}

	template <class S>
	S StateHandleGeneral<S>::get_state() const
	{
		return _state;
	}

	template <class S>
	std::vector<int> StateHandleGeneral<S>::evaluate_ui() const
	{
		return _state.to_vector_64();
	}

	template <class S>
	std::vector<int> StateHandleGeneral<S>::evaluate_ui_inverted() const
	{
		return _state.to_vector_64_inverted();
	}

	template class StateHandleGeneral<Checkers::CheckersState>;
	template class StateHandleGeneral<Chess::ChessState>;
}