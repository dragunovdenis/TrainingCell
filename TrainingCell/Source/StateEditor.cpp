//Copyright (c) 2024 Denys Dragunov, dragunovdenis@gmail.com
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

#include "../Headers/StateEditor.h"
#include "../Headers/IState.h"
#include "../Headers/Checkers/CheckersState.h"
#include "../Headers/Chess/ChessState.h"

namespace TrainingCell
{
	template <class S>
	StateEditor<S>::StateEditor() : _state(S::get_start_state())
	{}

	template <class S>
	std::unique_ptr<IState> StateEditor<S>::yield(const bool initialize_recorder) const
	{
		return _state.yield(initialize_recorder);
	}

	template <class S>
	StateTypeId StateEditor<S>::state_type() const
	{
		return S::type();
	}

	template <class S>
	std::vector<int> StateEditor<S>::to_vector() const
	{
		return _state.to_vector_64();
	}

	template <class S>
	std::vector<int> StateEditor<S>::get_options(const PiecePosition& pos) const
	{
		return _state.get_edit_options(pos);
	}

	template <class S>
	void StateEditor<S>::apply_option(const PiecePosition& pos, const int option_id)
	{
		return _state.apply_edit_option(pos, option_id);
	}

	template <class S>
	void StateEditor<S>::reset()
	{
		_state.reset();
	}

	template <class S>
	void StateEditor<S>::clear()
	{
		_state.clear();
	}

	template <class S>
	StateTypeId StateEditor<S>::get_state_type() const
	{
		return S::type();
	}

	template class StateEditor<Chess::ChessState>;
	template class StateEditor<Checkers::CheckersState>;
}
