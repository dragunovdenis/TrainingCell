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

#include "../../Headers/Chess/ChessState.h"
#include "../../Headers/Chess/PieceController.h"
#include "../../Headers/Chess/PosController.h"
#include "../../Headers/Chess/StateHandle.h"
#include <algorithm>

namespace TrainingCell::Chess
{
	std::vector<ChessMove> ChessState::get_moves() const
	{
		std::vector<ChessMove> result;
		get_moves(result);
		return result;
	}

	bool ChessState::get_moves(std::vector<ChessMove>& out_result) const
	{
		out_result.clear();

		const auto king_field_id = locate_king();
		const auto king_pos = PosController::from_linear(king_field_id);

		append_king_moves(king_field_id, out_result);

		int piece_score_sum = 0;
		int alive_pieces_cnt = 0;

		for (auto field_id = 0; field_id < Checkerboard::FieldsCount; ++field_id)
		{

			alive_pieces_cnt += is_piece(field_id);
			piece_score_sum += min_piece_rank(field_id);

			if (!is_ally(field_id) || king_field_id == field_id)
				continue;

			if (is_pawn(field_id))
			{
				append_pawn_moves(field_id, out_result, king_pos);
				continue;
			}

			const auto& attack_directions =
				_attack_controller.get_attack_directions(_data[field_id].piece);
			append_moves(field_id, out_result, attack_directions, king_pos);
		}

		piece_score_sum -= 2 * PieceController::King;
		const auto stale_mate = out_result.empty() && !is_threatened(king_field_id);

		return stale_mate ||
			    // technical draw
			    (alive_pieces_cnt <= 3 &&
			    (piece_score_sum == 0 ||
				piece_score_sum == PieceController::Bishop ||
				piece_score_sum == PieceController::Knight));
	}

	void ChessState::make_move(const ChessMove& move)
	{
		ChessMove second_component;
		const auto compound_move = is_compound_move(move, second_component);
		make_move_and_update_attack_field(move);

		if (compound_move)
			make_move_and_update_attack_field(second_component);
	}

	void ChessState::make_move_and_invert(const ChessMove& move)
	{
		make_move(move);
		invert();
	}

	std::size_t ChessState::state_size()
	{
		return Checkerboard::FieldsCount;
	}

	std::vector<int> ChessState::get_vector(const ChessMove& move) const
	{
		// A sanity check
		if (!is_ally(move.start_field_id) || is_ally(move.finish_field_id))
			throw std::exception("Invalid move");

		auto result = to_vector();

		ChessMove second_component{};
		const auto compound_move = is_compound_move(move, second_component);

		make_move(result, move);

		if (compound_move)
			make_move(result, second_component);

		return result;
	}

	void ChessState::invert()
	{
		constexpr auto half_field_count = Checkerboard::FieldsCount / 2;

		for (auto field_id = 0; field_id < half_field_count; ++field_id)
		{
			auto& field = _data[field_id];
			const auto temp = field;
			auto& mirror_field = _data[Checkerboard::FieldsCount - field_id - 1];

			field.assign_inverted(mirror_field);
			mirror_field.assign_inverted(temp);
		}

		_is_inverted = !_is_inverted;
	}

	/// <summary>
	/// Inverts given state represented as a plain vector.
	/// </summary>
	void invert_state_vector(std::vector<int>& state_vec)
	{
		constexpr auto half_field_count = Checkerboard::FieldsCount / 2;

		for (auto field_id = 0; field_id < half_field_count; ++field_id)
		{
			const auto temp = state_vec[field_id];
			const auto mirror_field_id = Checkerboard::FieldsCount - field_id - 1;
			state_vec[field_id] = PieceController::anti(state_vec[mirror_field_id]);
			state_vec[mirror_field_id] = PieceController::anti(temp);
		}
	}

	std::vector<int> ChessState::get_vector_inverted() const
	{
		auto result = to_vector();
		invert_state_vector(result);
		return result;
	}

	std::vector<int> ChessState::to_vector_64() const
	{
		return to_vector<true>();
	}

	std::vector<int> ChessState::to_vector_64_inverted() const
	{
		auto state_copy = *this;
		state_copy.invert();
		return state_copy.to_vector<true>();
	}

