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
		return is_space(piece_token) ? piece_token : (piece_token ^ AntiPieceFlag);
	}

	std::vector<int> PieceController::get_init_board_state()
	{
		return std::vector{
				InitRook, Knight, Bishop, InitKing, Queen, Bishop, Knight, InitRook,
				Pawn, Pawn, Pawn, Pawn, Pawn, Pawn, Pawn, Pawn,
				Space, Space, Space, Space, Space, Space, Space, Space,
				Space, Space, Space, Space, Space, Space, Space, Space,
				Space, Space, Space, Space, Space, Space, Space, Space,
				Space, Space, Space, Space, Space, Space, Space, Space,
				anti(Pawn), anti(Pawn), anti(Pawn), anti(Pawn), anti(Pawn), anti(Pawn), anti(Pawn), anti(Pawn),
				anti(InitRook), anti(Knight), anti(Bishop), anti(InitKing), anti(Queen), anti(Bishop), anti(Knight), anti(InitRook)
		};
	}

	std::vector<int> PieceController::get_ally_pieces()
	{
		return { Pawn, Rook, Knight, Bishop, Queen, King };
	}

	std::vector<int> PieceController::get_rival_pieces()
	{
		return { anti(Pawn), anti(Rook), anti(Knight), anti(Bishop), anti(Queen), anti(King) };
	}

	bool PieceController::is_ally_piece(const int piece_token)
	{
		return  is_piece(piece_token) && (piece_token & AntiPieceFlag) == 0;
	}

	bool PieceController::is_rival_piece(const int piece_token)
	{
		return (piece_token & AntiPieceFlag) != 0;
	}

	bool PieceController::is_space(const int piece_token)
	{
		return piece_token == Space;
	}

	bool PieceController::is_piece(const int piece_token)
	{
		return piece_token != Space;
	}

	bool PieceController::is_king(const int piece_token)
	{
		return is_ally_piece(piece_token) && is_a_king(piece_token);
	}

	bool PieceController::is_a_king(const int piece_token)
	{
		return extract_min_piece_rank(piece_token) == King;
	}

	bool PieceController::is_pawn(const int piece_token)
	{
		return is_ally_piece(piece_token) && extract_min_piece_rank(piece_token) == Pawn;
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
		const auto result_unsigned = extract_min_piece_rank(piece_token);
		return result_unsigned | (piece_token & AntiPieceFlag);
	}

	int PieceController::extract_full_piece_rank(const int piece_token)
	{
		return piece_token & BitMask;
	}
}
