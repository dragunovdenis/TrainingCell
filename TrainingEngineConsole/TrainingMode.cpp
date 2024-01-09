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

#include "TrainingMode.h"
#include <queue>
#include "ArgumentsTraining.h"
#include "ConsoleUtils.h"
#include "TrainingState.h"
#include "Headers/TrainingEngine.h"
#include "../../DeepLearning/DeepLearning/Utilities.h"
#include "Headers/TdlEnsembleAgent.h"

using namespace TrainingCell;

namespace Training::Modes
{
	/// <summary>
	/// Tries to load training state from the given file on disk, or construct state from the given script on disk
	/// Returns "true" if succeeded
	/// </summary>
	bool try_load_or_construct_sate(const std::filesystem::path& source_path, Training::TrainingState& state)
	{
		if (ConsoleUtils::try_load_state_silent(source_path, state))
		{
			// We need to reset everything related to training except the agents.
			// Add the "latest" performance records as a training record to each agent.
			state.write_training_records();
			state.reset(/*keep agents*/ true);
			return true;
		}

		try
		{
			//try to load as script
			state = TrainingState(source_path);
		}
		catch (...)
		{
			state.reset();
			return false;
		}

		return true;
	}

	/// <summary>
	/// Tries to load ensemble from the given file on disk and returns it in case of success
	/// </summary>
	std::optional<TdlEnsembleAgent> try_load_ensemble(const std::filesystem::path& ensemble_path)
	{
		try
		{
			return TdlEnsembleAgent::load_from_file(ensemble_path);
		}
		catch (...)
		{
			return std::nullopt;
		}
	}

	void run_training(int argc, char** argv)
	{
		const ArgumentsTraining args(argc, argv);
		ConsoleUtils::print_to_console(args.to_string());

		TrainingState state;
		if (!ConsoleUtils::try_load_state(args.get_state_dump_path(), state))
		{
			if (!try_load_or_construct_sate(args.get_source_path(), state))
				throw std::exception(std::format("Failed to load/construct state from the given source: {}",
					args.get_source_path().string()).c_str());

			ConsoleUtils::print_to_console("State was loaded/constructed from source: ");
			ConsoleUtils::print_to_console(state.get_agents_script());
			if (!ConsoleUtils::decision_prompt())
				return;

			if (std::filesystem::is_regular_file(args.get_adjustments_path()))
			{
				state.adjust_agent_hyper_parameters(args.get_adjustments_path());
				ConsoleUtils::horizontal_console_separator();
				ConsoleUtils::print_to_console("State was adjusted: ");
				ConsoleUtils::print_to_console(state.get_agents_script());
				if (!ConsoleUtils::decision_prompt())
					return;
			}
		}

		std::vector<TdLambdaAgent*> agent_pointers;
		agent_pointers.reserve(state.agents_count());
		for (auto agent_id = 0ull; agent_id < state.agents_count(); ++agent_id)
			agent_pointers.push_back(&state[agent_id]);

		TrainingEngine engine(agent_pointers);
		auto round_time_sum = 0ll; // to calculate average round time
		std::queue<long long> round_time_queue;

		const auto max_round_id = static_cast<int>(args.get_num_rounds());

		const auto saver = [&state, &args](const std::string& sub_folder_name)
		{
			const auto directory_path = !sub_folder_name.empty() ? args.get_output_folder() / sub_folder_name : args.get_output_folder();
			std::filesystem::create_directories(directory_path);

			const auto& last_performance_record = *state.get_performances().rbegin();
			const auto ensemble_path = state.save_current_ensemble(directory_path, std::to_string(last_performance_record.get_score()));
			ConsoleUtils::print_to_console("Ensemble was saved to " + ensemble_path.string());

			state.save_to_file(directory_path / args.get_state_dump_file_name(), true);
			state.save_performance_report(directory_path / "Performance_report.txt");

			const auto best_score_path = state.save_best_score_ensemble(directory_path, "best_score");
			ConsoleUtils::print_to_console("Best score ensemble was saved to " + best_score_path.string());
		};

		const auto reporter = [&state, max_round_id, &round_time_sum, &round_time_queue, &saver, &args]
		(const long long round_time_ms, const auto& performance)
		{
			const auto rounds_counter = state.increment_round();
			round_time_queue.push(round_time_ms);
			round_time_sum += round_time_ms;
			ConsoleUtils::print_to_console("Round " + std::to_string(rounds_counter) + " time: " +
			DeepLearning::Utils::milliseconds_to_dd_hh_mm_ss_string(round_time_ms));
			if (max_round_id != rounds_counter)
				ConsoleUtils::print_to_console("Expected time to finish training : " +
					DeepLearning::Utils::milliseconds_to_dd_hh_mm_ss_string((static_cast<long long>(max_round_id) -
						rounds_counter) * round_time_sum / round_time_queue.size()));

			if (round_time_queue.size() >= 5)
			{
				// take into account only the last five round time measurements
				// to calculate the "current average" round time
				round_time_sum -= round_time_queue.front();
				round_time_queue.pop();
			}

			for (auto agent_id = 0ull; agent_id < performance.size(); ++agent_id)
			{
				const auto& perf_item = performance[agent_id];
				ConsoleUtils::print_to_console(state[agent_id].get_name() + " (" + state[agent_id].get_id() + ") performance " + perf_item.to_string());
			}

			const auto average_performance = state.add_performance_record(performance);
			ConsoleUtils::print_to_console("Average performance " + average_performance.to_string());

			ConsoleUtils::horizontal_console_separator();


			if (args.get_dump_rounds() != 0 && (rounds_counter % args.get_dump_rounds() == 0))
				state.save_to_file(args.get_state_dump_path(), true);

			if (args.get_save_rounds() != 0 && (rounds_counter % args.get_save_rounds() == 0))
				saver(std::format("Round_{}", rounds_counter));

			constexpr auto bytes_in_megabyte = 1 << 20;
			ConsoleUtils::print_to_console("Current memory usage (Mb): " + std::to_string(ConsoleUtils::get_phys_mem_usage() / bytes_in_megabyte));
		};

		if (args.get_auto_training())
		{
			engine.run_auto(static_cast<int>(state.get_round_id()), max_round_id,
				static_cast<int>(args.get_num_episodes()), reporter,
			                static_cast<int>(args.get_num_eval_episodes()), args.get_smart_training(), args.get_remove_outliers());
		}
		else
		{
			engine.run(static_cast<int>(state.get_round_id()), max_round_id,
				static_cast<int>(args.get_num_episodes()), reporter,
				args.get_fixed_pairs(), static_cast<int>(args.get_num_eval_episodes()),
				args.get_smart_training(), args.get_remove_outliers());
		}

		saver("");
	}
}
