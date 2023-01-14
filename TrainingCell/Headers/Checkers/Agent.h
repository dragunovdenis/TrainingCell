#pragma once
#include "Utils.h"
#include "../../../DeepLearning/DeepLearning/NeuralNet/Net.h"

namespace TrainingCell::Checkers
{
	/// <summary>
	///	Abstract checkers agent (interface)
	/// </summary>
	class Agent
	{
	public:
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
		///	The method is supposed to be called by the "training environment" when the current training episode is over
		///	to notify the agent about the "final" state and the result of entire game (episode)
		/// </summary>
		virtual void game_over(const State& final_state, const GameResult& result) = 0;
	};

	/// <summary>
	///	An agent that always take random moves from the collection of possible ones
	/// </summary>
	class RandomAgent : public Agent
	{
	public:
		/// <summary>
		/// Returns index of a move from the given collection of available moves
		/// that the agent wants to take given the current state
		/// </summary>
		int make_move(const State& current_state, const std::vector<Move>& moves) override;

		/// <summary>
		///	The method is supposed to be called by the "training environment" when the current training episode is over
		///	to notify the agent about the "final" state and the result of entire game (episode)
		/// </summary>
		void game_over(const State& final_state, const GameResult& result) override;
	};

	/// <summary>
	///	Agent implementing TD("lambda") ("eligibility traces"-based) learning strategy
	/// </summary>
	class TdLambdaAgent : public Agent
	{
		DeepLearning::Net<DeepLearning::CpuDC> _net{};

		/// <summary>
		/// A flag indicating that we are about to start new game
		/// </summary>
		bool _new_game{};

		std::vector<DeepLearning::LayerGradient<DeepLearning::CpuDC>> _z{};

		/// <summary>
		///	Copy of the previous state, that is either the initial state of the game or a result of latest opponent move
		/// </summary>
		State _prev_state{};

		/// <summary>
		///	Previous state modified by the latest move taken by the agent
		/// </summary>
		State _prev_state_with_move{};

		/// <summary>
		///	Parameters defining the epsilon-greediness of the agent (exploration capabilities of the agent,
		///	i.e. probability that the agent takes a random action instead of the one with the highest predicted value)
		///	<= 0 means no exploration, >= 1.0 means only random exploration
		/// </summary>
		double _exploration_epsilon{};

		/// <summary>
		///	The "Lambda" parameter of TD("lambda") approach
		/// </summary>
		double _lambda = 0.0;

		/// <summary>
		///	Reward discount parameter
		/// </summary>
		double _gamma = 0.8;

		/// <summary>
		///	Learning rate
		/// </summary>
		double _alpha = 0.01;

		/// <summary>
		/// Defines whether the agent is going to train while playing
		/// </summary>
		bool _training_mode = true;

		/// <summary>
		/// Returns id of a move to take
		/// </summary>
		[[nodiscard]] int pick_move_id(const State state, const std::vector<Move>& moves) const;

		/// <summary>
		/// Updates "z" field
		/// </summary>
		void update_z();
	public:
		/// <summary>Constructor</summary>
		/// <param name="layer_dimensions">Dimensions of the fully connected layers that constitute the underlying neural network</param>
		/// <param name="exploration_epsilon">Parameter defining the probability of the agent taking a
		/// <param name="alpha">Gradient step</param>
		/// <param name="gamma">Reward discount parameter</param>
		/// <param name="lambda">The "lambda parameter of TD(lambda) approach"</param>
		/// random action instead of the one having highest predicted value </param>
		TdLambdaAgent(const std::vector<std::size_t>& layer_dimensions, const double exploration_epsilon,
			const double lambda, const double gamma, const double alpha);

		/// <summary>
		/// Returns index of a move from the given collection of available moves
		/// that the agent wants to take given the current state
		/// </summary>
		int make_move(const State& current_state, const std::vector<Move>& moves) override;

		/// <summary>
		/// The method is supposed to be called by the "training environment" when the current training episode is over
		/// to notify the agent about the "final" state and the result of entire game (episode)
		/// </summary>
		void game_over(const State& final_state, const GameResult& result) override;

		/// <summary>
		/// Sets probability of making random moves
		/// </summary>
		void set_exploration_probability(double epsilon);

		/// <summary>
		/// Sets "training_mode" flag for the agent defining whether the agent trains while playing
		/// </summary>
		void set_training_mode(const bool training_mode);
	};
}
