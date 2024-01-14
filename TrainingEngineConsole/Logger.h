//Copyright (c) 2024 Denys Dragunov, dragunovdenis@gmail.com
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
#include <fstream>
#include <filesystem>

namespace Training::ConsoleUtils
{
	/// <summary>
	/// Functionality to log into file and/or console.
	/// </summary>
	class Logger
	{
		std::ofstream _file{};

		/// <summary>
		/// Logs time stamp prefixed by the given message;
		/// </summary>
		void log_time_stamp(const std::string& message);

		std::chrono::system_clock::time_point _start_time = std::chrono::system_clock::now();

	public:
		/// <summary>
		/// Default constructor.
		/// </summary>
		Logger() = default;

		/// <summary>
		/// Constructor.
		/// </summary>
		Logger(const std::filesystem::path& log_file_path);

		/// <summary>
		/// Writes message to the log file (if open) and console.
		/// </summary>
		Logger& operator <<(const std::string& message);

		/// <summary>
		/// Writes message to the log file (if open) and console.
		/// </summary>
		Logger& operator <<(const char* message);

		/// <summary>
		/// Closes current log file and opens new one with the given name.
		/// </summary>
		void open(const std::filesystem::path& log_file_path);

		/// <summary>
		/// Closes current session.
		/// </summary>
		void close();

		/// <summary>
		/// Flushes streams.
		/// </summary>
		void flush();
	};
}

