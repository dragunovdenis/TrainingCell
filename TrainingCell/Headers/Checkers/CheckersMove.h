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
#include "../PiecePosition.h"

namespace TrainingCell
{
	struct Move;
	struct SubMove;
}

namespace TrainingCell::Checkers
{


	/// <summary>
	/// Move instruction for checkers.
	/// </summary>
	class CheckersMove
	{
		friend class CheckersState;

		/// <summary>
		/// Start position of the move.
		/// </summary>
		PiecePosition start{};

		/// <summary>
		/// Finish position of the move.
		/// </summary>
		PiecePosition finish{};

		/// <summary>
		/// Coordinate of captured pieces ordered from start position to the finish position of the move.
		/// </summary>
		std::vector<PiecePosition> captures{};

		/// <summary>
		/// Appends "continuation" to the current move.
		/// </summary>
		void continue_with(const CheckersMove& continuation);

		/// <summary>
		/// Constructor.
		/// </summary>
		CheckersMove(const PiecePosition& start, const PiecePosition& finish);

		/// <summary>
		/// Constructor.
		/// </summary>
		CheckersMove(const PiecePosition& start, const PiecePosition& finish, const std::vector<PiecePosition>& captures);

		/// <summary>
		/// Converts the move into a collection of sub-moves.
		/// </summary>
		std::vector<SubMove> to_sub_moves() const;

	public:

		/// <summary>
		/// Converts the instance to "general move".
		/// </summary>
		Move to_move() const;

		/// <summary>
		/// Default constructor.
		/// </summary>
		CheckersMove() = default;

		/// <summary>
		/// Returns "true" if collection of capturing positions is not empty.
		/// </summary>
		bool is_capturing() const;

		/// <summary>
		/// Constructor.
		/// </summary>
		CheckersMove(const Move& source);
	};
}
