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
#include "../../../DeepLearning/DeepLearning/Utilities.h"
#include "../../Headers/Checkers/TdlLegacyMsgPackAdapter.h"

namespace TrainingCell::Checkers
{
	TdlEnsembleAgent::TdlEnsembleAgent(const std::vector<TdLambdaAgent>& ensemble, const std::string& name)
	{
		std::ranges::for_each(ensemble, [&](const auto& a)
			{
				add(a);
			});

		set_name(name);
	}

	std::size_t TdlEnsembleAgent::add(const TdLambdaAgent& agent)
	{
		_ensemble.emplace_back(agent);
		_ensemble.rbegin()->set_training_mode(false);

		return _ensemble.size() - 1;
	}

	std::size_t TdlEnsembleAgent::set_single_agent_mode(const bool use_single_random_agent)
	{
		if (use_single_random_agent)
			_chosen_agent_id = DeepLearning::Utils::get_random_int(0, static_cast<int>(_ensemble.size()) - 1);
		else
			_chosen_agent_id = -1;

		return _chosen_agent_id;
	}

	bool TdlEnsembleAgent::is_single_agent_mode() const
	{
		return _chosen_agent_id >= 0 && _ensemble.size() > _chosen_agent_id;
	}

	std::size_t TdlEnsembleAgent::get_current_random_agent_id() const
	{
		return _chosen_agent_id;
	}

	bool TdlEnsembleAgent::remove_agent(const int id)
	{
		if (id < 0 || _ensemble.size() <= id)
			return false;

		_ensemble.erase(_ensemble.begin() + id);
		return true;
	}

	const Agent& TdlEnsembleAgent::operator [](const int id) const
	{
		return _ensemble[id];
	}

	int TdlEnsembleAgent::make_move(const State& current_state, const std::vector<Move>& moves, const bool as_white)
	{
		if (moves.empty())
			return -1;

		if (moves.size() == 1)
			return 0; // the choice is obvious

		if (is_single_agent_mode())
			return _ensemble[_chosen_agent_id].pick_move_id(current_state, moves, as_white);

		std::vector votes(moves.size(), 0);

		for (const auto& a : _ensemble)
			++votes[a.pick_move_id(current_state, moves, as_white)];

		return static_cast<int>(std::distance(votes.begin(), std::ranges::max_element(votes)));
	}

	void TdlEnsembleAgent::game_over(const State& final_state, const GameResult& result, const bool as_white)
	{
		set_single_agent_mode(is_single_agent_mode());
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

	namespace
	{
		/// <summary>
		/// A dummy class to be able to load "legacy" version opf the ensemble from MessagePack stream
		/// </summary>
		class TdlEnsembleLegacyMsgPackAdapter : public Agent
		{
		public:
			TdlEnsembleLegacyMsgPackAdapter() = default;
			std::vector<TdlLegacyMsgPackAdapter> _ensemble{};
			int _chosen_agent_id = -1;
			MSGPACK_DEFINE(MSGPACK_BASE(Agent), _ensemble, _chosen_agent_id);

			int make_move(const State& current_state, const std::vector<Move>& moves, const bool as_white) override
			{
				throw std::exception("Not implemented");
			};

			void game_over(const State& final_state, const GameResult& result, const bool as_white) override
			{
				throw std::exception("Not implemented");
			}

			[[nodiscard]] AgentTypeId get_type_id() const override
			{
				throw std::exception("Not implemented");
			}

			[[nodiscard]] bool can_train() const override
			{
				throw std::exception("Not implemented");
			}
		};
	}

	TdlEnsembleAgent TdlEnsembleAgent::load_from_file(const std::filesystem::path& file_path)
	{
		try
		{
			return DeepLearning::MsgPack::load_from_file<TdlEnsembleAgent>(file_path);
		} catch (...)
		{
			const auto legacy_container = DeepLearning::MsgPack::load_from_file<TdlEnsembleLegacyMsgPackAdapter>(file_path);

			TdlEnsembleAgent result{};

			result._ensemble.reserve(legacy_container._ensemble.size());
			std::ranges::transform(legacy_container._ensemble, std::back_inserter(result._ensemble),
				[](const auto& legacy_agent)
				{
					return TdLambdaAgent(legacy_agent);
				});

			result._chosen_agent_id = legacy_container._chosen_agent_id;
			static_cast<Agent&>(result) = static_cast<const Agent&>(legacy_container);

			return result;
		}
	}

	bool TdlEnsembleAgent::operator == (const TdlEnsembleAgent& anotherAgent) const
	{
		return equal(anotherAgent);
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
		return other_ensemble_ptr != nullptr && Agent::equal(agent) &&
			_ensemble == other_ensemble_ptr->_ensemble &&
			_chosen_agent_id == other_ensemble_ptr->_chosen_agent_id &&
			_msg_pack_version == other_ensemble_ptr->_msg_pack_version;
	}
}
