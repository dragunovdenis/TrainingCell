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
#include <vector>

namespace TrainingCell
{
	/// <summary>
	/// An interface providing an agent with a "minimal" read-only access to an environment's state
	/// </summary>
	class IMinimalStateReadonly
	{
	public:
		/// <summary>
		/// Virtual destructor
		/// </summary>
		virtual ~IMinimalStateReadonly() = default;

		/// <summary>
		/// Returns number of available moves to take in the current state of environment.
		/// It is assumed that integer values from "0" to "moves count" - 1  all represent
		/// valid ID's of the available moves.
		/// </summary>
		[[nodiscard]] virtual int get_moves_count() const = 0;

		/// <summary>
		/// Returns "int-vector" representation of the current state after a move with the given ID was "applied" to it.
		/// It is a responsibility of the caller to ensure validness of the provided "move ID".
		/// </summary>
		[[nodiscard]] virtual std::vector<int> evaluate(const int move_id) const = 0;

		/// <summary>
		/// Returns "int-vector" representation of the current state (without any move "applied").
		/// </summary>
		[[nodiscard]] virtual std::vector<int> evaluate() const = 0;

		/// <summary>
		/// For the given pair of previous and next states represented with "int-vectors",
		/// calculates reward "suggested" by the difference between the states.
		///</summary>
		[[nodiscard]] virtual double calc_reward(const std::vector<int>& prev_state,
		                                         const std::vector<int>& next_state) const = 0;
	};
}
