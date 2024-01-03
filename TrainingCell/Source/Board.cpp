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

#include "../Headers/Board.h"
#include "../Headers/StateTypeController.h"

namespace TrainingCell
{
	template <class A>
	Board::AgentManager<A>::AgentManager(A* agent_white, A* agent_black) : _agents{ agent_white, agent_black }
	{}

	template <class A>
	A& Board::AgentManager<A>::agent_to_move()
	{
		return *_agents[_agent_to_move_id];
	}

	template <class A>
	A& Board::AgentManager<A>::agent_to_wait()
	{
		return *_agents[next_agent_id()];
	}

	template <class A>
	bool Board::AgentManager<A>::is_agent_to_move_white() const
	{
		return _agent_to_move_id == 0;
	}

	template <class A>
	void Board::AgentManager<A>::take_turn()
	{
		_agent_to_move_id = next_agent_id();
	}

	template <class A>
	int Board::AgentManager<A>::next_agent_id() const
	{
		return (_agent_to_move_id + 1) % 2;
	}

	template <class A>
	void Board::AgentManager<A>::reset()
	{
		_agent_to_move_id = 0;
	}

	template <class A>
	Board::AgentManagerAdv<A>::AgentManagerAdv(A* agent_white, A* agent_black) :
		AgentManager<A>(agent_white, agent_black)
	{}

	template <class A>
	void Board::AgentManagerAdv<A>::set_exploration_mode()
	{
		for (const auto agent : this->_agents)
			if (agent->can_train())
				agent->set_training_mode(false);
	}

	template<class A>
	void Board::AgentManagerAdv<A>::set_training_mode()
	{
		for (const auto agent : this->_agents)
			if (agent->can_train())
				agent->set_training_mode(true);
	}

	Board::Stats::Stats(const int blacks_win_count, const int whites_win_count, const int total_episodes) :
	_blacks_win_count(blacks_win_count), _whites_win_count(whites_win_count), _total_episodes_count(total_episodes)
	{}

	int Board::Stats::blacks_win_count() const
	{
		return _blacks_win_count;
	}

	int Board::Stats::whites_win_count() const
	{
		return _whites_win_count;
	}

	int Board::Stats::total_episodes_count() const
	{
		return _total_episodes_count;
	}

	Board::Board(IMinimalAgent* white_agent_ptr, IMinimalAgent* black_agent_ptr) :
		_white_agent_ptr(white_agent_ptr), _black_agent_ptr(black_agent_ptr)
	{}

	void Board::swap_agents()
	{
		std::swap(_white_agent_ptr, _black_agent_ptr);
	}

	Board::Stats Board::play(const int episodes, const IStateSeed& start_state, const int max_moves_without_capture,
	                         PublishStateCallBack publish_state_callback, PublishEndEpisodeStatsCallBack publish_end_episode_stats_callback,
	                         CancelCallBack cancel, ErrorMessageCallBack error) const
	{
		return play(_white_agent_ptr, _black_agent_ptr,
			episodes, start_state, max_moves_without_capture, publish_state_callback, publish_end_episode_stats_callback, cancel, error);
	}

	/// <summary>
	/// Publishes current "state" if the corresponding call-back is assigned
	/// </summary>
	void publish_state(PublishStateCallBack publishCallback, const std::vector<int>& state, const Move& move, const IMinimalAgent& agent_to_play)
	{
		if (publishCallback)
		{
			publishCallback(state.data(), static_cast<int>(state.size()), move.sub_moves.data(),
				static_cast<int>(move.sub_moves.size()), &agent_to_play);
		}
	}

	template <class A>
	Board::EpisodeResult Board::play_episode(IState& state, AgentManager<A>& agent_manager,
		const int max_moves_without_capture, PublishStateCallBack publish_state_callback, CancelCallBack cancel)
	{
		auto moves_without_capture = 0;
		publish_state(publish_state_callback, state.evaluate_ui(), Move{}, agent_manager.agent_to_move());
		while (state.get_moves_count() > 0 && moves_without_capture <= max_moves_without_capture && !state.is_draw())
		{
			const auto is_capture_move = make_move(state, agent_manager, publish_state_callback);
			moves_without_capture = is_capture_move ? 0 : (moves_without_capture + 1);

			if (cancel != nullptr && cancel())
				break;//this will be qualified as a "draw"
		}

		EpisodeResult result;
		if (state.get_moves_count() <= 0 && !state.is_draw()) //win case
		{
			result = agent_manager.is_agent_to_move_white() ? BlackVictory : WhiteVictory;
			agent_manager.agent_to_move().game_over(state, GameResult::Loss, agent_manager.is_agent_to_move_white());
			agent_manager.agent_to_wait().game_over(state, GameResult::Victory, !agent_manager.is_agent_to_move_white());

		}
		else //draw case
		{
			result = state.get_moves_count() == 0 ? StrongDraw : Draw;
			agent_manager.agent_to_move().game_over(state, GameResult::Draw, agent_manager.is_agent_to_move_white());
			agent_manager.agent_to_wait().game_over(state, GameResult::Draw, !agent_manager.is_agent_to_move_white());
		}

		return result;
	}

