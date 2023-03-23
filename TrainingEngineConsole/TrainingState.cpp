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
#include <format>

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

	void TrainingState::save_to_file(const std::filesystem::path& file_path, const bool extended) const
	{
		DeepLearning::MsgPack::save_to_file(*this, file_path);

		if (extended)
		{
			if (file_path.extension() == ".txt")
				throw std::exception("State file cannot have `.txt` extension");

			auto description_file_path = file_path;
			description_file_path.replace_extension(".txt");
			std::ofstream file(description_file_path);

			if (!file)
				throw std::exception("Failed to create file");

			for(const auto& agent : _agents)
				file << agent.to_script() << std::endl;

			file.close();
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

	void TrainingState::reset()
	{
		_round_id = 0;
		_agents.clear();
		_performance.clear();
	}
}
