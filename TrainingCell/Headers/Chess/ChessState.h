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
#include "ChessMove.h"
#include "../Checkerboard.h"
#include "../IStateSeed.h"

namespace TrainingCellTest
{
	class ChessStateTest;
}

namespace TrainingCell::Chess
{
	/// <summary>
	///	Represents checkerboard with extra "attack" information on it.
	/// </summary>
	class ChessState : public IStateSeed
	{
		friend class TrainingCellTest::ChessStateTest; // for diagnostics purposes

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

			/// <summary>
			/// Returns an integer representation of itself (in "regular" or "debug" forms depending on the template parameter).
			/// </summary>
			template <bool D = false>
			int to_int() const;

			/// <summary>
			/// Equality operator.
			/// </summary>
			bool operator ==(const Field& other_field) const;

			/// <summary>
			/// Inequality operator.
			/// </summary>
			bool operator !=(const Field& other_field) const;
		};

		std::array<Field, Checkerboard::FieldsCount> _data;

		/// <summary>
		/// A flag to track if the current state is inverted with respect to the initial state.
		/// </summary>
		bool _is_inverted {false};

		/// <summary>
		/// An instance of attack controller.
		/// </summary>
		AttackController _attack_controller{};

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
		void process_attack(const std::vector<AttackController::Direction>& attack_directions, const PiecePosition& position, const bool negate_dir,
			const std::function<void(Field&, const int)>& operation);

		/// <summary>
		/// Returns "true" if the given "king" position is threatened after the move with the given start and finish positions is done.
		/// </summary>
		[[nodiscard]] bool is_king_threatened_after_move(const PiecePosition& move_start_pos,
		                                                 const PiecePosition& move_finish_pos,
		                                                 const PiecePosition& king_field_pos) const;

		/// <summary>
		/// Returns attack token of a single attack direction from the given collection that affects the given "focus field" when applied from the given "source position".
		///	Returns "0" if no such attack direction exists in the given collection.
		///	IMPORTANT: it is assumed that only one attack direction from the given collection can point towards the given focus field.
		///	It is responsibility of a caller to ensure that both given positions are valid positions on a checkerboard.
		/// </summary>
		[[nodiscard]] int get_rival_attack_on_field(const std::vector<AttackController::Direction>& rival_attack_directions,
		                                            const PiecePosition& source_position, const PiecePosition& focus_field_pos) const;

		/// <summary>
		/// Builds the attack field from the given board state.
		/// </summary>
		void build(const std::vector<int>& board_state);

		/// <summary>
		/// Makes the given move.
		/// Updates the attack field accordingly.
		/// </summary>
		void make_move_and_update_attack_field(const ChessMove& move);

		/// <summary>
		/// Applies given move to the given state vector.
		/// </summary>
		static void make_move(std::vector<int>& state_vector, const ChessMove& move);

		/// <summary>
		/// Returns field index of the "ally" king. Throws exception if the king can't be located.
		/// </summary>
		[[nodiscard]] int locate_king() const;

		/// <summary>
		/// Appends moves of the "King" piece to the given collection.
		/// </summary>
		/// <param name="king_field_id">Index of the field where "King" piece is located.</param>
		/// <param name="moves">Collection to append to.</param>
		void append_king_moves(const int king_field_id, std::vector<ChessMove>& moves) const;

		/// <summary>
		/// Appends moves of the pawn located on the field with the given ID to the given collection of moves.
		///	Does not take care of the proper handling of "promotion" moves.
		/// </summary>
		/// <param name="pawn_field_id">Index of a field where an "ally pawn" piece is located.</param>
		/// <param name="moves">Collection of moves to append to.</param>
		/// <param name="king_pos">Current position of the "ally king".</param>
		void append_pawn_moves_basic(const int pawn_field_id, std::vector<ChessMove>& moves, const PiecePosition& king_pos) const;

		/// <summary>
		/// Appends moves of the pawn located on the field with the given ID to the given collection of moves.
		///	Takes care of the proper handling of "promotion" moves.
		/// </summary>
		/// <param name="pawn_field_id">Index of a field where an "ally pawn" piece is located.</param>
		/// <param name="moves">Collection of moves to append to.</param>
		/// <param name="king_pos">Current position of the "ally king".</param>
		void append_pawn_moves(const int pawn_field_id, std::vector<ChessMove>& moves, const PiecePosition& king_pos) const;

		/// <summary>
		/// Append moves generated according ot the given
		/// collection of attack directions and the start
		/// position to the given collection of moves.
		/// </summary>
		void append_moves(const int start_field_id, std::vector<ChessMove>& moves,
			const std::vector<AttackController::Direction>& attack_directions, const PiecePosition& king_pos) const;

		/// <summary>
		/// Validates the move defined with its start and finish positions and appends
		/// it to the given collection of moves in case validation was successful.
		///	Returns "true" if, disregarding whether the current move was successfully validated and appended to the given collection of moves,
		///	it is possible in principle to continue moving in the same direction.
		/// </summary>
		bool validate_and_append_move(const PiecePosition& start_pos, const PiecePosition& finish_pos,
			std::vector<ChessMove>& moves, const PiecePosition& king_pos) const;

		/// <summary>
		/// Returns "true" if there is an "ally" piece on the field with the given ID.
		/// </summary>
		[[nodiscard]] bool is_ally(const long long field_id) const;

		/// <summary>
		/// Returns "true" if there is a piece on the field with the given ID.
		/// </summary>
		[[nodiscard]] bool is_piece(const long long field_id) const;

