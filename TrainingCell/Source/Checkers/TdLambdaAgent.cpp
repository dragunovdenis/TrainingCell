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

#include "../../Headers/Checkers/TdLambdaAgent.h"
#include "../../Headers/Checkers/TdLambdaSubAgent.h"
#include "../../Headers/Checkers/TdlTrainingAdapter.h"
#include "../../Headers/Checkers/Board.h"
#include "../../../DeepLearning/DeepLearning/MsgPackUtils.h"
#include "../../Headers/Checkers/TdlLegacyMsgPackAdapter.h"

namespace TrainingCell::Checkers
{
	TdLambdaAgent::TdLambdaAgent(const TdlLegacyMsgPackAdapter& legacyContainer)
	{
		_net = DeepLearning::Net<DeepLearning::CpuDC>(legacyContainer._net);
		_lambda = legacyContainer._lambda;
		_gamma = legacyContainer._gamma;
		_alpha = legacyContainer._alpha;
		_exploration_epsilon = legacyContainer._exploration_epsilon;
		_training_sub_mode = training_mode_to_sub_mode(legacyContainer._training_mode);
		_reward_factor = legacyContainer._reward_factor;
		static_cast<Agent&>(*this) = static_cast<const Agent&>(legacyContainer);
	}

	TdLambdaAgent::TdLambdaAgent(const std::string& script_str)
	{
		assign(script_str, /*hyper-params only*/false);
	}

	void TdLambdaAgent::assign_hyperparams(const std::string& script_str)
	{
		assign(script_str, /*hyper-params only*/true);
	}

	TdLambdaAgent::TdLambdaAgent(const std::vector<std::size_t>& layer_dimensions,
	                             const double exploration_epsilon, const double lambda, const double gamma, const double alpha,
	                             const std::string& name) : TdlAbstractAgent(layer_dimensions, exploration_epsilon, lambda, gamma, alpha, name)
	{}

	TdlSettings TdLambdaAgent::get_search_settings() const
	{
		TdlSettings result(*this); // copy setting of the agent
		//and modify them a bit
		result.set_training_mode(true, true);
		result.set_training_mode(true, false);

		return result;
	}

	MoveData TdLambdaAgent::run_search(const State& current_state, const std::vector<Move>& moves) const
	{
		if (!_search_net)
			_search_net = std::make_optional(_net); // copy the current net if search net is not defined

		TdlTrainingAdapter adapter(&_search_net.value(), get_search_settings());
		Board board(&adapter, &adapter);
		board.play(_td_search_iterations,
			100 /*max moves without capture for a draw*/, current_state);

		return TdLambdaSubAgent::pick_move(current_state, moves, _search_net.value());
	}


	int TdLambdaAgent::make_move(const State& current_state, const std::vector<Move>& moves, const bool as_white)
	{
		if (_search_method == TreeSearchMethod::TD_SEARCH)
		{
			const auto move_data = run_search(current_state, moves);

			if (get_training_mode())
				//If the agent is in a "training" mode then we force it to "make a move" suggested by the search procedure
				return _sub_agents[as_white].make_move(current_state, move_data, *this, _net);

			//Otherwise we just immediately output what search procedure has come up with
			return move_data.move_id;
		}

		return _sub_agents[as_white].make_move(current_state, moves, *this, _net);
	}

	void TdLambdaAgent::game_over(const State& final_state, const GameResult& result, const bool as_white)
	{
		if (_search_method != TreeSearchMethod::NONE)
			//in the current implementation search net should be reset at the end of each episode
			_search_net.reset();

		_sub_agents[as_white].game_over(final_state, result, *this, _net);
	}

	int TdLambdaAgent::pick_move_id(const State& current_state, const std::vector<Move>& moves, const bool as_white) const
	{
		if (_search_method == TreeSearchMethod::TD_SEARCH)
			return run_search(current_state, moves).move_id;

		return TdLambdaSubAgent::pick_move(current_state, moves, _net).move_id;
	}

	AgentTypeId TdLambdaAgent::TYPE_ID()
	{
		return AgentTypeId::TDL;
	}

	AgentTypeId TdLambdaAgent::get_type_id() const
	{
		return TYPE_ID();
	}

	bool TdLambdaAgent::can_train() const
	{
		return true;
	}

	bool TdLambdaAgent::equal(const Agent& agent) const
	{
		const auto agent_ptr = dynamic_cast<const TdLambdaAgent*>(&agent);
		return agent_ptr != nullptr && TdlAbstractAgent::equal(agent) &&
			_msg_pack_version == agent_ptr->_msg_pack_version;
	}

	bool TdLambdaAgent::operator==(const TdLambdaAgent& another_agent) const
	{
		return equal(another_agent);
	}

	bool TdLambdaAgent::operator!=(const TdLambdaAgent& another_agent) const
	{
		return !(*this == another_agent);
	}

	void TdLambdaAgent::save_to_file(const std::filesystem::path& file_path) const
	{
		DeepLearning::MsgPack::save_to_file(*this, file_path);
	}

	TdLambdaAgent TdLambdaAgent::load_from_file(const std::filesystem::path& file_path)
	{
		try
		{
			return DeepLearning::MsgPack::load_from_file<TdLambdaAgent>(file_path);
		} catch (...)
		{
			//Try load to the legacy container
			const auto legacy_container = DeepLearning::MsgPack::load_from_file<TdlLegacyMsgPackAdapter>(file_path);
			return { legacy_container };
		}
	}

	std::unique_ptr<Agent> TdLambdaAgent::clone() const
	{
		return std::make_unique<TdLambdaAgent>(*this);
	}
}
