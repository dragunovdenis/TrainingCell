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
#include <array>
#include <functional>

#include "AttackController.h"
#include "../Checkerboard.h"

namespace TrainingCell
{
	struct PiecePosition;
}

namespace TrainingCell::Chess
{
	/// <summary>
	///	Represents checkerboard with extra "attack" information on it.
	/// </summary>
	class ChessState
	{
	public:

		/// <summary>
		/// A move instruction.
		/// </summary>
		class Move
		{
			/// <summary>
			/// Only ChessState can construct a valid "move".
			/// </summary>
			friend class ChessState;

			/// <summary>
			/// Index of the "start" field.
			/// </summary>
			int start_field_id{ -1 };

			/// <summary>
			/// Index of the "finish" field.
			/// </summary>
			int finish_field_id{ -1 };

			/// <summary>
			/// If nonzero, defines "rank" of the piece (that moves) once it arrives to the "final" position (to do the "Pawn Promotion").
			///	Should be ignored if zero.
			/// </summary>
			int final_rank{ 0 };

			/// <summary>
			/// Returns position of the "start" field.
			/// </summary>
			[[nodiscard]] PiecePosition get_start() const;

			/// <summary>
			/// Returns position of the "finish" field.
			/// </summary>
			[[nodiscard]] PiecePosition get_finish() const;

			/// <summary>
			/// Constructor.
			/// </summary>
			Move(const int start_field_id, const int finish_field_id, const int final_rank = 0);

		public:

			/// <summary>
			/// Constructor.
			/// </summary>
			Move() = default;

		};

	private:
		/// <summary>
		/// Representation of a single checkerboard field.
		/// </summary>
		struct Field
		{
			/// <summary>
			/// Piece token.
			/// </summary>
			int piece{};

			/// <summary>
			/// Encoded data about attacks from ati-pieces on the current field.
			/// </summary>
			int rival_attack{};

			/// <summary>
			/// Encoded data about attacks from ally pieces on the current field.
			/// </summary>
			int ally_attack{};

			/// <summary>
			/// Assigns "inverted" from the given field.
			/// </summary>
			void assign_inverted(const Field& other_field);

			/// <summary>
			/// Adds given attack flag to the corresponding field.
			/// </summary>
			void add_attack_flag(const int attack_flag, const bool rival_token);

			/// <summary>
			/// Adds given attack flag from the corresponding field.
			/// </summary>
			void remove_attack_flag(const int attack_flag, const bool rival_token);

			/// <summary>
			/// Swaps "ally" and "rival" attack tokens.
			/// </summary>
			void swap_attack_tokens();
		};

		std::array<Field, Checkerboard::FieldsCount> _data;

		/// <summary>
		/// A flag to track if the current state is inverted with respect to the initial state.
		/// </summary>
		bool _is_inverted {false};

		/// <summary>
		/// "Commits" attacks suggested by the given collection with respect to the given position on the board.
		/// </summary>
		void commit_attack(const std::vector<AttackController::Direction>& attack_directions, const PiecePosition& position, const bool rival);

		/// <summary>
		/// "Withdraws" attacks suggested by the given collection with respect to the given position on the board.
		/// </summary>
		void withdraw_attack(const std::vector<AttackController::Direction>& attack_directions, const PiecePosition& position, const bool rival);

		/// <summary>
		/// "Commits" or "withdraws" attacks suggested by the given collection with respect
		/// to the given position of the board depending on the operation callback. 
		/// </summary>
		void process_attack(const std::vector<AttackController::Direction>& attack_directions, const PiecePosition& position,
			const std::function<void(Field&, const int)>& operation);

		/// <summary>
		/// Returns rival attack marker of the "focus_field" after a move with the given start and finish positions is done.
		/// </summary>
		[[nodiscard]] int predict_rival_attack(const PiecePosition& move_start_pos, const PiecePosition& move_finish_pos, const PiecePosition& focus_field_pos) const;

		/// <summary>
		/// Returns attack token of a single attack direction from the given collection that affects the given "focus field" when applied from the given "source position".
		///	Returns "0" if no such attack direction exists in the given collection.
		///	IMPORTANT: it is assumed that only one attack direction from the given collection can point towards the given focus field.
		///	It is responsibility of a caller to ensure that both given positions are valid positions on a checkerboard.
		/// </summary>
		[[nodiscard]] int get_rival_attack_on_field(const std::vector<AttackController::Direction>& attack_directions,
		                                            const PiecePosition& source_position, const PiecePosition& focus_field_pos) const;

		/// <summary>
		/// Builds the attack field from the given board state.
		/// </summary>
		void build(const std::array<int, Checkerboard::FieldsCount>& board_state);

		/// <summary>
		/// Moves an "ally" piece from the given "start" position to the given "stop" position.
		///	Updates attack field accordingly.
		/// </summary>
		void make_move(const PiecePosition& start, const PiecePosition& finish);

		/// <summary>
		/// Returns field index of the "ally" king. Throws exception if the king can't be located.
		/// </summary>
		[[nodiscard]] int locate_king() const;

