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
#include "../TrainingCell/Headers/Checkers/TdLambdaAgent.h"
#include <msgpack.hpp>

namespace Training
{
	/// <summary>
	/// Data structure to contain state of training
	///	Is supposed to be used for backup purposes
	/// </summary>
	class TrainingState
	{
		/// <summary>
		/// Data structure to contain performance information
		/// </summary>
		struct PerformanceRec
		{
			/// <summary>
			/// Round the record corresponds to
			/// </summary>
			unsigned int round{};

			/// <summary>
			/// Performance of an agent when it plays for "whites"
			/// </summary>
			double perf_white {};
			
			/// <summary>
			/// Performance of an agent when it plays for "blacks"
			/// </summary>
			double perf_black{};

			/// <summary>
			/// Message-pack stuff
			/// </summary>
			MSGPACK_DEFINE(round, perf_white, perf_black);
		};

		/// <summary>
		/// Collections of agents that undergo training
		/// </summary>
		std::vector<TrainingCell::Checkers::TdLambdaAgent> _agents{};

		/// <summary>
		/// Collection of averaged performance records (presumably one per round)
		/// </summary>
		std::vector<PerformanceRec> _performance{};

		/// <summary>
		/// Index of the current round
		/// </summary>
		unsigned int _round_id{};

		/// <summary>
		/// Saves all the agents in the current state to the given file in "script" format (hyper-parameters of all agents)
		/// </summary>
		void save_agents_script(const std::filesystem::path& script_file_path) const;

		/// <summary>
		/// Constructs collection of agents from the "script" in the given file
		/// </summary>
		void assign_agents_from_script(const std::filesystem::path& script_file_path);
	public:
		/// <summary>
		/// Returns "script" representation of all the agents in the state (one line per agent)
		/// </summary>
		[[nodiscard]] std::string get_agents_script() const;

		/// <summary>
		/// Adjusts hyper-parameters of the agents using "script" in the given file
		/// </summary>
		void adjust_agent_hyper_parameters(const std::filesystem::path& script_file_path);

		/// <summary>
		/// Adds the given agent to the corresponding collection
		/// </summary>
		void add_agent(const TrainingCell::Checkers::TdLambdaAgent& agent);

		/// <summary>
		/// Adds performance record to the corresponding collection
		/// </summary>
		void add_performance_record(const unsigned int round, const double perf_white, const double perf_black);

		/// <summary>
		/// Sub-script operator
		/// </summary>
		TrainingCell::Checkers::TdLambdaAgent& operator [](const std::size_t id);

		/// <summary>
		/// Sub-script operator (constant version)
		/// </summary>
		const TrainingCell::Checkers::TdLambdaAgent& operator [](const std::size_t id) const;

		/// <summary>
		/// Returns number of agents in the corresponding collection
		/// </summary>
		[[nodiscard]] std::size_t agents_count() const;

		/// <summary>
		/// Returns index of the current round
		/// </summary>
		[[nodiscard]] unsigned int get_round_id() const;

		/// <summary>
		/// Increments value of the rounds counter and returns its new value
		/// </summary>
		[[nodiscard]] unsigned int increment_round();

		/// <summary>
		/// Saves state to the given file
		/// </summary>
		/// <param name="file_path">Path to the file where state should be saved</param>
		/// <param name="extended">If "true" an additional text file with the description
		/// of all the agents will be saved along with the state. The description file will
		/// have the same name as state file and ".txt" extension</param>
		void save_to_file(const std::filesystem::path& file_path, const bool extended) const;

		/// <summary>
		/// Saves performance report to the given file
		/// </summary>
		void save_performance_report(const std::filesystem::path& file_path) const;

		/// <summary>
		/// Tries to load state from the given file throws exception if failed
		/// </summary>
		static TrainingState load_from_file(const std::filesystem::path& file_path);

		/// <summary>
		/// Default constructor
		/// </summary>
		TrainingState() = default;

		/// <summary>
		/// Construction from a script-file
		/// </summary>
		TrainingState(const std::filesystem::path& agent_script_file_path);

		/// <summary>
		/// Reset state of the object
		/// </summary>
		void reset(const bool keep_agents = false);

		/// <summary>
		/// Message-pack stuff
		/// </summary>
		MSGPACK_DEFINE(_round_id, _agents, _performance);
	};
	
}

