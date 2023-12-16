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
	Board::Board(IMinimalAgent* const agentA, IMinimalAgent* const agentB)
	{
		if (agentA == nullptr || agentB == nullptr)
			throw std::exception("Invalid agents");

		_agents = { agentA, agentB };
	}

	IMinimalAgent* Board::agent_to_move() const
	{
		return _agents[_agent_to_move_id];
	}

	IMinimalAgent* Board::agent_to_wait() const
	{
		return _agents[next_agent_id()];
	}

	IState& Board::state() const
	{
		return *_state_ptr;
	}

	bool Board::is_agent_to_move_white() const
	{
		return _agent_to_move_id == 0;
	}

	void Board::take_turn()
	{
		_agent_to_move_id = next_agent_id();
	}

	int Board::next_agent_id() const
	{
		return (_agent_to_move_id + 1) % 2;
	}

	IState& Board::reset_state(const IStateSeed& seed)
	{
		_agent_to_move_id = 0;
		_state_ptr = seed.yield();
		return *_state_ptr;
	}

	void Board::reset_wins()
	{
		_whitesWin = 0;
		_blacksWin = 0;
	}

	/// <summary>
	/// Publishes current "state" if the corresponding call-back is assigned
	/// </summary>
	void publish_state(PublishStateCallBack publishCallback, const std::vector<int>& state, const Move& move, const IMinimalAgent* agent_to_play)
	{
		if (publishCallback)
		{
			publishCallback(state.data(), static_cast<int>(state.size()), move.sub_moves.data(),
				static_cast<int>(move.sub_moves.size()), agent_to_play);
		}
	}

	void Board::play(const int episodes, const IStateSeed& start_state, const int max_moves_without_capture,
		PublishStateCallBack publish_state_callback,
		PublishEndEpisodeStatsCallBack publish_end_episode_stats_callback,
		CancelCallBack cancel,
		ErrorMessageCallBack error)
	{
		if (!StateTypeController::validate(*agent_to_move(), *agent_to_wait(), start_state))
			throw std::exception("Agents and state incompatible.");

		try
		{
			for (auto episode_id = 0; episode_id < episodes; episode_id++)
			{
				if (cancel != nullptr && cancel())
					return;

				auto moves_without_capture = 0;
				auto& state = reset_state(start_state);

				publish_state(publish_state_callback, state.evaluate_ui(), Move{}, agent_to_move());
				while (state.get_moves_count() > 0 && moves_without_capture <= max_moves_without_capture &&
					!state.is_draw())
				{
					const auto is_capture_move = make_move(state, publish_state_callback);
					moves_without_capture = is_capture_move ? 0 : (moves_without_capture + 1);

					if (cancel != nullptr && cancel())
						break;//this will be qualified as a "draw"
				}

				bool white_won = false;
				bool black_won = false;
				if (state.get_moves_count() <= 0 && !state.is_draw()) //win case
				{
					if (_agent_to_move_id == 1)
					{
						_whitesWin++;
						white_won = true;
					}
					else
					{
						_blacksWin++;
						black_won = true;
					}

					agent_to_move()->game_over(state, GameResult::Loss, is_agent_to_move_white());
					agent_to_wait()->game_over(state, GameResult::Victory, !is_agent_to_move_white());

				}
				else //draw case
				{
					white_won = black_won = state.get_moves_count() == 0; // to be able to distinguish between two kinds of draws.

					agent_to_move()->game_over(state, GameResult::Draw, is_agent_to_move_white());
					agent_to_wait()->game_over(state, GameResult::Draw, !is_agent_to_move_white());
				}

				if (publish_end_episode_stats_callback != nullptr)
					publish_end_episode_stats_callback(white_won, black_won, episode_id + 1);
			}
		}
		catch (std::exception& e)
		{
			if (error != nullptr)
				error(e.what());
		}
	}

	bool Board::make_move(IState& state_handle, PublishStateCallBack publish)
	{
		const auto chosen_move_id = agent_to_move()->make_move(state_handle, is_agent_to_move_white());

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
			publish_state(publish, state_std, move_adjusted, agent_to_wait());
		}

		take_turn();

		return is_capture_move;
	}

	int Board::get_whites_wins() const
	{
		return _whitesWin;
	}

	int Board::get_blacks_wins() const
	{
		return _blacksWin;
	}

	void Board::swap_agents()
	{
		std::swap(_agents[0], _agents[1]);
		reset_wins();
	}
}
