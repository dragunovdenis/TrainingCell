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
#include "../../../DeepLearning/DeepLearning/NeuralNet/Net.h"

namespace TrainingCell::Checkers
{
	/// <summary>
	/// Interface for TD(lambda) settings
	/// </summary>
	class TdlSettingsReadOnly
	{
	public:
		virtual ~TdlSettingsReadOnly() = default;

		/// <summary>
		/// Returns actual value of exploration probability
		/// </summary>
		[[nodiscard]] virtual double get_exploratory_probability() const = 0;

		/// <summary>
		/// Returns actual value of parameter gamma (reward discount)
		/// </summary>
		[[nodiscard]] virtual double get_discount() const = 0;

		/// <summary>
		/// Returns actual value of "lambda" parameter
		/// </summary>
		[[nodiscard]] virtual double get_lambda() const = 0;

		/// <summary>
		/// Returns actual value of the learning rate parameter ("alpha")
		/// </summary>
		[[nodiscard]] virtual double get_learning_rate() const = 0;

		/// <summary>
		/// Returns actual value of training mode
		/// </summary>
		[[nodiscard]] virtual bool get_training_mode() const = 0;

		/// <summary>
		/// Returns the current value of "reward factor" parameter
		/// </summary>
		[[nodiscard]] virtual double get_reward_factor() const = 0;
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
	class TdlAbstractAgent : virtual public Agent, virtual public TdlSettingsReadOnly
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
		bool _training_mode = true;

		/// <summary>
		/// Factor that is applied to the result of internal reward function during training
		///	If set to "0", only final reward (win/loose) will be taken into account 
		/// </summary>
		double _reward_factor{1};

		/// <summary>
		/// Initializes neural net according to the given dimension array
		/// </summary>
		void initialize_net(const std::vector<std::size_t>& layer_dimensions);

	public:
		MSGPACK_DEFINE(MSGPACK_BASE(Agent), _net, _exploration_epsilon, _training_mode, _lambda, _gamma, _alpha, _reward_factor)

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
		/// Sets the "reward factor" parameters of the agent
		/// </summary>
		void set_reward_factor(const double reward_factor);

		/// <summary>
		/// Returns the current value of "reward factor" parameter
		/// </summary>
		[[nodiscard]] double get_reward_factor() const override;
	};
}
