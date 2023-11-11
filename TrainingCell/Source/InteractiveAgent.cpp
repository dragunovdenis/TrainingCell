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

#include "../Headers/InteractiveAgent.h"

namespace TrainingCell
{
	InteractiveAgent::InteractiveAgent(const MakeMoveCallback& make_move_callback, const GameOverCallback& game_over_callback,
		const bool play_for_whites) : _make_move_callback(make_move_callback),
		_game_over_callback(game_over_callback), _play_for_whites(play_for_whites)
	{
		if (_make_move_callback == nullptr || _game_over_callback == nullptr)
			throw std::exception("Invalid callback pointers");
	}

	int InteractiveAgent::make_move(const IStateReadOnly& state, const bool as_white)
	{
		//Sanity check
		if (as_white != _play_for_whites)
			throw std::exception("Inconsistency encountered");

		if (_play_for_whites)
			return _make_move_callback(state.evaluate_ui(), state.get_all_moves());

		std::vector<Move> moves_inverted(state.get_all_moves().size());

		std::ranges::transform(state.get_all_moves(), moves_inverted.begin(), [](const auto m)
			{
				return m.get_inverted();
			});

		return _make_move_callback(state.evaluate_ui_inverted(), moves_inverted);
	}

	void InteractiveAgent::game_over(const IStateReadOnly& final_state, const GameResult& result, const bool as_white)
	{
		//Sanity check
		if (as_white != _play_for_whites)
			throw std::exception("Inconsistency encountered");

		if (_play_for_whites)
			_game_over_callback(final_state.evaluate_ui(), result);

		_game_over_callback(final_state.evaluate_ui_inverted(), result);
	}

	AgentTypeId InteractiveAgent::TYPE_ID()
	{
		return AgentTypeId::INTERACTIVE;
	}

	AgentTypeId InteractiveAgent::get_type_id() const
	{
		return TYPE_ID();
	}

	bool InteractiveAgent::can_train() const
	{
		return false;
	}

	bool InteractiveAgent::equal(const Agent& agent) const
	{
		//Can't be equal to any other agent by definition.
		//It is almost impossible to define what it means
		//to be equal for an agent that is supposed to represent an "external" player 
		return false;
	}
}