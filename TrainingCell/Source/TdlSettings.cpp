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

#include "../Headers/TdlSettings.h"

namespace TrainingCell
{
	/// <summary>
	/// Construct an instance fro the given read-only interface
	/// </summary>
	TdlSettings::TdlSettings(const ITdlSettingsReadOnly& settings)
	{
		_exploration_probability = settings.get_exploration_probability();
		_discount = settings.get_discount();
		_lambda = settings.get_lambda();
		_learning_rate = settings.get_learning_rate();
		_training_mode_white = settings.get_training_mode(true);
		_training_mode_black = settings.get_training_mode(false);
		_reward_factor = settings.get_reward_factor();
		_train_depth = settings.get_train_depth();
		_exploration_depth = settings.get_exploration_depth();
		_exploration_volume = settings.get_exploration_volume();
	}

	double TdlSettings::get_exploration_probability() const
	{
		return _exploration_probability;
	}

	void TdlSettings::set_exploration_probability(const double exploration_probability)
	{
		_exploration_probability = exploration_probability;
	}

	double TdlSettings::get_discount() const
	{
		return _discount;
	}

	void TdlSettings::set_discount(const double discount)
	{
		_discount = discount;
	}

	double TdlSettings::get_lambda() const
	{
		return _lambda;
	}

	void TdlSettings::set_lambda(const double lambda)
	{
		_lambda = lambda;
	}

	double TdlSettings::get_learning_rate() const
	{
		return _learning_rate;
	}

	void TdlSettings::set_learning_rate(const double learning_rate)
	{
		_learning_rate = learning_rate;
	}

	bool TdlSettings::get_training_mode(const bool as_white) const
	{
		return as_white ? _training_mode_white : _training_mode_black;
	}

	void TdlSettings::set_training_mode(const bool training_mode, const bool as_white)
	{
		if (as_white)
			_training_mode_white = training_mode;
		else
			_training_mode_black = training_mode;
	}

	double TdlSettings::get_reward_factor() const
	{
		return _reward_factor;
	}

	void TdlSettings::set_reward_factor(const double reward_factor)
	{
		_reward_factor = reward_factor;
	}

	int TdlSettings::get_train_depth() const
	{
		return _train_depth;
	}

	void TdlSettings::set_train_depth(const int train_depth)
	{
		_train_depth = train_depth;
	}

	int TdlSettings::get_exploration_depth() const
	{
		return _exploration_depth;
	}

	void TdlSettings::set_exploration_depth(const int depth)
	{
		_exploration_depth = depth;
	}

	int TdlSettings::get_exploration_volume() const
	{
		return _exploration_volume;
	}

	void TdlSettings::set_exploration_volume(const int volume)
	{
		_exploration_volume = volume;
	}

	bool TdlSettings::operator==(const TdlSettings& otherSettings) const
	{
		return _exploration_probability == otherSettings._exploration_probability &&
			   _discount == otherSettings._discount &&
			   _lambda == otherSettings._lambda &&
			   _learning_rate == otherSettings._learning_rate &&
			   _training_mode_white == otherSettings._training_mode_white &&
			   _training_mode_black == otherSettings._training_mode_black &&
			   _reward_factor == otherSettings._reward_factor &&
			   _train_depth == otherSettings._train_depth &&
			   _exploration_depth == otherSettings._exploration_depth &&
			   _exploration_volume == otherSettings._exploration_volume;
	}
}
