#pragma once
#include <memory>
#include "Agent.h"
#include <msgpack.hpp>

namespace TrainingCell::Checkers
{
	/// <summary>
	/// A wrapper for checkers agents to handle their message-pack serialization in a unified way
	/// </summary>
	class AgentPack
	{
		/// <summary>
		/// Agent identifier
		/// </summary>
		AgentTypeId _agent_id = AgentTypeId::UNKNOWN;

		/// <summary>
		/// Agent pointer
		/// </summary>
		std::unique_ptr<Agent> _agent_ptr = nullptr;

		/// <summary>
		/// Constructor
		/// </summary>
		/// <param name="id">Agent identifier</param>
		/// <param name="agent_ptr">Agent smart pointer</param>
		AgentPack(const AgentTypeId id, std::unique_ptr<Agent>&& agent_ptr);
	public:

		/// <summary>
		/// Default constructor
		/// </summary>
		AgentPack() = default;

		/// <summary>
		///	Copy constructor
		/// </summary>
		AgentPack(const AgentPack& anotherPack);

		/// <summary>
		/// Custom "packing" method
		/// </summary>
		template <typename Packer>
		void msgpack_pack(Packer& msgpack_pk) const
		{
			if (_agent_id == TdLambdaAgent::ID())
			{
				const auto& layer_ref_casted = dynamic_cast<const TdLambdaAgent&>(agent());
				msgpack::type::make_define_array(_agent_id, layer_ref_casted).msgpack_pack(msgpack_pk);
				return;
			}
			if (_agent_id == TdlEnsembleAgent::ID())
			{
				const auto& layer_ref_casted = dynamic_cast<const TdlEnsembleAgent&>(agent());
				msgpack::type::make_define_array(_agent_id, layer_ref_casted).msgpack_pack(msgpack_pk);
				return;
			}

			throw std::exception("Not implemented");
		}

		/// <summary>
		/// Custom "unpacking" method
		/// </summary>
		void msgpack_unpack(msgpack::object const& msgpack_o);

		/// <summary>
		/// Factory method
		/// </summary>
		template <class L, class... Types>
		static AgentPack make(Types&&... args)
		{
			return AgentPack(L::ID(), std::make_unique<L>(std::forward<Types>(args)...));
		}

		/// <summary>
		/// Reference to the agent
		/// </summary>
		Agent& agent();

		/// <summary>
		/// Reference to the agent (constant version)
		/// </summary>
		[[nodiscard]] const Agent& agent() const;

		/// <summary>
		/// Saves the pack to the given file on disk
		/// </summary>
		void save_to_file(const std::filesystem::path& file_path) const;

		/// <summary>
		/// Loads pack from the given file on disk
		/// </summary>
		static AgentPack load_from_file(const std::filesystem::path& file_path);

		/// <summary>
		/// Equality operator
		/// </summary>
		bool operator == (const AgentPack& anotherPack) const;

		/// <summary>
		/// Inequality operator
		/// </summary>
		bool operator != (const AgentPack& anotherPack) const;
	};
}
