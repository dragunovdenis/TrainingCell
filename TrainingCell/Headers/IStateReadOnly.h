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
#include "IMinimalStateReadonly.h"
#include "IStateSeed.h"
#include "Move.h"

namespace TrainingCell
{
	/// <summary>
	/// An interface providing an agent with all the needed read-only access to an environment's state.
	/// </summary>
	class IStateReadOnly : public IMinimalStateReadonly
	{
	public:
		/// <summary>
		/// Read-only access to the current state's seed.
		/// </summary>
		[[nodiscard]] virtual const IStateSeed& current_state_seed() const = 0;

		/// <summary>
		/// Returns collection of all the moves "available" for the current state.
		/// Notice, the data structure used to represent moves in the returned collection
		/// can differ from that used by the actual implementation of a state to represent moves-instructions.
		/// In fact, moves in this collection are supposed to be used strictly for UI visualization of the game-play.
		/// </summary>
		[[nodiscard]] virtual const std::vector<Move> get_all_moves() const = 0;

		/// <summary>
		/// Returns UI-suitable vector representation of the current state.
		/// </summary>
		[[nodiscard]] virtual std::vector<int> evaluate_ui() const = 0;

		/// <summary>
		/// Returns UI-suitable vector representation of the state inverted to the current one. 
		/// </summary>
		[[nodiscard]] virtual std::vector<int> evaluate_ui_inverted() const = 0;

		/// <summary>
		/// Returns "true" the given move (represented with its ID) is a "capture" move.
		/// It is a responsibility of the caller to ensure validness of the provided "move ID".
		/// </summary>
		[[nodiscard]] virtual bool is_capture_action(const int action_id) const = 0;

		/// <summary>
		/// Returns "true" if the state is "inverted".
		/// </summary>
		[[nodiscard]] virtual bool is_inverted() const = 0;

		/// <summary>
		/// Returns "true" if the current state is a draw.
		/// </summary>
		[[nodiscard]] virtual bool is_draw() const = 0;
	};
}