		/// <summary>
		/// Returns minimal rank of a piece on the field with the given ID.
		/// </summary>
		[[nodiscard]] int min_piece_rank(const long long field_id) const;

		/// <summary>
		/// Returns "true" if there is an "rival" piece on the field with the given ID.
		/// </summary>
		[[nodiscard]] bool is_rival(const long long field_id) const;

		/// <summary>
		/// Returns "true" if there are no pieces on the field with the given ID.
		/// </summary>
		[[nodiscard]] bool is_space(const long long field_id) const;

		/// <summary>
		/// Returns "true" if the field with the given ID is empty and not threatened.
		/// </summary>
		[[nodiscard]] bool is_space_and_not_threatened(const long long field_id) const;

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

		/// <summary>
		/// Returns "true" if the given move is a "castling" move.
		/// </summary>
		[[nodiscard]] bool is_castling_move(const ChessMove& move) const;

		/// <summary>
		/// Returns "true" if the given move is a "compound" one, i.e., consists of two moves,
		/// in which case the corresponding reference parameter will be initialized as the second component of the move.
		/// </summary>
		[[nodiscard]] bool is_compound_move(const ChessMove& move, ChessMove& second_component) const;

		/// <summary>
		/// Returns "true" if the given move is a pawn promotion.
		/// </summary>
		[[nodiscard]] bool is_promotion(const ChessMove& move) const;

	public:

		using Move = ChessMove;

		/// <summary>
		/// "Base" state.
		/// </summary>
		using BaseState = ChessState;

		/// <summary>
		/// Default constructor.
		/// </summary>
		ChessState() = default;

		/// <summary>
		/// Returns collection of moves available in the current state.
		/// </summary>
		[[nodiscard]] std::vector<ChessMove> get_moves() const;

		/// <summary>
		/// Fills the given collection with moves available in the current state.
		/// Returns "true" if the current state is a "draw".
		/// </summary>
		virtual bool get_moves(std::vector<ChessMove>& out_result) const;

		/// <summary>
		/// Applies given "move" to the current board state.
		/// </summary>
		void make_move(const ChessMove& move);

		/// <summary>
		/// "Applies" the given move to the current state and inverts the state.
		/// </summary>
		/// <param name="move">Move to "apply".</param>
		virtual void make_move_and_invert(const ChessMove& move);

		/// <summary>
		/// Inverts the current state so that "ally" and "rival" pieces "swap sides".
		/// </summary>
		void invert();

		/// <summary>
		/// Returns a plain vector representation of the current state.
		/// </summary>
		template <bool D = false>
		[[nodiscard]] std::vector<int> to_vector() const;

		/// <summary>
		/// Returns size of the state represented as a vector of integers delivered by `to_vector()` method;
		/// </summary>
		static std::size_t state_size();

		/// <summary>
		/// Returns a plain vector representation of the current state after applying the given move to it.
		/// </summary>
		[[nodiscard]] std::vector<int> get_vector(const ChessMove& move) const;

		/// <summary>
		/// Returns a plain vector representation of a state inverted to the current one.
		/// </summary>
		[[nodiscard]] std::vector<int> get_vector_inverted() const;

		/// <summary>
		/// Returns a plain vector representation of a state which is inverted to the current one subjected to the given move.
		/// </summary>
		[[nodiscard]] std::vector<int> get_vector_inverted(const ChessMove& move) const;

		/// <summary>
		/// Returns "true" if the current state is inverted with respect to the initial state. Otherwise "false" is returned.
		/// </summary>
		[[nodiscard]] bool is_inverted() const;

		/// <summary>
		/// Returns "true" if the given move is a capture move. Throws an exception if the given move is invalid.
		/// </summary>
		[[nodiscard]] bool _is_capture_move(const ChessMove& move) const;

		/// <summary>
		/// Constructor from the given checkerboard state.
		/// </summary>
		ChessState(const std::vector<int>& board_state, const bool inverted = false);

		/// <summary>
		/// Equality operator.
		/// </summary>
		bool operator ==(const ChessState& other_state) const;

		/// <summary>
		/// Inequality operator.
		/// </summary>
		bool operator !=(const ChessState& other_state) const;

		/// <summary>
		/// Returns an instance of chess state in the initial configuration (in terms of chess game).
		/// </summary>
		static ChessState get_start_state();

		/// <summary>
		/// Calculates reward for the given pair of previous and next states represented with the given "int-vectors".
		///</summary>
		static double calc_reward(const std::vector<int>& prev_state, const std::vector<int>& next_state);

		/// <summary>
		/// Returns an instance of "IState" 
		/// </summary>
		[[nodiscard]] std::unique_ptr<IState> yield(const bool initialize_recorder) const override;

		/// <summary>
		/// Returns type identifier of the state.
		/// </summary>
		static StateTypeId type();

		/// <summary>
		/// Returns type identifier of the state that can be yielded by the "seed".
		/// </summary>
		[[nodiscard]] StateTypeId state_type() const override;

		/// <summary>
		/// Returns a 64 elements long "int-vector" representation of the state.
		/// </summary>
		[[nodiscard]] std::vector<int> to_vector_64() const;

		/// <summary>
		/// Returns a 64 elements long "int-vector" representation of the state inverted to the current one.
		/// </summary>
		[[nodiscard]] std::vector<int> to_vector_64_inverted() const;
	};
}

