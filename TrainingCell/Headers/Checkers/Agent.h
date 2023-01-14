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
		/// </summary>
		double _epsilon{};

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
		///	Returns id of a move to take
		/// </summary>
		[[nodiscard]] int pick_move_id(const State state, const std::vector<Move>& moves) const;

		/// <summary>
		/// Updates "z" field
		/// </summary>
		void update_z();
	public:
		/// <summary>Constructor</summary>
		/// <param name="layer_dimensions">Dimensions of the fully connected layers that constitute the underlying neural network</param>
		/// <param name="epsilon">Parameter defining the probability of the agent taking a
		/// random action instead of the one having highest predicted value </param>
		TdLambdaAgent(const std::vector<std::size_t>& layer_dimensions, const double epsilon);

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
}
