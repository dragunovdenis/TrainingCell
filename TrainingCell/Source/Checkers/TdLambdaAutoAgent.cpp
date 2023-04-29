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

#include "../../Headers/Checkers/TdLambdaAutoAgent.h"
#include "../../Headers/Checkers/TdLambdaSubAgent.h"

namespace TrainingCell::Checkers
{
	DeepLearning::Net<DeepLearning::CpuDC>& TdLambdaAutoAgent::net()
	{
		return _net;
	}

	void TdLambdaAutoAgent::init_sub_agents()
	{
		_white_sub_agent_ptr = std::make_unique<TdLambdaSubAgent<true>>(this, this);
		_black_sub_agent_ptr = std::make_unique<TdLambdaSubAgent<false>>(this, this);
	}

	TdLambdaAutoAgent::TdLambdaAutoAgent()
	{
		init_sub_agents();
	}

	TdLambdaAutoAgent::TdLambdaAutoAgent(const std::vector<std::size_t>& layer_dimensions,
		const double exploration_epsilon, const double lambda, const double gamma, const double alpha,
		const std::string& name) : TdlAbstractAgent(layer_dimensions, exploration_epsilon, lambda, gamma, alpha, name)
	{
		init_sub_agents();
	}

	int TdLambdaAutoAgent::make_move(const State& current_state, const std::vector<Move>& moves, const bool as_white)
	{
		if (current_state.is_inverted() == as_white)
			throw std::exception("Inconsistency encountered");

		if (as_white)
			return _white_sub_agent_ptr->make_move(current_state, moves);

		return _black_sub_agent_ptr->make_move(current_state, moves);
	}

	void TdLambdaAutoAgent::game_over(const State& final_state, const GameResult& result, const bool as_white)
	{
		if (as_white)
			return _white_sub_agent_ptr->game_over(final_state, result);

		return _black_sub_agent_ptr->game_over(final_state, result);
	}

	AgentTypeId TdLambdaAutoAgent::TYPE_ID()
	{
		return AgentTypeId::TDL_AUTO;
	}

	AgentTypeId TdLambdaAutoAgent::get_type_id() const
	{
		return TYPE_ID();
	}

	bool TdLambdaAutoAgent::can_train() const
	{
		return true;
	}

	bool TdLambdaAutoAgent::equal(const Agent& agent) const
	{
		return TdlAbstractAgent::equal(agent);
	}
}
