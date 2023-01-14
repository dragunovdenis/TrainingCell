#include "..\..\Headers\Checkers\Board.h"

namespace TrainingCell::Checkers
{
	Board::Board(Agent* agentA, Agent* agentB)
	{
		if (agentA == nullptr || agentB == nullptr)
			throw std::exception("Invalid agents");

		_agents = { agentA, agentB };
	}

	Agent* Board::agent_to_move() const
	{
		return _agents[_agent_to_move_id];
	}

	Agent* Board::agent_to_wait() const
	{
		return _agents[next_agent_id()];
	}

	void Board::take_turn()
	{
		_state.invert();//invert state because next time it will be given to the "opponent" agent
		_is_inverted = !_is_inverted;
		_agent_to_move_id = next_agent_id();
	}

	int Board::next_agent_id() const
	{
		return (_agent_to_move_id + 1) % 2;
	}

	void Board::reset_state()
	{
		_is_inverted = false;
		_agent_to_move_id = 0;
		_state = State::set_start_state();
	}

	void Board::play(const int episodes, const int max_moves_without_capture,
		PublishCheckersStateCallBack publishState,
		PublishTrainingStatsCallBack publishStats)
	{
		int whitesWin = 0;
		int blacksWin = 0;
		for (auto episode_id = 0; episode_id < episodes; episode_id++)
		{
			auto moves_without_capture = 0;
			reset_state();
			Move last_move {};
			while ((last_move = make_move(publishState)).is_valid() && moves_without_capture < max_moves_without_capture)
			{
				if (last_move.sub_moves[0].capture.is_valid())
					moves_without_capture = 0;
				else
					moves_without_capture++;
			}

			if (!last_move.is_valid()) //win case
			{
				if (_agent_to_move_id == 1)
					whitesWin++;
				else
					blacksWin++;

				agent_to_move()->game_over(_state, GameResult::Loss);
				agent_to_wait()->game_over(_state, GameResult::Victory);

			} else //draw case
			{
				agent_to_move()->game_over(_state, GameResult::Draw);
				agent_to_wait()->game_over(_state, GameResult::Draw);
			}

			if (publishStats != nullptr)
				publishStats(whitesWin, blacksWin, episode_id + 1);
		}
	}

	Move Board::make_move(PublishCheckersStateCallBack publish)
	{
		const auto moves = _state.get_moves();

		if (moves.empty())
			return {};//invalid move

		const auto chosen_move_id = agent_to_move()->make_move(_state, moves);

		if (chosen_move_id < 0 || chosen_move_id >= moves.size())
			throw std::exception("Invalid move id");

		const auto& move = moves[chosen_move_id];

		if (publish != nullptr)
		{
			auto state_copy = _state;
			state_copy.make_move(move, false, true);
			if (_is_inverted)
				state_copy.invert();

			const auto move_copy = _is_inverted ? move : move.get_inverted();

			publish(reinterpret_cast<int*>(state_copy.data()),
				static_cast<int>(state_copy.size()),
				move_copy.sub_moves.data(), static_cast<int>(move_copy.sub_moves.size()));
		}

		_state.make_move(move, true);
		take_turn();

		return move;
	}
}