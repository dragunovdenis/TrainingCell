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
#include "TdLambdaAgent.h"

namespace TrainingCell::Checkers
{
	/// <summary>
	/// An ensemble of TdLambdaAgent-s
	/// Can't be trained, i.e., works in the "inferring" mode only (at least, in the current implementation) 
	/// </summary>
	class TdlEnsembleAgent : public Agent
	{
		/// <summary>
		/// The ensemble
		/// </summary>
		std::vector<TdLambdaAgent> _ensemble{};

		/// <summary>
		/// ID of a single agent to infer from (if non-negative and less than
		/// the number of elements in the underlying collection of agents)
		/// </summary>
		int _chosen_agent_id = -1;

		/// <summary>
		/// Returns "true" if we are in a mode when only one, "chosen", agent from the collection
		/// is used to infer moves
		/// </summary>
		[[nodiscard]] bool is_single_agent_mode() const;

	public:
		MSGPACK_DEFINE(MSGPACK_BASE(Agent), _ensemble, _chosen_agent_id);

		/// <summary>
		/// Default constructor
		/// </summary>
		TdlEnsembleAgent() = default;

		/// <summary>
		/// Constructor
		/// </summary>
		TdlEnsembleAgent(const std::vector<TdLambdaAgent>& ensemble, const std::string& name = "Ensemble");

		/// <summary>
		/// Adds a copy of the given agent to the ensemble
		/// Returns index of the newly added agent in the underlying collection
		/// </summary>
		std::size_t add(const TdLambdaAgent& agent);

		/// <summary>
		/// If the argument is "true", the agent enters a "special mode" when it behaves according to decisions of
		/// a single agent from the underlying collection which is chosen randomly (when the method is called).
		/// As long as the agent stays in the "special mode" a new "random" agent will be chosen at the end of each episode.
		/// Otherwise, if the argument is "false", the ensemble will make decisions
		/// based on the "votes" of all the agents in the underlying collection.
		///	Returns index of the chosen single random agent of "-1" depending on the value of the input parameter
		/// </summary>
		std::size_t set_single_agent_mode(const bool use_single_random_agent);

		/// <summary>
		/// returns index of the current random agent
		/// </summary>
		[[nodiscard]] std::size_t get_current_random_agent_id() const;

		/// <summary>
		/// Removes agent with the given index from the ensemble
		/// </summary>
		bool remove_agent(const int id);

		/// <summary>
		///	Read-only access to the agents in the ensemble
		/// </summary>
		const Agent& operator [](const int id) const;

		/// <summary>
		/// Returns index of a move from the given collection of available moves
		/// that the agent wants to take given the current state
		/// </summary>
		int make_move(const State& current_state, const std::vector<Move>& moves, const bool as_white) override;

		/// <summary>
		/// The method is supposed to be called by the "training environment" when the current training episode is over
		/// to notify the agent about the "final" state and the result of entire game (episode)
		/// </summary>
		void game_over(const State& final_state, const GameResult& result, const bool as_white) override;

		/// <summary>
		/// ID of the class
		/// </summary>
		static AgentTypeId TYPE_ID();

		/// <summary>
		/// Returns type identifier of the current instance
		/// </summary>
		[[nodiscard]] AgentTypeId get_type_id() const override;

		/// <summary>
		/// Serializes the current instance of the agent into the "message-pack" format and saves it
		/// to the given file. Trows exception if fails.
		/// </summary>
		void save_to_file(const std::filesystem::path& file_path) const;

		/// <summary>
		/// Instantiates agent from the given "message-pack" file.
		/// Throws exception if fails.
		/// </summary>
		static TdlEnsembleAgent load_from_file(const std::filesystem::path& file_path);

		/// <summary>
		/// Equality operator
		/// </summary>
		bool operator == (const TdlEnsembleAgent& anotherAgent) const;

		/// <summary>
		/// Inequality operator
		/// </summary>
		bool operator != (const TdlEnsembleAgent& anotherAgent) const;

		/// <summary>
		/// Returns number of agents in the ensemble
		/// </summary>
		[[nodiscard]] std::size_t size() const;

		/// <summary>
		/// Returns "true" if the agent can train otherwise returns "false"
		/// If "false" is returned one should avoid calling getter or setter of the "training mode"
		/// property because the later can throw exception (as not applicable)
		/// </summary>
		[[nodiscard]] bool can_train() const override;

		/// <summary>
		/// Returns true if the current agent is equal to the given one
		/// </summary>
		[[nodiscard]] bool equal(const Agent& agent) const override;
	};
}
