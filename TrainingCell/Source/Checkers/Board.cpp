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

#include "..\..\Headers\Checkers\Board.h"

namespace TrainingCell::Checkers
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

	bool Board::is_inverted() const
	{
		//Sanity check
		if (_state.is_inverted() == is_agent_to_move_white())
			throw std::exception("Inconsistency between the agent at move and the current state");

		return _state.is_inverted();
	}

	bool Board::is_agent_to_move_white() const
	{
		return _agent_to_move_id == 0;
	}

	void Board::take_turn()
	{
		_state.invert();//invert state because next time it will be given to the "opponent" agent
		_agent_to_move_id = next_agent_id();
	}

	int Board::next_agent_id() const
	{
		return (_agent_to_move_id + 1) % 2;
	}

	void Board::reset_state()
	{
		_agent_to_move_id = 0;
		_state = _start_state;
	}

	void Board::reset_wins()
	{
		_whitesWin = 0;
		_blacksWin = 0;
	}

	/// <summary>
	/// Publishes current "state" if the corresponding call-back is assigned
	/// </summary>
	void publish_state(PublishCheckersStateCallBack publishCallback, const State& state, const Move& move, const IMinimalAgent* agent_to_play)
	{
		if (publishCallback)
			publishCallback(reinterpret_cast<const int*>(state.data()),
				static_cast<int>(state.size()),
				move.sub_moves.data(),
				static_cast<int>(move.sub_moves.size()), agent_to_play);
	}

	void Board::play(const int episodes, const int max_moves_without_capture, const IState* start_state,
		PublishCheckersStateCallBack publish_state_callback,
		PublishTrainingStatsCallBack publish_stats_callback,
		CancelCallBack cancel,
		ErrorMessageCallBack error)
	{
		if (start_state)
			_start_state.assign(start_state->to_std_vector());

		try
		{
			for (auto episode_id = 0; episode_id < episodes; episode_id++)
			{
				if (cancel != nullptr && cancel())
					return;

				auto moves_without_capture = 0;
				reset_state();
				Move last_move{};
				publish_state(publish_state_callback, _state, last_move, agent_to_move());
				while (Utils::is_valid(last_move = make_move(publish_state_callback)) && moves_without_capture < max_moves_without_capture)
				{
					if (Utils::is_valid(last_move.sub_moves[0].capture))
						moves_without_capture = 0;
					else
						moves_without_capture++;

					if (cancel != nullptr && cancel())
						break;//this will be qualified as a "draw"
				}

				if (!Utils::is_valid(last_move)) //win case
				{
					if (_agent_to_move_id == 1)
						_whitesWin++;
					else
						_blacksWin++;

					agent_to_move()->game_over(_state, GameResult::Loss, is_agent_to_move_white());
					agent_to_wait()->game_over(_state, GameResult::Victory, !is_agent_to_move_white());

				}
				else //draw case
				{
					agent_to_move()->game_over(_state, GameResult::Draw, is_agent_to_move_white());
					agent_to_wait()->game_over(_state, GameResult::Draw, !is_agent_to_move_white());
				}

				if (publish_stats_callback != nullptr)
					publish_stats_callback(_whitesWin, _blacksWin, episode_id + 1);
			}
		}
		catch (std::exception& e)
		{
			if (error != nullptr)
				error(e.what());
		}
	}

	Move Board::make_move(PublishCheckersStateCallBack publish)
	{
		const auto moves = _state.get_moves();

		if (moves.empty())
			return {};//invalid move

		const auto chosen_move_id = agent_to_move()->make_move(_state, moves, is_agent_to_move_white());

		if (chosen_move_id < 0 || chosen_move_id >= moves.size())
			throw std::exception("Invalid move id");

		const auto& move = moves[chosen_move_id];

		if (publish != nullptr)
		{
			auto state_copy = _state;
			state_copy.make_move(move, false, true);
			if (is_inverted())
				state_copy.invert();

			const auto move_copy = is_inverted() ? move : move.get_inverted();

			//At the moment, agents have not been swapped, so "agent-to-play" is actually the "agent-to-wait"
			publish_state(publish, state_copy, move_copy, agent_to_wait());
		}

		_state.make_move(move, true);
		take_turn();

		return move;
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
		reset_state();
		reset_wins();
	}
}