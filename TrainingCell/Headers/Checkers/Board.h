#pragma once
#include "Agent.h"

namespace TrainingCell::Checkers
{
	/// <summary>
	///	Environment to train checkers-agents
	/// </summary>
	class Board
	{
		// two agents to play the game
		std::array<Agent*, 2> _agents {};
	public:
		/// <summary>
		///	Constructor
		/// </summary>
		Board(Agent* agentA, Agent* agentB);

		/// <summary>
		///	Runs the given number of episodes (games)
		/// </summary>
		void play(const int episodes, const int max_moves_without_capture = 200) const;
	};
}
