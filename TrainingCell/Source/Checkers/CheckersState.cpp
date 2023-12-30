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

#include <algorithm>
#include "../../Headers/Checkers/CheckersState.h"
#include "../../Headers/Checkers/StateHandle.h"

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

	CheckersState::CheckersState(const State_array& state_array, const bool inverted) : State_array(state_array), _inverted(inverted)
	{}

	bool CheckersState::is_inverted() const
	{
		return _inverted;
	}

	CheckersState CheckersState::get_start_state()
	{
		return CheckersState{ State_array{
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

	Piece CheckersState::get_piece(const PiecePosition& position) const
	{
		return (*this)[piece_position_to_plain_id(position)];
	}

	Piece& CheckersState::get_piece(const PiecePosition& position)
	{
		return (*this)[piece_position_to_plain_id(position)];
	}

	PiecePosition CheckersState::plain_id_to_piece_position(const long long plain_id)
	{
		if (plain_id < 0 || plain_id >= StateSize)
			throw std::exception("Invalid input");

		const auto rowId = plain_id / FieldsInRow;
		const auto colId = (plain_id % FieldsInRow) * 2 + ((rowId % 2) == 0);
		return PiecePosition{ rowId, colId };
	}

	long long CheckersState::piece_position_to_plain_id_unsafe(const PiecePosition& position)
	{
		const auto temp = position.col - ((position.row % 2) == 0);

		if ((temp % 2) != 0)//sanity check, this situation must be handled withing the "is_valid" method
			throw std::exception("Given position represents white field!?");

		return position.row * FieldsInRow + temp / 2;
	}

	long long CheckersState::piece_position_to_plain_id(const PiecePosition& position)
	{
		if (!is_valid(position))
			throw std::exception("Invalid input");

		return piece_position_to_plain_id_unsafe(position);
	}

	template <class T>
	void CheckersState::invert_internal(T* state_array, const std::size_t size)
	{
		const auto half_size = size / 2;
		for (auto field_id = 0ull; field_id < half_size; field_id++)
		{
			const auto temp = state_array[field_id];
			state_array[field_id] = CheckersState::get_anti_piece(state_array[size - 1 - field_id]);
			state_array[size - 1 - field_id] = CheckersState::get_anti_piece(temp);
		}
	}

	void CheckersState::invert()
	{
		invert_internal(data(), size());
		_inverted = !_inverted;
	}

	std::vector<int> CheckersState::get_vector_inverted() const
	{
		auto result = to_vector();
		invert_internal(result.data(), result.size());
		return result;
	}

	template <class S>
	StateScore CheckersState::calc_score_internal(const S& state)
	{
		StateScore result{};
		std::ranges::for_each(state, [&result](const auto& piece)
			{
				++result[static_cast<Piece>(piece)];
			});

		return result;
	}

	StateScore CheckersState::calc_score() const
	{
		return calc_score_internal(*this);
	}

	double CheckersState::calc_reward(const std::vector<int>& prev_state, const std::vector<int>& next_state)
	{
		const auto prev_score = calc_score_internal(prev_state);
		const auto next_score = calc_score_internal(next_state);
		const auto diff_score = next_score.diff(prev_score);

		return (2.0 * diff_score[Piece::King] +
			diff_score[Piece::Man] -
			diff_score[Piece::AntiMan] -
			2.0 * diff_score[Piece::AntiKing]) / 50.0;
	}

	[[nodiscard]] std::vector<int> CheckersState::to_vector() const
	{
		std::vector<int> result(size());
		std::memcpy(result.data(), data(), size() * sizeof(int));

		return result;
	}

	std::size_t CheckersState::state_size()
	{
		return StateSize;
	}

	std::vector<int> CheckersState::expand_to_64(const std::vector<int>& state_vec)
	{
		std::vector<int> result(state_vec.size() * 2, 0);

		for (auto element_id = 0ull; element_id < state_vec.size(); ++element_id)
		{
			const auto pos = plain_id_to_piece_position(element_id);
			result[pos.row * Checkerboard::Columns + pos.col] = state_vec[element_id];
		}

		return result;
	}

	std::vector<int> CheckersState::to_vector_64() const
	{
		return expand_to_64(to_vector());
	}

	std::vector<int> CheckersState::to_vector_64_inverted() const
	{
		return expand_to_64(get_vector_inverted());
	}

	std::vector<CheckersMove> CheckersState::get_moves() const
	{
		std::vector<CheckersMove> result;
		get_moves(*this, result);
		return result;
	}

	bool CheckersState::get_moves(std::vector<CheckersMove>& out_result) const
	{
		get_moves(*this, out_result);
		return false; // it is never a draw in checkers.
	}

	CheckersState CheckersState::get_inverted() const
	{
		auto result = *this;
		result.invert();
		return result;
	}

	bool CheckersState::is_valid_move(const std::vector<SubMove>& sub_moves) const
	{
		if (!is_valid(sub_moves))
			return false;

		const auto piece = get_piece(sub_moves[0].start);
		if (!is_allay_piece(piece))
			return false;

		for (auto subMoveId = 0ull; subMoveId < sub_moves.size(); subMoveId++)
		{
			const auto capture = sub_moves[subMoveId].capture;

			if (is_valid(capture) && !is_opponent_piece(get_piece(capture)))
				return false;

			auto temp = sub_moves[subMoveId].start;
			const auto end = sub_moves[subMoveId].end;

			if (piece == Piece::Man)
			{
				if (is_valid(capture))
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
				temp = CheckersState::move(temp, 1, end);
				const auto currentPiece = get_piece(temp);
				//if it is an "opponent" piece then it must be the one captured
				if ((is_opponent_piece(currentPiece) && sub_moves[subMoveId].capture != temp) ||
					//if it is the "ally" piece then it must be the one that is "moving", because, in principle,
					//moving piece can cross its initial position during the move (even multiple times)
					(is_allay_piece(currentPiece) && temp != sub_moves[0].start) ||
					//we also assume that the "board" is free of "diagnostics stuff"
					(currentPiece != Piece::Space && !is_opponent_piece(currentPiece) && !is_allay_piece(currentPiece)))
					return false;
			} while (temp != end || !is_valid(temp));

			if (!is_valid(temp))
				return false;
		}

		return true;
	}

	template <class T>
	void CheckersState::make_move_internal(const CheckersMove& move, T* arr, const bool remove_captured)
	{
		for (const auto& capturePos : move.captures)
		{
			if (is_valid(capturePos))
				arr[piece_position_to_plain_id_unsafe(capturePos)] = static_cast<T>(remove_captured ? Piece::Space : Piece::AntiCaptured);
		}

		auto piece_to_move = arr[piece_position_to_plain_id_unsafe(move.start)];

		//if our piece if a "Man" and during the move we have visited the last row of the board than
		//our pieced becomes a "King"
		if (piece_to_move == static_cast<T>(Piece::Man) && move.finish.row == Checkerboard::Rows - 1)
			piece_to_move = static_cast<T>(Piece::King);

		arr[piece_position_to_plain_id_unsafe(move.finish)] = piece_to_move;
		arr[piece_position_to_plain_id_unsafe(move.start)] = static_cast<T>(Piece::Space);
	}

	void CheckersState::make_move(const CheckersMove& move, const bool remove_captured)
	{
#ifdef DIAGNOSTICS
		if (!is_valid_move(move.to_sub_moves()))
			throw std::exception("Invalid move");
#endif

		make_move_internal(move, data(), remove_captured);
	}

	void CheckersState::make_move(const CheckersMove& move)
	{
		make_move(move, true);
	}

	void CheckersState::make_move_and_invert(const CheckersMove& move)
	{
		make_move(move);
		invert();
	}

	std::vector<int> CheckersState::get_vector(const CheckersMove& move) const
	{
		auto result = to_vector();
		make_move_internal(move, result.data(), true /*remove captured*/);
		return result;
	}

	std::vector<int> CheckersState::get_vector_inverted(const CheckersMove& move) const
	{
		auto result = get_vector(move);
		invert_internal(result.data(), result.size());
		return result;
	}

	bool CheckersState::operator==(const CheckersState& another_state) const
	{
		return std::equal(begin(), end(), another_state.begin()) && is_inverted() == another_state.is_inverted();
	}

	bool CheckersState::operator!=(const CheckersState & another_state) const
	{
		return !(*this == another_state);
	}

	std::unique_ptr<IState> CheckersState::yield(const bool initialize_recorder) const
	{
		return std::make_unique<StateHandle>(*this, initialize_recorder);
	}

	StateTypeId CheckersState::type()
	{
		return StateTypeId::CHECKERS;
	}

	StateTypeId CheckersState::state_type() const
	{
		return type();
	}

	bool CheckersState::is_allay_piece(const Piece piece)
	{
		return piece == Piece::Man || piece == Piece::King;
	}

	bool CheckersState::is_alive(const Piece piece)
	{
		return is_allay_piece(piece) || is_opponent_piece(piece);
	}

	bool CheckersState::is_dead(const Piece piece)
	{
		return piece == Piece::AntiCaptured || piece == Piece::Captured;
	}

	bool CheckersState::is_king(const Piece piece)
	{
		return piece == Piece::AntiKing || piece == Piece::King;
	}

	bool CheckersState::is_opponent_piece(const Piece piece)
	{
		return is_allay_piece(get_anti_piece(piece));
	}

	bool CheckersState::is_trace_marker(const Piece piece)
	{
		return piece == Piece::TraceMarker || piece == Piece::AntiTraceMarker;
	}

	template <class P>
	P CheckersState::get_anti_piece(const P& piece)
	{
		return static_cast<P>(-static_cast<int>(piece));
	}

	std::vector<CheckersMove> CheckersState::get_capturing_moves(const CheckersState& current_state,
	                                                             const PiecePosition& pos,
	                                                             const bool right_diagonal,
	                                                             const bool positive_direction)
	{
		if (!is_valid(pos))
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
			temp_pos = move(pos, search_dist, right_diagonal);
		} while (std::abs(search_dist) < max_search_dist && is_valid(temp_pos)
			&& current_state.get_piece(temp_pos) == Piece::Space);

		if (!is_valid(temp_pos) || !is_opponent_piece(current_state.get_piece(temp_pos)))
			return {}; //Nothing to capture

		const auto pos_to_capture = temp_pos;

		std::vector<CheckersMove> result;

		do
		{
			search_dist += step;
			temp_pos = move(pos, search_dist, right_diagonal);

			if (is_valid(temp_pos) && current_state.get_piece(temp_pos) == Piece::Space)
				result.push_back(CheckersMove{ pos, temp_pos, {pos_to_capture} });
			else
				break;

		} while (std::abs(search_dist) < max_search_dist);

		return result;
	}

	std::vector<CheckersMove> CheckersState::get_non_capturing_moves(const CheckersState& current_state, const PiecePosition& pos,
		const bool right_diagonal, const bool positive_direction)
	{
		if (!is_valid(pos))
			throw std::exception("Invalid current position");

		if (!is_allay_piece(current_state.get_piece(pos)))
			throw std::exception("Invalid position of a piece");

		const auto step = positive_direction ? 1 : -1;
		auto temp_pos = pos;
		auto search_dist = 0;
		const auto max_search_dist = current_state.get_piece(pos) == Piece::Man ? 1 : std::numeric_limits<int>::max();

		std::vector<CheckersMove> result{};

		do
		{
			search_dist += step;
			temp_pos = move(pos, search_dist, right_diagonal);

			if (is_valid(temp_pos) && current_state.get_piece(temp_pos) == Piece::Space)
			{
				result.push_back(CheckersMove(pos, temp_pos));
			} else
				break;
		} while (std::abs(search_dist) < max_search_dist);

		return result;
	}

	std::vector<CheckersMove> CheckersState::get_capturing_moves(const CheckersState& current_state,
	                                                             const PiecePosition& start_pos)
	{
		if (!is_valid(start_pos))
			throw std::exception("Invalid start position");

		const auto piece = current_state.get_piece(start_pos);

		if (!is_allay_piece(piece))
			throw std::exception("Invalid input data");

		std::vector<CheckersMove> result{};

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

					if (capturing_sub_moves.begin()->finish.row == (Checkerboard::Rows - 1))
					{
						//We reached the last row and became a King
						//According to the rules, we must stop and let the opponent to make a move
						result.emplace_back(capturing_sub_moves[0]);
						continue;
					}
				}

				for (const auto& base_move : capturing_sub_moves)
				{
					auto state_copy = current_state;
					state_copy.make_move(base_move, false);
					const auto continuation_moves = get_capturing_moves(state_copy, base_move.finish);

					result.push_back(base_move);

					for (const auto& move : continuation_moves)
					{
						auto base_move_copy = base_move;
						base_move_copy.continue_with(move);
						result.push_back(base_move_copy);
					}
				}
			}
		}

		return result;
	}

	std::vector<CheckersMove> CheckersState::get_non_capturing_moves(const CheckersState& current_state,
	                                                                 const PiecePosition& start_pos)
	{
		if (!is_valid(start_pos))
			throw std::exception("Invalid start position");

		if (!is_allay_piece(current_state.get_piece(start_pos)))
			throw std::exception("Invalid input data");

		std::vector<CheckersMove> result{};

		const auto piece = current_state.get_piece(start_pos);

		for (const auto right_diagonal : { false, true })
		{
			for (const auto positive_direction : { false, true })
			{
				if (piece == Piece::Man && !positive_direction)
					continue; //man can't move backwards

				auto non_capturing_moves = get_non_capturing_moves(
					current_state, start_pos, right_diagonal, positive_direction);

				result.insert(result.end(), non_capturing_moves.begin(), non_capturing_moves.end());
			}
		}

		return result;
	}

	void CheckersState::get_capturing_moves(const CheckersState& current_state, std::vector<CheckersMove>& out_result)
	{
		out_result.clear();

		for (auto field_id = 0; field_id < static_cast<int>(current_state.size()); field_id++)
		{
			if (!is_allay_piece(current_state[field_id]))
				continue;

			const auto moves = get_capturing_moves(current_state, plain_id_to_piece_position(field_id));

			if (moves.empty())
				continue;

			out_result.insert(out_result.end(), moves.begin(), moves.end());
		}
	}

	void CheckersState::get_non_capturing_moves(const CheckersState& current_state, std::vector<CheckersMove>& out_result)
	{
		out_result.clear();

		for (auto field_id = 0; field_id < static_cast<int>(current_state.size()); field_id++)
		{
			if (!is_allay_piece(current_state[field_id]))
				continue;

			const auto moves = get_non_capturing_moves(current_state, CheckersState::plain_id_to_piece_position(field_id));

			if (moves.empty())
				continue;

			out_result.insert(out_result.end(), moves.begin(), moves.end());
		}
	}

	void CheckersState::get_moves(const CheckersState& current_state, std::vector<CheckersMove>& out_result)
	{
		get_capturing_moves(current_state, out_result);

		if (!out_result.empty())
			return;

		get_non_capturing_moves(current_state, out_result);
	}

	PiecePosition CheckersState::move(const PiecePosition& start_pos, const int step, bool rightDiagonal)
	{
		return PiecePosition{ start_pos.row + step, rightDiagonal ? start_pos.col + step : start_pos.col - step };
	}

	/// <summary>
	///	"Signum" function
	/// </summary>
	template <class T>
	long long signum(const T& val)
	{
		return val > 0 ? 1 : (val < 0 ? -1 : 0);
	}

	PiecePosition CheckersState::move(const PiecePosition& start_pos, const int step, const PiecePosition& pointer)
	{
		if (!is_same_diagonal(start_pos, pointer))
			throw std::exception("The pointer must be on the same diagonal");

		if (start_pos == pointer)
			throw std::exception("Pointer can't coincide with the current position");

		const auto row_dir = signum(pointer.row - start_pos.row);
		const auto col_dir = signum(pointer.col - start_pos.col);

		return PiecePosition{ start_pos.row + row_dir * step, start_pos.col + col_dir * step };
	}

	bool CheckersState::is_same_diagonal(const PiecePosition& start_pos, const PiecePosition& pos)
	{
		return std::abs(start_pos.row - pos.row) == std::abs(start_pos.col - pos.col);
	}

	bool CheckersState::is_valid(const PiecePosition& pos)
	{
		return pos.is_valid() && (pos.col % 2 == (pos.row % 2 == 0) ? 1 : 0);
	}

	bool CheckersState::is_valid(const SubMove& sub_move)
	{
		return sub_move.start != sub_move.end && sub_move.start != sub_move.capture &&
			sub_move.end != sub_move.capture && is_same_diagonal(sub_move.start, sub_move.end) &&
			(!is_valid(sub_move.capture) || (is_same_diagonal(sub_move.start, sub_move.capture) &&
				is_same_diagonal(sub_move.end, sub_move.capture)));
	}

	bool CheckersState::is_valid(const std::vector<SubMove>& sub_moves)
	{
		if (sub_moves.empty() || std::ranges::any_of(sub_moves, [](const auto& x) { return !CheckersState::is_valid(x); }))
			return false;

		//The only case when number of captured pieces can differ from the number of sub-moves
		//is when there is only single sub-move and nothing is captured
		if (sub_moves.size() > 1 && std::ranges::any_of(sub_moves, [](const auto& x) { return !CheckersState::is_valid(x.capture); }))
			return false;

		for (auto subMoveId = 1ull; subMoveId < sub_moves.size(); subMoveId++)
		{
			if (sub_moves[subMoveId].start != sub_moves[subMoveId - 1ull].end)
				return false;
		}

		return true;
	}
}