	std::vector<int> ChessState::get_vector_inverted(const ChessMove& move) const
	{
		auto result = get_vector(move);
		invert_state_vector(result);
		return result;
	}

	void ChessState::make_move(std::vector<int>& state_vector, const ChessMove& move)
	{
		state_vector[move.finish_field_id] = PieceController::extract_min_piece_rank(move.get_final_piece_rank(state_vector[move.start_field_id]));
		state_vector[move.start_field_id] = PieceController::Space;
	}

	void ChessState::make_move_and_update_attack_field(const ChessMove& move)
	{
		const auto start = move.get_start();
		const auto finish = move.get_finish();

		if (!start.is_valid() || !finish.is_valid())
			throw std::exception("Invalid input positions");

		auto& start_field = _data[PosController::to_linear(start)];

		if (PieceController::is_rival_piece(start_field.piece))
			throw std::exception("Only ally piece can be moved");

		auto& finish_field = _data[PosController::to_linear(finish)];

		if (PieceController::is_ally_piece(finish_field.piece))
			throw std::exception("Can't capture an ally");

		commit_attack(_attack_controller.decode_long_range_attack_directions(start_field.rival_attack), start, true /*rival*/);
		withdraw_attack(_attack_controller.get_attack_directions(start_field.piece), start, false /*rival*/);
		commit_attack(_attack_controller.decode_long_range_attack_directions(start_field.ally_attack), start, false /*rival*/);

		const auto moving_piece = move.get_final_piece_rank(start_field.piece);
		start_field.piece = PieceController::Space;

		const auto rival_attacks_to_withdraw = PieceController::is_rival_piece(finish_field.piece) ?
			_attack_controller.get_attack_directions(finish_field.piece) :
			_attack_controller.decode_long_range_attack_directions(finish_field.rival_attack);

		withdraw_attack(rival_attacks_to_withdraw, finish, true /*rival*/);

		if (PieceController::is_space(finish_field.piece))
			withdraw_attack(_attack_controller.decode_long_range_attack_directions(finish_field.ally_attack), finish, false /*rival*/);

		commit_attack(_attack_controller.get_attack_directions(moving_piece), finish, false /*rival*/);

		finish_field.piece = PieceController::extract_min_piece_rank(moving_piece);
	}

	bool ChessState::is_inverted() const
	{
		return _is_inverted;
	}

	bool ChessState::_is_capture_move(const ChessMove& move) const
	{
		if (!is_ally(move.start_field_id) || is_ally(move.finish_field_id))
			throw std::exception("Invalid move");

		return is_rival(move.finish_field_id);
	}

	template <bool D>
	int ChessState::Field::to_int() const
	{
		if (!D)
			return PieceController::extract_full_piece_rank(piece);

		return PieceController::extract_full_piece_rank(piece) |
			(AttackController::compress_attack_directions(rival_attack) << PieceController::TotalBitsCount) |
			(AttackController::compress_attack_directions(ally_attack) <<
				(AttackController::TotalCompressedBits + PieceController::TotalBitsCount));
	}

	template <bool D>
	std::vector<int> ChessState::to_vector() const
	{
		std::vector<int> result(_data.size());

		std::ranges::transform(_data, result.begin(),
			[](const auto& f)
			{
				return f.template to_int<D>();
			});

		return result;
	}

	template std::vector<int> ChessState::to_vector<true>() const;
	template std::vector<int> ChessState::to_vector<false>() const;

	ChessState::ChessState(const std::vector<int>& board_state, const bool inverted)
	{
		_is_inverted = inverted;

		if (board_state.size() != Checkerboard::FieldsCount)
			throw std::exception("Unexpected state size");

		build(board_state);
	}

	bool ChessState::operator==(const ChessState& other_state) const
	{
		return _data == other_state._data && _is_inverted == other_state._is_inverted ;
	}

	bool ChessState::operator!=(const ChessState& other_state) const
	{
		return !(*this == other_state);
	}

	ChessState ChessState::get_start_state()
	{
		return ChessState{ PieceController::get_init_board_state() };
	}

