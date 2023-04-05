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
#include <iostream>

using namespace TrainingCell::Checkers;

/// <summary>
/// Prints to console while taking care about flushing the buffer
/// </summary>
void static print_to_console(const std::string& message, const bool endl)
{
	std::cout << message;
	if (endl)
		std::cout << std::endl;

	std::cout.flush();
	Sleep(10);
}

void report_fatal_error(const std::string& message)
{
	print_to_console(message, true);
	print_to_console("Press any key to exit", true);
	static_cast<void>(std::getchar());
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
/// Tries to load training state from the given file and returns "true" in case of success; Does not write anything to console
/// </summary>
bool try_load_state_silent(const std::filesystem::path& state_path, Training::TrainingState& state)
{
	try
	{
		state = Training::TrainingState::load_from_file(state_path);
	}
	catch (...)
	{
		state.reset();
		return false;
	}

	return true;
}

/// <summary>
/// Tries to load training state from the given file on disk, or construct state from the given script on disk
/// Returns "true" if succeeded
/// </summary>
bool try_load_or_construct_sate(const std::filesystem::path& source_path, Training::TrainingState& state)
{
	if (try_load_state_silent(source_path, state))
	{
		//we need to reset everything related to training except the agents
		state.reset(/*keep agents*/ true);
		return true;
	}

	try
	{
		//try to load as script
		state = Training::TrainingState(source_path);
	}
	catch (...)
	{
		state.reset();
		return false;
	}

	return true;
}

/// <summary>
/// Writes a "horizontal separator" to console
/// </summary>
static void horizontal_console_separator()
{
	print_to_console("=========================================", true);
}

/// <summary>
/// Returns true if user wants to continue
/// </summary>
bool decision_prompt(const std::string& prompt_string = "Continue? (y/n):")
{
	print_to_console(prompt_string, false);

	char decision;
	std::cin.get(decision);
	std::cin.ignore();
	horizontal_console_separator();

	return decision == 'y';
}

/// <summary>
/// Tries to load training state from the given file and returns "true" in case of success
/// </summary>
bool try_load_state(const std::filesystem::path& state_path, Training::TrainingState& state)
{
	if (try_load_state_silent(state_path, state))
	{
		horizontal_console_separator();
		print_to_console("State dump from round " + std::to_string(state.get_round_id()) + " was successfully loaded", true);

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

int main(int argc, char** argv)
{
	try
	{
		const Training::Arguments args(argc, argv);

		std::cout << args.to_string() << std::endl;

		Training::TrainingState state;
		if (!try_load_state(args.get_state_dump_path(), state))
		{
			if (!try_load_or_construct_sate(args.get_source_path(), state))
				throw std::exception(std::format("Failed to load/construct state from the given source: {}",
					args.get_source_path().string()).c_str());

			print_to_console("State was loaded/constructed from source: ", true);
			print_to_console(state.get_agents_script(), true);
			if (!decision_prompt())
				return 0;

			if (std::filesystem::is_regular_file(args.get_adjustments_path()))
			{
				state.adjust_agent_hyper_parameters(args.get_adjustments_path());
				horizontal_console_separator();
				print_to_console("State was adjusted: ", true);
				print_to_console(state.get_agents_script(), true);
				if (!decision_prompt())
					return 0;
			}
		}

		std::vector<Agent*> agent_pointers;
		agent_pointers.reserve(state.agents_count());
		for (auto agent_id = 0ull; agent_id < state.agents_count(); ++agent_id)
			agent_pointers.push_back(&state[agent_id]);

		TrainingEngine engine(agent_pointers);
		auto round_time_sum = 0ll; // to calculate average round time
		std::queue<long long> round_time_queue;

		const auto max_round_id = static_cast<int>(args.get_num_rounds());
		const auto num_rounds_left = static_cast<int>(args.get_num_rounds() - state.get_round_id());

		auto opponent_ensemble = try_load_ensemble(args.get_opponent_ensemble_path());

		const auto saver = [&state, &args](const std::string& sub_folder_name)
		{
			const auto directory_path = !sub_folder_name.empty()? args.get_output_folder() / sub_folder_name : args.get_output_folder();
			std::filesystem::create_directories(directory_path);

			TdlEnsembleAgent ensemble;
			ensemble.set_name("Ensemble_R_" + std::to_string(state.get_round_id()));
			for (auto agent_id = 0ull; agent_id < state.agents_count(); ++agent_id)
			{
				const auto& agent = state[agent_id];
				const auto agent_file_path = directory_path / (agent.get_name() + "--" + agent.get_id() + ".tda");
				print_to_console(agent_file_path.string(), true);
				agent.save_to_file(agent_file_path);
				ensemble.add(agent);
			}
			horizontal_console_separator();

			ensemble.save_to_file(directory_path / (ensemble.get_name() + ".ena"));

			state.save_to_file(directory_path / args.get_state_dump_file_name(), true);//Save final of state
			state.save_performance_report(directory_path / "Performance_report.txt");//Save performance report
		};

		const auto reporter = [&state, max_round_id, &round_time_sum, &round_time_queue, &saver, &args]
		(const long long round_time_ms, const auto& performance)
		{
			const auto rounds_counter = state.increment_round();
			round_time_queue.push(round_time_ms);
			round_time_sum += round_time_ms;
			print_to_console("Round " + std::to_string(rounds_counter) + " time: " +
				DeepLearning::Utils::milliseconds_to_dd_hh_mm_ss_string(round_time_ms), true);
			if (max_round_id != rounds_counter)
				print_to_console("Expected time to finish training : " +
				DeepLearning::Utils::milliseconds_to_dd_hh_mm_ss_string((static_cast<long long>(max_round_id) -
					rounds_counter) * round_time_sum / round_time_queue.size()), true);

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
				print_to_console(state[agent_id].get_name() + " (" + state[agent_id].get_id() + ") performance : "
					+ std::to_string(perf_item[0]) + "/" + std::to_string(perf_item[1]), true);

				average_performance_white += perf_item[0];
				average_performance_black += perf_item[1];
			}

			average_performance_white /= performance.size();
			average_performance_black /= performance.size();
			print_to_console("Average performance : " + std::to_string(average_performance_white) + "/" +
				std::to_string(average_performance_black), true);

			horizontal_console_separator();

			state.add_performance_record(rounds_counter, average_performance_white, average_performance_black);

			if (args.get_dump_rounds() != 0 && (rounds_counter % args.get_dump_rounds() == 0))
				state.save_to_file(args.get_state_dump_path(), true);

			if (args.get_save_rounds() != 0 && (rounds_counter % args.get_save_rounds() == 0))
				saver(std::format("Round_{}", rounds_counter));
		};

		if (opponent_ensemble.has_value())
		{
			horizontal_console_separator();
			print_to_console("Training against loaded ensemble", true);
			horizontal_console_separator();
			opponent_ensemble.value().set_single_agent_mode(true);
			engine.run(opponent_ensemble.value(), num_rounds_left, static_cast<int>(args.get_num_episodes()),
				reporter, args.get_fixed_pairs());

		} else
			engine.run(num_rounds_left, static_cast<int>(args.get_num_episodes()), reporter, args.get_fixed_pairs());

		saver("");
	}
	catch (std::exception& e)
	{
		report_fatal_error(e.what());
	}
}

