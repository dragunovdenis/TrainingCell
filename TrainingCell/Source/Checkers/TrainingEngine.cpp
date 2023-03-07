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
#include "../../../DeepLearning/DeepLearning/Utilities.h"
#include "../../Headers/Checkers/Board.h"
#include <numeric>
#include <ppl.h>
#include <chrono>

namespace TrainingCell::Checkers
{
	void TrainingEngine::add_agent(Agent* agent_ptr)
	{
		if (agent_ptr == nullptr)
			throw std::exception("Invalid agent pointer");

		_agent_pointers.push_back(agent_ptr);
	}

	TrainingEngine::TrainingEngine(const std::vector<Agent*>& agent_pointers) : _agent_pointers(agent_pointers)
	{}

	std::vector<std::array<int, 2>> TrainingEngine::split_for_pairs(const std::size_t agents_count)
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
				const auto index_id = DeepLearning::Utils::get_random_int(0, static_cast<int>(ids.size()) - 1);
				agent_id = ids[index_id];
				ids.erase(ids.begin() + index_id);
			}
		}

		return result;
	}

	std::vector<Agent*> TrainingEngine::GetFixedCollectionOfAgents()
	{
		if (_agent_pointers.size() % 2 == 0)
			return _agent_pointers;

		auto result = _agent_pointers;
		result.push_back(&_random_agent);
		return result;
	}

	std::array<double, 2> TrainingEngine::evaluate_performance(Agent& agent)
	{
		constexpr int episodes_to_play = 1000;
		constexpr auto factor = 1.0 / episodes_to_play;
		RandomAgent random_agent{};

		Board board(&agent, &random_agent);
		board.play(episodes_to_play);
		const auto white_wins = board.get_whites_wins() * factor;

		board.swap_agents();
		board.play(episodes_to_play);
		const auto black_wins = board.get_blacks_wins() * factor;

		return { white_wins, black_wins };
	}

	void TrainingEngine::run(const int rounds_cnt, const int episodes_cnt,
		const std::function<void(const std::string& time, const std::vector<std::array<double, 2>>& agent_performances)>& round_callback)
	{
		const auto agents = GetFixedCollectionOfAgents();

		for (auto round_id = 0; round_id < rounds_cnt; round_id++)
		{
			const auto pairs = split_for_pairs(agents.size());
			Concurrency::parallel_for(0ull, pairs.size(), [&agents, episodes_cnt, &pairs](const auto& pair_id)
				{
					auto agent_white_ptr = agents[pairs[pair_id][0]];
					auto agent_black_ptr = agents[pairs[pair_id][1]];

					Board board(agent_white_ptr, agent_black_ptr);
					board.play(episodes_cnt);
				});

			std::vector<std::array<double, 2>> performance_scores(_agent_pointers.size());
			Concurrency::parallel_for(0ull, _agent_pointers.size(), [this, &performance_scores](const auto agent_id)
				{
					performance_scores[agent_id] = evaluate_performance(*_agent_pointers[agent_id]);
				});

			round_callback(std::string("Round ") + std::to_string(round_id), performance_scores);
		}
	}
}