	double ChessState::calc_reward(const std::vector<int>& prev_state, const std::vector<int>& next_state)
	{
		return 0;
	}

	std::unique_ptr<IState> ChessState::yield(const bool initialize_recorder) const
	{
		return std::make_unique<StateHandle>(*this, initialize_recorder);
	}

	StateTypeId ChessState::type()
	{
		return StateTypeId::CHESS;
	}

	StateTypeId ChessState::state_type() const
	{
		return type();
	}

	int ChessState::locate_king() const
	{
		for (auto field_id = 0ull; field_id < _data.size(); ++field_id)
		{
			if (PieceController::is_king(_data[field_id].piece))
				return static_cast<int>(field_id);
		}

		throw std::exception("King can't be found");
	}

	void ChessState::Field::assign_inverted(const Field& other_field)
	{
		piece = PieceController::anti(other_field.piece);
		ally_attack = other_field.rival_attack;
		rival_attack = other_field.ally_attack;
	}

	void ChessState::Field::add_attack_flag(const int attack_flag, const bool rival_token)
	{
		if (rival_token)
			rival_attack |= attack_flag;
		else
			ally_attack |= attack_flag;
	}

	void ChessState::Field::remove_attack_flag(const int attack_flag, const bool rival_token)
	{
		if (rival_token)
			rival_attack &= ~attack_flag;
		else
			ally_attack &= ~attack_flag;
	}

	void ChessState::Field::swap_attack_tokens()
	{
		const auto temp = rival_attack;
		rival_attack = ally_attack;
		ally_attack = temp;
	}

	bool ChessState::Field::operator==(const Field& other_field) const
	{
		return piece == other_field.piece &&
			rival_attack == other_field.rival_attack &&
			ally_attack == other_field.ally_attack;
	}

	bool ChessState::Field::operator!=(const Field& other_field) const
	{
		return !(*this == other_field);
	}

	void ChessState::commit_attack(const std::vector<AttackController::Direction>& attack_directions,
	                               const PiecePosition& position, const bool rival)
	{
		process_attack(attack_directions, position, rival, [rival](Field& field, const int attack)
			{
				field.add_attack_flag(attack, rival);
			});
	}

	void ChessState::withdraw_attack(const std::vector<AttackController::Direction>& attack_directions,
	                            const PiecePosition& position, const bool rival)
	{
		process_attack(attack_directions, position, rival, [rival](Field& field, const int attack)
			{
				field.remove_attack_flag(attack, rival);
			});
	}

	void ChessState::process_attack(const std::vector<AttackController::Direction>& attack_directions,
		const PiecePosition& position, const bool negate_dir, const std::function<void(Field&, const int)>& operation)
	{
		for (const auto& attack : attack_directions)
		{
			const auto dir = negate_dir ? -attack.dir : attack.dir;

			auto next_field_pos = position + dir;

			if (!next_field_pos.is_valid())
				continue;

			auto next_field_id = PosController::to_linear(next_field_pos);
			operation(_data[next_field_id], attack.token);

			if (attack.is_long_range())
			{
				while (PieceController::is_space(_data[next_field_id].piece) && (next_field_pos += dir).is_valid())
				{
					next_field_id = PosController::to_linear(next_field_pos);
					operation(_data[next_field_id], attack.token);
				}
			}
		}
	}

