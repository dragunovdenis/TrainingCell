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
#include "Agent.h"
#include "MoveData.h"
#include "NetWithConverter.h"
#include "TdlSettings.h"
#include "../../DeepLearning/DeepLearning/NeuralNet/Net.h"
#include "TdLambdaSubAgent.h"

namespace TrainingCellTest
{
	class TdlambdaAgentRegressionTest;
}

namespace TrainingCell
{
	/// <summary>
	/// Enumerates auto training modes
	/// </summary>
	enum class AutoTrainingSubMode : int
	{
		//No training
		NONE = 0,
		//Training only when playing white pieces
		WHITE_ONLY = 1,
		//Training only when playing black pieces
		BLACK_ONLY = 1 << 1,
		//Training when playing either black or white pieces (possibly simultaneously)
		FULL = WHITE_ONLY | BLACK_ONLY,
	};

	/// <summary>
	/// Enumerates tree search methods for TD-lambda agent
	/// </summary>
	enum class TreeSearchMethod : int
	{
		NONE = 0, //no search
		TD_SEARCH = 1, //Temporal difference search
	};

	/// <summary>
	/// General interface of TD(lambda) agent
	/// </summary>
	class TdlAbstractAgent : public Agent, ITdlSettingsReadOnly, protected NetWithConverterAbstract
	{
		friend class TrainingCellTest::TdlambdaAgentRegressionTest; // for diagnostics purposes
		StateConverter _converter{ StateConversionType::CheckersStandard }; // default value to preserve backward compatibility
		StateTypeId _state_type_id{ StateTypeId::CHECKERS }; // default value to preserve backward compatibility

		bool _performance_evaluation_mode{false};

		/// <summary>
		/// Throws an exception if parameters of the agent are invalid/incompatible.
		/// </summary>
		void validate() const;

		/// <summary>
		/// Augments given collection of "hidden" layer dimensions
		/// with input and output layers compatible with the current state type ID and state converter.
		/// </summary>
		std::vector<std::size_t> augment_hidden_layer_dimensions(const std::vector<std::size_t>& hidden_layer_dimensions) const;

		/// <summary>
		/// Updates the corresponding field and state converter.
		/// </summary>
		void set_state_type_id(const StateTypeId state_type_id);

	protected:

		/// <summary>
		/// See the summary in the base class.
		/// </summary>
		DeepLearning::Net<DeepLearning::CpuDC>& net() override;

		/// <summary>
		/// See the summary in the base class.
		/// </summary>
		const DeepLearning::Net<DeepLearning::CpuDC>& net() const override;

		/// <summary>
		/// See the summary in the base class.
		/// </summary>
		const StateConverter& converter() const override;

		/// <summary>
		///	The neural net to approximate state value function
		/// </summary>
		DeepLearning::Net<DeepLearning::CpuDC> _net{};

		/// <summary>
		/// Parameters defining the epsilon-greediness of the agent (exploration capabilities of the agent,
		/// i.e. probability that the agent takes a random action instead of the one with the highest predicted value)
		/// <= 0 means no exploration, >= 1.0 means only random exploration
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
		AutoTrainingSubMode _training_sub_mode{ AutoTrainingSubMode::FULL };

		/// <summary>
		/// Readonly access to training sub-mode.
		/// </summary>
		AutoTrainingSubMode training_sub_mode() const;

		/// <summary>
		/// Factor that is applied to the result of internal reward function during training
		///	If set to "0", only final reward (win/loose) will be taken into account 
		/// </summary>
		double _reward_factor{1};

		/// <summary>
		/// Tree search method
		/// </summary>
		TreeSearchMethod _search_method{ TreeSearchMethod::NONE };

		/// <summary>
		/// Number of TD-tree search iterations. Ignored if TD-tree search is not used
		/// </summary>
		int _td_search_iterations{ 1000 };

		/// <summary>
		/// Number of first moves in each episode during which the "search" neural net should be updated
		/// (provided that "training mode" is on, otherwise the parameter is ignored)
		/// </summary>
		int _td_search_depth{ 1000 };

		/// <summary>
		/// Exploration depth used in search mode.
		/// </summary>
		int _search_exploration_depth{ 10000 };

		/// <summary>
		/// Exploration volume used in search mode.
		/// </summary>
		int _search_exploration_volume{ 10000 };

		/// <summary>
		/// Exploration probability used in search mode.
		/// </summary>
		double _search_exploration_probability{ 0.05 };

		/// <summary>
		/// Initializes neural net according to the given dimension array
		/// </summary>
		void initialize_net(const std::vector<std::size_t>& layer_dimensions);

		/// <summary>
		/// Assigns parameters of the agent from the given script-string
		/// </summary>
		void assign(const std::string& script_str, const bool hyper_params_only);

		/// <summary>
		/// Convertor from training mode to training sub-mode
		/// </summary>
		static AutoTrainingSubMode training_mode_to_sub_mode(const bool training_mode);

