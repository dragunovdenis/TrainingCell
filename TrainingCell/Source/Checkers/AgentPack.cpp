#include "../../Headers/Checkers/AgentPack.h"
#include "../../../DeepLearning/DeepLearning/MsgPackUtils.h"

namespace TrainingCell::Checkers
{
	AgentPack::AgentPack(const AgentTypeId id, std::unique_ptr<Agent>&& agent_ptr) :
	_agent_id(id), _agent_ptr(std::move(agent_ptr)) {}

	AgentPack::AgentPack(const AgentPack& anotherPack): _agent_id(anotherPack._agent_id)
	{
		if (_agent_id == TdLambdaAgent::ID())
		{
			_agent_ptr = std::make_unique<TdLambdaAgent>(dynamic_cast<const TdLambdaAgent&>(anotherPack.agent()));
			return;
		}
		if (_agent_id == TdlEnsembleAgent::ID())
		{
			_agent_ptr = std::make_unique<TdlEnsembleAgent>(dynamic_cast<const TdlEnsembleAgent&>(anotherPack.agent()));
			return;
		}
		if (_agent_id == RandomAgent::ID())
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
		if (_agent_id == TdLambdaAgent::ID())
		{
			auto proxy = TdLambdaAgent();
			//Read once again, but this time we read the instance of the agent as well
			msgpack::type::make_define_array(_agent_id, proxy).msgpack_unpack(msgpack_o);
			_agent_ptr = std::make_unique<decltype(proxy)>(std::move(proxy));
			return;
		}
		if (_agent_id == TdlEnsembleAgent::ID())
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
			(_agent_ptr == nullptr && anotherPack._agent_ptr == nullptr ||
				agent().equal(anotherPack.agent()));
	}

	bool AgentPack::operator != (const AgentPack& anotherPack) const
	{
		return !(*this == anotherPack);
	}
}