	bool ChessState::is_king_threatened_after_move(const PiecePosition& move_start_pos,
	                                               const PiecePosition& move_finish_pos,
	                                               const PiecePosition& king_field_pos) const
	{
		if (!move_start_pos.is_valid() || !move_finish_pos.is_valid() || !king_field_pos.is_valid())
			throw std::exception("Invalid input data");

		const auto& start_field = _data[PosController::to_linear(move_start_pos)];
		const auto& finish_field = _data[PosController::to_linear(move_finish_pos)];

		if (!PieceController::is_ally_piece(start_field.piece) || PieceController::is_ally_piece(finish_field.piece))
			throw std::exception("Impossible move");

		auto result = _data[PosController::to_linear(king_field_pos)].rival_attack;

		const auto attack_directions_to_commit =
			_attack_controller.decode_long_range_attack_directions(start_field.rival_attack);

		const auto induced_attack = get_rival_attack_on_field(attack_directions_to_commit, move_start_pos, king_field_pos);

		if (induced_attack != 0 && result != 0)
			return true; //the king is already under the attack and the current piece can't cover it since is is already covering it from another attack

		if (induced_attack == 0 && result == 0)
			return false; //the lifting of the current piece will not result in any threat

		if (induced_attack != 0)
		{
			const auto covered_attack = _attack_controller.decode_long_range_attack_directions(induced_attack);

			if (covered_attack.size() != 1)
				throw std::exception("One piece can't cover more than one attack direction with respect to King position");

			const auto covered_attack_dir = covered_attack[0].dir;
			const auto move_dir = move_finish_pos - move_start_pos;

			// In this particular case the current piece is allowed to move only along the direction of the attack it is covering.
			return move_dir.col * covered_attack_dir.row != move_dir.row * covered_attack_dir.col;
		}

		const auto attack_directions_to_withdraw = PieceController::is_rival_piece(finish_field.piece) ?
			_attack_controller.get_attack_directions(finish_field.piece) :
			_attack_controller.decode_long_range_attack_directions(finish_field.rival_attack);

		result &= ~get_rival_attack_on_field(attack_directions_to_withdraw, move_finish_pos, king_field_pos);


		return result;
	}

	int ChessState::get_rival_attack_on_field(const std::vector<AttackController::Direction>& rival_attack_directions,
		const PiecePosition& source_position, const PiecePosition& focus_field_pos) const
	{
		for (const auto& attack_dir : rival_attack_directions)
		{
			int steps_count = -1;
			const auto neg_attack_dir = AttackController::Direction{ -attack_dir.dir, attack_dir.token };
			if (neg_attack_dir.can_reach(source_position, focus_field_pos, steps_count) && steps_count > 0)
			{
				if (steps_count == 1)
					return neg_attack_dir.token;

				auto next_pos = source_position + neg_attack_dir.dir;

				while (PieceController::is_space(_data[PosController::to_linear(next_pos)].piece) && next_pos != focus_field_pos)
					next_pos += neg_attack_dir.dir;

				if (next_pos == focus_field_pos)
					return neg_attack_dir.token;

				return 0; // we expect that only one attack direction from the given collection can in principle point towards the focus field;
			}
		}

		return 0;
	}

	void ChessState::build(const std::vector<int>& board_state)
	{
		std::ranges::transform(board_state, _data.begin(),
			[](const auto& x) { return Field{ x }; });

		for (auto field_id = 0; field_id < Checkerboard::FieldsCount; ++field_id)
		{
			const auto& field_data_ref = _data[field_id];

			if (PieceController::is_space(field_data_ref.piece))
				continue;

			const auto attack_directions =
				_attack_controller.get_attack_directions(field_data_ref.piece);
			const auto rival_attack = PieceController::is_rival_piece(field_data_ref.piece);
			const auto current_pos = PosController::from_linear(field_id);

			commit_attack(attack_directions, current_pos, rival_attack);
		}
	}

	/// <summary>
	/// Returns only those directions from "source" collection that are not
	/// opposite to any of the directions in the "dirs_to_check" collection.
	/// This is equivalent to removing "coinciding" directions, provided that
	/// collection "dirs_to_check" represent attack directions of a rival piece.
	/// </summary>
	std::vector<AttackController::Direction> remove_same_directions(const std::vector<AttackController::Direction>& source,
		const std::vector<AttackController::Direction>& dirs_to_check)
	{
		if (dirs_to_check.empty())
			return source;

		std::vector<AttackController::Direction> result{};

		for (const auto& source_dir : source)
		{
			const auto op_source_dir = -source_dir.dir;
			if (std::ranges::all_of(dirs_to_check, [&op_source_dir](const auto& check_dir)
			{
				return check_dir.dir != op_source_dir;
			}))
				result.push_back(source_dir);
		}

		return result;
	}