		/// <summary>
		/// Array of sub-agents (the first one "plays" black pieces and the second one "plays" white pieces)
		/// </summary>
		std::vector<TdLambdaSubAgent> _sub_agents{ TdLambdaSubAgent{false} , TdLambdaSubAgent{true} };

		/// <summary>
		/// Can contain a search net if the TD-tree search mode is engaged 
		/// </summary>
		mutable std::optional<NetWithConverter> _search_net{};

		/// <summary>
		/// Returns settings that will be used in TD-tree search process
		/// </summary>
		TdlSettings get_search_settings() const;

		/// <summary>
		/// Runs TD-tree search and returns the "found" move (together with auxiliary data)
		/// </summary>
		MoveData run_search(const IStateReadOnly& state) const;

	public:
		MSGPACK_DEFINE(MSGPACK_BASE(Agent), _net, _exploration_epsilon,
			_training_sub_mode, _lambda, _gamma, _alpha, _reward_factor,
			_search_method, _td_search_iterations, _td_search_depth, _converter,
			_state_type_id, _performance_evaluation_mode, _search_exploration_depth,
			_search_exploration_probability, _search_exploration_volume)

		/// <summary>
		/// Returns script representation of all the hyper-parameters of the agent
		/// </summary>
		[[nodiscard]] std::string to_script() const;

		/// <summary>
		/// Returns "true" if the given agent has the same hyper-parameters as the current one
		/// </summary>
		[[nodiscard]] bool equal_hyperparams(const TdlAbstractAgent& anotherAgent) const;

		/// <summary>
		/// Returns true if the current agent is equal to the given one
		/// </summary>
		[[nodiscard]] bool equal(const Agent& agent) const override;

		/// <summary>
		/// Default constructor
		/// </summary>
		TdlAbstractAgent() = default;

		/// <summary>Constructor</summary>
		/// <param name="hidden_layer_dimensions">Dimensions of the hidden(!) layers of the neural net that will serve as "afterstate value function";
		/// They will be automatically augmented with "input" layer compatible with the suggested state type and the "output" layer of dimension "1".</param>
		/// <param name="exploration_epsilon">Probability of taking an exploratory move during the training process, (0,1)</param>
		/// <param name="lambda">Lambda parameters used to determine "strength" of eligibility traces, (0,1)</param>
		/// <param name="gamma">Discount reward determining how reward decay with each next move, (0,1) </param>
		/// <param name="alpha">Learning rate</param>
		/// <param name="state_type_id">Type ID of the state the agent will be "compatible" with.</param>
		/// <param name="name">Name of the agent</param>
		/// <param name="search_exploration_prob">Probability of exploration moves during the search phase.</param>
		/// <param name="search_exploration_depth">Depth of the exploration (in moves) during the search phase.</param>
		/// <param name="search_exploration_volume">Volume (how many moves with the best score take part in exploration act)
		/// of the exploration during the search phase</param>
		TdlAbstractAgent(const std::vector<std::size_t>& hidden_layer_dimensions, const double exploration_epsilon,
		                 const double lambda, const double gamma, const double alpha, const StateTypeId state_type_id,
						 const double search_exploration_prob, const int search_exploration_depth,
					     const int search_exploration_volume, const std::string& name);

		/// <summary>
		/// Returns index of a move from the given collection of available moves
		/// that the agent wants to take given the current state
		/// </summary>
		int make_move(const IStateReadOnly& state, const bool as_white) override;

		/// <summary>
		/// The method is supposed to be called by the "training environment" when the current training episode is over
		/// to notify the agent about the "final" state and the result of entire game (episode)
		/// </summary>
		void game_over(const IStateReadOnly& final_state, const GameResult& result, const bool as_white) override;

		/// <summary>
		/// Returns ID of the "best score" move, no training, no exploration
		/// </summary>
		[[nodiscard]] int pick_move_id(const IStateReadOnly& state, const bool as_white) const;

		/// <summary>
		/// Assigns hyper-parameters of the agent from the given script
		/// </summary>
		void assign_hyperparams(const std::string& script_str);

		/// <summary>
		/// Sets probability of making random moves
		/// </summary>
		void set_exploration_probability(double epsilon);

		/// <summary>
		/// Returns actual value of exploration probability
		/// </summary>
		[[nodiscard]] double get_exploration_probability() const override;

		/// <summary>
		/// Updates parameter gamma with the given value;
		/// </summary>
		void set_discount(double gamma);

		/// <summary>
		/// Returns actual value of parameter gamma (reward discount)
		/// </summary>
		[[nodiscard]] double get_discount() const override;

		/// <summary>
		/// Updates "lambda" parameter with the given value
		/// </summary>
		void set_lambda(const double lambda);

		/// <summary>
		/// Returns actual value of "lambda" parameter
		/// </summary>
		[[nodiscard]] double get_lambda() const override;

