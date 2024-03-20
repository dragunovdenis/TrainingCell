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

#include <chrono>
#include "ConsoleUtils.h"
#include "TrainingMode.h"
#include "OptimizationMode.h"
#include "../DeepLearning/DeepLearning/Utilities.h"

using namespace Training::Modes;

enum class Mode: int { Training = 0, Optimization = 1, };

int main(int argc, char** argv)
{
	try
	{
		if (argc < 2)
			throw std::exception("Invalid command line");

		const auto mode_str = std::string(argv[1]);
		const auto mode = static_cast<Mode>(DeepLearning::Utils::parse_scalars<int>(mode_str)[0]);

		argv[1] = argv[0];
		switch (mode)
		{
			case Mode::Training: run_training(argc - 1, &argv[1]); break;
			case Mode::Optimization: run_parameter_optimization(argc - 1, &argv[1]); break;
			default:
				throw std::exception("Unexpected mode");
		}
	}
	catch (std::exception& e)
	{
		Training::ConsoleUtils::report_fatal_error(e.what());
	}
}

