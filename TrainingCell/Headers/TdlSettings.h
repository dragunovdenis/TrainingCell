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

namespace TrainingCell
{
	/// <summary>
	/// Interface for TD(lambda) settings
	/// </summary>
	class ITdlSettingsReadOnly
	{
	public:
		/// <summary>
		/// Virtual default destructor
		/// </summary>
		virtual ~ITdlSettingsReadOnly() = default;

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
		/// Returns actual value of training mode depending on the color of the pieces
		/// </summary>
		[[nodiscard]] virtual bool get_training_mode(const bool as_white) const = 0;

		/// <summary>
		/// Returns the current value of "reward factor" parameter
		/// </summary>
		[[nodiscard]] virtual double get_reward_factor() const = 0;

		/// <summary>
		/// Returns number of first moves in each episode during which the neural net should be updated
		/// (provided that "training mode" is on, otherwise the parameter is ignored)
		/// </summary>
		[[nodiscard]] virtual int get_train_depth() const = 0;
	};

	/// <summary>
	/// Implementation of the corresponding interface
	/// </summary>
	class TdlSettings : public ITdlSettingsReadOnly
	{
		double _exploratory_probability{};
		double _discount{};
		double _lambda{};
		double _learning_rate{};
		bool _training_mode_white{};
		bool _training_mode_black{};
		double _reward_factor{};
		int _train_depth{};

	public:
		/// <summary>
		/// Delete the default constructor as it seems to be redundant (at least for now)
		/// </summary>
		TdlSettings() = delete;

		/// <summary>
		/// Construct an instance fro the given read-only interface
		/// </summary>
		TdlSettings(const ITdlSettingsReadOnly& settings);

		/// <summary>
		/// Returns actual value of exploration probability
		/// </summary>
		[[nodiscard]] double get_exploratory_probability() const override;

		/// <summary>
		/// Setter for the corresponding property
		/// </summary>
		void set_exploratory_probability(const double exploratory_probability);

		/// <summary>
		/// Returns actual value of parameter gamma (reward discount)
		/// </summary>
		[[nodiscard]] double get_discount() const override;

		/// <summary>
		/// Setter for the corresponding property
		/// </summary>
		void set_discount(const double discount);

		/// <summary>
		/// Returns actual value of "lambda" parameter
		/// </summary>
		[[nodiscard]] double get_lambda() const override;

		/// <summary>
		/// Setter for the corresponding property
		/// </summary>
		void set_lambda(const double lambda);

		/// <summary>
		/// Returns actual value of the learning rate parameter ("alpha")
		/// </summary>
		[[nodiscard]] double get_learning_rate() const override;

		/// <summary>
		/// Setter for the corresponding property
		/// </summary>
		void set_learning_rate(const double learning_rate);

		/// <summary>
		/// Returns actual value of training mode depending on the color of the pieces
		/// </summary>
		[[nodiscard]] bool get_training_mode(const bool as_white) const override;

		/// <summary>
		/// Setter for the corresponding property
		/// </summary>
		void set_training_mode(const bool training_mode, const bool as_white);

		/// <summary>
		/// Returns the current value of "reward factor" parameter
		/// </summary>
		[[nodiscard]] double get_reward_factor() const override;

		/// <summary>
		/// Setter for the corresponding property
		/// </summary>
		void set_reward_factor(const double reward_factor);

		/// <summary>
		/// Returns number of first moves in each episode during which the neural net should be updated
		/// (provided that "training mode" is on, otherwise the parameter is ignored)
		/// </summary>
		[[nodiscard]] int get_train_depth() const override;

		/// <summary>
		/// Setter for the corresponding property
		/// </summary>
		void set_train_depth(const int train_depth);

		/// <summary>
		/// Equality operator.
		/// </summary>
		bool operator ==(const TdlSettings& otherSettings) const;
	};
}