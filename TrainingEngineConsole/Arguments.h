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

#include <filesystem>

/// <summary>
/// Parsed command line arguments
/// </summary>
class Arguments
{
	/// <summary>
	/// Number of agent pairs to train
	/// </summary>
	unsigned int _num_pairs;

	/// <summary>
	/// Number of training rounds to do
	/// </summary>
	unsigned int _num_rounds;

	/// <summary>
	/// Number of episodes in each training round
	/// </summary>
	unsigned int _num_episodes;

	/// <summary>
	/// Path to the folder to save output
	/// </summary>
	std::filesystem::path _output_folder;

	/// <summary>
	/// Reward discount [0, 1]
	/// </summary>
	double _discount;

	/// <summary>
	/// Lambda [0, 1]
	/// </summary>
	double _lambda;

	/// <summary>
	/// Exploration probability
	/// </summary>
	double _exploration_probability;

	/// <summary>
	/// Learning rate
	/// </summary>
	double _learning_rate;

	/// <summary>
	/// Dimensions of the neural net layers
	/// </summary>
	std::vector<std::size_t> _net_dimensions;

	/// <summary>
	/// Path to the "opponent" ensemble (optional)
	/// </summary>
	std::filesystem::path _opponent_ensemble_path;

public:
	/// <summary>
	/// Read-only access to the corresponding field
	/// </summary>
	[[nodiscard]] unsigned int get_num_pairs() const;

	/// <summary>
	/// Read-only access to the corresponding field
	/// </summary>
	[[nodiscard]] unsigned int get_num_rounds() const;

	/// <summary>
	/// Read-only access to the corresponding field
	/// </summary>
	[[nodiscard]] unsigned int get_num_episodes() const;

	/// <summary>
	/// Read-only access to the corresponding field
	/// </summary>
	[[nodiscard]] const std::filesystem::path& get_output_folder() const;

	/// <summary>
	/// Read-only access to the corresponding field
	/// </summary>
	[[nodiscard]] double get_discount() const;

	/// <summary>
	/// Read-only access to the corresponding field
	/// </summary>
	[[nodiscard]] double get_lambda() const;

	/// <summary>
	/// Read-only access to the corresponding field
	/// </summary>
	[[nodiscard]] double get_exploration_probability() const;

	/// <summary>
	/// Read-only access to the corresponding field
	/// </summary>
	[[nodiscard]] double get_learning_rate() const;

	/// <summary>
	/// Read-only access to the corresponding field
	/// </summary>
	[[nodiscard]] const std::vector<std::size_t>& get_net_dimensions() const;

	/// <summary>
	/// Read-only access to the corresponding field
	/// </summary>
	[[nodiscard]] const std::filesystem::path& get_opponent_ensemble_path() const;

	/// <summary>
	/// Constructor (from command line arguments)
	/// </summary>
	Arguments(int argc, char** argv);

	/// <summary>
	/// Returns human readable string representation of all the arguments
	/// </summary>
	[[nodiscard]] std::string to_string() const;
};

