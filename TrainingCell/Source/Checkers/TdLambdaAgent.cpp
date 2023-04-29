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
#include "../../../DeepLearning/DeepLearning/MsgPackUtils.h"
#include "../../../DeepLearning/DeepLearning/Utilities.h"
#include <nlohmann/json.hpp>

namespace TrainingCell::Checkers
{
	void TdLambdaAgent::reset()
	{
		_new_game = true;
		_z.clear();
	}

	void TdLambdaAgent::game_over(const State& final_state, const GameResult& result, const bool as_white)
	{
		if (_training_mode)
		{
			const auto reward = 2 * static_cast<int>(result);
			const auto delta = reward - update_z_and_evaluate_prev_after_state();
			_net.update(_z, -_alpha * delta, 0.0);
		}

		reset();
	}

	double TdLambdaAgent::update_z_and_evaluate_prev_after_state()
	{
		auto calc_result = _net.calc_gradient_and_value(_prev_afterstate.to_tensor(),
			DeepLearning::Tensor(1, 1, 1, false), DeepLearning::CostFunctionId::LINEAR);

		auto& gradient = std::get<0>(calc_result);

		if (_z.empty())
		{
			_z = std::move(gradient);
		}
		else
		{
			if (gradient.size() != _z.size())
				throw std::exception("Incompatible data");

			for (auto layer_id = 0ull; layer_id < gradient.size(); ++layer_id)
			{
				gradient[layer_id] += _z[layer_id] * _lambda * _gamma;
				_z[layer_id] = std::move(gradient[layer_id]);
			}
		}

		return std::get<1>(calc_result)(0, 0, 0);
	}

	int TdLambdaAgent::make_move(const State& current_state, const std::vector<Move>& moves, const bool as_white)
	{
		if (!_training_mode)
			return pick_move_id(current_state, moves);

		const auto move_data = pick_move(current_state, moves);

		if (_new_game)
		{
			_prev_afterstate = move_data.after_state;
			_prev_state = current_state;
			_new_game = false;
			return move_data.move_id;
		}

		const auto reward = get_reward_factor() * Utils::calculate_reward(_prev_state, current_state);

		const auto prev_afterstate_value = update_z_and_evaluate_prev_after_state();
		const auto delta = reward + _gamma * move_data.value - prev_afterstate_value;

		_net.update(_z, -_alpha * delta, 0.0);

		_prev_afterstate = move_data.after_state;
		_prev_state = current_state;

		return move_data.move_id;
	}

	int TdLambdaAgent::pick_move_id(const State& state, const std::vector<Move>& moves) const
	{
		return pick_move(state, moves).move_id;
	}

	MoveData TdLambdaAgent::evaluate(const State& state, const std::vector<Move>& moves, const int move_id) const
	{
		auto afterstate = state;
		afterstate.make_move(moves[move_id], true, false);
		const auto value = _net.act(afterstate.to_tensor())(0, 0, 0);
		return { move_id,  value, afterstate };
	}

	MoveData TdLambdaAgent::pick_move(const State& state, const std::vector<Move>& moves) const
	{
		if (moves.empty())
			return { -1 };

		if (_training_mode && DeepLearning::Utils::get_random(0, 1.0) <= _exploration_epsilon)
			return evaluate(state, moves, DeepLearning::Utils::get_random_int(0, static_cast<int>(moves.size()) - 1));

		MoveData best_move_data{ -1, -std::numeric_limits<double>::max() };

		for (auto move_id = 0ull; move_id < moves.size(); ++move_id)
		{
			const auto trial_move_data = evaluate(state, moves, static_cast<int>(move_id));

			if (trial_move_data.value > best_move_data.value)
				best_move_data = trial_move_data;
		}

		if (best_move_data.move_id < 0)
			throw std::exception((get_name() + ": neural network is NaN. Try decreasing learning rate parameter.").c_str());

		return best_move_data;
	}

	const char* json_agent_type_id = "AgentType";
	const char* json_name_id = "Name";
	const char* json_net_dim_id = "NetDim";
	const char* json_lambda_id = "Lambda";
	const char* json_discount_id = "Discount";
	const char* json_learning_rate_id = "LearnRate";
	const char* json_exploration_rate_id = "Exploration";
	const char* json_training_mode_id = "TrainingMode";
	const char* json_reward_factor_id = "RewardFactor";

