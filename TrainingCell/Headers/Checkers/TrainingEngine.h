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
#include "Agent.h"
#include <functional>

#include "RandomAgent.h"

namespace TrainingCell::Checkers
{
	/// <summary>
	/// Functionality to "train" groups of agents following different strategies
	/// </summary>
	class TrainingEngine
	{
		/// <summary>
		/// Collection of pointer to the agents to be trained
		/// </summary>
		std::vector<Agent*> _agent_pointers{};

		/// <summary>
		/// A "random" agent
		/// </summary>
		RandomAgent _random_agent{};

		/// <summary>
		/// Returns collection of integer pairs representing indices of agents in the corresponding collection
		///	Each index appears exactly once (in one of the pairs)
		///	The number of agents must be even, otherwise exception will be thrown
		/// </summary>
		static [[nodiscard]] std::vector<std::array<int, 2>> split_for_pairs(const std::size_t agents_count);

		/// <summary>
		///	Returns collection of agent pointers with even number of elements. The collection is either equal to `_agent_pointers`
		///	(if the latter has even number of elements) or to `_agent_pointers` + `_random_agent`
		/// </summary>
		std::vector<Agent*> GetFixedCollectionOfAgents();

		/// <summary>
		/// Evaluates performance of the given agent by "running" them against "random" agents
		/// Returns a pair of "winning" percentages: the first value in the pair represents
		/// winning percentage when the agent played as a "white" player and the second one represents winning percentage when
		/// the agent played as a "black" player
		/// </summary>
		static std::array<double, 2> evaluate_performance(Agent& agent);
	public:
		/// <summary>
		/// Adds the given agent pointer to the collection of agent pointers
		/// </summary>
		void add_agent(Agent* agent_ptr);

		/// <summary>
		/// Default constructor
		/// </summary>
		TrainingEngine() = default;

		/// <summary>
		/// Constructor
		/// </summary>
		TrainingEngine(const std::vector<Agent*>& agent_pointers);

		/// <summary>
		/// Method to run training
		/// </summary>
		/// <param name="rounds_cnt">Number of rounds to run. After each round agents get re-grouped in pairs</param>
		/// <param name="episodes_cnt">Number of episodes in a round to play</param>
		/// <param name="round_callback">Call-back function that is called after
		/// each round to provide some intermediate information to the caller</param>
		void run(const int rounds_cnt, const int episodes_cnt,
		         const std::function<void(const std::string& time, const std::vector<std::array<double, 2>>& agent_performances)>& round_callback);
	};
}
