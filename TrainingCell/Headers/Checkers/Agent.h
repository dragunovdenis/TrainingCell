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
#include "Utils.h"

namespace TrainingCell::Checkers
{
	/// <summary>
	///	Enumerates different agent types
	///	Used to handle message-pack serialization of agents through their base class (class Agent)
	/// </summary>
	enum class AgentTypeId : int
	{
		UNKNOWN = 0,
		RANDOM = 1,
		INTERACTIVE = 2,
		TDL = 3,
		TDL_ENSEMBLE = 4,
	};

	/// <summary>
	///	Abstract checkers agent (interface)
	/// </summary>
	class Agent
	{
	public:
		/// <summary>
		/// Constructor
		/// </summary>
		Agent();

		/// <summary>
		///	Virtual destructor
		/// </summary>
		virtual ~Agent() = default;

		/// <summary>
		/// Returns index of a move from the given collection of available moves
		/// that the agent wants to take given the current state
		/// </summary>
		virtual int make_move(const State& current_state, const std::vector<Move>& moves) = 0;

		/// <summary>
		/// The method is supposed to be called by the "training environment" when the current training episode is over
		/// to notify the agent about the "final" state and the result of entire game (episode)
		/// </summary>
		virtual void game_over(const State& final_state, const GameResult& result) = 0;

		/// <summary>
		/// Returns type identifier of the current instance
		/// </summary>
		[[nodiscard]] virtual AgentTypeId get_type_id() const = 0;

		/// <summary>
		/// Returns "true" if the agent can be trained otherwise returns "false"
		/// If "false" is returned one should avoid calling getter or setter of the "training mode"
		/// property because the later can throw exception (as not applicable)
		/// </summary>
		[[nodiscard]] virtual bool can_train() const = 0;

		/// <summary>
		/// Sets "training_mode" flag for the agent defining whether the agent trains while playing
		/// </summary>
		virtual void set_training_mode(const bool training_mode);

		/// <summary>
		/// Returns actual value of training mode
		/// </summary>
		[[nodiscard]] virtual bool get_training_mode() const;

		/// <summary>
		/// Returns true if the current agent is equal to the given one
		/// </summary>
		virtual bool equal(const Agent& agent) const = 0;

		std::string _id;
		std::string _name;

	public:

		MSGPACK_DEFINE(_name, _id);

		/// <summary>
		/// Getter for a string name of the agent
		/// </summary>
		[[nodiscard]] const std::string& get_name() const;

		/// <summary>
		/// Getter for a string identifier of the agent
		/// </summary>
		void set_name(const std::string& name);

		/// <summary>
		/// Read-only access to the unique identifier of the object
		/// </summary>
		[[nodiscard]] const std::string& get_id() const;
	};
}

MSGPACK_ADD_ENUM(TrainingCell::Checkers::AgentTypeId)

