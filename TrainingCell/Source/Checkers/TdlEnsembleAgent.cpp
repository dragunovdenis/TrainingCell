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

#include "../../Headers/Checkers/TdlEnsembleAgent.h"
#include "../../../DeepLearning/DeepLearning/MsgPackUtils.h"

namespace TrainingCell::Checkers
{
	TdlEnsembleAgent::TdlEnsembleAgent(const std::vector<TdLambdaAgent>& ensemble, const std::string& id)
	{
		std::ranges::for_each(ensemble, [&](const auto& a)
			{
				_ensemble.emplace_back(a);
				_ensemble.rbegin()->set_training_mode(false);
			});

		set_id(id);
	}

	void TdlEnsembleAgent::Add(const TdLambdaAgent& agent)
	{
		_ensemble.emplace_back(agent);
	}

	int TdlEnsembleAgent::make_move(const State& current_state, const std::vector<Move>& moves)
	{
		if (moves.empty())
			return -1;

		if (moves.size() == 1)
			return 0; // the choice is obvious

		std::vector<int> votes(moves.size(), 0);

		for (const auto& a : _ensemble)
			++votes[a.pick_move_id(current_state, moves)];

		return static_cast<int>(std::distance(votes.begin(), std::ranges::max_element(votes)));
	}

	void TdlEnsembleAgent::game_over(const State& final_state, const GameResult& result)
	{
		/*do nothing*/
	}

	AgentTypeId TdlEnsembleAgent::TYPE_ID()
	{
		return AgentTypeId::TDL_ENSEMBLE;
	}

	AgentTypeId TdlEnsembleAgent::get_type_id() const
	{
		return TYPE_ID();
	}

	void TdlEnsembleAgent::save_to_file(const std::filesystem::path& file_path) const
	{
		DeepLearning::MsgPack::save_to_file(*this, file_path);
	}

	TdlEnsembleAgent TdlEnsembleAgent::load_from_file(const std::filesystem::path& file_path)
	{
		return DeepLearning::MsgPack::load_from_file<TdlEnsembleAgent>(file_path);
	}

	bool TdlEnsembleAgent::operator == (const TdlEnsembleAgent& anotherAgent) const
	{
		return this->_ensemble == anotherAgent._ensemble && _id == anotherAgent._id;
	}

	bool TdlEnsembleAgent::operator != (const TdlEnsembleAgent& anotherAgent) const
	{
		return !(*this == anotherAgent);
	}

	std::size_t TdlEnsembleAgent::size() const
	{
		return _ensemble.size();
	}

	bool TdlEnsembleAgent::can_train() const
	{
		return false;
	}

	bool TdlEnsembleAgent::equal(const Agent& agent) const
	{
		const auto other_ensemble_ptr = dynamic_cast<const TdlEnsembleAgent*>(&agent);
		return other_ensemble_ptr != nullptr && (*other_ensemble_ptr) == *this;
	}
}