	Board::Stats Board::play(IMinimalAgent* const agent_white_ptr, IMinimalAgent* const agent_black_ptr,
	const int episodes, const IStateSeed& start_state, const int max_moves_without_capture,
		PublishStateCallBack publish_state_callback,
		PublishEndEpisodeStatsCallBack publish_end_episode_stats_callback,
		CancelCallBack cancel,
		ErrorMessageCallBack error)
	{
		AgentManager agent_manager(agent_white_ptr, agent_black_ptr);

		if (!StateTypeController::validate(agent_manager.agent_to_move(), agent_manager.agent_to_wait(), start_state))
			throw std::exception("Agents and state incompatible.");

		int blacks_win_counter = 0;
		int whites_win_counter = 0;

		try
		{
			for (auto episode_id = 1; episode_id <= episodes; episode_id++)
			{
				if (cancel != nullptr && cancel())
					return { blacks_win_counter, whites_win_counter, episode_id - 1 };

				agent_manager.reset();
				auto state_ptr = start_state.yield(/*initialize_recorder*/ false);

				const auto episode_result = play_episode(*state_ptr, agent_manager,
					max_moves_without_capture, publish_state_callback, cancel);

				whites_win_counter += episode_result == WhiteVictory;
				blacks_win_counter += episode_result == BlackVictory;

				if (publish_end_episode_stats_callback != nullptr)
				{
					const auto white_won = (episode_result & (WhiteVictory | StrongDraw)) != 0;
					const auto black_won = (episode_result & (BlackVictory | StrongDraw)) != 0;
					publish_end_episode_stats_callback(white_won, black_won, episode_id);
				}
			}
		}
		catch (std::exception& e)
		{
			if (error != nullptr)
				error(e.what());
		}

		return { blacks_win_counter, whites_win_counter, episodes };
	}

	Board::Stats Board::train(ITrainableAgent* const agent_white_ptr, ITrainableAgent* const agent_black_ptr,
		const int episodes, const IStateSeed& start_state, const int max_moves_without_capture,
		const int  max_consequent_draw_episodes, PublishEndEpisodeStatsCallBack publish_end_episode_stats_callback, CancelCallBack cancel,
		ErrorMessageCallBack error)
	{
		AgentManagerAdv agent_manager(agent_white_ptr, agent_black_ptr);

		if (!StateTypeController::validate(agent_manager.agent_to_move(), agent_manager.agent_to_wait(), start_state))
			throw std::exception("Agents and state incompatible.");

		int blacks_win_counter = 0;
		int whites_win_counter = 0;
		auto total_episodes_count = 0;

		try
		{
			auto episode_id = 0;
			auto consequent_draw_episodes = 0;
			while (episode_id < episodes)
			{
				if (cancel != nullptr && cancel())
					return { blacks_win_counter, whites_win_counter, total_episodes_count };

				total_episodes_count++;
				agent_manager.reset();
				agent_manager.set_exploration_mode();
				auto state_ptr = start_state.yield(/*initialize_recorder*/ true);

				const auto episode_result = play_episode(*state_ptr, agent_manager,
					max_moves_without_capture, nullptr, cancel);

				if (episode_result == Draw && consequent_draw_episodes < max_consequent_draw_episodes)
				{
					consequent_draw_episodes++;
					continue;
				}

				consequent_draw_episodes = 0;
				episode_id++;
				agent_manager.reset();
				agent_manager.set_training_mode();
				auto recorder_state_ptr = state_ptr->get_recorded_state();

				const auto replay_episode_result = play_episode(*recorder_state_ptr, agent_manager,
					max_moves_without_capture, nullptr, cancel);

				if (replay_episode_result != episode_result)
					throw std::exception("Result of exploration episode differs from that of the re-play episode");

				whites_win_counter += episode_result == WhiteVictory;
				blacks_win_counter += episode_result == BlackVictory;

				if (publish_end_episode_stats_callback != nullptr)
				{
					const auto white_won = (episode_result & (WhiteVictory | StrongDraw)) != 0;
					const auto black_won = (episode_result & (BlackVictory | StrongDraw)) != 0;
					publish_end_episode_stats_callback(white_won, black_won, episode_id);
				}
			}
		}
		catch (std::exception& e)
		{
			if (error != nullptr)
				error(e.what());
		}

		return { blacks_win_counter, whites_win_counter, total_episodes_count };
	}

	template <class A>
	bool Board::make_move(IState& state_handle, AgentManager<A>& agent_manager, PublishStateCallBack publish)
	{
		const auto chosen_move_id = agent_manager.agent_to_move().make_move(state_handle, agent_manager.is_agent_to_move_white());

		// sanity check
		if (chosen_move_id < 0 || chosen_move_id >= state_handle.get_moves_count())
			throw std::exception("Invalid move id");

		const auto is_capture_move = state_handle.is_capture_action(chosen_move_id);

		if (publish == nullptr)
			state_handle.move_invert_reset(chosen_move_id);
		else
		{
			const auto move = state_handle.get_all_moves()[chosen_move_id];
			const auto move_adjusted = state_handle.is_inverted() ? move.get_inverted() : move;
			state_handle.move_invert_reset(chosen_move_id);
			const auto state_std = state_handle.is_inverted() ?
				state_handle.evaluate_ui_inverted() : state_handle.evaluate_ui();

			//At the moment, agents have not been swapped, so "agent-to-play" is actually the "agent-to-wait"
			publish_state(publish, state_std, move_adjusted, agent_manager.agent_to_wait());
		}

		agent_manager.take_turn();

		return is_capture_move;
	}
}
