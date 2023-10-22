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
#include "../../Headers/Chess/AttackController.h"
#include "../../Headers/Chess/PosController.h"
#include <algorithm>

namespace TrainingCell::Chess
{
	std::vector<ChessState::Move> ChessState::get_moves() const
	{
		const auto king_field_id = locate_king();
		const auto king_pos = PosController::from_linear(king_field_id);

		std::vector<Move> result{};
		append_king_moves(king_field_id, result);

		for (auto field_id = 0; field_id < Checkerboard::FieldsCount; ++field_id)
		{
			if (!is_ally(field_id) || is_king(field_id))
				continue;

			if (is_pawn(field_id))
				append_pawn_moves(field_id, result, king_pos);

			const auto& attack_directions =
				AttackController::get_attack_directions(_data[field_id].piece);
			append_moves(field_id, result, attack_directions, king_pos);
		}

		return result;
	}

	void ChessState::make_move(const Move& move)
	{
		make_move(move.get_start(), move.get_finish());
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

	std::vector<int> ChessState::get_vector_inverted(const Move& move) const
	{
		auto result = get_vector(move);
		invert_state_vector(result);
		return result;
	}

	bool ChessState::is_inverted() const
	{
		return _is_inverted;
	}

	bool ChessState::_is_capture_move(const Move& move) const
	{
		if (!is_ally(move.start_field_id) || is_ally(move.finish_field_id))
			throw std::exception("Invalid move");

		return is_rival(move.finish_field_id);
	}

	std::vector<int> ChessState::to_vector() const
	{
		std::vector<int> result(_data.size());

		std::ranges::transform(_data, result.begin(),
			[](const auto& f)
			{
				return PieceController::extract_min_signed_piece_rank(f.piece);
			});

		return result;
	}

	ChessState::ChessState(const std::array<int, Checkerboard::FieldsCount>& board_state)
	{
		build(board_state);
	}

	ChessState ChessState::create_start_state()
	{
		return ChessState(PieceController::get_init_board_state());
	}

	std::vector<int> ChessState::get_vector(const Move& move) const
	{
		// A sanity check
		if (!is_ally(move.start_field_id) || is_ally(move.finish_field_id))
			throw std::exception("Invalid move");

		auto result = to_vector();

		result[move.finish_field_id] = PieceController::extract_min_piece_rank(result[move.start_field_id]);
		result[move.start_field_id] = PieceController::Space;

		return result;
	}

	void ChessState::make_move(const PiecePosition& start, const PiecePosition& finish)
	{
		if (!start.is_valid() || !finish.is_valid())
			throw std::exception("Invalid input positions");

		auto& start_field = _data[PosController::to_linear(start)];

		if (PieceController::is_rival_piece(start_field.piece))
			throw std::exception("Only ally piece can be moved");

		auto& finish_field = _data[PosController::to_linear(finish)];

		if (PieceController::is_ally_piece(finish_field.piece))
			throw std::exception("Can't capture an ally");

		commit_attack(AttackController::decode_long_range_attack_directions(start_field.rival_attack), start, true /*rival*/);

		const auto attacks_to_withdraw = PieceController::is_rival_piece(finish_field.piece) ?
			AttackController::get_attack_directions(finish_field.piece) :
			AttackController::decode_long_range_attack_directions(finish_field.rival_attack);

		withdraw_attack(attacks_to_withdraw, finish, true /*rival*/);

		commit_attack(AttackController::get_attack_directions(start_field.piece), finish, false /*rival*/);

		finish_field.piece = PieceController::extract_min_piece_rank(start_field.piece);
		start_field.piece = PieceController::Space;
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
			rival_attack &= !attack_flag;
		else
			ally_attack &= !attack_flag;
	}

	void ChessState::Field::swap_attack_tokens()
	{
		const auto temp = rival_attack;
		rival_attack = ally_attack;
		ally_attack = temp;
	}

	void ChessState::commit_attack(const std::vector<AttackController::Direction>& attack_directions,
	                          const PiecePosition& position, const bool rival)
	{
		process_attack(attack_directions, position, [rival](Field& field, const int attack)
			{
				field.add_attack_flag(attack, rival);
			});
	}

	void ChessState::withdraw_attack(const std::vector<AttackController::Direction>& attack_directions,
	                            const PiecePosition& position, const bool rival)
	{
		process_attack(attack_directions, position, [rival](Field& field, const int attack)
			{
				field.remove_attack_flag(attack, rival);
			});
	}

	void ChessState::process_attack(const std::vector<AttackController::Direction>& attack_directions,
		const PiecePosition& position, const std::function<void(Field&, const int)>& operation)
	{
		for (const auto& attack : attack_directions)
		{
			auto next_field_pos = position + attack.dir;

			if (!next_field_pos.is_valid())
				continue;

			auto& next_field = _data[PosController::to_linear(next_field_pos)];
			operation(next_field, attack.token);

			if (attack.is_long_range())
			{
				while (PieceController::is_space(next_field.piece) && (next_field_pos += attack.dir).is_valid())
				{
					next_field = _data[PosController::to_linear(next_field_pos)];
					operation(next_field, attack.token);
				}
			}
		}
	}

	int ChessState::predict_rival_attack(const PiecePosition& move_start_pos, const PiecePosition& move_finish_pos,
	                                     const PiecePosition& focus_field_pos) const
	{
		if (!move_start_pos.is_valid() || !move_finish_pos.is_valid() || !focus_field_pos.is_valid())
			throw std::exception("Invalid input data");

		const auto& start_field = _data[PosController::to_linear(move_start_pos)];
		const auto& finish_field = _data[PosController::to_linear(move_finish_pos)];

		if (!PieceController::is_ally_piece(start_field.piece) || PieceController::is_ally_piece(finish_field.piece))
			throw std::exception("Impossible move");

		auto result = _data[PosController::to_linear(focus_field_pos)].rival_attack;

		const auto attack_directions_to_commit =
			AttackController::decode_long_range_attack_directions(start_field.rival_attack);

		result |= get_rival_attack_on_field(attack_directions_to_commit, move_start_pos, focus_field_pos);

		const auto attack_directions_to_withdraw = PieceController::is_rival_piece(finish_field.piece) ?
			AttackController::get_attack_directions(finish_field.piece) :
			AttackController::decode_long_range_attack_directions(finish_field.rival_attack);

		result &= !get_rival_attack_on_field(attack_directions_to_withdraw, move_finish_pos, focus_field_pos);


		return result;
	}

	int ChessState::get_rival_attack_on_field(const std::vector<AttackController::Direction>& attack_directions,
		const PiecePosition& source_position, const PiecePosition& focus_field_pos) const
	{
		for (const auto& attack_dir : attack_directions)
		{
			int steps_count = -1;
			if (attack_dir.can_reach(source_position, focus_field_pos, steps_count) && steps_count > 0)
			{
				if (steps_count == 1)
					return attack_dir.token;

				auto next_pos = source_position + attack_dir.dir;
				next_pos += attack_dir.dir; // start from "double increment" for obvious reasons

				while (PieceController::is_space(_data[PosController::to_linear(next_pos)].piece) && next_pos != focus_field_pos)
					next_pos += attack_dir.dir;

				if (next_pos == focus_field_pos)
					return attack_dir.token;

				return 0; // we expect that only one attack direction from the given collection can in principle point towards the focus field;
			}
		}

		return 0;
	}

	void ChessState::build(const std::array<int, Checkerboard::FieldsCount>& board_state)
	{
		std::ranges::transform(board_state, _data.begin(),
			[](const auto& x) { return Field{ x }; });

		for (auto field_id = 0; field_id < Checkerboard::FieldsCount; ++field_id)
		{
			const auto& field_data_ref = _data[field_id];

			if (PieceController::is_space(field_data_ref.piece))
				continue;

			const auto attack_directions =
				AttackController::get_attack_directions(field_data_ref.piece);
			const auto rival_attack = PieceController::is_rival_piece(field_data_ref.piece);
			const auto current_pos = PosController::from_linear(field_id);

			commit_attack(attack_directions, current_pos, rival_attack);
		}
	}

	PiecePosition ChessState::Move::get_start() const
	{
		return PosController::from_linear(start_field_id);
	}

	PiecePosition ChessState::Move::get_finish() const
	{
		return PosController::from_linear(finish_field_id);
	}

	ChessState::Move::Move(const int start_field_id, const int finish_field_id, const int final_rank) :
	start_field_id(start_field_id), finish_field_id(finish_field_id), final_rank(final_rank)
	{}

	void ChessState::append_king_moves(const int king_field_id, std::vector<Move>& moves) const
	{
		const auto& king_field = _data[king_field_id];

		// sanity check
		if (!PieceController::is_king(king_field.piece))
			throw std::exception("There is not 'King' on the start field");

		const auto start_pos = PosController::from_linear(king_field_id);

		const auto& attack_directions = AttackController::get_king_attack_directions();

		for (const auto& attack_dir : attack_directions)
		{
			const auto finish_field_pos = start_pos + attack_dir.dir;
			const auto finish_field_id = PosController::to_linear(finish_field_pos);
			if (!finish_field_pos.is_valid() || is_threatened(finish_field_id) || is_ally(finish_field_id))
				continue;

			moves.push_back(Move(king_field_id, static_cast<int>(finish_field_id)));
		}

		// Handle the "Castle" move

		if (!PieceController::is_in_init_pos(king_field.piece) || is_threatened(king_field_id))
			return;

		// Check "long castling"
		if (PieceController::is_in_init_pos(_data[0].piece) &&
			is_space(1) &&
			is_space(2) && !is_threatened(2) &&
			is_space(3) && !is_threatened(3))
		{
			// Long castling
			moves.push_back(Move(king_field_id, king_field_id - 2));
		}

		// Check "short castling"
		if (PieceController::is_in_init_pos(_data[8].piece) &&
			is_space(6) && !is_threatened(6) &&
			is_space(7) && !is_threatened(7))
		{
			// Short castling
			moves.push_back(Move(king_field_id, king_field_id + 2));
		}
	}

	void ChessState::append_pawn_moves_basic(const int pawn_field_id,
		std::vector<Move>& moves, const PiecePosition& king_pos) const
	{
		const auto& pawn_field = _data[pawn_field_id];

		// sanity check
		if (!PieceController::is_pawn(pawn_field.piece))
			throw std::exception("There is not 'Pawn'");

		const auto& attack_directions = AttackController::get_pawn_attack_directions();

		append_moves(pawn_field_id, moves, attack_directions, king_pos);

		// Now handle "pawn-specific" moves

		const auto start_pos = PosController::from_linear(pawn_field_id);
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

				const auto rival_attack_on_king = predict_rival_attack(start_pos, finish_pos, king_pos);

				if (rival_attack_on_king == 0)
					moves.push_back(Move(pawn_field_id,static_cast<int>(next_field_id)));
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

	void ChessState::append_pawn_moves(const int pawn_field_id, std::vector<Move>& moves,
		const PiecePosition& king_pos) const
	{
		const auto start_pos = PosController::from_linear(pawn_field_id);

		if (start_pos.row != (Checkerboard::Rows - 2))
		{
			append_pawn_moves_basic(pawn_field_id, moves, king_pos);
			return;
		}

		std::vector<Move> temp_moves;
		append_pawn_moves_basic(pawn_field_id, temp_moves, king_pos);

		for (const auto& temp_move : temp_moves)
			for (const auto promo_option : PromotionOptions)
				moves.push_back(Move(temp_move.start_field_id, temp_move.finish_field_id, promo_option));
	}

	void ChessState::append_moves(const int start_field_id, std::vector<Move>& moves,
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
		std::vector<Move>& moves, const PiecePosition& king_pos) const
	{
		if (!finish_pos.is_valid())
			return false; // we are outside the board

		const auto finish_field_id = PosController::to_linear(finish_pos);
		if (is_ally(finish_field_id))
			return false; // we are stopped by the "ally" piece

		const auto rival_attack_on_king = predict_rival_attack(start_pos, finish_pos, king_pos);

		if (rival_attack_on_king != 0)
			return true; // although this move results in "check" it still makes sense to try to move in the same direction (if possible)

		moves.push_back(Move(static_cast<int>(PosController::to_linear(start_pos)),
			static_cast<int>(finish_field_id)));

		// if it is a "capture move", we can't continue moving in the same
		// direction (as we would otherwise do in case of long-range moves).
		return !is_rival(finish_field_id); 
	}

	bool ChessState::is_ally(const long long field_id) const
	{
		return PieceController::is_ally_piece(_data[field_id].piece);
	}

	bool ChessState::is_rival(const long long field_id) const
	{
		return PieceController::is_rival_piece(_data[field_id].piece);
	}

	bool ChessState::is_space(const long long field_id) const
	{
		return PieceController::is_space(_data[field_id].piece);
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
}
