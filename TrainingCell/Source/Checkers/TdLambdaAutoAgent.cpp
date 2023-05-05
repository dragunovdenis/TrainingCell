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
#include "../../../DeepLearning/DeepLearning/MsgPackUtils.h"

namespace TrainingCell::Checkers
{
	TdLambdaAutoAgent::TdLambdaAutoAgent(const std::vector<std::size_t>& layer_dimensions,
	                                     const double exploration_epsilon, const double lambda, const double gamma, const double alpha,
	                                     const std::string& name) : TdlAbstractAgent(layer_dimensions, exploration_epsilon, lambda, gamma, alpha, name)
	{}

	int TdLambdaAutoAgent::make_move(const State& current_state, const std::vector<Move>& moves, const bool as_white)
	{
		//Sanity check
		if (current_state.is_inverted() == as_white)
			throw std::exception("Inconsistency encountered");

		if (as_white)
			return _white_sub_agent.make_move(current_state, moves, this, &_net);

		return _black_sub_agent.make_move(current_state, moves, this, &_net);
	}

	void TdLambdaAutoAgent::game_over(const State& final_state, const GameResult& result, const bool as_white)
	{
		if (as_white)
			return _white_sub_agent.game_over(final_state, result, this, &_net);

		return _black_sub_agent.game_over(final_state, result, this, &_net);
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
		const auto agent_ptr = dynamic_cast<const TdLambdaAutoAgent*>(&agent);
		return agent_ptr != nullptr && TdlAbstractAgent::equal(agent);
	}

	bool TdLambdaAutoAgent::operator==(const TdLambdaAutoAgent& another_agent) const
	{
		return equal(another_agent);
	}

	bool TdLambdaAutoAgent::operator!=(const TdLambdaAutoAgent& another_agent) const
	{
		return !(*this == another_agent);
	}

	void TdLambdaAutoAgent::save_to_file(const std::filesystem::path& file_path) const
	{
		DeepLearning::MsgPack::save_to_file(*this, file_path);
	}

	namespace
	{
		/// <summary>
		/// Class that serves a single purpose: to load legacy agent from message-pack stream
		/// </summary>
		class LegacyTdLambdaAgent : public Agent
		{
		public:

			LegacyTdLambdaAgent() = default;

			int make_move(const State& current_state, const std::vector<Move>& moves, const bool as_white) override
			{ throw std::exception("Not implemented");};

			void game_over(const State& final_state, const GameResult& result, const bool as_white) override
			{ throw std::exception("Not implemented");}

			[[nodiscard]] AgentTypeId get_type_id() const override
			{ throw std::exception("Not implemented"); }

			[[nodiscard]] bool can_train() const override
			{ throw std::exception("Not implemented"); }

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

	TdLambdaAutoAgent TdLambdaAutoAgent::load_from_file(const std::filesystem::path& file_path)
	{
		try
		{
			return DeepLearning::MsgPack::load_from_file<TdLambdaAutoAgent>(file_path);
		} catch (...)
		{
			//Try load to the legacy container
			const auto temp_container = DeepLearning::MsgPack::load_from_file<LegacyTdLambdaAgent>(file_path);

			TdLambdaAutoAgent result{};
			result._net = DeepLearning::Net<DeepLearning::CpuDC>(temp_container._net);
			result._lambda = temp_container._lambda;
			result._gamma = temp_container._gamma;
			result._alpha = temp_container._alpha;
			result._exploration_epsilon = temp_container._exploration_epsilon;
			result._training_mode = temp_container._training_mode;
			result._reward_factor = temp_container._reward_factor;
			static_cast<Agent&>(result) = static_cast<const Agent&>(temp_container);

			return result;
		}
	}
}
