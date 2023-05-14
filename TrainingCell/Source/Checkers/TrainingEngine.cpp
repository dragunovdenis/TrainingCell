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

#include "../../Headers/Checkers/TrainingEngine.h"
#include "../../Headers/Checkers/RandomAgent.h"
#include "../../../DeepLearning/DeepLearning/Utilities.h"
#include "../../../DeepLearning/DeepLearning/StopWatch.h"
#include "../../Headers/Checkers/Board.h"
#include <numeric>
#include <ppl.h>
#include <format>

namespace TrainingCell::Checkers
{
	std::size_t TrainingEngine::add_agent(Agent* agent_ptr)
	{
		if (agent_ptr == nullptr)
			throw std::exception("Invalid agent pointer");

		_agent_pointers.push_back(agent_ptr);

		return _agent_pointers.size() - 1;
	}

	TrainingEngine::TrainingEngine(const std::vector<Agent*>& agent_pointers) : _agent_pointers(agent_pointers)
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

	TrainingEngine::PerformanceRec TrainingEngine::evaluate_performance(const Agent& agent, const int episodes_to_play,
		const int round_id, const double draw_percentage)
	{
		const auto agent_clone = agent.clone();
		agent_clone->set_training_mode(false);

		const auto factor = 1.0 / episodes_to_play;
		RandomAgent random_agent{};

		Board board(agent_clone.get(), &random_agent);
		board.play(episodes_to_play);
		const auto white_wins = board.get_whites_wins() * factor;

		board.swap_agents();
		board.play(episodes_to_play);
		const auto black_wins = board.get_blacks_wins() * factor;

		return  PerformanceRec{ round_id, white_wins, black_wins, draw_percentage };
	}

	void TrainingEngine::run(const int rounds_cnt, const int episodes_cnt,
		const std::function<void(const long long& time_per_round_ms,
			const std::vector<PerformanceRec>& agent_performances)>& round_callback,
		const bool fixed_pairs, const int test_episodes) const
	{
		if (_agent_pointers.empty() || _agent_pointers.size() % 2 == 1)
			throw std::exception("Collection of agents must be nonempty and contain an even number of elements");

		std::vector<PerformanceRec> performance_scores(_agent_pointers.size());
		auto pairs = split_for_pairs(_agent_pointers.size(), fixed_pairs);

		for (auto round_id = 0; round_id < rounds_cnt; round_id++)
		{
			DeepLearning::StopWatch sw;
			Concurrency::parallel_for(0ull, pairs.size(),
				[this, &performance_scores, episodes_cnt, &pairs, test_episodes, round_id](const auto& pair_id)
				{
					const auto white_agent_id = pairs[pair_id][0];
					auto agent_white_ptr = _agent_pointers[white_agent_id];

					const auto black_agent_id = pairs[pair_id][1];
					auto agent_black_ptr = _agent_pointers[black_agent_id];

					Board board(agent_white_ptr, agent_black_ptr);
					board.play(episodes_cnt);

					const auto draw_percentage = (episodes_cnt - board.get_blacks_wins() - board.get_whites_wins()) * 1.0 / episodes_cnt;

					performance_scores[white_agent_id] = evaluate_performance(*_agent_pointers[white_agent_id], test_episodes,
						round_id, draw_percentage);

					performance_scores[black_agent_id] = evaluate_performance(*_agent_pointers[black_agent_id], test_episodes,
						round_id, draw_percentage);
				});

			round_callback(sw.elapsed_time_in_milliseconds(), performance_scores);

			if (round_id != rounds_cnt - 1 && !fixed_pairs) //re-generate pairs
				pairs = split_for_pairs(_agent_pointers.size(), fixed_pairs);
		}
	}

	double TrainingEngine::PerformanceRec::get_score() const
	{
		return 0.5 * (perf_white + perf_black);
	}
}
