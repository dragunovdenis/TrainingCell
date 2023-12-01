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

namespace TrainingCell::Chess
{
	/// <summary>
	/// Handles piece-descriptors of checkers state
	/// </summary>
	class PieceController
	{
	public:
		/// <summary>
		/// Number of bits that are reserved for a piece rank
		/// </summary>
		static constexpr int RankBitsCount = 3;

		/// <summary>
		/// Bitmask used to extract plain rank of a piece from the "full" piece token
		/// (that can possibly contain flags and sign)
		/// </summary>
		static constexpr int MinBitMask = (1 << RankBitsCount) - 1;

		/// <summary>
		/// Number of bits reserved for flags.
		/// </summary>
		static constexpr int FlagBitsCount = 3;

		/// <summary>
		/// A flag telling that the piece is in its initial position
		/// </summary>
		static constexpr int InitialPositionFlag = 1 << RankBitsCount;

		/// <summary>
		/// A flag that a Pawn piece should be marked with in case it can be captured under the "En Passant Rule".
		/// </summary>
		static constexpr int EnPassantFlag = InitialPositionFlag << 1;

		/// <summary>
		/// A flag to mark ani-pieces
		/// </summary>
		static constexpr int AntiPieceFlag = InitialPositionFlag << 2;

		/// <summary>
		/// Total number of bits (except the sign bit) set aside for the piece token.
		/// </summary>
		static constexpr int TotalBitsCount = RankBitsCount + FlagBitsCount;

		/// <summary>
		/// Mask to cover all the bits of the piece rank token including the sign bit.
		/// </summary>
		static constexpr int BitMask = (1 << TotalBitsCount) - 1;

		/// <summary>
		/// Piece ranks
		/// </summary>
		static constexpr int Space = 0;
		static constexpr int Pawn = 1;
		static constexpr int Bishop = 2;
		static constexpr int Knight = 3;
		static constexpr int Rook = 4;
		static constexpr int Queen = 5;
		static constexpr int King = 6;

		/// <summary>
		/// Rook and King in their initial positions (required to determine the "castle" position).
		/// </summary>
		static constexpr int InitRook = Rook | InitialPositionFlag;
		static constexpr int InitKing = King | InitialPositionFlag;

		/// <summary>
		/// A pawn in the "En Passant" position.
		/// </summary>
		static constexpr int PawnEnPassant = Pawn | EnPassantFlag;

		/// <summary>
		/// Returns integer representation of an ani-piece for the given piece;
		/// preserves all the flags of the input piece;
		/// </summary>
		static int anti(const int piece_token);

		/// <summary>
		/// Returns "true" if the given piece is an "ally" one (i.e., has positive integer token)
		/// </summary>
		static bool is_ally_piece(const int piece_token);

		/// <summary>
		/// Returns "true" if the given piece is a "rival" one (i.e., has negative integer token)
		/// </summary>
		static bool is_rival_piece(const int piece_token);

		/// <summary>
		/// Returns "true" if the given piece is a "space" (i.e., has zero integer token)
		/// </summary>
		static bool is_space(const int piece_token);

		/// <summary>
		/// Returns "true" if the given piece token represents a valid piece.
		/// </summary>
		static bool is_piece(const int piece_token);

		/// <summary>
		/// Returns "true" if the given token represents a "positive" king.
		/// </summary>
		static bool is_king(const int piece_token);

		/// <summary>
		/// Returns "true" if the given token represents a "positive" pawn.
		/// </summary>
		static bool is_pawn(const int piece_token);

		/// <summary>
		/// Returns "true" if the given piece is in its "initial" position on the board;
		/// </summary>
		static bool is_in_init_pos(const int piece_token);

		/// <summary>
		/// Returns "true" if the given piece has "en passant" flag
		/// </summary>
		static bool is_en_passant(const int piece_token);

		/// <summary>
		/// Returns minimal piece "rank" token from the given "full" rank token (i.e., without any extra data like flags, sign).
		/// </summary>
		static int extract_min_piece_rank(const int piece_token);

		/// <summary>
		/// Extracts minimal signed piece "rank" token from the given "full" rank token.
		/// </summary>
		static int extract_min_signed_piece_rank(const int piece_token);
		
		/// <summary>
		/// Returns full piece "rank" token from the given, i.e., erises those bits
		/// of the input token that kas nothing to do with the ranks
		/// </summary>
		static int extract_full_piece_rank(const int piece_token);

		/// <summary>
		/// Returns array representation of "checkerboard" with the chess pieces in the initial position on it.
		/// </summary>
		static std::vector<int> get_init_board_state();


	};
}
