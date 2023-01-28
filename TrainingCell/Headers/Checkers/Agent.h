#pragma once
#include "Utils.h"
#include "../../../DeepLearning/DeepLearning/NeuralNet/Net.h"
#include <msgpack.hpp>
#include <filesystem>

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
		/// The method is supposed to be called by the "training environment" when the current training episode is over
		/// to notify the agent about the "final" state and the result of entire game (episode)
		/// </summary>
		void game_over(const State& final_state, const GameResult& result) override;

		/// <summary>
		/// Returns type ID
		/// </summary>
		static AgentTypeId ID();

		/// <summary>
		/// Returns type identifier of the current instance
		/// </summary>
		[[nodiscard]] AgentTypeId get_type_id() const override;

		/// <summary>
		/// Returns "true" if the agent can be trained otherwise returns "false"
		/// If "false" is returned one should avoid calling getter or setter of the "training mode"
		/// property because the later can throw exception (as not applicable)
		/// </summary>
		[[nodiscard]] bool can_train() const override;

		/// <summary>
		/// Returns true if the current agent is equal to the given one
		/// </summary>
		bool equal(const Agent& agent) const override;
	};

	using MakeMoveCallback = std::function<int(const State&, const std::vector<Move>&)>;
	using GameOverCallback = std::function<void(const State&, const GameResult&)>;

	/// <summary>
	///	Interface for a human player
	/// </summary>
	class InteractiveAgent : public Agent
	{
		const MakeMoveCallback _make_move_callback{};
		const GameOverCallback _game_over_callback{};
		const bool _play_for_whites{};
	public:
		/// <summary>
		/// Constructor
		/// </summary>
		/// <param name="make_move_callback">Pointer to a method that
		/// will be called when "make_move" method of the agent is called</param>
		/// <param name="game_over_callback">Pointer to a method that
		/// will be called when "game_over" method of the agent is called</param>
		/// <param name="play_for_whites">Determines whether state and move parameters of the callback methods
		/// should be inverted (if "false") or not (if "true")</param>
		InteractiveAgent(const MakeMoveCallback& make_move_callback, const GameOverCallback& game_over_callback,
		                 const bool play_for_whites);

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
		/// Returns type ID
		/// </summary>
		static AgentTypeId ID();

		/// <summary>
		/// Returns type identifier of the current instance
		/// </summary>
		[[nodiscard]] AgentTypeId get_type_id() const override;

		/// <summary>
		/// Returns "true" if the agent can be trained otherwise returns "false"
		/// If "false" is returned one should avoid calling getter or setter of the "training mode"
		/// property because the later can throw exception (as not applicable)
		/// </summary>
		[[nodiscard]] bool can_train() const override;

		/// <summary>
		/// Returns true if the current agent is equal to the given one
		/// </summary>
		bool equal(const Agent& agent) const override;
	};

	/// <summary>
	///	Agent implementing TD("lambda") ("eligibility traces"-based) learning strategy
	/// </summary>
	class TdLambdaAgent : public Agent
	{
		friend class TdlEnsembleAgent;
		/// <summary>
		///	The neural net to approximate state value function
		/// </summary>
		DeepLearning::Net<DeepLearning::CpuDC> _net{};

		/// <summary>
		/// A flag indicating that we are about to start new game
		/// </summary>
		bool _new_game{};

		/// <summary>
		///	Auxiliary data structure that is used during training process
		/// </summary>
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
		[[nodiscard]] int pick_move_id(const State& state, const std::vector<Move>& moves) const;

		/// <summary>
		/// Updates "z" field
		/// </summary>
		void update_z();

		/// <summary>
		///	Resets training state of the object which is an obligatory procedure to start new episode
		/// </summary>
		void reset();
	public:

		MSGPACK_DEFINE(_net, _z, _prev_state, _prev_state_with_move, _new_game,
			_exploration_epsilon, _training_mode, _lambda, _gamma, _alpha);

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
		/// Default constructor
		/// </summary>
		TdLambdaAgent() = default;

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
		/// Returns actual value of exploration probability
		/// </summary>
		[[nodiscard]] double get_exploratory_probability() const;

		/// <summary>
		/// Updates parameter gamma with the given value;
		/// </summary>
		void set_discount(double gamma);

		/// <summary>
		/// Returns actual value of parameter gamma (reward discount)
		/// </summary>
		[[nodiscard]] double get_discount() const;

		/// <summary>
		/// Sets "training_mode" flag for the agent defining whether the agent trains while playing
		/// </summary>
		void set_training_mode(const bool training_mode) override;

		/// <summary>
		/// Returns actual value of training mode
		/// </summary>
		[[nodiscard]] bool get_training_mode() const override;

		/// <summary>
		/// Updates "lambda" parameter with the given value
		/// </summary>
		void set_lambda(const double lambda);

		/// <summary>
		/// Returns actual value of "lambda" parameter
		/// </summary>
		[[nodiscard]] double get_lambda() const;

		/// <summary>
		/// Updates learning rate with the given value
		/// </summary>
		void set_learning_rate(const double alpha);

		/// <summary>
		/// Returns acutal value of the learning rate parameter ("alpha")
		/// </summary>
		[[nodiscard]] double get_learning_rate() const;

		/// <summary>
		/// Equality operator
		/// </summary>
		bool operator == (const TdLambdaAgent & anotherAgent) const;

		/// <summary>
		/// Inequality operator
		/// </summary>
		bool operator != (const TdLambdaAgent& anotherAgent) const;

		/// <summary>
		/// Serializes the current instance of the agent into the "message-pack" format and saves it
		/// to the given file. Trows exception if fails.
		/// </summary>
		void save_to_file(const std::filesystem::path & file_path) const;

		/// <summary>
		/// Instantiates agent from the given "message-pack" file.
		/// Throws exception if fails.
		/// </summary>
		static TdLambdaAgent load_from_file(const std::filesystem::path & file_path);

		/// <summary>
		/// Returns type ID
		/// </summary>
		static AgentTypeId ID();

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
	public:
		MSGPACK_DEFINE(_ensemble);

		/// <summary>
		/// Default constructor
		/// </summary>
		TdlEnsembleAgent() = default;

		/// <summary>
		/// Constructor
		/// </summary>
		TdlEnsembleAgent(const std::vector<TdLambdaAgent>& ensemble);

		/// <summary>
		/// Adds a copy of the given agent to the ensemble
		/// </summary>
		void Add(const TdLambdaAgent& agent);

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
		/// ID of the class
		/// </summary>
		static AgentTypeId ID();

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

MSGPACK_ADD_ENUM(TrainingCell::Checkers::AgentTypeId)