	void ChessState::append_king_moves(const int king_field_id, std::vector<ChessMove>& moves) const
	{
		const auto& king_field = _data[king_field_id];

		// sanity check
		if (!PieceController::is_king(king_field.piece))
			throw std::exception("There is not 'King' on the start field");

		const auto start_pos = PosController::from_linear(king_field_id);

		const auto is_check = is_threatened(king_field_id);
		const auto& attack_directions = 
			is_check ? remove_same_directions(_attack_controller.get_king_attack_directions(),
			_attack_controller.decode_long_range_attack_directions(king_field.rival_attack)) :
			_attack_controller.get_king_attack_directions();

		for (const auto& attack_dir : attack_directions)
		{
			const auto finish_field_pos = start_pos + attack_dir.dir;
			const auto finish_field_id = PosController::to_linear(finish_field_pos);
			if (!finish_field_pos.is_valid() || is_threatened(finish_field_id) || is_ally(finish_field_id))
				continue;

			moves.push_back(ChessMove(king_field_id, static_cast<int>(finish_field_id),
				is_rival(finish_field_id)));
		}

		// Handle the "Castle" move

		if (!PieceController::is_in_init_pos(king_field.piece) || is_threatened(king_field_id))
			return;

		// Check "left castling"
		if (PieceController::is_in_init_pos(_data[0].piece) &&
			(is_space(king_field_id - 3)  || king_field_id - 3 == 0) &&
			is_space_and_not_threatened(king_field_id - 2) &&
			is_space_and_not_threatened(king_field_id - 1))
		{
			// Left castling
			moves.push_back(ChessMove(king_field_id, king_field_id - 2, false));
		}

		// Check "right castling"
		if (PieceController::is_in_init_pos(_data[7].piece) &&
			is_space_and_not_threatened(king_field_id + 1) &&
			is_space_and_not_threatened(king_field_id + 2) &&
			(is_space(king_field_id + 3) || king_field_id + 3 == 7))
		{
			// Right castling
			moves.push_back(ChessMove(king_field_id, king_field_id + 2, false));
		}
	}

	void ChessState::append_pawn_moves_basic(const int pawn_field_id,
		std::vector<ChessMove>& moves, const PiecePosition& king_pos) const
	{
		const auto& pawn_field = _data[pawn_field_id];

		// sanity check
		if (!PieceController::is_pawn(pawn_field.piece))
			throw std::exception("There is not 'Pawn'");

		const auto& attack_directions = _attack_controller.get_pawn_attack_directions();

		const auto start_pos = PosController::from_linear(pawn_field_id);

		for (const auto& attack_dir : attack_directions)
		{
			const auto finish_pos = start_pos + attack_dir.dir;

			if (!finish_pos.is_valid())
				continue;

			const auto finish_pos_lin = PosController::to_linear(finish_pos);
			if (is_rival(finish_pos_lin) && !is_king_threatened_after_move(start_pos, finish_pos, king_pos))
				moves.push_back(ChessMove(pawn_field_id, static_cast<int>(finish_pos_lin), true));
		}

		// Now handle "pawn-specific" moves
		auto finish_pos = start_pos;
		const auto possible_steps_forward_count = finish_pos.row == 1 ? 2 : 1;

		for (auto step_counter = 0; step_counter < possible_steps_forward_count; ++step_counter)
		{
			finish_pos.row++;

			if (finish_pos.is_valid())
			{
				const auto next_field_id = PosController::to_linear(finish_pos);

				if (!is_space(next_field_id))
					return;

				if (!is_king_threatened_after_move(start_pos, finish_pos, king_pos))
					moves.push_back(ChessMove(pawn_field_id,static_cast<int>(next_field_id), false));
			}
			else
				return;
		}
	}

	static const std::vector PromotionOptions{
		PieceController::Queen,
		PieceController::Bishop,
		PieceController::Knight,
		PieceController::Rook };

	void ChessState::append_pawn_moves(const int pawn_field_id, std::vector<ChessMove>& moves,
		const PiecePosition& king_pos) const
	{
		const auto start_pos = PosController::from_linear(pawn_field_id);

		if (start_pos.row != (Checkerboard::Rows - 2))
		{
			append_pawn_moves_basic(pawn_field_id, moves, king_pos);
			return;
		}

		std::vector<ChessMove> temp_moves;
		append_pawn_moves_basic(pawn_field_id, temp_moves, king_pos);

		for (const auto& temp_move : temp_moves)
			for (const auto promo_option : PromotionOptions)
				moves.push_back(ChessMove(temp_move.start_field_id, temp_move.finish_field_id, temp_move.is_capturing(), promo_option));
	}

