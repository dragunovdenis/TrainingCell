#include "..\..\Headers\Checkers\Board.h"

namespace TrainingCell::Checkers
{
	Board::Board(Agent* agentA, Agent* agentB)
	{
		if (agentA == nullptr || agentB == nullptr)
			throw std::exception("Invalid agents");

		_agents = { agentA, agentB };
	}

	void Board::play(const int episodes, const int max_moves_without_capture) const
	{
		for (auto episode_id = 0; episode_id < episodes; episode_id++)
		{
			auto moves_without_capture = 0;
			State state = State::set_start_state();
			std::vector<Move> moves = Utils::get_moves(state);
			auto moves_counter = 0;
			while (!moves.empty() && moves_without_capture < max_moves_without_capture)
			{
				Agent* current_agent = _agents[moves_counter % 2];
				moves_counter++;

				const auto chosen_move_id = current_agent->make_move(state, moves);

				if (chosen_move_id < 0 || chosen_move_id >= moves.size())
					throw std::exception("Invalid move id");

				const auto move = moves[chosen_move_id];

				if (move.sub_moves[0].capture.is_valid())
					moves_without_capture = 0;
				else
					moves_without_capture++;

				state.make_move(move, true);
				moves = Utils::get_moves(state);
			}

			if (moves.empty()) //win case
			{
				_agents[(moves_counter + 1) % 2]->game_over(state, GameResult::Victory);
				_agents[(moves_counter) % 2]->game_over(state, GameResult::Loss);

			} else //draw case
			{
				_agents[0]->game_over(state, GameResult::Draw);
				_agents[1]->game_over(state, GameResult::Draw);
			}
		}
	}
}