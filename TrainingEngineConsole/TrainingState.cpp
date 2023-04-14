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

#include "TrainingState.h"
#include "../DeepLearning/DeepLearning/MsgPackUtils.h"
#include <fstream>
#include <sstream>
#include <format>
#include "../../DeepLearning/DeepLearning/Utilities.h"

namespace Training
{
	void TrainingState::add_agent(const TrainingCell::Checkers::TdLambdaAgent& agent)
	{
		_agents.push_back(agent);
	}

	void TrainingState::add_performance_record(const unsigned round, const double perf_white, const double perf_black)
	{
		_performance.emplace_back(PerformanceRec{ round, perf_white, perf_black });
	}

	TrainingCell::Checkers::TdLambdaAgent& TrainingState::operator[](const std::size_t id)
	{
		if (id >= _agents.size())
			throw std::exception("Index out of bounds");

		return _agents[id];
	}

	const TrainingCell::Checkers::TdLambdaAgent& TrainingState::operator[](const std::size_t id) const
	{
		if (id >= _agents.size())
			throw std::exception("Index out of bounds");

		return _agents[id];
	}

	std::size_t TrainingState::agents_count() const
	{
		return _agents.size();
	}

	unsigned int TrainingState::get_round_id() const
	{
		return _round_id;
	}

	unsigned int TrainingState::increment_round()
	{
		return ++_round_id;
	}

	void TrainingState::save_agents_script(const std::filesystem::path& script_file_path) const
	{
		std::ofstream file(script_file_path);

		if (!file)
			throw std::exception("Failed to create file");

		file << get_agents_script();
		file.close();
	}

	std::string TrainingState::get_agents_script() const
	{
		std::stringstream ss;

		for (const auto& agent : _agents)
			ss << "{" << agent.to_script() << "}" << std::endl;

		return ss.str();
	}

	/// <summary>
	/// Splits the given script onto pairs consisting of an agent scripts and an
	/// integer number, defining number of agents the script is associated with
	/// </summary>
	std::vector<std::pair<std::string, int>> parse_script(const std::string& script)
	{
		auto script_copy = script;
		std::vector<std::pair<std::string, int>> result;

		do
		{
			auto aggregate_agent_script = DeepLearning::Utils::extract_balanced_sub_string(script_copy, '{', '}');
			if (aggregate_agent_script.empty())
				break;

			const auto agent_script = DeepLearning::Utils::extract_balanced_sub_string(aggregate_agent_script, '{', '}', true);
			const auto factor_v = DeepLearning::Utils::parse_scalars<int>(aggregate_agent_script);
			if (factor_v.size() > 1)
				throw std::exception("Unexpected syntax in the script");

			const auto clones_count = factor_v.empty() ? 1 : factor_v[0];

			result.emplace_back(agent_script, clones_count);

		} while (true);

		return result;
	}

	void TrainingState::assign_agents_from_script_file(const std::filesystem::path& script_file_path)
	{
		const auto script = DeepLearning::Utils::read_all_text(script_file_path);
		assign_agents_from_script(script);
	}

	void TrainingState::assign_agents_from_script(const std::string& script_str)
	{
		const auto script_collection = parse_script(script_str);

		_agents.clear();

		for (const auto& script_pair : script_collection)
		{
			for (auto clone_id = 0; clone_id < script_pair.second; ++clone_id)
			{
				_agents.emplace_back(script_pair.first);
				_agents.rbegin()->set_name(_agents.rbegin()->get_name() + "-" + std::to_string(clone_id));
			}
		}
	}

	void TrainingState::adjust_agent_hyper_parameters(const std::filesystem::path& script_file_path)
	{
		const auto script = DeepLearning::Utils::read_all_text(script_file_path);
		const auto script_collection = parse_script(script);
		auto agent_id = 0ull;

		for (const auto& script_pair : script_collection)
		{
			for (auto clone_id = 0; clone_id < script_pair.second; ++clone_id)
				_agents[agent_id++].assign_hyperparams(script_pair.first);
		}

		if (agent_id < _agents.size())
			throw std::exception("Not all agents have been adjusted");
	}

	void TrainingState::save_to_file(const std::filesystem::path& file_path, const bool extended) const
	{
		DeepLearning::MsgPack::save_to_file(*this, file_path);

		if (extended)
		{
			if (file_path.extension() == ".txt")
				throw std::exception("State file cannot have `.txt` extension");

			auto description_file_path = file_path;
			description_file_path.replace_extension(".txt");
			save_agents_script(description_file_path);
		}
	}

	void TrainingState::save_performance_report(const std::filesystem::path& file_path) const
	{
		std::ofstream file(file_path);

		if (!file)
			throw std::exception(std::format("Cant create file: {}", file_path.string()).c_str());

		file << std::format("{:10} {:14} {:14}", "Round", "White Score", "Black Score") << std::endl;

		for (const auto& rec : _performance)
			file << std::format("{:10} {:10.5f} {:10.5f}", rec.round, rec.perf_white, rec.perf_black) << std::endl;
	}

	TrainingState TrainingState::load_from_file(const std::filesystem::path& file_path)
	{
		return DeepLearning::MsgPack::load_from_file<TrainingState>(file_path);
	}

	TrainingState::TrainingState(const std::filesystem::path& agent_script_file_path)
	{
		assign_agents_from_script_file(agent_script_file_path);
	}

	void TrainingState::reset(const bool keep_agents)
	{
		_round_id = 0;
		_performance.clear();

		if (keep_agents)
			return;

		_agents.clear();
	}

	void TrainingState::set_discount(const double& discount)
	{
		for (auto& agent : _agents)
			agent.set_discount(discount);
	}

	void TrainingState::set_lambda(const double& lambda)
	{
		for (auto& agent : _agents)
			agent.set_lambda(lambda);
	}

	void TrainingState::set_learning_rate(const double& l_rate)
	{
		for (auto& agent : _agents)
			agent.set_learning_rate(l_rate);
	}

	void TrainingState::set_exploration(const double& exploration)
	{
		for (auto& agent : _agents)
			agent.set_exploration_probability(exploration);
	}
}
