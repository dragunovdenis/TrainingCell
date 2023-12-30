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

#pragma once
#include <vector>
#include <array>
#include "../TdLambdaAgent.h"
#include <functional>
#include "msgpack.hpp"

namespace TrainingCell::Checkers
{
	/// <summary>
	/// Functionality to "train" groups of agents following different strategies
	/// </summary>
	class TrainingEngine
	{
	public:
		/// <summary>
		/// Data structure to contain performance information
		/// </summary>
		struct PerformanceRec
		{
			/// <summary>
			/// Round the record corresponds to
			/// </summary>
			int round{};

			/// <summary>
			/// Performance of an agent when it plays for "whites"
			/// </summary>
			double perf_white{};

			/// <summary>
			/// Performance of an agent when it plays for "blacks"
			/// </summary>
			double perf_black{};

			/// <summary>
			/// Percentage of "draw" games during the training in the current round
			/// </summary>
			double draws{};

			/// <summary>
			/// Returns score
			/// </summary>
			[[nodiscard]] double get_score() const;

			/// <summary>
			/// Message-pack stuff
			/// </summary>
			MSGPACK_DEFINE(round, perf_white, perf_black, draws);
		};

	private:
		/// <summary>
		/// Collection of pointer to the agents to be trained
		/// </summary>
		std::vector<TdLambdaAgent*> _agent_pointers{};

		/// <summary>
		/// Returns collection of integer pairs representing indices of agents in the corresponding collection
		///	Each index appears exactly once (in one of the pairs)
		///	The number of agents must be even, otherwise exception will be thrown
		/// </summary>
		static std::vector<std::array<int, 2>> split_for_pairs(const std::size_t agents_count, const bool fixed_pairs);

		/// <summary>
		/// Evaluates performance of the given agent by "running" them against "random" agents
		/// Returns a pair of "winning" percentages: the first value in the pair represents
		/// winning percentage when the agent played as a "white" player and the second one represents winning percentage when
		/// the agent played as a "black" player
		/// </summary>
		static PerformanceRec evaluate_performance(const TdLambdaAgent& agent, const int episodes_to_play,
			const int round_id, const double draw_percentage);
	public:

		/// <summary>
		/// Adds the given agent pointer to the collection of agent pointers
		/// Returns index of the added agent in the collection of agents
		/// </summary>
		std::size_t add_agent(TdLambdaAgent* agent_ptr);

		/// <summary>
		/// Default constructor
		/// </summary>
		TrainingEngine() = default;

		/// <summary>
		/// Due to the collection of pointers we would be better off prohibiting copying.
		/// </summary>
		TrainingEngine(const TrainingEngine&) = delete;

		/// <summary>
		/// Constructor
		/// </summary>
		TrainingEngine(const std::vector<TdLambdaAgent*>& agent_pointers);

		/// <summary>
		/// Method to run training
		/// </summary>
		/// <param name="rounds_cnt">Number of rounds to run. After each round agents get re-grouped in pairs</param>
		/// <param name="episodes_cnt">Number of episodes in a round to play</param>
		/// <param name="round_callback">Call-back function that is called after
		/// each round to provide some intermediate information to the caller</param>
		/// <param name="fixed_pairs">If "true" training pairs are fixed stale during all the training</param>
		/// <param name="test_episodes">Number of episodes to run when evaluating performance of trained agents</param>
		void run(const int rounds_cnt, const int episodes_cnt,
		         const std::function<void(const long long& time_per_round_ms,
					 const std::vector<PerformanceRec>& agent_performances)>& round_callback,
			const bool fixed_pairs, const int test_episodes = 1000) const;

		/// <summary>
		/// Method to run auto-training
		/// </summary>
		/// <param name="rounds_cnt">Number of rounds to run</param>
		/// <param name="episodes_cnt">Number of episodes in a round to play</param>
		/// <param name="round_callback">Call-back function that is called after
		/// each round to provide some intermediate information to the caller</param>
		/// <param name="test_episodes">Number of episodes to run when evaluating performance of trained agents</param>
		void run_auto(const int rounds_cnt, const int episodes_cnt,
			const std::function<void(const long long& time_per_round_ms,
				const std::vector<PerformanceRec>& agent_performances)>& round_callback,
			const int test_episodes = 1000) const;
	};
}
