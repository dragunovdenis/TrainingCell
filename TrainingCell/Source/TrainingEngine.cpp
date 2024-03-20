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

#include "../Headers/TrainingEngine.h"
#include "../Headers/RandomAgent.h"
#include "../../DeepLearning/DeepLearning/Utilities.h"
#include "../../DeepLearning/DeepLearning/StopWatch.h"
#include "../Headers/Board.h"
#include "../Headers/StateTypeController.h"
#include <numeric>
#include <ppl.h>

namespace TrainingCell
{
	std::size_t TrainingEngine::add_agent(TdLambdaAgent* agent_ptr)
	{
		if (agent_ptr == nullptr)
			throw std::exception("Invalid agent pointer");

		_agent_pointers.push_back(agent_ptr);

		return _agent_pointers.size() - 1;
	}

	TrainingEngine::TrainingEngine(const std::vector<TdLambdaAgent*>& agent_pointers) : _agent_pointers(agent_pointers)
	{}

	std::vector<std::array<int, 2>> TrainingEngine::split_for_pairs(const std::size_t agents_count, const bool fixed_pairs)
	{
		if (agents_count == 0 || agents_count % 2 == 1)
			throw std::exception("Invalid collection of agents. Can't build pairs");

		std::vector<int> ids(agents_count);
		std::iota(ids.begin(), ids.end(), 0);

		const auto pairs_count = agents_count / 2;
		std::vector<std::array<int, 2>> result(pairs_count);
		for (auto& pair : result)
		{
			for (int& agent_id : pair)
			{
				const auto index_id = fixed_pairs ? 0 : DeepLearning::Utils::get_random_int(0, static_cast<int>(ids.size()) - 1);
				agent_id = ids[index_id];
				ids.erase(ids.begin() + index_id);
			}
		}

		return result;
	}

	/// <summary>
	/// Returns ID of the agent with best performance as well as a collection of IDs of the agents with too low performance (outliers).
	/// </summary>
	int find_best_score_agent_id(const std::vector<TrainingEngine::PerformanceRec>& performance_scores, std::vector<int>& outlier_ids)
	{
		const auto best_item_id = static_cast<int>(std::ranges::max_element(performance_scores, [](const auto& x, const auto& y)
			{return x.get_score() < y.get_score(); }) - performance_scores.begin());

		const auto average_score = std::accumulate(performance_scores.begin(), performance_scores.end(), 0.0,
			[](const double& sum, const auto& x) { return sum + x.get_score(); }) / static_cast<double>(performance_scores.size());

		outlier_ids.clear();

		for (auto score_item_id = 0ull; score_item_id < performance_scores.size(); ++score_item_id)
		{
			if (performance_scores[score_item_id].get_score() < 0.8 * average_score)
				outlier_ids.push_back(static_cast<int>(score_item_id));
		}

		return best_item_id;
	}

	/// <summary>
	/// Substitutes agents with poor performance with a copy of a best-score agent.
	/// </summary>
	void remove_low_score_outliers(const std::vector<TrainingEngine::PerformanceRec>& performance_scores, const std::vector<TdLambdaAgent*>& agent_pointers)
	{
		std::vector<int> outlier_ids;
		const auto best_agent_id = find_best_score_agent_id(performance_scores, outlier_ids);

		for (const auto outlier_agent_id : outlier_ids)
			*agent_pointers[outlier_agent_id] = TdLambdaAgent(*agent_pointers[best_agent_id]);
	}

	TrainingEngine::PerformanceRec TrainingEngine::evaluate_performance(TdLambdaAgent& agent, const int training_episodes, const int episodes_to_play,
		const int round_id, const double draw_percentage)
	{
		agent.set_performance_evaluation_mode(true);

		const auto factor = 1.0 / episodes_to_play;
		RandomAgent random_agent{};

		Board board(&agent, &random_agent);
		const auto stats0 = board.play(episodes_to_play, *StateTypeController::get_start_seed(agent.get_state_type_id()), _max_moves_without_capture);
		const auto white_wins = stats0.whites_win_count() * factor;
		const auto white_losses = stats0.blacks_win_count() * factor;

		board.swap_agents();
		const auto stats1 = board.play(episodes_to_play, *StateTypeController::get_start_seed(agent.get_state_type_id()), _max_moves_without_capture);
		const auto black_wins = stats1.blacks_win_count() * factor;
		const auto black_losses = stats1.whites_win_count() * factor;

		agent.set_performance_evaluation_mode(false);

		return PerformanceRec{ round_id, white_wins, white_losses, black_wins,
			black_losses, draw_percentage, training_episodes, episodes_to_play };
	}

