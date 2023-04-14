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

#include "ArgumentsOptimization.h"
#include <tclap/CmdLine.h>

#include "ConsoleUtils.h"
#include "../../DeepLearning/DeepLearning/Utilities.h"

namespace Training::Modes
{
	std::string ArgumentsOptimization::calc_hash() const
	{
		std::string str;

		str += ConsoleUtils::calc_file_hash(_source_path);

		str += std::to_string(_num_eval_episodes);
		str += std::to_string(_num_episodes);
		str += std::to_string(_min_simplex_size);
		str += std::to_string(_dump_rounds);
		str += DeepLearning::Utils::to_upper_case(_output_folder.string());
		str += std::to_string(_lambda_flag);
		str += std::to_string(_discount_flag);
		str += std::to_string(_learning_rate_flag);
		str += std::to_string(_exploration_flag);

		return DeepLearning::Utils::get_hash_as_hex_str(str);
	}

	ArgumentsOptimization::ArgumentsOptimization(const int argc, char** const argv)
	{
		TCLAP::CmdLine cmd("Parameter optimization engine", ' ', "1.0");

		auto source_arg = TCLAP::ValueArg<std::string>("", "source",
			"String-script to instantiate agents", true, "", "string");
		cmd.add(source_arg);

		auto min_simplex_arg = TCLAP::ValueArg<double>("", "min_simplex", "Minimal size of simplex to stop optimization",
			false, 0.001, "double");
		cmd.add(min_simplex_arg);

		auto num_episodes_arg = TCLAP::ValueArg<unsigned int>("", "episodes", "Number of episodes (plays) in each round", true, 1, "integer");
		cmd.add(num_episodes_arg);

		auto num_eval_episodes_arg = TCLAP::ValueArg<unsigned int>("", "eval_episodes", "Number of episodes (plays) in evaluation session", false, 1000, "integer");
		cmd.add(num_eval_episodes_arg);

		auto output_folder_arg = TCLAP::ValueArg<std::string>("", "output", "Output folder path", true, "", "string");
		cmd.add(output_folder_arg);

		auto lambda_flag_arg = TCLAP::ValueArg<bool>("", "lambda_flag",
			"Flag determining if lambda parameter is involved into the optimization process", false, false, "boolean");
		cmd.add(lambda_flag_arg);

		auto discount_flag_arg = TCLAP::ValueArg<bool>("", "discount_flag",
			"Flag determining if discount parameter is involved into the optimization process", false, false, "boolean");
		cmd.add(discount_flag_arg);

		auto rate_flag_arg = TCLAP::ValueArg<bool>("", "rate_flag",
			"Flag determining if learning rate parameter is involved into the optimization process", false, false, "boolean");
		cmd.add(rate_flag_arg);

		auto exploration_flag_arg = TCLAP::ValueArg<bool>("", "exploration_flag",
			"Flag determining if exploration parameter is involved into the optimization process", false, false, "boolean");
		cmd.add(exploration_flag_arg);

		auto dump_rounds_arg = TCLAP::ValueArg<unsigned int>("", "dump_rounds",
			"Number of rounds after which state should be dumped to disk", false, 0, "unsigned int");
		cmd.add(dump_rounds_arg);

		cmd.parse(argc, argv);

		_source_path = source_arg.getValue();
		if (!std::filesystem::is_regular_file(_source_path))
			throw std::exception("Invalid source file");

		_min_simplex_size = min_simplex_arg.getValue();
		if (_min_simplex_size < 0)
			throw std::exception("Invalid minimal simplex size");

		_num_episodes = static_cast<int>(num_episodes_arg.getValue());
		if (_num_episodes == 0)
			throw std::exception("Number of episodes should be positive integer");

		_num_eval_episodes = num_eval_episodes_arg.getValue();
		if (_num_eval_episodes == 0)
			throw std::exception("Number of evaluation episodes should be positive integer");

		_output_folder = std::filesystem::path(output_folder_arg.getValue());

		//The "trick" below (with casting to void and using error code value)
		//is the only solution I managed to come up with
		//to overcome a strange situation when `is_directory` returns `false`
		//for an existing directory and `true` otherwise
		std::error_code rc;
		static_cast<void>(std::filesystem::is_directory(_output_folder, rc));
		if (rc.value() != 0)
			throw std::exception("Output path must be a valid directory");

		_dump_rounds = dump_rounds_arg.getValue();

		_lambda_flag = lambda_flag_arg.getValue();

		_discount_flag = discount_flag_arg.getValue();

		_learning_rate_flag = rate_flag_arg.getValue();

		_exploration_flag = exploration_flag_arg.getValue();

		_hash = calc_hash();
	}

	std::string ArgumentsOptimization::to_string() const
	{
		return std::format(" Source Path: {}\n Episodes per round: {}\n Evaluation episodes: {}\n Min simplex size: {}\n Output folder: {}\n\
 Dump Rounds: {}\n Lambda flag: {}\n Discount flag: {}\n Learning rate flag: {}\n Exploration flag {}\n Hash: {}\n",
			_source_path.string(), _num_episodes, _num_eval_episodes, _min_simplex_size, _output_folder.string(),
			_dump_rounds, _lambda_flag, _discount_flag, _learning_rate_flag, _exploration_flag, _hash);
	}

	std::string ArgumentsOptimization::get_hash() const
	{
		return _hash;
	}

	unsigned ArgumentsOptimization::get_dump_rounds() const
	{
		return _dump_rounds;
	}

	std::filesystem::path ArgumentsOptimization::get_state_dump_path() const
	{
		return get_output_folder() / get_state_dump_file_name();
	}

	std::filesystem::path ArgumentsOptimization::get_state_dump_file_name() const
	{
		return get_hash() + ".sdmp";
	}

	std::filesystem::path ArgumentsOptimization::get_optimizer_dump_path() const
	{
		return get_output_folder() / get_optimizer_dump_file_name();
	}

	std::filesystem::path ArgumentsOptimization::get_optimizer_dump_file_name() const
	{
		return get_hash() + ".amoeba";
	}

	double ArgumentsOptimization::get_min_simplex_size() const
	{
		return _min_simplex_size;
	}

	unsigned ArgumentsOptimization::get_num_episodes() const
	{
		return _num_episodes;
	}

	const std::filesystem::path& ArgumentsOptimization::get_output_folder() const
	{
		return _output_folder;
	}

	std::filesystem::path ArgumentsOptimization::get_source_path() const
	{
		return _source_path;
	}

	bool ArgumentsOptimization::get_lambda_flag() const
	{
		return _lambda_flag;
	}

	bool ArgumentsOptimization::get_discount_flag() const
	{
		return _discount_flag;
	}

	bool ArgumentsOptimization::get_learning_rate_flag() const
	{
		return _learning_rate_flag;
	}

	bool ArgumentsOptimization::get_exploration_flag() const
	{
		return _exploration_flag;
	}

	unsigned ArgumentsOptimization::get_num_eval_episodes() const
	{
		return _num_eval_episodes;
	}
}
