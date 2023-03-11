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

#include "../../Headers/Checkers/AgentPack.h"
#include "../../Headers/Checkers/RandomAgent.h"
#include "../../../DeepLearning/DeepLearning/MsgPackUtils.h"

namespace TrainingCell::Checkers
{
	AgentPack::AgentPack(const AgentTypeId id, std::unique_ptr<Agent>&& agent_ptr) :
	_agent_id(id), _agent_ptr(std::move(agent_ptr)) {}

	AgentPack::AgentPack(const AgentPack& anotherPack): _agent_id(anotherPack._agent_id)
	{
		if (_agent_id == TdLambdaAgent::TYPE_ID())
		{
			_agent_ptr = std::make_unique<TdLambdaAgent>(dynamic_cast<const TdLambdaAgent&>(anotherPack.agent()));
			return;
		}
		if (_agent_id == TdlEnsembleAgent::TYPE_ID())
		{
			_agent_ptr = std::make_unique<TdlEnsembleAgent>(dynamic_cast<const TdlEnsembleAgent&>(anotherPack.agent()));
			return;
		}
		if (_agent_id == RandomAgent::TYPE_ID())
		{
			_agent_ptr = std::make_unique<RandomAgent>(dynamic_cast<const RandomAgent&>(anotherPack.agent()));
			return;
		}

		throw std::exception("Not implemented");
	}

	void AgentPack::msgpack_unpack(msgpack::object const& msgpack_o)
	{
		//Read identifier only
		msgpack::type::make_define_array(_agent_id).msgpack_unpack(msgpack_o);
		//Instantiate an agent based on the identifier
		if (_agent_id == TdLambdaAgent::TYPE_ID())
		{
			auto proxy = TdLambdaAgent();
			//Read once again, but this time we read the instance of the agent as well
			msgpack::type::make_define_array(_agent_id, proxy).msgpack_unpack(msgpack_o);
			_agent_ptr = std::make_unique<decltype(proxy)>(std::move(proxy));
			return;
		}
		if (_agent_id == TdlEnsembleAgent::TYPE_ID())
		{
			auto proxy = TdlEnsembleAgent();
			//Read once again, but this time we read the instance of the agent as well
			msgpack::type::make_define_array(_agent_id, proxy).msgpack_unpack(msgpack_o);
			_agent_ptr = std::make_unique<decltype(proxy)>(std::move(proxy));
			return;
		}

		throw std::exception("Not implemented");
	}

	Agent& AgentPack::agent()
	{
		if (_agent_ptr == nullptr)
			throw std::exception("Layer is not initialized");

		return *_agent_ptr;
	}

	const Agent& AgentPack::agent() const
	{
		if (_agent_ptr == nullptr)
			throw std::exception("Layer is not initialized");

		return *_agent_ptr;
	}

	void AgentPack::save_to_file(const std::filesystem::path& file_path) const
	{
		DeepLearning::MsgPack::save_to_file(*this, file_path);
	}

	AgentPack AgentPack::load_from_file(const std::filesystem::path& file_path)
	{
		return DeepLearning::MsgPack::load_from_file<AgentPack>(file_path);
	}

	bool AgentPack::operator == (const AgentPack& anotherPack) const
	{
		return _agent_id == anotherPack._agent_id &&
			((_agent_ptr == nullptr && anotherPack._agent_ptr == nullptr) ||
				agent().equal(anotherPack.agent()));
	}

	bool AgentPack::operator != (const AgentPack& anotherPack) const
	{
		return !(*this == anotherPack);
	}
}