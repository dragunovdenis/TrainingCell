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

#include "../Headers/RandomAgent.h"
#include "../../DeepLearning/DeepLearning/Utilities.h"

namespace TrainingCell
{
	int RandomAgent::make_move(const IActionEvaluator& evaluator, const bool as_white)
	{
		return DeepLearning::Utils::get_random_int(0, evaluator.get_actions_count() - 1);
	}

	void RandomAgent::game_over(const IState& final_state, const GameResult& result, const bool as_white)
	{
		//Just do nothing because this agent can't improve its performance
	}

	AgentTypeId RandomAgent::TYPE_ID()
	{
		return AgentTypeId::RANDOM;
	}

	AgentTypeId RandomAgent::get_type_id() const
	{
		return TYPE_ID();
	}

	bool RandomAgent::can_train() const
	{
		return false;
	}

	bool RandomAgent::equal(const Agent& agent) const
	{
		return false; //Random agent can't be equal to another agent by definition
	}

}