		/// <summary>
		/// Updates learning rate with the given value
		/// </summary>
		void set_learning_rate(const double alpha);

		/// <summary>
		/// Returns actual value of the learning rate parameter ("alpha")
		/// </summary>
		[[nodiscard]] double get_learning_rate() const override;

		/// <summary>
		/// Returns dimensions of the layers of the underlying neural network
		/// </summary>
		[[nodiscard]] std::vector<unsigned int> get_net_dimensions() const;

		/// <summary>
		/// Sets "training_mode" flag for the agent defining whether the agent trains while playing
		/// </summary>
		void set_training_mode(const bool training_mode) override;

		/// <summary>
		/// Returns actual value of training mode
		/// </summary>
		[[nodiscard]] bool get_training_mode() const override;

		/// <summary>
		/// Returns actual value of training sub-mode
		/// </summary>
		void set_training_sub_mode(const AutoTrainingSubMode sub_mode);

		/// <summary>
		/// Returns actual value of training mode depending on the color of the pieces
		/// </summary>
		[[nodiscard]] bool get_training_mode(const bool as_white) const override;

		/// <summary>
		/// Sets the "reward factor" parameter of the agent
		/// </summary>
		void set_reward_factor(const double reward_factor);

		/// <summary>
		/// Returns the current value of "reward factor" parameter
		/// </summary>
		[[nodiscard]] double get_reward_factor() const override;

		/// <summary>
		/// Sets the "tree search method" parameter of the agent
		/// </summary>
		void set_tree_search_method(const TreeSearchMethod search_method);

		/// <summary>
		/// Returns the current value of "tree search method" parameter
		/// </summary>
		[[nodiscard]] TreeSearchMethod get_tree_search_method() const;

		/// <summary>
		/// Sets the "TD search iterations" parameter of the agent
		/// </summary>
		void set_td_search_iterations(const int search_iterations);

		/// <summary>
		/// Returns the current value of "TD search iterations" parameter
		/// </summary>
		[[nodiscard]] int get_td_search_iterations() const;

		/// <summary>
		/// Returns number of first moves in each episode during which the neural net should be updated
		/// (provided that "training mode" is on, otherwise the parameter is ignored)
		/// </summary>
		[[nodiscard]] int get_train_depth() const override;

		/// <summary>
		/// Determines number of moves (actions) counted from the
		/// beginning of an episode through which exploratory decisions can be take.
		/// </summary>
		[[nodiscard]] int get_exploration_depth() const override;

		/// <summary>
		/// Determines number of best options that are going to be taken
		/// into account during the exploration action each it takes place
		/// (i.e. only this number of best options will be "explored").
		/// </summary>
		[[nodiscard]] int get_exploration_volume() const override;

		/// <summary>
		/// Returns number of first moves in each search episode during which the "search" neural net should be updated
		/// </summary>
		[[nodiscard]] int get_search_depth() const;

		/// <summary>
		/// Sets number of first moves in each search episode during which the "search" neural net should be updated
		/// </summary>
		void set_search_depth(const int depth);

		/// <summary>
		/// Returns value of exploration depth used in search mode.
		/// </summary>
		[[nodiscard]] int get_search_exploration_depth() const;

		/// <summary>
		/// Sets value of exploration depth used in search mode.
		/// </summary>
		void set_search_exploration_depth(const int depth);

		/// <summary>
		/// Returns value of exploration volume used in search mode.
		/// </summary>
		[[nodiscard]] int get_search_exploration_volume() const;

		/// <summary>
		/// Sets value of exploration volume used in search mode.
		/// </summary>
		void set_search_exploration_volume(const int volume);

		/// <summary>
		/// Returns value of exploration probability used in search mode.
		/// </summary>
		[[nodiscard]] double get_search_exploration_probability() const;

		/// <summary>
		/// Sets value of exploration probability used in search mode.
		/// </summary>
		void set_search_exploration_probability(const double probability);

		/// <summary>
		/// See documentation of the base class.
		/// </summary>
		[[nodiscard]] StateTypeId get_state_type_id() const override;

		/// <summary>
		/// Setter for the corresponding property.
		/// </summary>
		void set_performance_evaluation_mode(const bool value);

		/// <summary>
		/// Getter for the corresponding property.
		/// </summary>
		[[nodiscard]] bool get_performance_evaluation_mode() const;

		/// <summary>
		/// Resets functionality that ensures randomness of the exploration component of training.
		/// </summary>
		static void reset_explorer(const unsigned int seed = std::random_device{}());

		/// <summary>
		/// Frees auxiliary memory used during training, allowing to decrease memory usage of the agent by about 66%. 
		/// </summary>
		void free_aux_mem();
	};
}

MSGPACK_ADD_ENUM(TrainingCell::AutoTrainingSubMode)
MSGPACK_ADD_ENUM(TrainingCell::TreeSearchMethod)
