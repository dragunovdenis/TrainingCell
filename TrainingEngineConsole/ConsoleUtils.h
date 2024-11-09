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
#include <string>
#include "Logger.h"

namespace Training
{
	class TrainingState;
}

namespace Training
{
	class ConsoleUtils
	{
	public:
		/// <summary>
		/// Global instance of logger.
		/// </summary>
		static Logger logger;

		/// <summary>
		/// Returns amount of physical memory in bytes used by the current process at the moment of call
		/// </summary>
		static std::size_t get_phys_mem_usage();

		/// <summary>
		/// Writes a "horizontal separator" to console
		/// </summary>
		static void horizontal_console_separator(unsigned int line_count = 1);

		/// <summary>
		/// Starts new line in the console and log file.
		/// </summary>
		static void new_line(unsigned int line_count = 1);

		/// <summary>
		/// Prints to console while taking care about flushing the buffer
		/// </summary>
		static void print_to_console(const std::string& message, const bool new_line_at_end = true);

		/// <summary>
		/// Reports an error with the given message
		/// </summary>
		static void report_fatal_error(const std::string& message);

		/// <summary>
		/// Calculates hexadecimal representation of a 8 bytes hash of the given file
		/// </summary>
		static std::string calc_file_hash(const std::filesystem::path& file_path);

		/// <summary>
		/// Returns true if user wants to continue
		/// </summary>
		static bool decision_prompt(const std::string& prompt_string = "Continue? (y/n):");

		/// <summary>
		/// Tries to load training state from the given file and returns "true" in case of success; Does not write anything to console
		/// </summary>
		static bool try_load_state_silent(const std::filesystem::path& state_path, Training::TrainingState& state);

		/// <summary>
		/// Tries to load training state from the given file and returns "true" in case of success
		/// </summary>
		static bool try_load_state(const std::filesystem::path& state_path, Training::TrainingState& state);
	};
}