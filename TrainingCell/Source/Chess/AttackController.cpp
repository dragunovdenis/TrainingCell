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

#include "../../Headers/Chess/AttackController.h"
#include "../../Headers/Chess/PieceController.h"

namespace TrainingCell::Chess
{
	bool AttackController::Direction::is_long_range() const
	{
		return (token & LongRangeDirGroupMask) != 0;
	}

	bool check_divisibility_and_calc_quotient(long long diff, long long inc, int& out_quotient)
	{
		const auto div_res = std::div(diff, inc);
		out_quotient = static_cast<int>(div_res.quot);
		return div_res.rem == 0;
	}

	bool AttackController::Direction::can_reach(const PiecePosition& start, const PiecePosition& end, int& out_param) const
	{
        if (dir.row == 0 && dir.col == 0)
            throw std::exception("Undefined direction");

		if (!is_long_range())
		{
			out_param = 1;
			return start + dir == end;
		}

		// The implementation below assumes that the current direction vector
		// can be represented in one of the following forms: (1, 1), (-1, 1), (1, -1), (-1, -1)

        if (dir.row == 0)
        {
			if (start.row == end.row)
			{
				out_param = static_cast<int>((end.col - start.col) * dir.col);
				return true;
			}

			return false;
        }

		out_param = static_cast<int>((end.row - start.row) * dir.row);

        if (dir.col == 0)
			return start.col == end.col;

		return out_param == static_cast<int>((end.col - start.col) * dir.col);
	}

	const std::vector<AttackController::Direction>& AttackController::get_attack_directions(const int piece_rank_token) const
	{
		switch (PieceController::extract_min_piece_rank(piece_rank_token))
		{
			case PieceController::Pawn:   return _pawn_directions;
			case PieceController::Bishop: return _bishop_directions;
			case PieceController::Rook:   return _rook_directions;
			case PieceController::Knight: return _knight_directions;
			case PieceController::Queen:  return _queen_directions;
			case PieceController::King:   return _king_directions;
			default:
				throw std::exception("Unknown piece token");
		}
	}

	const std::vector<AttackController::Direction>& AttackController::get_king_attack_directions() const
	{
		return _king_directions;
	}

	const std::vector<AttackController::Direction>& AttackController::get_pawn_attack_directions() const
	{
		return _pawn_directions;
	}

	std::vector<AttackController::Direction> AttackController::decode_long_range_attack_directions(const int encoded_attack_directions) const
	{
		std::vector<Direction> result;

		if ((encoded_attack_directions & LongRangeDirGroupMask) == 0)
			return result;

		for (const auto& dir : _queen_directions)
		{
			if ((dir.token & encoded_attack_directions) != 0)
				result.push_back(dir);
		}

		return result;
	}

	int AttackController::compress_attack_directions(const int encoded_attack_directions)
	{
		return (encoded_attack_directions & LongRangeDirGroupMask) |
			((encoded_attack_directions & ShortRangeDirGroupMask) != 0) << BitsPerDirectionGroup |
			((encoded_attack_directions & KnightDirGroupMask) != 0) << (BitsPerDirectionGroup + 1);
	}
}
