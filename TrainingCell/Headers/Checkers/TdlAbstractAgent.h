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
#include "TdlSettings.h"
#include "../../../DeepLearning/DeepLearning/NeuralNet/Net.h"

namespace TrainingCell::Checkers
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
	/// Hold data related to a picked move
	/// </summary>
	struct MoveData
	{
		/// <summary>
		/// ID of the move
		/// </summary>
		int move_id{};

		/// <summary>
		/// Value of the after-state
		/// </summary>
		double value{};

		/// <summary>
		/// After-sate resulted from the move
		/// </summary>
		State after_state{};
	};

	/// <summary>
	/// General interface of TD(lambda) agent
	/// </summary>
	class TdlAbstractAgent : public Agent, public ITdlSettingsReadOnly
	{
	protected:
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

	public:
		MSGPACK_DEFINE(MSGPACK_BASE(Agent), _net, _exploration_epsilon,
			_training_sub_mode, _lambda, _gamma, _alpha, _reward_factor, _search_method, _td_search_iterations)

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
		/// <param name="layer_dimensions">Dimensions of the layers of the neural net that will serve as "afterstate value function";
		/// Must start with "32" and end with "1"</param>
		/// <param name="exploration_epsilon">Probability of taking an exploratory move during the training process, (0,1)</param>
		/// <param name="lambda">Lambda parameters used to determine "strength" of eligibility traces, (0,1)</param>
		/// <param name="gamma">Discount reward determining how reward decay with each next move, (0,1) </param>
		/// <param name="alpha">Learning rate</param>
		/// <param name="name">Name of the agent</param>
		TdlAbstractAgent(const std::vector<std::size_t>& layer_dimensions, const double exploration_epsilon,
		                 const double lambda, const double gamma, const double alpha, const std::string& name);

		/// <summary>
		/// Sets probability of making random moves
		/// </summary>
		void set_exploration_probability(double epsilon);

		/// <summary>
		/// Returns actual value of exploration probability
		/// </summary>
		[[nodiscard]] double get_exploratory_probability() const override;

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
	};
}

MSGPACK_ADD_ENUM(TrainingCell::Checkers::AutoTrainingSubMode)
MSGPACK_ADD_ENUM(TrainingCell::Checkers::TreeSearchMethod)
