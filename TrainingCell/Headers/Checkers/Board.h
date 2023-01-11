#pragma once
#include "Agent.h"

namespace TrainingCell::Checkers
{
	typedef void (*PublishCheckersStateCallBack)(const int* state, const int state_size,
		const SubMove* sub_moves, const int sub_moves_count);

	typedef void (*PublishTrainingStatsCallBack)(const int white_wins, const int black_wins, const int total_games);

	/// <summary>
	///	Environment to train checkers-agents
	/// </summary>
	class Board
	{
		// two agents to play the game
		std::array<Agent*, 2> _agents {};

		/// <summary>
		///	Id of the agent that is to make a move
		/// </summary>
		int _agent_to_move_id = 0;

		/// <summary>
		///	Switches agent roles (the one that was "waiting" for its turn receives a right to make next move)
		/// </summary>
		void take_turn();

		/// <summary>
		///	Returns id of the agent whose turn will be next
		/// </summary>
		int next_agent_id() const;

		/// <summary>
		///	Returns pointer to an agent that is to move
		/// </summary>
		[[nodiscard]] Agent* agent_to_move() const;

		/// <summary>
		///	Pointer to an agent that is waiting for its turn
		/// </summary>
		[[nodiscard]] Agent* agent_to_wait() const;

		/// <summary>
		///	State of the board
		/// </summary>
		State _state;

		/// <summary>
		///	A flag that indicates whether the state is "inverted" or not
		/// </summary>
		bool _is_inverted{};
	public:
		/// <summary>
		///	Constructor
		/// </summary>
		Board(Agent* agentA, Agent* agentB);

		/// <summary>
		///	Runs the given number of episodes (games)
		/// </summary>
		void play(const int episodes, const int max_moves_without_capture = 200, PublishCheckersStateCallBack publishState = nullptr,
			PublishTrainingStatsCallBack publishStats = nullptr);

		/// <summary>
		///	Resets state of the board to the "initial one" (beginning of the game)
		/// </summary>
		void reset_state();

		/// <summary>
		/// The agent whose turn is makes a move and the tunr switches to another agent.
		/// </summary>
		Move make_move(PublishCheckersStateCallBack publish);
	};
}
