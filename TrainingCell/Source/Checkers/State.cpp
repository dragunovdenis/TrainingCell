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

#include "../../Headers/Checkers/State.h"
#include <algorithm>
#include "../../../DeepLearning/DeepLearning/Math/Tensor.h"

namespace TrainingCell::Checkers
{
	int StateScore::operator [](const Piece& pc) const
	{
		return _scores[static_cast<int>(pc) - static_cast<int>(Piece::MinValue)];
	}

	int& StateScore::operator [](const Piece& pc)
	{
		return _scores[static_cast<int>(pc) - static_cast<int>(Piece::MinValue)];
	}

	StateScore StateScore::diff(const StateScore& score_to_subtract) const
	{
		StateScore result{};

		for (auto piece = Piece::MinValue; piece <= Piece::MaxValue; 
			piece = static_cast<Piece>(static_cast<int>(piece) + 1))
			result[piece] = (*this)[piece] - score_to_subtract[piece];

		return result;
	}

	bool PiecePosition::is_valid() const
	{
		return col >= 0 && col < BoardColumns && row >= 0 && row < BoardRows && 
			(col % 2 == (row % 2 == 0) ? 1 : 0);
	}

	PiecePosition PiecePosition::create_invalid()
	{
		return PiecePosition{ -1, -1 };
	}

	PiecePosition PiecePosition::move(const int step, bool rightDiagonal) const
	{
		return PiecePosition{ row + step, rightDiagonal ? col + step : col - step };
	}

	bool PiecePosition::operator ==(const PiecePosition& pos) const
	{
		return row == pos.row && col == pos.col;
	}

	bool PiecePosition::operator !=(const PiecePosition& pos) const
	{
		return !(*this == pos);
	}

	/// <summary>
	///	Returns square of the given value
	/// </summary>
	template <class T>
	T sqr(const T& val)
	{
		return val * val;
	}

	/// <summary>
	///	"Signum" function
	/// </summary>
	template <class T>
	long long signum(const T& val)
	{
		return val > 0 ? 1 : (val < 0 ? -1 : 0);
	}

	bool PiecePosition::is_same_diagonal(const PiecePosition& pos) const
	{
		return sqr(row - pos.row) == sqr(col - pos.col);
	}

	PiecePosition PiecePosition::move(const int step, const PiecePosition& pointer) const
	{
		if (!is_same_diagonal(pointer))
			throw std::exception("The pointer must be on the same diagonal");

		if ((*this) == pointer)
			throw std::exception("Pointer can't coincide with the current position");

		const auto row_dir = signum(pointer.row - row);
		const auto col_dir = signum(pointer.col - col);

		return PiecePosition{ row + row_dir * step, col + col_dir * step };
	}

	bool SubMove::is_valid() const
	{
		return start != end && start != capture && end != capture && start.is_same_diagonal(end) &&
			(!capture.is_valid() || (start.is_same_diagonal(capture) && end.is_same_diagonal(capture)));
	}

	void SubMove::invert()
	{
		start = Utils::invert(start);
		end = Utils::invert(end);
		capture = Utils::invert(capture);
	}

	SubMove SubMove::get_inverted() const
	{
		auto result = *this;
		result.invert();
		return result;
	}

	bool Move::is_valid() const
	{
		if (sub_moves.empty() || std::ranges::any_of(sub_moves, [](const auto& x) { return !x.is_valid(); }))
			return false;

		//The only case when number of captured pieces can differ from the number of sub-moves
		//is when there is only single sub-move and nothing is captured
		if (sub_moves.size() > 1 && std::ranges::any_of(sub_moves, [](const auto& x) { return !x.capture.is_valid(); }))
			return false;

		for (auto subMoveId = 1ull; subMoveId < sub_moves.size(); subMoveId++)
		{
			if (sub_moves[subMoveId].start != sub_moves[subMoveId - 1ull].end)
				return false;
		}

		return true;
	}

	void Move::append(const Move& move)
	{
		if (!move.is_valid())
			throw std::exception("Invalid move to append");

		sub_moves.insert(sub_moves.end(), move.sub_moves.begin(), move.sub_moves.end());

		if (!is_valid())
			throw std::exception("Inconsistent move");
	}

	Move::Move(const SubMove& sub_move)
	{
		sub_moves.push_back(sub_move);
	}

	void Move::invert()
	{
		for (auto& sub_move : sub_moves)
			sub_move.invert();
	}

	/// <summary>
	///	Returns "inverted" move
	/// </summary>
	Move Move::get_inverted() const
	{
		auto result = *this;
		result.invert();
		return result;
	}

