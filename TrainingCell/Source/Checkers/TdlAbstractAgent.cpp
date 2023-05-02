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

#include "../../Headers/Checkers/TdlAbstractAgent.h"
#include "../../../DeepLearning/DeepLearning/Utilities.h"
#include <nlohmann/json.hpp>

namespace TrainingCell::Checkers
{
	void TdlAbstractAgent::initialize_net(const std::vector<std::size_t>& layer_dimensions)
	{
		if (layer_dimensions.empty() || layer_dimensions[0] != StateSize || layer_dimensions.rbegin()[0] != 1)
			throw std::exception("Invalid Net configuration");

		std::vector activ_func_ids(layer_dimensions.size() - 1, DeepLearning::ActivationFunctionId::RELU);
		activ_func_ids.rbegin()[0] = DeepLearning::ActivationFunctionId::LINEAR;

		_net = DeepLearning::Net(layer_dimensions, activ_func_ids);
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

	void TdlAbstractAgent::assign(const std::string& script_str, const bool hyper_params_only)
	{
		const auto json = nlohmann::json::parse(script_str);

		if (parse_agent_type_id(json[json_agent_type_id].get<std::string>()) != get_type_id())
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

	std::string TdlAbstractAgent::to_script() const
	{
		nlohmann::json json;
		json[json_agent_type_id] = to_string(get_type_id());
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

	bool TdlAbstractAgent::equal_hyperparams(const TdlAbstractAgent& anotherAgent) const
	{
		return _net.equal_hyperparams(anotherAgent._net) &&
			_name == anotherAgent._name &&
			_exploration_epsilon == anotherAgent._exploration_epsilon &&
			_training_mode == anotherAgent._training_mode &&
			_lambda == anotherAgent._lambda &&
			_gamma == anotherAgent._gamma &&
			_alpha == anotherAgent._alpha;
	}

	bool TdlAbstractAgent::equal(const Agent& agent) const
	{
		const auto other_agent_ptr = dynamic_cast<const TdlAbstractAgent*>(&agent);

		return other_agent_ptr != nullptr && Agent::equal(agent) &&
			_net.equal(other_agent_ptr->_net) &&
			_exploration_epsilon == other_agent_ptr->_exploration_epsilon &&
			_alpha == other_agent_ptr->_alpha &&
			_gamma == other_agent_ptr->_gamma &&
			_lambda == other_agent_ptr->_lambda &&
			_training_mode == other_agent_ptr->_training_mode &&
			_reward_factor == other_agent_ptr->_reward_factor;
	}

	TdlAbstractAgent::TdlAbstractAgent(const std::vector<std::size_t>& layer_dimensions,
	                                   const double exploration_epsilon, const double lambda, const double gamma, const double alpha,
	                                   const std::string& name) : _exploration_epsilon(exploration_epsilon), _lambda(lambda), _gamma(gamma), _alpha(alpha)
	{
		set_name(name);
		initialize_net(layer_dimensions);
	}

	void TdlAbstractAgent::set_exploration_probability(double epsilon)
	{
		_exploration_epsilon = epsilon;
	}

	double TdlAbstractAgent::get_exploratory_probability() const
	{
		return _exploration_epsilon;
	}

	void TdlAbstractAgent::set_discount(double gamma)
	{
		_gamma = gamma;
	}

	double TdlAbstractAgent::get_discount() const
	{
		return _gamma;
	}

	void TdlAbstractAgent::set_training_mode(const bool training_mode)
	{
		_training_mode = training_mode;
	}

	bool TdlAbstractAgent::get_training_mode() const
	{
		return _training_mode;
	}

	void TdlAbstractAgent::set_reward_factor(const double reward_factor)
	{
		_reward_factor = reward_factor;
	}

	double TdlAbstractAgent::get_reward_factor() const
	{
		return _reward_factor;
	}

	void TdlAbstractAgent::set_lambda(const double lambda)
	{
		_lambda = lambda;
	}

	double TdlAbstractAgent::get_lambda() const
	{
		return _lambda;
	}

	void TdlAbstractAgent::set_learning_rate(const double alpha)
	{
		_alpha = alpha;
	}

	double TdlAbstractAgent::get_learning_rate() const
	{
		return _alpha;
	}

	std::vector<unsigned int> TdlAbstractAgent::get_net_dimensions() const
	{
		const auto dims = _net.get_dimensions();
		std::vector<unsigned int> result(dims.size());

		std::ranges::transform(dims, result.begin(), [](const auto& dim)
			{
				return static_cast<unsigned int>(dim.coord_prod());
			});

		return result;
	}
}
