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

#include <tclap/CmdLine.h>
#include "Headers/Checkers/TrainingEngine.h"
#include "Headers/Checkers/TdLambdaAgent.h"
#include "Headers/Checkers/TdlEnsembleAgent.h"
#include <chrono>
#include <sstream>
#include <filesystem>

using namespace TrainingCell::Checkers;

void report_fatal_error(const std::string& message)
{
	std::cout << message << std::endl;
	std::cout << "Press any key to exit" << std::endl;
	std::getchar();
}

/// <summary>
/// Returns a "standard" agent to train
/// </summary>
TdLambdaAgent create_standard_agent(const std::string& name)
{
	return TdLambdaAgent({ 32, 64, 32, 16, 8, 1 }, 0.05, 0.2, 0.8, 0.01, name);
}

/// <summary>
/// Returns hh:mm:ss string representation of the given time duration in milliseconds
/// </summary>
std::string milliseconds_to_hh_mm_ss_string(const long long& time_ms)
{
	std::stringstream ss;
	ss << std::chrono::hh_mm_ss(std::chrono::duration<int64_t, std::ratio<1, 1000>>(time_ms));
	return ss.str();
}

int main(int argc, char** argv)
{
	TCLAP::CmdLine cmd("Checkers training engine", ' ', "1.0");
	auto num_pairs_arg = TCLAP::ValueArg<unsigned int>("p", "pairs", "Number of agents pairs to train", true, 1, "integer");
	cmd.add(num_pairs_arg);

	auto num_rounds_arg = TCLAP::ValueArg<unsigned int>("r", "rounds", "Number of training rounds", true, 1, "integer");
	cmd.add(num_rounds_arg);

	auto num_episodes_arg = TCLAP::ValueArg<unsigned int>("e", "episodes", "Number of episodes (plays) in each round", true, 1, "integer");
	cmd.add(num_episodes_arg);

	auto output_folder_arg = TCLAP::ValueArg<std::string>("o", "output", "Output folder path", true, "", "string");
	cmd.add(output_folder_arg);

	cmd.parse(argc, argv);

	const auto num_pairs = static_cast<int>(num_pairs_arg.getValue());
	if (num_pairs == 0)
	{
		report_fatal_error("Number of agent pairs should be positive integer");
		return 1;
	}

	const auto num_rounds = static_cast<int>(num_rounds_arg.getValue());
	if (num_rounds == 0)
	{
		report_fatal_error("Number of rounds should be positive integer");
		return 1;
	}

	const auto num_episodes = static_cast<int>(num_episodes_arg.getValue());
	if (num_episodes == 0)
	{
		report_fatal_error("Number of episodes should be positive integer");
		return 1;
	}

	const auto output_folder = std::filesystem::path(output_folder_arg.getValue());
	std::cout << "Output folder : " << output_folder << std::endl;

	//The "trick" below (with casting to void and using error code value)
	//is the only solution I managed to come up with
	//to overcome a strange situation when `is_directory` returns `false`
	//for an existing directory and `true` otherwise
	std::error_code rc;
	static_cast<void>(is_directory(output_folder, rc));
	if (rc.value() != 0)
	{
		std::cout << rc.message() << rc.value() << std::endl;
		report_fatal_error("Output path must be a valid directory");
		return 1;
	}

	std::cout << "Train " << num_pairs << " pairs of agents for " <<
		num_rounds << " rounds each consisting of " << num_episodes << " episodes" << std::endl;

	std::vector<TdLambdaAgent> agents(num_pairs * 2ull);
	std::vector<Agent*> agent_pointers(num_pairs * 2ull);

	for (auto agent_id = 0; agent_id < 2 * num_pairs; ++agent_id)
	{
		agents[agent_id] = create_standard_agent(std::string("Agent ") + std::to_string(agent_id));
		agent_pointers[agent_id] = &agents[agent_id];
	}

	TrainingEngine engine(agent_pointers);
	int rounds_counter = 0;
	auto round_time_sum = 0ll; // to calculate average round time

	engine.run(num_rounds, num_episodes, [&agents, &rounds_counter, num_rounds, &round_time_sum](long long round_time_ms, const auto& performance)
		{
			++rounds_counter;
			round_time_sum += round_time_ms;
			std::cout << "Round " << rounds_counter << " time: " << milliseconds_to_hh_mm_ss_string(round_time_ms) << " ms." << std::endl;
			if (num_rounds != rounds_counter)
				std::cout << "Expected time to finish training : " <<
				milliseconds_to_hh_mm_ss_string((num_rounds - rounds_counter) * round_time_sum / rounds_counter) << " ms." << std::endl;

		   for (auto agent_id = 0ull; agent_id < performance.size(); ++agent_id)
		   {
			   const auto& perf_item = performance[agent_id];
			   std::cout << agents[agent_id].get_name() << " (" << agents[agent_id].get_id() << ") performance : "
		   		<< perf_item[0] << "/" << perf_item[1] << std::endl;
		   }
		});

	for (const auto& agent : agents)
	{
		std::cout << output_folder / (agent.get_name() + ".tda") << std::endl;
		agent.save_to_file(output_folder / (agent.get_name() + ".tda"));
	}

	TdlEnsembleAgent ensemble(agents);
	ensemble.save_to_file(output_folder / (ensemble.get_name() + ".ena"));
}