	std::string TdLambdaAgent::to_script() const
	{
		nlohmann::json json;
		json[json_agent_type_id] = to_string(TYPE_ID());
		json[json_name_id] = _name;
		json[json_net_dim_id] = DeepLearning::Utils::vector_to_str(get_net_dimensions());
		json[json_lambda_id] = _lambda;
		json[json_discount_id] = _gamma;
		json[json_learning_rate_id] = _alpha;
		json[json_exploration_rate_id] = _exploration_epsilon;
		json[json_training_mode_id] = _training_mode;
		json[json_reward_factor_id] = _reward_factor;

		return json.dump();
	}

	TdLambdaAgent::TdLambdaAgent(const std::string& script_str):_new_game(true)
	{
		assign(script_str, /*hyper_params_only*/ false);
	}

	void TdLambdaAgent::assign_hyperparams(const std::string& script_str)
	{
		assign(script_str, /*hyper_params_only*/ true);
	}

	void TdLambdaAgent::assign(const std::string& script_str, const bool hyper_params_only)
	{
		const auto json = nlohmann::json::parse(script_str);

		if (parse_agent_type_id(json[json_agent_type_id].get<std::string>()) != TYPE_ID())
			throw std::exception("Unexpected agent type");

		if (json.contains(json_name_id))
			set_name(json[json_name_id]);

		if (json.contains(json_net_dim_id))
		{
			if (hyper_params_only)
			{
				//Sanity check
				auto dim_str = json[json_net_dim_id].get<std::string>();
				if (DeepLearning::Utils::parse_vector<unsigned int>(dim_str) != get_net_dimensions())
					throw std::exception("Net dimension in the script differs from that of the agent");
			}
			else
			{
				auto layer_dims_str = json[json_net_dim_id].get<std::string>();
				initialize_net(DeepLearning::Utils::parse_vector<std::size_t>(layer_dims_str));
			}
		}

		if (json.contains(json_lambda_id))
			_lambda = json[json_lambda_id].get<double>();

		if (json.contains(json_discount_id))
			_gamma = json[json_discount_id].get<double>();

		if (json.contains(json_learning_rate_id))
			_alpha = json[json_learning_rate_id].get<double>();

		if (json.contains(json_exploration_rate_id))
			_exploration_epsilon = json[json_exploration_rate_id].get<double>();

		if (json.contains(json_training_mode_id))
			_training_mode = json[json_training_mode_id].get<bool>();

		if (json.contains(json_reward_factor_id))
			_reward_factor = json[json_reward_factor_id].get<double>();
	}

	TdLambdaAgent::TdLambdaAgent(
		const std::vector<std::size_t>& layer_dimensions, const double exploration_epsilon,
		const double lambda, const double gamma, const double alpha, const std::string& name) :
		TdlAbstractAgent(layer_dimensions, exploration_epsilon, lambda, gamma, alpha, name)
	{}

	bool TdLambdaAgent::equal_hyperparams(const TdLambdaAgent& anotherAgent) const
	{
		return _net.equal_hyperparams(anotherAgent._net) &&
			_name == anotherAgent._name &&
		    _exploration_epsilon == anotherAgent._exploration_epsilon &&
			_training_mode == anotherAgent._training_mode &&
			_lambda == anotherAgent._lambda &&
			_gamma == anotherAgent._gamma &&
			_alpha == anotherAgent._alpha;
	}

	bool TdLambdaAgent::operator ==(const TdLambdaAgent& anotherAgent) const
	{
		return equal(anotherAgent);
	}

	bool TdLambdaAgent::operator !=(const TdLambdaAgent& anotherAgent) const
	{
		return !(*this == anotherAgent);
	}

	void TdLambdaAgent::save_to_file(const std::filesystem::path& file_path) const
	{
		DeepLearning::MsgPack::save_to_file(*this, file_path);
	}

	TdLambdaAgent TdLambdaAgent::load_from_file(const std::filesystem::path& file_path)
	{
		return DeepLearning::MsgPack::load_from_file<TdLambdaAgent>(file_path);
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
		const auto other_agent_ptr = dynamic_cast<const TdLambdaAgent*>(&agent);
		return other_agent_ptr != nullptr && TdlAbstractAgent::equal(agent) &&
			_z == other_agent_ptr->_z &&
			_prev_state == other_agent_ptr->_prev_state &&
			_prev_afterstate == other_agent_ptr->_prev_afterstate &&
			_new_game == other_agent_ptr->_new_game;
	}
}