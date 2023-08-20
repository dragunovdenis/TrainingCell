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

	TdLambdaAgent::TdLambdaAgent(const std::vector<std::size_t>& layer_dimensions,
	                             const double exploration_epsilon, const double lambda, const double gamma, const double alpha,
	                             const std::string& name) : TdlAbstractAgent(layer_dimensions, exploration_epsilon, lambda, gamma, alpha, name)
	{}

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
