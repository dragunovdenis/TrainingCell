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

#include "../Headers/StateTraceRecorder.h"
#include "../Headers/Chess/ChessState.h"
#include "../Headers/Checkers/CheckersState.h"

namespace TrainingCell
{
	template <class S>
	StateTraceRecorder<S>::StateTraceRecorder(const S& init_state) : S(init_state)
	{}

	template <class S>
	void StateTraceRecorder<S>::add_record(const typename S::Move& move, const bool draw_flag)
	{
		// sanity check
		if (_moves.size() != _draw_flags.size())
			throw std::exception("A attempt to add a record after recorder was finalized.");

		_moves.push_back(move);
		_draw_flags.push_back(draw_flag);
	}

	template <class S>
	void StateTraceRecorder<S>::add_final_record(const bool draw_flag)
	{
		_draw_flags.push_back(draw_flag);
	}

	template <class S>
	void StateTraceRecorder<S>::adjust_last_move(const typename S::Move& move)
	{
		*_moves.rbegin() = move;
	}

	template <class S>
	bool StateTraceRecorder<S>::get_moves(std::vector<typename S::Move>& out_result) const
	{
		// sanity check
		if (_moves_counter > _moves.size() || _moves_counter >= _draw_flags.size())
			throw std::exception("Can't generate moves past the recorded trace.");

		out_result.clear();
		if (_moves_counter < _moves.size())
			out_result.push_back(_moves[_moves_counter]);

		return _draw_flags[_moves_counter];
	}

	template <class S>
	void StateTraceRecorder<S>::make_move_and_invert(const typename S::Move& move)
	{
		if (_moves_counter >= _moves.size() || move != _moves[_moves_counter])
			throw std::exception("Illegal move");

		S::make_move_and_invert(_moves[_moves_counter++]);
	}

	template class StateTraceRecorder<Chess::ChessState>;
	template class StateTraceRecorder<Checkers::CheckersState>;
}
