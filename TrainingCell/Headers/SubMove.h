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
#include "PiecePosition.h"

namespace TrainingCell
{
	/// <summary>
	/// The simplest move
	/// </summary>
	struct SubMove
	{
		/// <summary>
		/// Start position of the piece that "moves"
		/// </summary>
		PiecePosition start;
		/// <summary>
		/// End position of the piece that "moves"
		/// </summary>
		PiecePosition end;
		/// <summary>
		/// Position of a captured piece (if valid)
		/// </summary>
		PiecePosition capture;

		/// <summary>
		/// "Inverts" the sub-move, i.e. aligns the sub-move with "inverted" state
		/// </summary>
		void invert();

		/// <summary>
		/// Returns "inverted" sub-move
		/// </summary>
		[[nodiscard]] SubMove get_inverted() const;
	};
}
