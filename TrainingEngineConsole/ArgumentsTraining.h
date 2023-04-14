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

namespace Training::Modes
{
	/// <summary>
	/// Parsed command line arguments (for the training mode)
	/// </summary>
	class ArgumentsTraining
	{
		/// <summary>
		/// Returns hexadecimal string representation of the current command line arguments
		/// </summary>
		[[nodiscard]] std::string calc_hash() const;

		/// <summary>
		/// Hash string of command line arguments
		/// </summary>
		std::string _hash{};

		/// <summary>
		/// Path to a file with "source" agents to train
		/// </summary>
		std::filesystem::path _source_path{};

		/// <summary>
		/// Path to a file with hyper-parameter adjustments for the "source" agents
		/// </summary>
		std::filesystem::path _adjustments_path{};

		/// <summary>
		/// Number of training rounds to do
		/// </summary>
		unsigned int _num_rounds{};

		/// <summary>
		/// Number of episodes in each training round
		/// </summary>
		unsigned int _num_episodes{};

		/// <summary>
		/// Path to the folder to save output
		/// </summary>
		std::filesystem::path _output_folder{};

		/// <summary>
		/// Path to the "opponent" ensemble (optional)
		/// </summary>
		std::filesystem::path _opponent_ensemble_path{};

		/// <summary>
		/// Flag determining if pairs should be re-arranged after each round
		/// </summary>
		bool _fixed_pairs{};

		/// <summary>
		/// Number of rounds after which state should be dumped to disk 
		/// </summary>
		unsigned int _dump_rounds{};

		/// <summary>
		/// Number of rounds after which all the agents should be saved
		/// </summary>
		unsigned int _save_rounds{};
	public:

		/// <summary>
		/// Read-only access to the corresponding field
		/// </summary>
		[[nodiscard]] std::filesystem::path get_source_path() const;

		/// <summary>
		/// Read-only access to the corresponding field
		/// </summary>
		[[nodiscard]] std::filesystem::path get_adjustments_path() const;

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
		[[nodiscard]] const std::filesystem::path& get_opponent_ensemble_path() const;

		/// <summary>
		/// Constructor (from command line arguments)
		/// </summary>
		ArgumentsTraining(const int argc, char** const argv);

		/// <summary>
		/// Returns human readable string representation of all the arguments
		/// </summary>
		[[nodiscard]] std::string to_string() const;

		/// <summary>
		/// Read-only access to the corresponding field
		/// </summary>
		[[nodiscard]] bool get_fixed_pairs() const;

		/// <summary>
		/// Read-only access to the corresponding field
		/// </summary>
		[[nodiscard]] std::string get_hash() const;

		/// <summary>
		/// Read-only access to the corresponding field
		/// </summary>
		[[nodiscard]] unsigned int get_dump_rounds() const;

		/// <summary>
		/// Read-only access to the corresponding field
		/// </summary>
		[[nodiscard]] unsigned int get_save_rounds() const;

		/// <summary>
		/// Returns path to the file where state should be dumped
		/// </summary>
		[[nodiscard]] std::filesystem::path get_state_dump_path() const;

		/// <summary>
		/// Returns name of the file where state should be dumped
		/// </summary>
		[[nodiscard]] std::filesystem::path get_state_dump_file_name() const;
	};
}