	void ChessState::append_moves(const int start_field_id, std::vector<ChessMove>& moves,
	                              const std::vector<AttackController::Direction>& attack_directions, const PiecePosition& king_pos) const
	{
		const auto start_pos = PosController::from_linear(start_field_id);

		for (const auto& attack_dir : attack_directions)
		{
			auto finish_pos = start_pos + attack_dir.dir;

			if (attack_dir.is_long_range())
				while (validate_and_append_move(start_pos, finish_pos, moves, king_pos))
				{
					finish_pos += attack_dir.dir;
				}
			else
				validate_and_append_move(start_pos, finish_pos, moves, king_pos);
		}
	}

	bool ChessState::validate_and_append_move(const PiecePosition& start_pos, const PiecePosition& finish_pos,
		std::vector<ChessMove>& moves, const PiecePosition& king_pos) const
	{
		if (!finish_pos.is_valid())
			return false; // we are outside the board

		const auto finish_field_id = PosController::to_linear(finish_pos);
		if (is_ally(finish_field_id))
			return false; // we are stopped by the "ally" piece

		// TODO: the line below can be optimized since in some cases there is no sence to explore moves along the current direction
		if (is_king_threatened_after_move(start_pos, finish_pos, king_pos))
			return true; // although this move results in "check" it still makes sense to try to move in the same direction (if possible)

		moves.push_back(ChessMove(static_cast<int>(PosController::to_linear(start_pos)),
			static_cast<int>(finish_field_id), is_rival(finish_field_id)));

		// if it is a "capture move", we can't continue moving in the same
		// direction (as we would otherwise do in case of long-range moves).
		return !is_rival(finish_field_id); 
	}

	bool ChessState::is_ally(const long long field_id) const
	{
		return PieceController::is_ally_piece(_data[field_id].piece);
	}

	bool ChessState::is_piece(const long long field_id) const
	{
		return PieceController::is_piece(_data[field_id].piece);
	}

	int ChessState::min_piece_rank(const long long field_id) const
	{
		return PieceController::extract_min_piece_rank(_data[field_id].piece);
	}

	bool ChessState::is_rival(const long long field_id) const
	{
		return PieceController::is_rival_piece(_data[field_id].piece);
	}

	bool ChessState::is_space(const long long field_id) const
	{
		return PieceController::is_space(_data[field_id].piece);
	}

	bool ChessState::is_space_and_not_threatened(const long long field_id) const
	{
		return is_space(field_id) && !is_threatened(field_id);
	}

	bool ChessState::is_threatened(const long long field_id) const
	{
		return _data[field_id].rival_attack != 0;
	}

	bool ChessState::is_king(const long long field_id) const
	{
		return PieceController::is_king(_data[field_id].piece);
	}

	bool ChessState::is_pawn(const long long field_id) const
	{
		return PieceController::is_pawn(_data[field_id].piece);
	}

	bool ChessState::is_castling_move(const ChessMove& move) const
	{
		return is_king(move.start_field_id) && std::abs(move.start_field_id - move.finish_field_id) == 2;
	}

	bool ChessState::is_compound_move(const ChessMove& move, ChessMove& second_component) const
	{
		if (is_castling_move(move))
		{
			second_component = (move.start_field_id - move.finish_field_id) > 0 ?
				ChessMove{ 0, move.finish_field_id + 1, false } :
				ChessMove{ 7, move.finish_field_id - 1, false };

			return true;
		}

		return false;
	}

	bool ChessState::is_promotion(const ChessMove& move) const
	{
		if (move.final_rank == PieceController::Space)
			return false;

		if (!is_pawn(move.start_field_id) ||
			move.get_start().row != Checkerboard::Rows - 2 ||
			move.get_finish().row != Checkerboard::Rows - 1)
			throw std::exception("Invalid promotion move");

		return true;
	}
}
