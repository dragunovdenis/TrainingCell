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
#include "SubMove.h"

namespace TrainingCell
{
	/// <summary>
	/// Representation of a compound "move" in the checkers game
	/// </summary>
	class Move
	{
	public:
		/// <summary>
		/// Component moves
		/// </summary>
		std::vector<SubMove> sub_moves{};

		/// <summary>
		/// Default constructor
		/// </summary>
		Move() = default;

		/// <summary>
		/// Constructs "move" from a single "sub-move"
		/// </summary>
		Move(const SubMove& sub_move);

		/// <summary>
		/// "Inverts" the move, i.e. aligns the sub-move with "inverted" state
		/// </summary>
		void invert();

		/// <summary>
		/// Returns "inverted" move
		/// </summary>
		[[nodiscard]] Move get_inverted() const;
	};
}
