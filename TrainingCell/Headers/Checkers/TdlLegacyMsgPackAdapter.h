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

#pragma once
#include "Agent.h"
#include "AgentTypeId.h"
#include "State.h"
#include "../../../DeepLearning/DeepLearning/NeuralNet/Net.h"

namespace TrainingCell::Checkers
{
	/// <summary>
	/// Class that serves a single purpose: to load legacy agent from message-pack stream
	/// </summary>
	class TdlLegacyMsgPackAdapter : public Agent
	{
	public:

		TdlLegacyMsgPackAdapter() = default;

		int make_move(const State& current_state, const std::vector<Move>& moves, const bool as_white) override
		{
			throw std::exception("Not implemented");
		};

		void game_over(const State& final_state, const GameResult& result, const bool as_white) override
		{
			throw std::exception("Not implemented");
		}

		[[nodiscard]] AgentTypeId get_type_id() const override
		{
			throw std::exception("Not implemented");
		}

		[[nodiscard]] bool can_train() const override
		{
			throw std::exception("Not implemented");
		}

		DeepLearning::Net<DeepLearning::CpuDC> _net{};
		double _exploration_epsilon{};
		double _lambda = 0.0;
		double _gamma = 0.8;
		double _alpha = 0.01;
		bool _training_mode = true;
		double _reward_factor{ 1 };
		bool _new_game{ true };
		std::vector<DeepLearning::LayerGradient<DeepLearning::CpuDC>> _z{};
		State _prev_state{};
		State _prev_afterstate{};

		MSGPACK_DEFINE(MSGPACK_BASE(Agent), _net, _z, _prev_state, _prev_afterstate, _new_game,
			_exploration_epsilon, _training_mode, _lambda, _gamma, _alpha, _reward_factor)
	};

}
