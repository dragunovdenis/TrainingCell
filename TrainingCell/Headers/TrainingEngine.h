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
#include "TdLambdaAgent.h"
#include <functional>
#include "msgpack.hpp"

namespace TrainingCell
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
			/// Round the record corresponds to.
			/// </summary>
			int round{};

			/// <summary>
			/// Performance of an agent when it plays for "whites".
			/// </summary>
			double perf_white{};

			/// <summary>
			/// Performance of "random" agent played for "blacks".
			/// </summary>
			double losses_white{};

			/// <summary>
			/// Performance of an agent when it plays for "blacks".
			/// </summary>
			double perf_black{};

			/// <summary>
			/// Performance of "random" agent player for "whites".
			/// </summary>
			double losses_black{};

			/// <summary>
			/// Percentage of "draw" games during the training in the current round.
			/// </summary>
			double draws{};

			/// <summary>
			/// Number of training episodes in the current round.
			/// </summary>
			int training_episodes{};

			/// <summary>
			/// Number of performance evaluation episodes in the current round.
			/// </summary>
			int test_episodes{};

			/// <summary>
			/// Returns score.
			/// </summary>
			[[nodiscard]] double get_score() const;

			/// <summary>
			/// Returns string representation of the record.
			/// </summary>
			/// <returns></returns>
			[[nodiscard]] std::string to_string(const bool extended = false) const;

			/// <summary>
			/// Message-pack stuff.
			/// </summary>
			MSGPACK_DEFINE(round, perf_white, perf_black, draws, losses_white, losses_black, training_episodes, test_episodes);
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
		static PerformanceRec evaluate_performance(TdLambdaAgent& agent, const int training_episodes, const int episodes_to_play,
			const int round_id, const double draw_percentage);

		/// <summary>
		/// Maximal number of consequent moves without a capture that will be qualified as a draw.
		/// </summary>
		static constexpr int _max_moves_without_capture = 50;
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
		/// <param name="round_id_start">Id of the round to start with.</param>
		/// <param name="max_round_id">ID of the maximal round plus one. After each round agents get re-grouped in pairs</param>
		/// <param name="training_episodes_cnt">Number of episodes in a round to play</param>
		/// <param name="round_callback">Call-back function that is called after
		/// each round to provide some intermediate information to the caller</param>
		/// <param name="fixed_pairs">If "true" training pairs are fixed stale during all the training</param>
		/// <param name="test_episodes_cnt">Number of episodes to run when evaluating performance of trained agents</param>
		/// <param name="smart_training">If "true" training on non-draw episodes will be used.</param>
		/// <param name="remove_outliers">If "true" agents with low score will be substituted
		/// with copies of best-score agents (on a round basis).</param>
		void run(const int round_id_start, const int max_round_id, const int training_episodes_cnt,
		         const std::function<void(const long long& time_per_round_ms,
					 const std::vector<PerformanceRec>& agent_performances)>& round_callback,
			const bool fixed_pairs, const int test_episodes_cnt = 1000,
			const bool smart_training = false, const bool remove_outliers = false) const;

		/// <summary>
		/// Method to run auto-training
		/// </summary>
		/// <param name="round_id_start">Id of the round to start with.</param>
		/// <param name="max_round_id">ID of the maximal round plus one.</param>
		/// <param name="training_episodes_cnt">Number of episodes in a round to play</param>
		/// <param name="round_callback">Call-back function that is called after
		/// each round to provide some intermediate information to the caller</param>
		/// <param name="test_episodes_cnt">Number of episodes to run when evaluating performance of trained agents</param>
		/// <param name="smart_training">If "true" training on non-draw episodes will be used.</param>
		/// <param name="remove_outliers">If "true" agents with low score will be substituted
		/// with copies of best-score agents (on a round basis).</param>
		void run_auto(const int round_id_start, const int max_round_id, const int training_episodes_cnt,
			const std::function<void(const long long& time_per_round_ms,
				const std::vector<PerformanceRec>& agent_performances)>& round_callback,
			const int test_episodes_cnt = 1000, const bool smart_training = false, const bool remove_outliers = false) const;
	};
}
