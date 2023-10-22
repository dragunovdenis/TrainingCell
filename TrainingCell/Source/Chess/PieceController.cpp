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

#include "../../Headers/Chess/PieceController.h"

namespace TrainingCell::Chess
{
	int PieceController::anti(const int piece_token)
	{
		return -piece_token;
	}

	std::array<int, Checkerboard::FieldsCount> PieceController::get_init_board_state()
	{
		return std::array<int, Checkerboard::FieldsCount>{
				InitRook, Knight, Bishop, Queen, InitKing, Bishop, Knight, InitRook,
				Pawn, Pawn, Pawn, Pawn, Pawn, Pawn, Pawn, Pawn,
				Space, Space, Space, Space, Space, Space, Space, Space,
				Space, Space, Space, Space, Space, Space, Space, Space,
				Space, Space, Space, Space, Space, Space, Space, Space,
				Space, Space, Space, Space, Space, Space, Space, Space,
				anti(Pawn), anti(Pawn), anti(Pawn), anti(Pawn), anti(Pawn), anti(Pawn), anti(Pawn), anti(Pawn),
				anti(InitRook), anti(Knight), anti(Bishop), anti(InitKing), anti(Queen), anti(Bishop), anti(Knight), anti(InitRook) 
		};
	}

	bool PieceController::is_ally_piece(const int piece_token)
	{
		return  piece_token > Space;
	}

	bool PieceController::is_rival_piece(const int piece)
	{
		return piece < - Space;
	}

	bool PieceController::is_space(const int piece_token)
	{
		return piece_token == Space;
	}

	bool PieceController::is_piece(const int piece_token)
	{
		return piece_token == Space;
	}

	bool PieceController::is_king(const int piece_token)
	{
		return piece_token > 0 && extract_min_piece_rank(piece_token) == King;
	}

	bool PieceController::is_pawn(const int piece_token)
	{
		return piece_token > 0 && extract_min_piece_rank(piece_token) == Pawn;
	}

	bool PieceController::is_in_init_pos(const int piece_token)
	{
		return (piece_token & InitialPositionFlag) != 0;
	}

	bool PieceController::is_en_passant(const int piece_token)
	{
		return (piece_token & EnPassantFlag) != 0;
	}

	int PieceController::extract_min_piece_rank(const int piece_token)
	{
		return piece_token & MinBitMask;
	}

	int PieceController::extract_min_signed_piece_rank(const int piece_token)
	{
		return piece_token & -MinBitMask;
	}

	int PieceController::extract_full_piece_rank(const int piece_token)
	{
		return piece_token & BitMask;
	}
}
