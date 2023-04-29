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

namespace TrainingCell::Checkers
{
	/// <summary>
	///	Agent implementing TD("lambda") ("eligibility traces"-based) learning strategy
	/// </summary>
	class TdLambdaAgent : public TdlAbstractAgent
	{
		friend class TdlEnsembleAgent;

		/// <summary>
		/// A flag indicating that we are about to start new game
		/// </summary>
		bool _new_game{};

		/// <summary>
		///	Auxiliary data structure that is used during training process
		/// </summary>
		std::vector<DeepLearning::LayerGradient<DeepLearning::CpuDC>> _z{};

		/// <summary>
		/// Previous state
		/// </summary>
		State _prev_state{};

		/// <summary>
		/// Previous afterstate
		/// </summary>
		State _prev_afterstate{};

		/// <summary>
		/// Returns id of a move to take
		/// </summary>
		[[nodiscard]] int pick_move_id(const State& state, const std::vector<Move>& moves) const;

		/// <summary>
		/// Returns index of the picked move and the related data
		/// </summary>
		[[nodiscard]] MoveData pick_move(const State& state, const std::vector<Move>& moves) const;

		/// <summary>
		/// Calculates afterstate and its value
		/// </summary>
		[[nodiscard]] MoveData evaluate(const State& state, const std::vector<Move>& moves, const int move_id) const;

		/// <summary>
		/// Updates "z" field and returns value of the afterstate value function at the "previous afterstate"
		/// </summary>
		double update_z_and_evaluate_prev_after_state();

		/// <summary>
		///	Resets training state of the object which is an obligatory procedure to start new episode
		/// </summary>
		void reset();

		/// <summary>
		/// Assigns parameters of the agent from the given script-string
		/// </summary>
		void assign(const std::string& script_str, const bool hyper_params_only);
	public:

		MSGPACK_DEFINE(MSGPACK_BASE(Agent), _net, _z, _prev_state, _prev_afterstate, _new_game,
			_exploration_epsilon, _training_mode, _lambda, _gamma, _alpha, _reward_factor)

		/// <summary>
		/// Returns script representation of all the hyper-parameters of the agent
		/// </summary>
		[[nodiscard]] std::string to_script() const;

		/// <summary>
		/// Construction from the given string-script
		/// </summary>
		TdLambdaAgent(const std::string& script_str);

		/// <summary>Constructor</summary>
		/// <param name="layer_dimensions">Dimensions of the fully connected layers that constitute the underlying neural network</param>
		/// <param name="exploration_epsilon">Parameter defining the probability of the agent taking a
		/// random action instead of the one having highest predicted value </param>
		/// <param name="alpha">Gradient step</param>
		/// <param name="gamma">Reward discount parameter</param>
		/// <param name="lambda">The "lambda parameter of TD(lambda) approach"</param>
		/// <param name="name">Name of the agent</param>
		TdLambdaAgent(const std::vector<std::size_t>& layer_dimensions, const double exploration_epsilon,
			const double lambda, const double gamma, const double alpha, const std::string& name = "TdlAgent");

		/// <summary>
		/// Assigns hyper-parameters of the agent from the given script-string (keeps net untouches)
		/// </summary>
		void assign_hyperparams(const std::string& script_str);

		/// <summary>
		/// Default constructor
		/// </summary>
		TdLambdaAgent() = default;

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
		/// Equality operator
		/// </summary>
		bool operator == (const TdLambdaAgent& anotherAgent) const;

		/// <summary>
		/// Inequality operator
		/// </summary>
		bool operator != (const TdLambdaAgent& anotherAgent) const;

		/// <summary>
		/// Returns "true" if the given agent has the same hyper-parameters as the current one
		/// </summary>
		bool equal_hyperparams(const TdLambdaAgent & anotherAgent) const;

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
	};
}
