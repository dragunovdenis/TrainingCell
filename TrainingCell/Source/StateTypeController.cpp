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

#include "../Headers/StateTypeController.h"
#include "../Headers/Checkers/CheckersState.h"
#include "../Headers/Chess/ChessState.h"

namespace TrainingCell
{
	std::size_t StateTypeController::get_state_size(const StateTypeId type)
	{
		if (type == Checkers::CheckersState::type()) return Checkers::CheckersState::state_size();
		if (type == Chess::ChessState::type())       return Chess::ChessState::state_size();
		throw std::exception("State size is undefined.");
	}

	std::unique_ptr<IStateSeed> StateTypeController::get_start_seed(const StateTypeId type)
	{
		if (type == Checkers::CheckersState::type())  return std::make_unique<Checkers::CheckersState>(Checkers::CheckersState::get_start_state());
		if (type == Chess::ChessState::type())        return std::make_unique <Chess::ChessState>(Chess::ChessState::get_start_state());
		throw std::exception("State seed is undefined.");
	}

	StateTypeId StateTypeController::get_common_state(const StateTypeId st0, const StateTypeId st1)
	{
		return static_cast<StateTypeId>(static_cast<int>(st0) & static_cast<int>(st1));
	}

	bool StateTypeController::states_are_compatible(const StateTypeId st0, const StateTypeId st1)
	{
		return get_common_state(st0, st1) != StateTypeId::INVALID;
	}

	bool StateTypeController::states_are_compatible(const StateTypeId st0, const StateTypeId st1, const StateTypeId st2)
	{
		return get_common_state(get_common_state(st0, st1), st2) != StateTypeId::INVALID;
	}

	bool StateTypeController::validate(const IMinimalAgent& agent0, const IMinimalAgent& agent1,
		const IStateSeed& state_seed)
	{
		return states_are_compatible(agent0.get_state_type_id(), agent1.get_state_type_id(), state_seed.state_type());
	}

	bool StateTypeController::can_play(const IMinimalAgent& agent0, const IMinimalAgent& agent1,
		StateTypeId& out_state_type_id)
	{
		out_state_type_id = get_common_state(agent0.get_state_type_id(), agent1.get_state_type_id());
		return out_state_type_id == StateTypeId::CHECKERS || out_state_type_id == StateTypeId::CHESS;
	}
}
