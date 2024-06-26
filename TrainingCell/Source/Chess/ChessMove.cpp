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

#include "../../Headers/Chess/ChessMove.h"
#include "../../Headers/Chess/PosController.h"
#include "../../Headers/Move.h"

namespace TrainingCell::Chess
{
	PiecePosition ChessMove::get_start() const
	{
		return PosController::from_linear(start_field_id);
	}

	PiecePosition ChessMove::get_finish() const
	{
		return PosController::from_linear(finish_field_id);
	}

	int ChessMove::get_final_piece_rank(const int start_piece_rank) const
	{
		return final_rank != PieceController::Space ? final_rank : start_piece_rank;
	}

	ChessMove::ChessMove(const int start_field_id, const int finish_field_id, const bool captures, const int final_rank) :
		start_field_id(start_field_id), finish_field_id(finish_field_id), captures(captures), final_rank(final_rank)
	{}

	bool ChessMove::is_capturing() const
	{
		return captures;
	}

	Move ChessMove::to_move() const
	{
		return Move{ {PosController::from_linear(start_field_id), PosController::from_linear(finish_field_id)}, final_rank };
	}

	bool ChessMove::operator==(const ChessMove& anotherMove) const
	{
		return start_field_id == anotherMove.start_field_id &&
			   finish_field_id == anotherMove.finish_field_id &&
			   captures == anotherMove.captures &&
			   final_rank == anotherMove.final_rank;
	}

	ChessMove ChessMove::invalid()
	{
		return { -1, -1, false, -1 };
	}
}
