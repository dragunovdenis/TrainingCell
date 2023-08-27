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
#include <filesystem>
#include <msgpack.hpp>
#include "Headers/TdLambdaAgent.h"
#include "Headers/Checkers/TrainingEngine.h"

namespace Training
{
	/// <summary>
	/// Data structure to contain state of training
	///	Is supposed to be used for backup purposes
	/// </summary>
	class TrainingState
	{
		/// <summary>
		/// Collections of agents that undergo training
		/// </summary>
		std::vector<TrainingCell::TdLambdaAgent> _agents{};

		/// <summary>
		/// Collection of averaged performance records (presumably one per round)
		/// </summary>
		std::vector<TrainingCell::Checkers::TrainingEngine::PerformanceRec> _performances{};

		/// <summary>
		/// Collections of agents that have been copied from the regular collection of agents on the "registration" of "best score"
		/// </summary>
		std::vector<TrainingCell::TdLambdaAgent> _agents_best_performance{};

		/// <summary>
		/// Best performance records for each agent in the collection of "best score" agents
		/// </summary>
		std::vector<TrainingCell::Checkers::TrainingEngine::PerformanceRec> _best_performance {};

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
		void assign_agents_from_script_file(const std::filesystem::path& script_file_path);

		/// <summary>
		/// If the given score if higher than the "best score", the corresponding collection of "best agents" and the "best score"
		/// get updated accordingly. 
		/// </summary>
		void register_performance(const std::vector<TrainingCell::Checkers::TrainingEngine::PerformanceRec>& performance);
	public:
		/// <summary>
		/// Constructs collection of agents from the given "script-string"
		/// </summary>
		void assign_agents_from_script(const std::string& script_str);

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
		void add_agent(const TrainingCell::TdLambdaAgent& agent);

		/// <summary>
		/// Adds performance record to the corresponding collection
		/// Returns average performance record
		/// </summary>
		TrainingCell::Checkers::TrainingEngine::PerformanceRec add_performance_record(
			const std::vector<TrainingCell::Checkers::TrainingEngine::PerformanceRec>& performance);

		/// <summary>
		/// Sub-script operator
		/// </summary>
		TrainingCell::TdLambdaAgent& operator [](const std::size_t id);

		/// <summary>
		/// Sub-script operator (constant version)
		/// </summary>
		const TrainingCell::TdLambdaAgent& operator [](const std::size_t id) const;

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
		/// Constructs an ensemble from the current collection of agents and saves it to the given folder
		/// Returns full path to the saved file
		/// </summary>
		[[nodiscard]] std::filesystem::path save_current_ensemble(const std::filesystem::path& folder_path, const std::string& tag) const;

		/// <summary>
		/// Constructs an ensemble from the current collection of "best score" agents and saves it to the given folder
		/// Returns full path to the saved file
		/// </summary>
		[[nodiscard]] std::filesystem::path save_best_score_ensemble(const std::filesystem::path& folder_path, const std::string& tag) const;

		/// <summary>
		/// Read-only access to the collection of performances
		/// </summary>
		[[nodiscard]] const std::vector<TrainingCell::Checkers::TrainingEngine::PerformanceRec>& get_performances() const;

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
		/// Updates the reward discounts of all the agents by the given value
		/// </summary>
		void set_discount(const double& discount);

		/// <summary>
		/// Updates "lambda" parameter of all the agents by the given value
		/// </summary>
		void set_lambda(const double& lambda);

		/// <summary>
		/// Updates "learning rate" parameter of all the agents by the given value
		/// </summary>
		void set_learning_rate(const double& l_rate);

		/// <summary>
		/// Updates "exploration" parameter of all the agents by the given value
		/// </summary>
		void set_exploration(const double& exploration);

		/// <summary>
		/// Message-pack stuff
		/// </summary>
		MSGPACK_DEFINE(_round_id, _agents, _performances, _best_performance, _agents_best_performance);
	};
	
}