		/// <summary>
		/// Appends moves of the "King" piece to the given collection.
		/// </summary>
		/// <param name="king_field_id">Index of the field where "King" piece is located.</param>
		/// <param name="moves">Collection to append to.</param>
		void append_king_moves(const int king_field_id, std::vector<Move>& moves) const;

		/// <summary>
		/// Appends moves of the pawn located on the field with the given ID to the given collection of moves.
		///	Does not take care of the proper handling of "promotion" moves.
		/// </summary>
		/// <param name="pawn_field_id">Index of a field where an "ally pawn" piece is located.</param>
		/// <param name="moves">Collection of moves to append to.</param>
		/// <param name="king_pos">Current position of the "ally king".</param>
		void append_pawn_moves_basic(const int pawn_field_id, std::vector<Move>& moves, const PiecePosition& king_pos) const;

		/// <summary>
		/// Appends moves of the pawn located on the field with the given ID to the given collection of moves.
		///	Takes care of the proper handling of "promotion" moves.
		/// </summary>
		/// <param name="pawn_field_id">Index of a field where an "ally pawn" piece is located.</param>
		/// <param name="moves">Collection of moves to append to.</param>
		/// <param name="king_pos">Current position of the "ally king".</param>
		void append_pawn_moves(const int pawn_field_id, std::vector<Move>& moves, const PiecePosition& king_pos) const;

		/// <summary>
		/// Append moves generated according ot the given
		/// collection of attack directions and the start
		/// position to the given collection of moves.
		/// </summary>
		void append_moves(const int start_field_id, std::vector<Move>& moves,
			const std::vector<AttackController::Direction>& attack_directions, const PiecePosition& king_pos) const;

		/// <summary>
		/// Validates the move defined with its start and finish positions and appends
		/// it to the given collection of moves in case validation was successful.
		///	Returns "true" if, disregarding whether the current move was successfully validated and appended to the given collection of moves,
		///	it is possible in principle to continue moving in the same direction.
		/// </summary>
		bool validate_and_append_move(const PiecePosition& start_pos, const PiecePosition& finish_pos,
			std::vector<Move>& moves, const PiecePosition& king_pos) const;

		/// <summary>
		/// Returns "true" if there is an "ally" piece on the field with the given ID.
		/// </summary>
		[[nodiscard]] bool is_ally(const long long field_id) const;

		/// <summary>
		/// Returns "true" if there is an "rival" piece on the field with the given ID.
		/// </summary>
		[[nodiscard]] bool is_rival(const long long field_id) const;

		/// <summary>
		/// Returns "true" if there are no pieces on the field with the given ID.
		/// </summary>
		[[nodiscard]] bool is_space(const long long field_id) const;

		/// <summary>
		/// Returns "true" if the field with the given ID is threatened by a rival piece.
		/// </summary>
		[[nodiscard]] bool is_threatened(const long long field_id) const;

		/// <summary>
		/// Returns "true" if there is an ally "king" piece on the field with the given ID.
		/// </summary>
		[[nodiscard]] bool is_king(const long long field_id) const;

		/// <summary>
		/// Returns "true" if there is an ally "pawn" piece on the field with the given ID.
		/// </summary>
		[[nodiscard]] bool is_pawn(const long long field_id) const;

	public:

		/// <summary>
		/// Returns collection of moves available in the current state.
		/// </summary>
		[[nodiscard]] std::vector<ChessState::Move> get_moves() const;

		/// <summary>
		/// Applies given "move" to the current board state.
		/// </summary>
		void make_move(const Move& move);

		/// <summary>
		/// Inverts the current state so that "ally" and "rival" pieces "swap sides".
		/// </summary>
		void invert();

		/// <summary>
		/// Returns a plain vector representation of the current state.
		/// </summary>
		[[nodiscard]] std::vector<int> to_vector() const;

		/// <summary>
		/// Returns a plain vector representation of the current state after applying the given move to it.
		/// </summary>
		[[nodiscard]] std::vector<int> get_vector(const Move& move) const;

		/// <summary>
		/// Returns a plain vector representation of a state inverted to the current one.
		/// </summary>
		[[nodiscard]] std::vector<int> get_vector_inverted() const;

		/// <summary>
		/// Returns a plain vector representation of a state which is inverted to the current one subjected to the given move.
		/// </summary>
		[[nodiscard]] std::vector<int> get_vector_inverted(const Move& move) const;

		/// <summary>
		/// Returns "true" if the current state is inverted with respect to the initial state. Otherwise "false" is returned.
		/// </summary>
		[[nodiscard]] bool is_inverted() const;

		/// <summary>
		/// Returns "true" if the given move is a capture move. Throws an exception if the given move is invalid.
		/// </summary>
		[[nodiscard]] bool _is_capture_move(const Move& move) const;

		/// <summary>
		/// Constructor from the given checkerboard state.
		/// </summary>
		ChessState(const std::array<int, Checkerboard::FieldsCount>& board_state);

		/// <summary>
		/// Returns an instance of chess state in the initial configuration (in terms of chess game).
		/// </summary>
		static ChessState create_start_state();
	};
}

