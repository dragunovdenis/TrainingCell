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
#include "IStateReadOnly.h"

namespace TrainingCell
{
	/// <summary>
	///	Representation of possible game statuses
	/// </summary>
	enum class GameResult : int {
		Victory = 1,
		Loss = -1,
		Draw = 0,
	};

	/// <summary>
	/// Minimal "interface" that each checkers agent must possess
	/// </summary>
	class IMinimalAgent
	{
	public:
		/// <summary>
		/// Virtual destructor
		/// </summary>
		virtual ~IMinimalAgent() = default;

		/// <summary>
		/// Returns index of a move from the given collection of available moves
		/// that the agent "prefers" to take given the current state
		/// </summary>
		virtual int make_move(const IStateReadOnly& state, const bool as_white) = 0;

		/// <summary>
		/// The method is supposed to be called by the "training environment" when the current training episode is over
		/// to notify the agent about the "final" state and the result of entire game (episode)
		/// </summary>
		virtual void game_over(const IStateReadOnly& final_state, const GameResult& result, const bool as_white) = 0;
	};
}
