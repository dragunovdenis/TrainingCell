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
#include "Arguments.h"

using namespace TrainingCell::Checkers;

void report_fatal_error(const std::string& message)
{
	std::cout << message << std::endl;
	std::cout << "Press any key to exit" << std::endl;
	static_cast<void>(std::getchar());
}

/// <summary>
/// Returns a "standard" agent to train
/// </summary>
TdLambdaAgent create_agent(const std::string& name, const Arguments& args)
{
	return { args.get_net_dimensions(),
		args.get_exploration_probability(),
		args.get_lambda(),
		args.get_discount(),
		args.get_learning_rate(), name
};
}

/// <summary>
/// Tries to load ensemble from the given file on disk and returns it in case of success
/// </summary>
std::optional<TdlEnsembleAgent> try_load_ensemble(const std::filesystem::path& ensemble_path)
{
	try
	{
		return TdlEnsembleAgent::load_from_file(ensemble_path);
	} catch (...)
	{
		return std::nullopt;
	}
}

int main(int argc, char** argv)
{
	try
	{
		const Arguments args(argc, argv);

		std::cout << args.to_string() << std::endl;

		std::vector<TdLambdaAgent> agents(args.get_num_pairs() * 2ull);
		std::vector<Agent*> agent_pointers(args.get_num_pairs() * 2ull);

		for (auto agent_id = 0u; agent_id < 2 * args.get_num_pairs(); ++agent_id)
		{
			agents[agent_id] = create_agent(std::string("Agent ") + std::to_string(agent_id), args);
			agent_pointers[agent_id] = &agents[agent_id];
		}

		TrainingEngine engine(agent_pointers);
		int rounds_counter = 0;
		auto round_time_sum = 0ll; // to calculate average round time

		const auto num_rounds = static_cast<int>(args.get_num_rounds());

		auto opponent_ensemble = try_load_ensemble(args.get_opponent_ensemble_path());

		const auto reporter = [&agents, &rounds_counter, num_rounds, &round_time_sum](long long round_time_ms, const auto& performance)
		{
			++rounds_counter;
			round_time_sum += round_time_ms;
			std::cout << "Round " << rounds_counter << " time: " << 
				DeepLearning::Utils::milliseconds_to_dd_hh_mm_ss_string(round_time_ms) << " ms." << std::endl;
			if (num_rounds != rounds_counter)
				std::cout << "Expected time to finish training : " <<
				DeepLearning::Utils::milliseconds_to_dd_hh_mm_ss_string((static_cast<long long>(num_rounds) -
					rounds_counter) * round_time_sum / rounds_counter) << " ms." << std::endl;

			for (auto agent_id = 0ull; agent_id < performance.size(); ++agent_id)
			{
				const auto& perf_item = performance[agent_id];
				std::cout << agents[agent_id].get_name() << " (" << agents[agent_id].get_id() << ") performance : "
					<< perf_item[0] << "/" << perf_item[1] << std::endl;
			}
		};

		if (opponent_ensemble.has_value())
		{
			std::cout << "================================" << std::endl;
			std::cout << "Training against loaded ensemble" << std::endl;
			std::cout << "================================" << std::endl;
			opponent_ensemble.value().set_single_agent_mode(true);
			engine.run(opponent_ensemble.value(), num_rounds, static_cast<int>(args.get_num_episodes()), reporter);

		} else
			engine.run(num_rounds, static_cast<int>(args.get_num_episodes()), reporter);

		for (const auto& agent : agents)
		{
			std::cout << args.get_output_folder() / (agent.get_name() + ".tda") << std::endl;
			agent.save_to_file(args.get_output_folder() / (agent.get_name() + ".tda"));
		}

		TdlEnsembleAgent ensemble(agents);
		ensemble.save_to_file(args.get_output_folder() / (ensemble.get_name() + ".ena"));
	}
	catch (std::exception& e)
	{
		report_fatal_error(e.what());
	}
}

