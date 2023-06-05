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

#include "../../Headers/Checkers/TdlTrainingAdapter.h"

namespace TrainingCell::Checkers
{
	TdlTrainingAdapter::TdlTrainingAdapter(DeepLearning::Net<DeepLearning::CpuDC>* net_ptr, const TdlSettings& settings) :
		_net_ptr(net_ptr), _settings(settings)
	{
		if (!_net_ptr)
			throw std::exception("Invalid pointer to the neural network");
	}

	int TdlTrainingAdapter::make_move(const State& current_state, const std::vector<Move>& moves, const bool as_white)
	{
		return _sub_agents[as_white].make_move(current_state, moves, _settings, *_net_ptr);
	}

	void TdlTrainingAdapter::game_over(const State& final_state, const GameResult& result, const bool as_white)
	{
		_sub_agents[as_white].game_over(final_state, result, _settings, *_net_ptr);
	}
}
