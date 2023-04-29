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

#pragma once
#include "Agent.h"

namespace TrainingCell::Checkers
{
	/// <summary>
	///	Callback to publish current state and move
	/// </summary>
	typedef void (*PublishCheckersStateCallBack)(const int* state, const int state_size,
	                                             const SubMove* sub_moves, const int sub_moves_count);

	/// <summary>
	///	Callback to publish training statistics
	/// </summary>
	typedef void (*PublishTrainingStatsCallBack)(const int white_wins, const int black_wins, const int total_games);

	/// <summary>
	/// Callback to report error messages
	/// </summary>
	typedef void (*ErrorMessageCallBack)(const char* message);

	/// <summary>
	///	Callback to cancel ongoing operation
	/// </summary>
	typedef bool (*CancelCallBack)();

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
		[[nodiscard]] bool is_inverted() const;

		/// <summary>
		/// Returns true it the current "agent-to-move" is that one that plays white pieces
		/// </summary>
		[[nodiscard]] bool is_agent_to_move_white() const;

		/// <summary>
		/// Number of times "whites" (agent with index "0") won during the training
		/// </summary>
		int _whitesWin{};

		/// <summary>
		/// Number of times "blacks" (agent with index "1") won during the training
		/// </summary>
		int _blacksWin{};

		/// <summary>
		/// Retrieves move from the "current" agent and changes the "current" agent
		/// </summary>
		Move make_move(PublishCheckersStateCallBack publish);

		/// <summary>
		/// Resets state of the board to the "initial one" (beginning of the game)
		/// </summary>
		void reset_state();
	public:
		/// <summary>
		///	Constructor
		/// </summary>
		Board(Agent* const agentA, Agent* const agentB);

		/// <summary>
		///	Runs the given number of episodes (games)
		/// </summary>
		void play(const int episodes, const int max_moves_without_capture = 200, PublishCheckersStateCallBack publishState = nullptr,
			PublishTrainingStatsCallBack publishStats = nullptr, CancelCallBack cancel = nullptr,
			ErrorMessageCallBack error = nullptr);

		/// <summary>
		/// Reset winning statistics
		/// </summary>
		void reset_wins();

		/// <summary>
		/// Returns number of times "whites" (agent with index "0") won during the training
		/// </summary>
		[[nodiscard]] int get_whites_wins() const;

		/// <summary>
		/// Returns number of times "blacks" (agent with index "1") won during the training
		/// </summary>
		[[nodiscard]] int get_blacks_wins() const;

		/// <summary>
		/// Swaps the agents
		/// </summary>
		void swap_agents();
	};
}
