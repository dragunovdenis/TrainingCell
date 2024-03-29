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

#include "../Headers/TdlTrainingAdapter.h"
#include "../Headers/StateTypeController.h"

namespace TrainingCell
{
	TdlTrainingAdapter::TdlTrainingAdapter(INet* net_ptr, const TdlSettings& settings, const StateTypeId state_type_id) :
		_net_ptr(net_ptr), _settings(settings), _state_type_id(state_type_id)
	{
		if (!_net_ptr)
			throw std::exception("Invalid pointer to the neural network");

		if (!_net_ptr->validate_net_input_size(StateTypeController::get_state_size(_state_type_id)))
			throw std::exception("Net is incompatible with the suggested state type.");
	}

	int TdlTrainingAdapter::make_move(const IStateReadOnly& state, const bool as_white)
	{
		return _sub_agents[as_white].make_move(state, _settings, *_net_ptr);
	}

	void TdlTrainingAdapter::game_over(const IStateReadOnly& final_state, const GameResult& result, const bool as_white)
	{
		_sub_agents[as_white].game_over(final_state, result, _settings, *_net_ptr);
	}

	StateTypeId TdlTrainingAdapter::get_state_type_id() const
	{
		return _state_type_id;
	}
}