	void State::remove_captured_pieces()
	{
		for (auto id = 0ull; id < size(); id++)
		{
			if ((*this)[id] == Piece::Captured)
				(*this)[id] = Piece::Space;
		}
	}

	State::State(const State_array& state_array, const bool inverted) : State_array(state_array), _inverted(inverted)
	{}

	bool State::is_inverted() const
	{
		return _inverted;
	}

	State State::get_start_state()
	{
		return State{ State_array{
				 Piece::Man, Piece::Man, Piece::Man, Piece::Man,
				 Piece::Man, Piece::Man, Piece::Man, Piece::Man,
				 Piece::Man, Piece::Man, Piece::Man, Piece::Man,
				 Piece::Space, Piece::Space, Piece::Space, Piece::Space,
				 Piece::Space, Piece::Space, Piece::Space, Piece::Space,
				 Piece::AntiMan, Piece::AntiMan, Piece::AntiMan, Piece::AntiMan,
				 Piece::AntiMan, Piece::AntiMan, Piece::AntiMan, Piece::AntiMan,
				 Piece::AntiMan, Piece::AntiMan, Piece::AntiMan, Piece::AntiMan,
		}, false};
	}

	Piece State::get_piece(const PiecePosition& position) const
	{
		return (*this)[piece_position_to_plain_id(position)];
	}

	Piece& State::get_piece(const PiecePosition& position)
	{
		return (*this)[piece_position_to_plain_id(position)];
	}

	PiecePosition State::plain_id_to_piece_position(const long long plain_id)
	{
		if (plain_id < 0 || plain_id >= StateSize)
			throw std::exception("Invalid input");

		const auto rowId = plain_id / FieldsInRow;
		const auto colId = (plain_id % FieldsInRow) * 2 + ((rowId % 2) == 0);
		return PiecePosition{ rowId, colId };
	}

	long long State::piece_position_to_plain_id(const PiecePosition& position)
	{
		if (!position.is_valid())
			throw std::exception("Invalid input");

		const auto temp = position.col - ((position.row % 2) == 0);

		if ((temp % 2) != 0)//sanity check, this situation must be handled withing the "is_valid" method
			throw std::exception("Given position represents white field!?");

		return position.row * FieldsInRow + temp / 2;
	}

	void State::invert()
	{
		const auto sz = size();
		const auto half_size = sz / 2;
		for (auto field_id = 0ull; field_id < half_size; field_id++)
		{
			const auto temp = (*this)[field_id];
			(*this)[field_id] = Utils::get_anti_piece((*this)[sz - 1 - field_id]);
			(*this)[sz - 1 - field_id] = Utils::get_anti_piece(temp);
		}

		_inverted = !_inverted;
	}

	StateScore State::calc_score() const
	{
		StateScore result{};
		std::ranges::for_each(*this, [&result](const auto& piece)
			{
				++result[piece];
			});

		return result;
	}

	DeepLearning::Tensor State::to_tensor() const
	{
		DeepLearning::Tensor result(1, 1, size(), false);
		std::ranges::transform(*this, result.begin(), 
			[](const auto& piece) { return static_cast<double>(piece); });
		return result;
	}

	std::vector<Move> State::get_moves() const
	{
		return Utils::get_moves(*this);
	}

	State State::get_inverted() const
	{
		auto result = *this;
		result.invert();
		return result;
	}

	bool State::is_valid_move(const Move& move) const
	{
		if (!move.is_valid())
			return false;

		const auto piece = get_piece(move.sub_moves[0].start);
		if (!Utils::is_allay_piece(piece))
			return false;

		for (auto subMoveId = 0ull; subMoveId < move.sub_moves.size(); subMoveId++)
		{
			const auto capture = move.sub_moves[subMoveId].capture;

			if (capture.is_valid() && !Utils::is_opponent_piece(get_piece(capture)))
				return false;

			auto temp = move.sub_moves[subMoveId].start;
			const auto end = move.sub_moves[subMoveId].end;

			if (piece == Piece::Man)
			{
				if (capture.is_valid())
				{
					if (std::abs(temp.row - end.row) != 2 || std::abs(temp.col - end.col) != 2)
						return false;
				}
				else
				{
					if (std::abs(temp.row - end.row) != 1 || std::abs(temp.col - end.col) != 1)
						return false;
				}
			}

			do
			{
				temp = temp.move(1, end);
				const auto currentPiece = get_piece(temp);
				//if it is an "opponent" piece then it must be the one captured
				if ((Utils::is_opponent_piece(currentPiece) && move.sub_moves[subMoveId].capture != temp) ||
					//if it is the "ally" piece then it must be the one that is "moving", because, in principle,
					//moving piece can cross its initial position during the move (even multiple times)
					(Utils::is_allay_piece(currentPiece) && temp != move.sub_moves[0].start) ||
					//we also assume that the "board" is free of "diagnostics stuff"
					(currentPiece != Piece::Space && !Utils::is_opponent_piece(currentPiece) && !Utils::is_allay_piece(currentPiece)))
					return false;
			} while (temp != end || !temp.is_valid());

			if (!temp.is_valid())
				return false;
		}

		return true;
	}

