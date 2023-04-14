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
	///	"Interface" to access parameter flags
	/// </summary>
	class ParameterFlags
	{
	protected:
		virtual ~ParameterFlags() = default;

	public:
		/// <summary>
		/// Read-only access to the corresponding field
		/// </summary>
		[[nodiscard]] virtual bool get_lambda_flag() const = 0;

		/// <summary>
		/// Read-only access to the corresponding field
		/// </summary>
		[[nodiscard]] virtual bool get_discount_flag() const = 0;

		/// <summary>
		/// Read-only access to the corresponding field
		/// </summary>
		[[nodiscard]] virtual bool get_learning_rate_flag() const = 0;

		/// <summary>
		/// Read-only access to the corresponding field
		/// </summary>
		[[nodiscard]] virtual bool get_exploration_flag() const = 0;
	};

	/// <summary>
	/// Parsed command line arguments to the optimization mode
	/// </summary>
	class ArgumentsOptimization : public ParameterFlags
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
		/// Number of episodes in each training round
		/// </summary>
		unsigned int _num_episodes{};

		/// <summary>
		/// Number of episodes in in evaluation session
		/// </summary>
		unsigned int _num_eval_episodes{};

		/// <summary>
		/// Minimal size of the simplex in the amoeba optimization procedure to stop the optomization
		/// </summary>
		double _min_simplex_size{};

		/// <summary>
		/// Path to the folder to save output
		/// </summary>
		std::filesystem::path _output_folder{};

		/// <summary>
		/// Script-string to instantiate pool of agents ("training state")
		/// </summary>
		std::filesystem::path _source_path{};

		/// <summary>
		/// Number of rounds after which state should be dumped to disk 
		/// </summary>
		unsigned int _dump_rounds{};

		/// <summary>
		/// Script used to instantiate collection of agents to take part in the optimization process
		/// </summary>
		std::string _basic_agent_script;

		/// <summary>
		/// Flag determining whether "lambda" parameter takes part in the optimization process
		/// </summary>
		bool _lambda_flag{};

		/// <summary>
		/// Flag determining whether "reward discount" parameter takes part in the optimization process
		/// </summary>
		bool _discount_flag{};

		/// <summary>
		/// Flag determining whether "learning rate" parameter takes part in the optimization process
		/// </summary>
		bool _learning_rate_flag{};

		/// <summary>
		/// Flag determining whether "exploration" parameter takes part in the optimization process
		/// </summary>
		bool _exploration_flag{};

	public:
		/// <summary>
		/// Constructor
		/// </summary>
		ArgumentsOptimization(const int argc, char** const argv);

		/// <summary>
		/// Returns human readable string representation of all the arguments
		/// </summary>
		[[nodiscard]] std::string to_string() const;

		/// <summary>
		/// Read-only access to the corresponding field
		/// </summary>
		[[nodiscard]] std::string get_hash() const;

		/// <summary>
		/// Read-only access to the corresponding field
		/// </summary>
		[[nodiscard]] unsigned int get_dump_rounds() const;

		/// <summary>
		/// Returns path to the file where state should be dumped
		/// </summary>
		[[nodiscard]] std::filesystem::path get_state_dump_path() const;

		/// <summary>
		/// Returns name of the file where state should be dumped
		/// </summary>
		[[nodiscard]] std::filesystem::path get_state_dump_file_name() const;

		/// <summary>
		/// Returns path to the file where optimizer should be dumped
		/// </summary>
		[[nodiscard]] std::filesystem::path get_optimizer_dump_path() const;

		/// <summary>
		/// Returns name of the file where optimizer should be dumped
		/// </summary>
		[[nodiscard]] std::filesystem::path get_optimizer_dump_file_name() const;

		/// <summary>
		/// Read-only access to the corresponding field
		/// </summary>
		[[nodiscard]] double get_min_simplex_size() const;

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
		[[nodiscard]] std::filesystem::path get_source_path() const;

		/// <summary>
		/// Read-only access to the corresponding field
		/// </summary>
		[[nodiscard]] bool get_lambda_flag() const override;

		/// <summary>
		/// Read-only access to the corresponding field
		/// </summary>
		[[nodiscard]] bool get_discount_flag() const override;

		/// <summary>
		/// Read-only access to the corresponding field
		/// </summary>
		[[nodiscard]] bool get_learning_rate_flag() const override;

		/// <summary>
		/// Read-only access to the corresponding field
		/// </summary>
		[[nodiscard]] bool get_exploration_flag() const override;

		/// <summary>
		/// Read-only access to the corresponding field
		/// </summary>
		[[nodiscard]] unsigned int get_num_eval_episodes() const;
	};
}
