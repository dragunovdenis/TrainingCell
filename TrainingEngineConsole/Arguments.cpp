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
#include <fstream>
#include <sstream>
#include "../DeepLearning/DeepLearning/Utilities.h"

namespace Training
{
	/// <summary>
	/// Calculates hexadecimal representation of a 8 bytes hash of the given file
	/// </summary>
	std::string calc_file_hash(const std::filesystem::path& file_path)
	{
		const std::ifstream file(file_path);

		if (!file)
			throw std::exception(std::format("Cant open file {}", file_path.string()).c_str());

		std::stringstream ss;
		ss << file.rdbuf();

		return DeepLearning::Utils::get_hash_as_hex_str(ss.str());
	}

	std::string Arguments::calc_hash() const
	{
		std::string str;

		str += calc_file_hash(_source_path);

		if (std::filesystem::is_regular_file(_adjustments_path))
			str += calc_file_hash(_adjustments_path);

		str += std::to_string(_num_rounds);
		str += std::to_string(_num_episodes);
		str += std::to_string(_save_rounds);
		str += std::to_string(_dump_rounds);
		str += DeepLearning::Utils::to_upper_case(_output_folder.string());
		str += DeepLearning::Utils::to_upper_case(_opponent_ensemble_path.string());
		str += std::to_string(_fixed_pairs);

		return DeepLearning::Utils::get_hash_as_hex_str(str);
	}

	std::filesystem::path Arguments::get_source_path() const
	{
		return _source_path;
	}

	std::filesystem::path Arguments::get_adjustments_path() const
	{
		return _adjustments_path;
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

	const std::filesystem::path& Arguments::get_opponent_ensemble_path() const
	{
		return _opponent_ensemble_path;
	}

	Arguments::Arguments(const int argc, char** const argv)
	{
		TCLAP::CmdLine cmd("Checkers training engine", ' ', "1.0");

		auto source_path_arg = TCLAP::ValueArg<std::string>("", "source", 
"Path to saved state or to an agent script file", true, "", "string");
		cmd.add(source_path_arg);

		auto adjustments_path_arg = TCLAP::ValueArg<std::string>("", "adjustments", 
"Path an agent script file (to adjust parameters of `source` agents)", false, "", "string");
		cmd.add(adjustments_path_arg);

		auto num_rounds_arg = TCLAP::ValueArg<unsigned int>("", "rounds", "Number of training rounds", true, 1, "integer");
		cmd.add(num_rounds_arg);

		auto num_episodes_arg = TCLAP::ValueArg<unsigned int>("", "episodes", "Number of episodes (plays) in each round", true, 1, "integer");
		cmd.add(num_episodes_arg);

		auto output_folder_arg = TCLAP::ValueArg<std::string>("", "output", "Output folder path", true, "", "string");
		cmd.add(output_folder_arg);

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

		_source_path = source_path_arg.getValue();

		if (!std::filesystem::is_regular_file(_source_path))
			throw std::exception("Invalid source file");

		_adjustments_path = adjustments_path_arg.getValue();

		if (_adjustments_path != "" && !std::filesystem::is_regular_file(_adjustments_path))
			throw std::exception("Invalid adjustments file");

		_num_rounds = static_cast<int>(num_rounds_arg.getValue());
		if (_num_rounds == 0)
			throw std::exception("Number of rounds should be positive integer");

		_num_episodes = static_cast<int>(num_episodes_arg.getValue());
		if (_num_episodes == 0)
			throw std::exception("Number of episodes should be positive integer");

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

		_hash = calc_hash();
	}

	std::string Arguments::to_string() const
	{
		return std::format(" Source Path: {}\n Adjustments Path: {}\n Rounds: {}\n Episodes per round: {}\n Output folder: {}\n\
 Opponent ensemble path: {}\n Fixed pairs: {}\n Dump Rounds: {}\n Save Rounds: {}\n Hash: {}\n",
			_source_path.string(), _adjustments_path.string(), _num_rounds, _num_episodes, _output_folder.string(),
			_opponent_ensemble_path.string(), _fixed_pairs, _dump_rounds, _save_rounds, _hash);
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
