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
#include "PieceController.h"
#include "../PiecePosition.h"

namespace TrainingCell
{
	struct Move;
}

namespace TrainingCell::Chess
{
	/// <summary>
	/// A move instruction.
	/// </summary>
	class ChessMove
	{
		/// <summary>
		/// Only ChessState can construct a valid "move".
		/// </summary>
		friend class ChessState;

		/// <summary>
		/// Index of the "start" field.
		/// </summary>
		int start_field_id{ -1 };

		/// <summary>
		/// Index of the "finish" field.
		/// </summary>
		int finish_field_id{ -1 };

		/// <summary>
		/// Flag indicating if the current move "captures".
		/// </summary>
		bool captures{};

		/// <summary>
		/// If nonzero, defines "rank" of the piece (that moves) once it arrives to the "final" position (to do the "Pawn Promotion").
		/// Should be ignored if zero.
		/// </summary>
		int final_rank{ 0 };

		/// <summary>
		/// Returns position of the "start" field.
		/// </summary>
		[[nodiscard]] PiecePosition get_start() const;

		/// <summary>
		/// Returns position of the "finish" field.
		/// </summary>
		[[nodiscard]] PiecePosition get_finish() const;

		/// <summary>
		/// Deduces rank of the moving piece after applying the current move.
		/// </summary>
		[[nodiscard]] int get_final_piece_rank(const int start_piece_rank) const;

		/// <summary>
		/// Constructor.
		/// </summary>
		ChessMove(const int start_field_id, const int finish_field_id, const bool captures, const int final_rank = PieceController::Space);

	public:

		/// <summary>
		/// Constructor.
		/// </summary>
		ChessMove() = default;

		/// <summary>
		/// Returns "true" if the move "captures".
		/// </summary>
		bool is_capturing() const;

		/// <summary>
		/// Convertor to "general move".
		/// </summary>
		Move to_move() const;
	};

}
