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

#include "Logger.h"
#include <iostream>
#include "../DeepLearning/DeepLearning/Utilities.h"

namespace Training
{
	void Logger::log_time_stamp(const std::string& message)
	{
		const auto now = std::chrono::system_clock::now();
		const auto new_session_message = message + DeepLearning::Utils::format_date_time(now);
		operator<<(new_session_message);
	}

	Logger::Logger(const std::filesystem::path& log_file_path)
	{
		open(log_file_path);
	}

	Logger& Logger::operator <<(const std::string& message)
	{
		return operator<<(message.c_str());
	}

	Logger& Logger::operator<<(const char* message)
	{
		if (_file)
			_file << message;

		std::cout << message;

		return *this;
	}

	void Logger::open(const std::filesystem::path& log_file_path)
	{
		_start_time = std::chrono::system_clock::now();

		if (_file)
			_file.close();

		_file.open(log_file_path, std::ios::app);

		log_time_stamp("Logger: start new session, ");
	}

	void Logger::close()
	{
		log_time_stamp("Logger: close session, ");
		const auto elapsed_time_in_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now() - _start_time).count();
		*this << "Total session time: " << DeepLearning::Utils::
		milliseconds_to_dd_hh_mm_ss_string(elapsed_time_in_milliseconds) << "\n";
	}

	void Logger::flush()
	{
		if (_file)
			_file.flush();

		std::cout.flush();
	}
}
