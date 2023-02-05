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
			if (_agent_id == TdLambdaAgent::TYPE_ID())
			{
				const auto& layer_ref_casted = dynamic_cast<const TdLambdaAgent&>(agent());
				msgpack::type::make_define_array(_agent_id, layer_ref_casted).msgpack_pack(msgpack_pk);
				return;
			}
			if (_agent_id == TdlEnsembleAgent::TYPE_ID())
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
			return AgentPack(L::TYPE_ID(), std::make_unique<L>(std::forward<Types>(args)...));
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
