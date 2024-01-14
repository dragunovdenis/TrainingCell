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

#include "ConsoleUtils.h"
#include <iostream>
#include "TrainingState.h"
#include "fstream"
#include "../../DeepLearning/DeepLearning/Utilities.h"
#include "windows.h"
#include "psapi.h"


namespace Training::ConsoleUtils
{
	std::size_t get_phys_mem_usage()
	{
		PROCESS_MEMORY_COUNTERS_EX pmc;
		GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
		return pmc.PrivateUsage;
	}

	void horizontal_console_separator()
	{
		print_to_console("=========================================");
	}

	void print_to_console(const std::string& message, const bool new_line_at_end)
	{
		const auto lines = DeepLearning::Utils::split_by_char(message, '\n');

		for (auto line_id = 0ull; line_id < lines.size() - 1; ++line_id)
			Logger << lines[line_id] << "\n";

		Logger << *lines.rbegin();

		if (new_line_at_end)
			Logger << "\n";
		else
			Logger.flush();
	}

	void report_fatal_error(const std::string& message)
	{
		print_to_console(message);
		print_to_console("Press any key to exit");
		static_cast<void>(std::getchar());
	}

	std::string calc_file_hash(const std::filesystem::path& file_path)
	{
		const std::ifstream file(file_path);

		if (!file)
			throw std::exception(std::format("Cant open file {}", file_path.string()).c_str());

		std::stringstream ss;
		ss << file.rdbuf();

		return DeepLearning::Utils::get_hash_as_hex_str(ss.str());
	}

	bool decision_prompt(const std::string& prompt_string)
	{
		print_to_console(prompt_string, false);

		char decision;
		std::cin.get(decision);
		std::cin.ignore();
		horizontal_console_separator();

		return decision == 'y';
	}

	bool try_load_state_silent(const std::filesystem::path& state_path, Training::TrainingState& state)
	{
		try
		{
			state = TrainingState::load_from_file(state_path);
		}
		catch (...)
		{
			state.reset();
			return false;
		}

		return true;
	}

	bool try_load_state(const std::filesystem::path& state_path, Training::TrainingState& state)
	{
		if (try_load_state_silent(state_path, state))
		{
			horizontal_console_separator();
			print_to_console("State dump from round " + std::to_string(state.get_round_id()) + " was successfully loaded");

			if (decision_prompt("Discard? (y/n):"))
			{
				state.reset();
				return false;
			}
		}
		else
			return false;

		return true;
	}
}
