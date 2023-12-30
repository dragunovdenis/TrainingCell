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

#include "../../Headers/Checkers/CheckersMove.h"
#include "../../Headers/Move.h"

namespace TrainingCell::Checkers
{
	void CheckersMove::continue_with(const CheckersMove& continuation)
	{
		if (finish != continuation.start)
			throw std::exception("Invalid continuation");

		finish = continuation.finish;

		captures.insert(captures.end(), continuation.captures.begin(), continuation.captures.end());
	}

	CheckersMove::CheckersMove(const PiecePosition& start, const PiecePosition& finish) :
	start(start), finish(finish) {}

	CheckersMove::CheckersMove(const PiecePosition& start, const PiecePosition& finish,
		const std::vector<PiecePosition>& captures) :
		start(start), finish(finish), captures(captures) {}

	/// <summary>
	/// Returns next position of a piece that currently is located at the given "prev_piece_pos" and about to capture
	/// rival pieces on positions "capture_pos" and "next_capture_pos".
	/// </summary>
	PiecePosition calculate_intermediate_capture_position(const PiecePosition& prev_piece_pos,
		const PiecePosition& capture_pos, const PiecePosition& next_capture_pos)
	{
		const auto v = capture_pos - prev_piece_pos;

		if (std::abs(v.row) != std::abs(v.col) || v.row == 0 || capture_pos == next_capture_pos)
			throw std::exception("Piece position must be on the same diagonal as the capture");

		const auto delta = v.col * v.col + v.row * v.row;
		const auto c_1 = prev_piece_pos.row * v.col - prev_piece_pos.col * v.row;
		const auto c_2 = next_capture_pos.row * v.row + next_capture_pos.col * v.col;
		const auto delta_row = c_1 * v.col + c_2 * v.row;
		const auto delta_col = c_2 * v.col - c_1 * v.row;

		if (delta_row % delta != 0 || delta_col % delta != 0)
			throw std::exception("Invalid input");

		const PiecePosition result{ delta_row / delta, delta_col / delta };

		if (result != next_capture_pos)
			return result;

		// this is the case when both capture positions are on the same diagonal
		// sanity check
		const auto capture_diff = next_capture_pos - capture_pos;
		if (std::abs(capture_diff.col) != std::abs(capture_diff.row) || std::abs(capture_diff.col) < 2)
			throw std::exception("Impossible to find a position between two captures");

		return capture_pos + v / std::abs(v.row);
	}

	std::vector<SubMove> CheckersMove::to_sub_moves() const
	{
		std::vector<SubMove> result;

		auto prev_pos = start;
		for (auto capture_id = 0; capture_id < static_cast<int>(captures.size()) - 1; ++capture_id)
		{
			const auto& capture = captures[capture_id];
			const auto next_pos = calculate_intermediate_capture_position(prev_pos,
				capture, captures[capture_id + 1]);

			result.push_back(SubMove{ prev_pos , next_pos, capture });
			prev_pos = next_pos;
		}

		SubMove final_sub_move{ prev_pos , finish };
		if (!captures.empty())
			final_sub_move.capture = *captures.rbegin();

		result.push_back(final_sub_move);

		return result;
	}

	Move CheckersMove::to_move() const
	{
		return { to_sub_moves() };
	}

	bool CheckersMove::is_capturing() const
	{
		return !captures.empty();
	}

	CheckersMove::CheckersMove(const Move& source)
	{
		if (source.sub_moves.empty())
			throw std::exception("Invalid argument");

		start = source.sub_moves.begin()->start;
		finish = source.sub_moves.rbegin()->end;

		if (source.sub_moves.begin()->capture.is_valid())
			for (const auto& sub_move : source.sub_moves)
			{
				if (!sub_move.capture.is_valid())
					throw std::exception("Invalid argument");

				captures.push_back(sub_move.capture);
			}
	}

	bool CheckersMove::operator==(const CheckersMove& anotherMove) const
	{
		return start == anotherMove.start && finish == anotherMove.finish &&
			captures == anotherMove.captures;
	}

	CheckersMove CheckersMove::invalid()
	{
		return { PiecePosition::invalid(), PiecePosition::invalid(), {} };
	}
}
