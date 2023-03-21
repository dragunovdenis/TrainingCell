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

#include "Arguments.h"
#include <tclap/CmdLine.h>
#include "../DeepLearning/DeepLearning/Utilities.h"

namespace Training
{
	std::string Arguments::calc_hash(const int argc, char** const argv)
	{
		std::string accumulated_str;

		for (auto arg_id = 0; arg_id < argc; ++arg_id)
			accumulated_str += argv[arg_id];

		return DeepLearning::Utils::get_hash_as_hex_str(
			DeepLearning::Utils::normalize_string(accumulated_str));
	}

	unsigned Arguments::get_num_pairs() const
	{
		return _num_pairs;
	}

	unsigned Arguments::get_num_rounds() const
	{
		return _num_rounds;
	}

	unsigned Arguments::get_num_episodes() const
	{
		return _num_episodes;
	}

	const std::filesystem::path& Arguments::get_output_folder() const
	{
		return _output_folder;
	}

	double Arguments::get_discount() const
	{
		return _discount;
	}

	double Arguments::get_lambda() const
	{
		return _lambda;
	}

	double Arguments::get_exploration_probability() const
	{
		return _exploration_probability;
	}

	double Arguments::get_learning_rate() const
	{
		return _learning_rate;
	}

	const std::vector<std::size_t>& Arguments::get_net_dimensions() const
	{
		return _net_dimensions;
	}

	const std::filesystem::path& Arguments::get_opponent_ensemble_path() const
	{
		return _opponent_ensemble_path;
	}

	Arguments::Arguments(const int argc, char** const argv) : _hash(calc_hash(argc, argv))
	{
		TCLAP::CmdLine cmd("Checkers training engine", ' ', "1.0");
		auto num_pairs_arg = TCLAP::ValueArg<unsigned int>("", "pairs", "Number of agents pairs to train", true, 1, "integer");
		cmd.add(num_pairs_arg);

		auto num_rounds_arg = TCLAP::ValueArg<unsigned int>("", "rounds", "Number of training rounds", true, 1, "integer");
		cmd.add(num_rounds_arg);

		auto num_episodes_arg = TCLAP::ValueArg<unsigned int>("", "episodes", "Number of episodes (plays) in each round", true, 1, "integer");
		cmd.add(num_episodes_arg);

		auto output_folder_arg = TCLAP::ValueArg<std::string>("", "output", "Output folder path", true, "", "string");
		cmd.add(output_folder_arg);

		auto discount_arg = TCLAP::ValueArg<double>("", "discount", "Value of reward discount", false, 0.9, "double");
		cmd.add(discount_arg);

		auto lambda_arg = TCLAP::ValueArg<double>("", "lambda", "Value of lambda", false, 0.2, "double");
		cmd.add(lambda_arg);

		auto exploration_arg = TCLAP::ValueArg<double>("", "exploration", "Exploration probability", false, 0.05, "double");
		cmd.add(exploration_arg);

		auto learning_rate_arg = TCLAP::ValueArg<double>("", "rate", "Learning rate", false, 0.01, "double");
		cmd.add(learning_rate_arg);

		auto net_dimensions_arg = TCLAP::ValueArg<std::string>("", "net",
			"Neural net dimensions", false, "{32, 64, 32, 16, 8, 1}", "string");
		cmd.add(net_dimensions_arg);

		auto opponent_ensemble_path_arg = TCLAP::ValueArg<std::string>("", "opponent", "Path to opponent ensemble", false, "", "string");
		cmd.add(opponent_ensemble_path_arg);

		auto fixed_pairs_arg = TCLAP::ValueArg<bool>("", "fixed_pairs",
			"Flag determining if agent pairs are kept fixed during all the training", false, false, "boolean");
		cmd.add(fixed_pairs_arg);

		auto dump_rounds_arg = TCLAP::ValueArg<unsigned int>("", "dump_rounds",
			"Number of rounds after which state should be dumped to disk", false, 0, "unsigned int");
		cmd.add(dump_rounds_arg);

		auto save_rounds_arg = TCLAP::ValueArg<unsigned int>("", "save_rounds",
			"Number of rounds after which all the agents should be saved to disk", false, 0, "unsigned int");
		cmd.add(save_rounds_arg);

		cmd.parse(argc, argv);

		_num_pairs = static_cast<int>(num_pairs_arg.getValue());
		if (_num_pairs == 0)
			throw std::exception("Number of agent pairs should be positive integer");

		_num_rounds = static_cast<int>(num_rounds_arg.getValue());
		if (_num_rounds == 0)
			throw std::exception("Number of rounds should be positive integer");

		_num_episodes = static_cast<int>(num_episodes_arg.getValue());
		if (_num_episodes == 0)
			throw std::exception("Number of episodes should be positive integer");

		_discount = discount_arg.getValue();
		if (_discount < 0.0 || _discount > 1.0)
			throw std::exception("Discount must be within [0, 1]");

		_lambda = lambda_arg.getValue();
		if (_lambda < 0.0 || _lambda > 1.0)
			throw std::exception("Lambda must be within [0, 1]");

		_exploration_probability = exploration_arg.getValue();

		_learning_rate = learning_rate_arg.getValue();

		_net_dimensions = DeepLearning::Utils::parse_vector<std::size_t>(net_dimensions_arg.getValue());

		_output_folder = std::filesystem::path(output_folder_arg.getValue());

		//The "trick" below (with casting to void and using error code value)
		//is the only solution I managed to come up with
		//to overcome a strange situation when `is_directory` returns `false`
		//for an existing directory and `true` otherwise
		std::error_code rc;
		static_cast<void>(std::filesystem::is_directory(_output_folder, rc));
		if (rc.value() != 0)
			throw std::exception("Output path must be a valid directory");

		_opponent_ensemble_path = opponent_ensemble_path_arg.getValue();

		if (_opponent_ensemble_path != "" && !std::filesystem::is_regular_file(_opponent_ensemble_path))
			throw std::exception("Invalid path to the opponent ensemble agent");

		_fixed_pairs = fixed_pairs_arg.getValue();

		_dump_rounds = dump_rounds_arg.getValue();

		_save_rounds = save_rounds_arg.getValue();
	}

	std::string Arguments::to_string() const
	{
		const auto dim_string = DeepLearning::Utils::vector_to_str(_net_dimensions);
		return std::format(" Pairs to train: {}\n Rounds : {} \n Episodes per round: {}\n Output folder: {}\n Discount: {}\n\
 Lambda: {}\n Learning rate: {}\n Exploration probability: {}\n Net dimensions: {}\n Opponent ensemble path: {}\n Fixed pairs: {}\n\
 Dump Rounds: {}\n Save Rounds: {}\n Hash: {}\n",
			_num_pairs, _num_rounds, _num_episodes, _output_folder.string(), _discount,
			_lambda, _learning_rate, _exploration_probability,
			dim_string, _opponent_ensemble_path.string(), _fixed_pairs, _dump_rounds, _save_rounds, _hash);
	}

	bool Arguments::get_fixed_pairs() const
	{
		return _fixed_pairs;
	}

	std::string Arguments::get_hash() const
	{
		return _hash;
	}

	unsigned Arguments::get_dump_rounds() const
	{
		return _dump_rounds;
	}

	unsigned Arguments::get_save_rounds() const
	{
		return _save_rounds;
	}

	std::filesystem::path Arguments::get_state_dump_path() const
	{
		return get_output_folder() / get_state_dump_file_name();
	}

	std::filesystem::path Arguments::get_state_dump_file_name() const
	{
		return get_hash() + ".sdmp";
	}
}