	void TrainingEngine::run(const int round_id_start, const int max_round_id, const int training_episodes_cnt,
		const std::function<void(const long long& time_per_round_ms,
			const std::vector<PerformanceRec>& agent_performances)>& round_callback,
		const bool fixed_pairs, const int test_episodes_cnt, const bool smart_training,
		const bool remove_outliers) const
	{
		if (_agent_pointers.empty() || _agent_pointers.size() % 2 == 1)
			throw std::exception("Collection of agents must be nonempty and contain an even number of elements");

		std::vector<PerformanceRec> performance_scores(_agent_pointers.size());
		auto pairs = split_for_pairs(_agent_pointers.size(), fixed_pairs);

		for (auto round_id = round_id_start; round_id < max_round_id; round_id++)
		{
			DeepLearning::StopWatch sw;
			Concurrency::parallel_for(0ull, pairs.size(),
				[this, &performance_scores, training_episodes_cnt, &pairs, test_episodes_cnt, round_id, smart_training](const auto& pair_id)
				{
					const auto white_agent_id = pairs[pair_id][0];
					auto agent_white_ptr = _agent_pointers[white_agent_id];

					const auto black_agent_id = pairs[pair_id][1];
					auto agent_black_ptr = _agent_pointers[black_agent_id];

					auto state_seed_ptr = StateTypeController::get_start_seed(agent_white_ptr->get_state_type_id());
					const auto stats = smart_training ?
						Board::train(agent_white_ptr, agent_black_ptr, training_episodes_cnt, *state_seed_ptr, _max_moves_without_capture) :
						Board::play(agent_white_ptr, agent_black_ptr, training_episodes_cnt, *state_seed_ptr, _max_moves_without_capture);
					const auto draw_percentage = (training_episodes_cnt - stats.blacks_win_count() - stats.whites_win_count()) * 1.0 / training_episodes_cnt;

					performance_scores[white_agent_id] = evaluate_performance(*_agent_pointers[white_agent_id], training_episodes_cnt, test_episodes_cnt,
						round_id, draw_percentage);

					performance_scores[black_agent_id] = evaluate_performance(*_agent_pointers[black_agent_id], training_episodes_cnt, test_episodes_cnt,
						round_id, draw_percentage);
				});

			round_callback(sw.elapsed_time_in_milliseconds(), performance_scores);

			if (remove_outliers)
				remove_low_score_outliers(performance_scores, _agent_pointers);

			if (round_id != max_round_id - 1 && !fixed_pairs) //re-generate pairs
				pairs = split_for_pairs(_agent_pointers.size(), fixed_pairs);
		}
	}
 
	void TrainingEngine::run_auto(const int round_id_start, const int max_round_id, const int training_episodes_cnt,
		const std::function<void(const long long& time_per_round_ms, const std::vector<PerformanceRec>&
			agent_performances)>& round_callback, const int test_episodes_cnt, const bool smart_training,
			const bool remove_outliers) const
	{
		if (_agent_pointers.empty())
			throw std::exception("Collection of agents must be nonempty");

		std::vector<PerformanceRec> performance_scores(_agent_pointers.size());

		for (auto round_id = round_id_start; round_id < max_round_id; round_id++)
		{
			DeepLearning::StopWatch sw;
			Concurrency::parallel_for(0ull, _agent_pointers.size(),
				[this, &performance_scores, training_episodes_cnt, test_episodes_cnt, round_id, smart_training](const auto& agent_id)
			{
				const auto agent_ptr = _agent_pointers[agent_id];
			    auto state_seed_ptr = StateTypeController::get_start_seed(agent_ptr->get_state_type_id());
				const auto stats = smart_training ?
					Board::train(agent_ptr, agent_ptr, training_episodes_cnt, *state_seed_ptr, _max_moves_without_capture) :
					Board::play(agent_ptr, agent_ptr, training_episodes_cnt, *state_seed_ptr, _max_moves_without_capture);
				const auto draw_percentage = (training_episodes_cnt - stats.blacks_win_count() - stats.whites_win_count()) * 1.0 / training_episodes_cnt;
				performance_scores[agent_id] = evaluate_performance(*agent_ptr, training_episodes_cnt, test_episodes_cnt, round_id, draw_percentage);
			});

			round_callback(sw.elapsed_time_in_milliseconds(), performance_scores);

			if (remove_outliers)
				remove_low_score_outliers(performance_scores, _agent_pointers);
		}
	}

	double TrainingEngine::PerformanceRec::get_score() const
	{
		return 0.5 * (perf_white + perf_black);
	}

	std::string TrainingEngine::PerformanceRec::to_string(const bool extended) const
	{
		std::string result{};

		if (extended)
		{
			result += std::string("Round: " + std::to_string(round) + "; Training episodes: " +
				std::to_string(training_episodes) + "; Test episodes: " + std::to_string(test_episodes) + "; Performance ");
		}

		result += std::string("w.w./w.l.-b.w/b.l.-d: ")
			+ std::to_string(perf_white) + "/" + std::to_string(losses_white) + "-" +
			std::to_string(perf_black) + "/" + std::to_string(losses_black) + "-" + std::to_string(draws);

		return result;
	}
}