	void State::make_move(const Move& move, const bool remove_captured, const bool mark_trace)
	{
		if (!is_valid_move(move))
			throw std::exception("Invalid move");

		for (auto subMoveId = 0ull; subMoveId < move.sub_moves.size(); subMoveId++)
		{
			const auto capturePos = move.sub_moves[subMoveId].capture;

			if (capturePos.is_valid())
				get_piece(capturePos) = remove_captured ? Piece::Space : Piece::AntiCaptured;
		}

		auto piece_to_move = get_piece(move.sub_moves[0].start);

		//if our piece if a "Man" and during the move we have visited the last row of the board than
		//our pieced becomes a "King"
		if (piece_to_move == Piece::Man &&
			std::ranges::any_of(move.sub_moves, [](const auto& x) { return x.end.row == BoardRows - 1; }))
			piece_to_move = Piece::King;

		get_piece(move.sub_moves.rbegin()[0].end) = piece_to_move;

		if (mark_trace)
			for (auto subMoveId = 0ull; subMoveId < move.sub_moves.size(); subMoveId++)
				get_piece(move.sub_moves[subMoveId].start) = Piece::TraceMarker;
		else
			get_piece(move.sub_moves[0].start) = Piece::Space;
	}

	void State::make_move(const SubMove& sub_move, const bool remove_captured)
	{
		make_move({ {sub_move} }, remove_captured);
	}

	bool State::operator==(const State& another_state) const
	{
		return std::equal(begin(), end(), another_state.begin()) && is_inverted() == another_state.is_inverted();
	}

	bool State::operator!=(const State & another_state) const
	{
		return !(*this == another_state);
	}

	bool Utils::is_allay_piece(const Piece piece)
	{
		return piece == Piece::Man || piece == Piece::King;
	}

	bool Utils::is_alive(const Piece piece)
	{
		return is_allay_piece(piece) || is_opponent_piece(piece);
	}

	bool Utils::is_dead(const Piece piece)
	{
		return piece == Piece::AntiCaptured || piece == Piece::Captured;
	}

	bool Utils::is_king(const Piece piece)
	{
		return piece == Piece::AntiKing || piece == Piece::King;
	}

	bool Utils::is_opponent_piece(const Piece piece)
	{
		return is_allay_piece(get_anti_piece(piece));
	}

	bool Utils::is_trace_marker(const Piece piece)
	{
		return piece == Piece::TraceMarker || piece == Piece::AntiTraceMarker;
	}

	Piece Utils::get_anti_piece(const Piece& piece)
	{
		return static_cast<Piece>(-static_cast<int>(piece));
	}

	PiecePosition Utils::invert(const PiecePosition& pos)
	{
		return PiecePosition{ BoardRows - 1 - pos.row,  BoardColumns - 1 - pos.col };
	}

	std::vector<SubMove> Utils::get_capturing_moves(const State& current_state, const PiecePosition& pos,
		const bool right_diagonal, const bool positive_direction)
	{
		if (!pos.is_valid())
			throw std::exception("Invalid current position");

		if (!is_allay_piece(current_state.get_piece(pos)))
			throw std::exception("Invalid position of a piece");

		const auto step = positive_direction ? 1 : -1;
		auto temp_pos = pos;
		auto search_dist = 0;
		const auto max_search_dist = current_state.get_piece(pos) == Piece::Man ? 1 : std::numeric_limits<int>::max();

		do
		{
			search_dist += step;
			temp_pos = pos.move(search_dist, right_diagonal);
		} while (std::abs(search_dist) < max_search_dist && temp_pos.is_valid() && current_state.get_piece(temp_pos) == Piece::Space);

		if (!temp_pos.is_valid() || !is_opponent_piece(current_state.get_piece(temp_pos)))
			return {}; //Nothing to capture

		const auto pos_to_capture = temp_pos;

		std::vector<SubMove> result;

		do
		{
			search_dist += step;
			temp_pos = pos.move(search_dist, right_diagonal);

			if (temp_pos.is_valid() && current_state.get_piece(temp_pos) == Piece::Space)
				result.push_back(SubMove{pos, temp_pos, pos_to_capture });
			else
				break;

		} while (std::abs(search_dist) < max_search_dist);

		return result;
	}

