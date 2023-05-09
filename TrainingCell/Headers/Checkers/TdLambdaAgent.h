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
#include "TdlAbstractAgent.h"
#include "TdLambdaSubAgent.h"

namespace TrainingCell::Checkers
{
	class TdlLegacyMsgPackAdapter;

	/// <summary>
	/// Enumerates auto training modes
	/// </summary>
	enum class AutoTrainingSubMode : int
	{
		//No training
		NONE = 0,
		//Training only while playing white pieces
		WHITE_ONLY = 1,
		//Training only when playing black pieces
		BLACK_ONLY = 1 << 1,
		//Training when playing either black or white pieces (possibly simultaneously)
		FULL = WHITE_ONLY | BLACK_ONLY,
	};

	/// <summary>
	/// Self-training capable TD(lambda) agent
	/// </summary>
	class TdLambdaAgent : public TdlAbstractAgent
	{
		/// <summary>
		/// Field to track version of the container and facilitate backward compatibility if needed
		/// </summary>
		int _msg_pack_version = 1;

		/// <summary>
		/// The "white" sub-agent
		/// </summary>
		TdLambdaSubAgent _white_sub_agent{};

		/// <summary>
		/// The "black" sub-agent
		/// </summary>
		TdLambdaSubAgent _black_sub_agent{};

	public:

		MSGPACK_DEFINE(_msg_pack_version, MSGPACK_BASE(TdlAbstractAgent))

		/// <summary>
		/// Construction from a legacy adapter container
		/// </summary>
		TdLambdaAgent(const TdlLegacyMsgPackAdapter& legacyContainer);

		/// <summary>
		/// Default constructor
		/// </summary>
		TdLambdaAgent() = default;

		/// <summary>
		///	Construction from script
		/// </summary>
		TdLambdaAgent(const std::string& script_str);

		/// <summary>Constructor</summary>
		/// <param name="layer_dimensions">Dimensions of the layers of the neural net that will serve as "afterstate value function".
		/// Must start with "32" and end with "1"</param>
		/// <param name="exploration_epsilon">Probability of taking an exploratory move during the training process, (0,1)</param>
		/// <param name="lambda">Lambda parameters used to determine "strength" of eligibility traces, (0,1)</param>
		/// <param name="gamma">Discount reward determining how reward decay with each next move, (0,1) </param>
		/// <param name="alpha">Learning rate</param>
		/// <param name="name">Name of the agent</param>
		TdLambdaAgent(const std::vector<std::size_t>& layer_dimensions, const double exploration_epsilon,
			const double lambda, const double gamma, const double alpha, const std::string& name = "AutoAgent");

		/// <summary>
		/// Assigns hyper-parameters of the agent from the given script
		/// </summary>
		void assign_hyperparams(const std::string& script_str);

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
		/// Returns ID of the chosen move, no training
		/// </summary>
		[[nodiscard]] int pick_move_id(const State& current_state, const std::vector<Move>& moves, const bool as_white) const;

		/// <summary>
		/// Returns type ID
		/// </summary>
		static AgentTypeId TYPE_ID();

		/// <summary>
		/// Returns type identifier of the current instance
		/// </summary>
		[[nodiscard]] AgentTypeId get_type_id() const override;

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

		/// <summary>
		/// Equality operator
		/// </summary>
		bool operator == (const TdLambdaAgent& another_agent) const;

		/// <summary>
		/// Inequality operator
		/// </summary>
		bool operator != (const TdLambdaAgent& another_agent) const;

		/// <summary>
		/// Serializes the current instance of the agent into the "message-pack" format and saves it
		/// to the given file. Trows exception if fails.
		/// </summary>
		void save_to_file(const std::filesystem::path& file_path) const;

		/// <summary>
		/// Instantiates agent from the given "message-pack" file.
		/// Throws exception if fails.
		/// </summary>
		static TdLambdaAgent load_from_file(const std::filesystem::path& file_path);
	};

}
