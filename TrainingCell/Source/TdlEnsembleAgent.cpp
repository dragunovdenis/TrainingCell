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

#include "../Headers/TdlEnsembleAgent.h"
#include "../../DeepLearning/DeepLearning/MsgPackUtils.h"
#include "../../DeepLearning/DeepLearning/Utilities.h"
#include "../Headers/TdlLegacyMsgPackAdapter.h"
#include "../Headers/StateTypeController.h"
#include <ppl.h>

namespace TrainingCell
{
	void TdlEnsembleAgent::msgpack_unpack(msgpack::object const& msgpack_o)
	{
		auto msg_pack_version = 0;
		msgpack::type::make_define_array(msg_pack_version, MSGPACK_BASE(Agent), _ensemble, _chosen_agent_id,
			_search_method, _search_iterations, _search_depth, _run_multi_threaded).msgpack_unpack(msgpack_o);

		if (msg_pack_version <= 1)
			synchronize_parameters();

		validate_synchronization();
	}

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
		update_agent_params(*_ensemble.rbegin());

		return _ensemble.size() - 1;
	}

	std::size_t TdlEnsembleAgent::add(TdLambdaAgent&& agent)
	{
		static_assert(std::is_move_constructible_v<TdLambdaAgent>, "Agent class is supposed to have move constructor in place.");
		_ensemble.emplace_back(std::move(agent));
		update_agent_params(*_ensemble.rbegin());

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

	void TdlEnsembleAgent::update_agent_params(TdLambdaAgent& agent) const
	{
		agent.set_search_depth(_search_depth);
		agent.set_tree_search_method(_search_method);
		agent.set_td_search_iterations(_search_iterations);
		agent.set_performance_evaluation_mode(true);
	}

	void TdlEnsembleAgent::synchronize_parameters()
	{
		for (auto& agent : _ensemble)
			update_agent_params(agent);
	}

	void TdlEnsembleAgent::validate_synchronization() const
	{
		for (const auto& agent : _ensemble)
			if (agent.get_search_depth() != _search_depth ||
				agent.get_tree_search_method() != _search_method ||
				agent.get_td_search_iterations() != _search_iterations ||
				!agent.get_performance_evaluation_mode())
				throw std::exception("Parameters of some agent(-s) are out of sync.");
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

	int TdlEnsembleAgent::make_move(const IStateReadOnly& state, const bool as_white)
	{
		if (state.get_moves_count() <= 0)
			return -1;

		if (state.get_moves_count() == 1)
			return 0; // the choice is obvious

		if (is_single_agent_mode())
			return _ensemble[_chosen_agent_id].pick_move_id(state, as_white);

		std::vector votes(state.get_moves_count(), 0);

		if (_run_multi_threaded)
		{
			std::mutex mtx;
			Concurrency::parallel_for(0ull, _ensemble.size(), [&state, as_white, &votes, &mtx, this](const auto agent_id)
				{
					const int move_id = _ensemble[agent_id].pick_move_id(state, as_white);
					std::lock_guard lock(mtx);
					++votes[move_id];
				});
		}
		else
		{
			for (const auto& a : _ensemble)
				++votes[a.pick_move_id(state, as_white)];
		}

		return static_cast<int>(std::distance(votes.begin(), std::ranges::max_element(votes)));
	}

	void TdlEnsembleAgent::game_over(const IStateReadOnly& final_state, const GameResult& result, const bool as_white)
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

			int make_move(const IStateReadOnly& state, const bool as_white) override
			{
				throw std::exception("Not implemented");
			};

			void game_over(const IStateReadOnly& final_state, const GameResult& result, const bool as_white) override
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
			_msg_pack_version == other_ensemble_ptr->_msg_pack_version &&
			_search_method == other_ensemble_ptr->_search_method &&
			_search_iterations == other_ensemble_ptr->_search_iterations &&
			_search_depth == other_ensemble_ptr->_search_depth &&
			_run_multi_threaded == other_ensemble_ptr->_run_multi_threaded;
	}

	StateTypeId TdlEnsembleAgent::get_state_type_id() const
	{
		if (_ensemble.empty())
			return StateTypeId::INVALID;

		auto result = _ensemble[0].get_state_type_id();

		for (auto agent_id = 1ull; agent_id < _ensemble.size(); ++agent_id)
		{
			const auto trial_state_id = _ensemble[agent_id].get_state_type_id();

			// Sanity check
			if (!StateTypeController::states_are_compatible(result, trial_state_id))
				throw std::exception("Incompatible agents in ensemble");

			result = StateTypeController::get_common_state(result, trial_state_id);
		}

		return result;
	}

	TreeSearchMethod TdlEnsembleAgent::get_search_method() const
	{
		return _search_method;
	}

	void TdlEnsembleAgent::set_search_method(const TreeSearchMethod& search_method)
	{
		_search_method = search_method;
		synchronize_parameters();
	}

	int TdlEnsembleAgent::get_search_iterations() const
	{
		return _search_iterations;
	}

	void TdlEnsembleAgent::set_search_iterations(const int search_iterations)
	{
		_search_iterations = search_iterations;
		synchronize_parameters();
	}

	int TdlEnsembleAgent::get_search_depth() const
	{
		return _search_depth;
	}

	void TdlEnsembleAgent::set_search_depth(const int search_depth)
	{
		_search_depth = search_depth;
		synchronize_parameters();
	}

	bool TdlEnsembleAgent::get_run_multi_threaded() const
	{
		return _run_multi_threaded;
	}

	void TdlEnsembleAgent::set_run_multi_threaded(const bool run_multi_threaded)
	{
		_run_multi_threaded = run_multi_threaded;
	}
}
