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

namespace TrainingCell::Chess
{
	/// <summary>
	/// Controls move directions of chess pieces.
	/// </summary>
	class AttackController
	{
		/// <summary>
		/// 8 bits to encode each of the three direction groups. 
		/// </summary>
		static constexpr int BitsPerDirectionGroup = 8;

		/// <summary>
		/// The first group - iterative straight directions (rook, bishop, queen).
		/// </summary>
		static constexpr int LongRangeDirGroupMask = (1 << BitsPerDirectionGroup) - 1;

	public:

		/// <summary>
		/// Represents direction of a move.
		/// </summary>
		struct Direction
		{
			/// <summary>
			/// Increment of a current position of a piece that defines the direction.
			/// </summary>
			PiecePosition dir;

			/// <summary>
			/// Unique token encoding the current direction.
			/// </summary>
			int token;

			/// <summary>
			/// Returns "true" if the increment can be applied more than once, i.e., the attack direction is "long-range".
			/// </summary>
			bool is_long_range() const;

			/// <summary>
			/// Returns "true" if the given "end" position can be "reached" by
			/// adding (subtracting) sufficient number of current direction vectors to (from) "start" position,
			/// in which case `out_param` contains the corresponding multiplier.
			///	Returns "false" otherwise (in which case value of `out_param` should be discarded).
			///	IMPORTANT: in case the current attack direction is not a "long range" one, the only possible value of "out_param" is "1".
			/// </summary>
			bool can_reach(const PiecePosition& start, const PiecePosition& end, int& out_param) const;
		};

		/// <summary>
		/// Returns collection of all the possible attack move directions for the given piece represented with its token.
		/// </summary>
		static const std::vector<Direction>& get_attack_directions(const int piece_rank_token);

		/// <summary>
		/// Returns "king" attack directions.
		/// </summary>
		static const std::vector<Direction>& get_king_attack_directions();

		/// <summary>
		/// Returns "positive pawn" attack directions.
		/// </summary>
		static const std::vector<Direction>& get_pawn_attack_directions();

		/// <summary>
		/// Decodes given encoded iterative attack directions;
		/// </summary>
		static std::vector<Direction> decode_long_range_attack_directions(const int encoded_attack_directions);

		/// <summary>
		/// Total number of bits used by the controller.
		/// </summary>
		static constexpr int TotalBitsCount = 3 * BitsPerDirectionGroup;

		/// <summary>
		/// Mask covering all the bits used by the controller
		/// </summary>
		static constexpr int BitMask = (1 << TotalBitsCount) - 1;

	private:

		/// <summary>
		/// All the possible long-range straight attack directions.
		/// </summary>
		static constexpr std::vector<Direction> QueenDirections{
			{ { 1, 0 }, 1 << 0 },
			{ { -1, 0 }, 1 << 1 },
			{ { 0, 1 }, 1 << 2 },
			{ { 0, -1 }, 1 << 3 },
			{ { 1, 1 }, 1 << 4 },
			{ { -1, -1 }, 1 << 5 },
			{ { -1, 1 }, 1 << 6 },
			{ { 1, -1 }, 1 << 7 },
		};

		/// <summary>
		/// All the possible short-range attack directions.
		/// </summary>
		static constexpr std::vector<Direction> KingDirections{
			{ { 1, 0 }, 1 << BitsPerDirectionGroup },
			{ { -1, 0 }, 1 << (BitsPerDirectionGroup + 1) },
			{ { 0, 1 }, 1 << (BitsPerDirectionGroup + 2) },
			{ { 0, -1 }, 1 << (BitsPerDirectionGroup + 3) },
			{ { 1, 1 }, 1 << (BitsPerDirectionGroup + 4) },
			{ { -1, -1 }, 1 << (BitsPerDirectionGroup + 5) },
			{ { -1, 1 }, 1 << (BitsPerDirectionGroup + 6) },
			{ { 1, -1 }, 1 << (BitsPerDirectionGroup + 7) },
		};

		/// <summary>
		/// All the possible knight attack directions.
		/// </summary>
		static constexpr std::vector<Direction> KnightDirections{
			{ { 1, 2 }, 1 << (2 * BitsPerDirectionGroup) },
			{ { -1, -2 }, 1 << (2 * BitsPerDirectionGroup + 1) },
			{ { 2, 1 }, 1 << (2 * BitsPerDirectionGroup + 2) },
			{ { -2, -1 }, 1 << (2 * BitsPerDirectionGroup + 3) },
			{ { -1, 2 }, 1 << (2 * BitsPerDirectionGroup + 4) },
			{ { 1, -2 }, 1 << (2 * BitsPerDirectionGroup + 5) },
			{ { -2, 1 }, 1 << (2 * BitsPerDirectionGroup + 6) },
			{ { 2, -1 }, 1 << (2 * BitsPerDirectionGroup + 7) },
		};

		/// <summary>
		/// Collection of pawn attack directions.
		/// </summary>
		inline static const std::vector<Direction> PawnDirections{
				KingDirections[4],
				KingDirections[7] };

		/// <summary>
		/// Collection of anti-pawn attack directions.
		/// </summary>
		inline static const std::vector<Direction> AntiPawnDirections{
				KingDirections[5],
				KingDirections[6] };

		/// <summary>
		/// Collection of bishop attack directions.
		/// </summary>
		inline static const std::vector<Direction> BishopDirections{
				QueenDirections[4],
				QueenDirections[5],
				QueenDirections[6],
				QueenDirections[7] };

		/// <summary>
		/// Collection of rook attack directions.
		/// </summary>
		inline static const std::vector<Direction> RookDirections{
				QueenDirections[0],
				QueenDirections[1],
				QueenDirections[2],
				QueenDirections[3] };
	};
}