	std::vector<SubMove> Utils::get_non_capturing_moves(const State& current_state, const PiecePosition& pos,
		const bool right_diagonal, const bool positive_direction)
	{
		if (!pos.is_valid())
			throw std::exception("Invalid current position");

		if (!is_allay_piece(current_state.get_piece(pos)))
			throw std::exception("Invalid position of a piece");

		const auto step = positive_direction ? 1 : -1;
		auto temp_pos = pos;
		auto search_dist = 0;
		const auto max_search_dist = current_state.get_piece(pos) == Piece::Man ? 1 : std::numeric_limits<int>::max();

		std::vector<SubMove> result{};

		do
		{
			search_dist += step;
			temp_pos = pos.move(search_dist, right_diagonal);

			if (temp_pos.is_valid() && current_state.get_piece(temp_pos) == Piece::Space)
			{
				result.push_back(SubMove{ pos, temp_pos, PiecePosition::create_invalid()});
			} else
				break;
		} while (std::abs(search_dist) < max_search_dist);

		return result;
	}
	
	std::vector<Move> Utils::get_capturing_moves(const State& current_state, const PiecePosition& start_pos)
	{
		if (!start_pos.is_valid())
			throw std::exception("Invalid start position");

		const auto piece = current_state.get_piece(start_pos);

		if (!is_allay_piece(piece))
			throw std::exception("Invalid input data");

		std::vector<Move> result{};

		for (const auto right_diagonal : {false, true})
		{
			for (const auto positive_direction : {false, true})
			{
				auto capturing_sub_moves = get_capturing_moves(current_state, start_pos, right_diagonal, positive_direction);

				if (capturing_sub_moves.empty())
					continue;

				if (piece == Piece::Man)
				{
					if (capturing_sub_moves.size() != 1)
						throw std::exception("Unexpected number of sub-moves for a Man piece in a single direction");

					if (capturing_sub_moves[0].end.row == (BoardRows - 1))
					{
						//We reached the last row and became a King
						//According to the rules, we must stop and let the opponent to make a move
						result.emplace_back(capturing_sub_moves[0]);
						continue;
					}
				}

				for (const auto& sub_move : capturing_sub_moves)
				{
					auto state_copy = current_state;
					state_copy.make_move(sub_move, false);
					auto continuation_moves = get_capturing_moves(state_copy, sub_move.end);

					const Move base_move(sub_move);

					result.push_back(base_move);

					for (const auto& move : continuation_moves)
					{
						auto base_move_copy = base_move;
						base_move_copy.append(move);
						result.push_back(base_move_copy);
					}
				}
			}
		}

		return result;
	}

	std::vector<Move> Utils::get_non_capturing_moves(const State& current_state, const PiecePosition& start_pos)
	{
		if (!start_pos.is_valid())
			throw std::exception("Invalid start position");

		if (!is_allay_piece(current_state.get_piece(start_pos)))
			throw std::exception("Invalid input data");

		std::vector<Move> result{};

		const auto piece = current_state.get_piece(start_pos);

		for (const auto right_diagonal : { false, true })
		{
			for (const auto positive_direction : { false, true })
			{
				if (piece == Piece::Man && !positive_direction)
					continue; //man can't move backwards

				auto non_capturing_sub_moves = get_non_capturing_moves(
					current_state, start_pos, right_diagonal, positive_direction);

				for (const auto& sub_move : non_capturing_sub_moves)
					result.emplace_back(sub_move);
			}
		}

		return result;
	}

	std::vector<Move> Utils::get_capturing_moves(const State& current_state)
	{
		std::vector<Move> result{};

		for (auto field_id = 0; field_id < static_cast<int>(current_state.size()); field_id++)
		{
			if (!is_allay_piece(current_state[field_id]))
				continue;

			const auto moves = get_capturing_moves(current_state, State::plain_id_to_piece_position(field_id));

			if (moves.empty())
				continue;

			result.insert(result.end(), moves.begin(), moves.end());
		}

		return result;
	}

	std::vector<Move> Utils::get_non_capturing_moves(const State& current_state)
	{
		std::vector<Move> result{};

		for (auto field_id = 0; field_id < static_cast<int>(current_state.size()); field_id++)
		{
			if (!is_allay_piece(current_state[field_id]))
				continue;

			const auto moves = get_non_capturing_moves(current_state, State::plain_id_to_piece_position(field_id));

			if (moves.empty())
				continue;

			result.insert(result.end(), moves.begin(), moves.end());
		}

		return result;
	}

	std::vector<Move> Utils::get_moves(const State& current_state)
	{
		const auto capturing_moves = get_capturing_moves(current_state);

		if (!capturing_moves.empty())
			return capturing_moves;

		return get_non_capturing_moves(current_state);
	}
}
