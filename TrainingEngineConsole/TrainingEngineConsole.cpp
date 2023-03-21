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
#include <filesystem>
#include <queue>
#include "Arguments.h"
#include "TrainingState.h"

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
TdLambdaAgent create_agent(const std::string& name, const Training::Arguments& args)
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

/// <summary>
/// Tries to load training state from the given file and returns "true" in case of success
/// </summary>
bool try_load_state(const std::filesystem::path& state_path, Training::TrainingState& state)
{
	try
	{
		state = Training::TrainingState::load_from_file(state_path);

		std::cout << "=========================================" << std::endl;
		std::cout << "State dump from round " << state.get_round_id() << " was successfully loaded" << std::endl;
		std::cout << "Discard? (y/n)";
		char decision;
		std::cin.get(decision);
		std::cout << "=========================================" << std::endl;

		if (decision == 'y')
		{
			state.reset();
			return false;
		}
	}
	catch (...)
	{
		return false;
	}

	return true;
}

int main(int argc, char** argv)
{
	try
	{
		const Training::Arguments args(argc, argv);

		std::cout << args.to_string() << std::endl;

		Training::TrainingState state;
		if (!try_load_state(args.get_state_dump_path(), state))
		{
			for (auto agent_id = 0u; agent_id < 2 * args.get_num_pairs(); ++agent_id)
				state.add_agent(create_agent(std::string("Agent ") + std::to_string(agent_id), args));
		}

		std::vector<Agent*> agent_pointers;
		agent_pointers.reserve(state.agents_count());
		for (auto agent_id = 0ull; agent_id < state.agents_count(); ++agent_id)
			agent_pointers.push_back(&state[agent_id]);

		TrainingEngine engine(agent_pointers);
		auto round_time_sum = 0ll; // to calculate average round time
		std::queue<long long> round_time_queue;

		const auto num_rounds = static_cast<int>(args.get_num_rounds() - state.get_round_id());

		auto opponent_ensemble = try_load_ensemble(args.get_opponent_ensemble_path());

		const auto saver = [&state, &args](const std::string& sub_folder_name)
		{
			const auto directory_path = !sub_folder_name.empty()? args.get_output_folder() / sub_folder_name : args.get_output_folder();
			std::filesystem::create_directories(directory_path);

			TdlEnsembleAgent ensemble;
			ensemble.set_name("Ensemble");
			for (auto agent_id = 0ull; agent_id < state.agents_count(); ++agent_id)
			{
				const auto& agent = state[agent_id];
				const auto agent_file_path = directory_path / (agent.get_name() + ".tda");
				std::cout << agent_file_path << std::endl;
				agent.save_to_file(agent_file_path);
				ensemble.add(agent);
			}
			std::cout << "=============================================" << std::endl;

			ensemble.save_to_file(directory_path / (ensemble.get_name() + ".ena"));

			state.save_to_file(directory_path / args.get_state_dump_file_name());//Save final of state
			state.save_performance_report(directory_path / "Performance_report.txt");//Save performance report
		};

		const auto reporter = [&state, num_rounds, &round_time_sum, &round_time_queue, &saver, &args]
		(const long long round_time_ms, const auto& performance)
		{
			const auto rounds_counter = state.increment_round();
			round_time_queue.push(round_time_ms);
			round_time_sum += round_time_ms;
			std::cout << "Round " << rounds_counter << " time: " << 
				DeepLearning::Utils::milliseconds_to_dd_hh_mm_ss_string(round_time_ms) << std::endl;
			if (num_rounds != rounds_counter)
				std::cout << "Expected time to finish training : " <<
				DeepLearning::Utils::milliseconds_to_dd_hh_mm_ss_string((static_cast<long long>(num_rounds) -
					rounds_counter) * round_time_sum / round_time_queue.size()) << std::endl;

			if (round_time_queue.size() >= 5) 
			{
				// take into account only the last five round time measurements
				// to calculate the "current average" round time
				round_time_sum -= round_time_queue.front();
				round_time_queue.pop();
			}

			auto average_performance_white = 0.0;
			auto average_performance_black = 0.0;

			for (auto agent_id = 0ull; agent_id < performance.size(); ++agent_id)
			{
				const auto& perf_item = performance[agent_id];
				std::cout << state[agent_id].get_name() << " (" << state[agent_id].get_id() << ") performance : "
					<< perf_item[0] << "/" << perf_item[1] << std::endl;

				average_performance_white += perf_item[0];
				average_performance_black += perf_item[1];
			}

			average_performance_white /= performance.size();
			average_performance_black /= performance.size();
			std::cout << "Average performance : " << average_performance_white << "/" <<
				average_performance_black << std::endl;

			std::cout << std::endl << "=======================================" << std::endl;

			state.add_performance_record(rounds_counter, average_performance_white, average_performance_black);

			if (args.get_dump_rounds() != 0 && (rounds_counter % args.get_dump_rounds() == 0))
				state.save_to_file(args.get_state_dump_path());

			if (args.get_save_rounds() != 0 && (rounds_counter % args.get_save_rounds() == 0))
				saver(std::format("Round_{}", rounds_counter));
		};

		if (opponent_ensemble.has_value())
		{
			std::cout << "================================" << std::endl;
			std::cout << "Training against loaded ensemble" << std::endl;
			std::cout << "================================" << std::endl;
			opponent_ensemble.value().set_single_agent_mode(true);
			engine.run(opponent_ensemble.value(), num_rounds, static_cast<int>(args.get_num_episodes()),
				reporter, args.get_fixed_pairs());

		} else
			engine.run(num_rounds, static_cast<int>(args.get_num_episodes()), reporter, args.get_fixed_pairs());

		saver("");
	}
	catch (std::exception& e)
	{
		report_fatal_error(e.what());
	}
}

