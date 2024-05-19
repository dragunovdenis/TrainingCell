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
#include <vector>
#include <msgpack.hpp>
#include "../Checkerboard.h"
#include "CheckersMove.h"
#include "../IStateSeed.h"

namespace TrainingCell::Checkers
{
	/// <summary>
	/// Representation of checker pieces
	/// </summary>
	enum class Piece : int {
		Man = 1,
		King = 2,
		AntiMan = -1,
		AntiKing = -2,
		Space = 0,
		Captured = 3,
		AntiCaptured = -3,
		TraceMarker = 4,
		AntiTraceMarker = -4,
		MinValue = -4,
		MaxValue = 4,
	};

	/// <summary>
	///	Span of different integer values that Piece enumerable can take (must be in sync with the latter)
	/// </summary>
	constexpr int PieceValueSpan = static_cast<int>(Piece::MaxValue) - static_cast<int>(Piece::MinValue) + 1;

	/// <summary>
	///	Data structure to count number of pieces of different type in a state
	/// </summary>
	class StateScore
	{
		int _scores[PieceValueSpan]{};
	public:

		/// <summary>
		///	Subscript operator (constant version)
		/// </summary>
		int operator [](const Piece& pc) const;

		/// <summary>
		///	Subscript operator (constant version)
		/// </summary>
		int& operator [](const Piece& pc);

		/// <summary>Returns difference between the current and given scores</summary>
		/// <param name="score_to_subtract">Score that should be subtracted from the current one</param>
		[[nodiscard]] StateScore diff(const StateScore& score_to_subtract) const;
	};

	constexpr int FieldsInRow = Checkerboard::Columns / 2;
	constexpr int StateSize = Checkerboard::Rows * FieldsInRow;

	/// <summary>
	///	Alias to make the messge-pack macros below happy
	/// </summary>
	using State_array = std::array<Piece, StateSize>;

	/// <summary>
	/// A data structure to represent state of the checkers game
	/// </summary>
	class CheckersState : public State_array, public IStateSeed
	{
		bool _inverted{};

		/// <summary>
		/// Calculates score of the state.
		/// </summary>
		[[nodiscard]] StateScore calc_score() const;

		/// <summary>
		/// Returns "true" if the given piece represents either "Man" or "King".
		/// </summary>
		static bool is_allay_piece(const Piece piece);

		/// <summary>
		/// Returns "true" if the given piece is not a "space" or (anti-)captured.
		/// </summary>
		static bool is_alive(const Piece piece);

		/// <summary>
		/// Returns "true" if the given piece is (anti-)captured.
		/// </summary>
		static bool is_dead(const Piece piece);

		/// <summary>
		/// Returns "true" if the given piece is (anti-)king.
		/// </summary>
		static bool is_king(const Piece piece);

		/// <summary>
		/// Returns "true" if the given piece represents an opponent's piece ("AntiMan" or "AntiKing").
		/// </summary>
		static bool is_opponent_piece(const Piece piece);

		/// <summary>
		/// Returns "true" if the given piece represents (anti-)trace marker.
		/// </summary>
		static bool is_trace_marker(const Piece piece);

		/// <summary>
		/// Returns collection of capturing moves that we can get for the given state,
		/// the given position of a piece on the board, along the given diagonal and in the given direction.
		/// Only "single capture" moves are considered.
		/// </summary>
		/// <param name="current_state">State.</param>
		/// <param name="pos">Position of "Man" or "King" on the board (in the state "coordinate" system).</param>
		/// <param name="right_diagonal">Direction along which we count capturing moves.</param>
		/// <param name="positive_direction">Direction on the diagonal in which we search for capturing moves.</param>
		static std::vector<CheckersMove> get_capturing_moves(const CheckersState& current_state,
		                                                     const PiecePosition& pos,
		                                                     const bool right_diagonal, const bool positive_direction);

		/// <summary>
		/// Returns all the possible capturing moves for the given "ally" piece (represented with its position in the given "state").
		/// </summary>
		static std::vector<CheckersMove> get_capturing_moves(const CheckersState& current_state,
		                                                     const PiecePosition& start_pos);

		/// <summary>
		/// Returns collection of all the possible non-capturing moves starting from the given position along the given diagonal
		/// and in the given direction.
		/// </summary>
		static std::vector<CheckersMove> get_non_capturing_moves(const CheckersState& current_state, const PiecePosition& pos,
			const bool right_diagonal, const bool positive_direction);

		/// <summary>
		/// Returns collection of all the possible non-capturing moves starting from the given position.
		/// </summary>
		static std::vector<CheckersMove> get_non_capturing_moves(const CheckersState& current_state,
		                                                         const PiecePosition& start_pos);

		/// <summary>
		/// Fills the given collection with all the capturing moves available for the given state.
		/// </summary>
		static void get_capturing_moves(const CheckersState& current_state, std::vector<CheckersMove>& out_result);

		/// <summary>
		/// Fills the given collection with all the non-capturing moves available for the given state.
		/// </summary>
		static void get_non_capturing_moves(const CheckersState& current_state, std::vector<CheckersMove>& out_result);

		/// <summary>
		/// Fills the given collection with all the available moves for the given state.
		/// </summary>
		static void get_moves(const CheckersState& current_state, std::vector<CheckersMove>& out_result);

		/// <summary>
		/// Returns a piece position that is achieved from the given one by moving for the given (signed) number of steps
		/// along the "right" or "left" diagonals. A move in the "right" diagonal is assumed to be the one when both row and column
		/// coordinates are either simultaneously increase (for positive direction) or decrease (for negative direction).
		/// A move along the "left" diagonal is when either row coordinate increases and column coordinate decreases (for positive direction)
		/// of vice versa (for negative direction). The returned position might be invalid (reside outside the board),
		/// use the corresponding method to check validity.
		/// </summary>
		static PiecePosition move(const PiecePosition& start_pos, const int step, bool rightDiagonal);

		/// <summary>
		/// Returns position on the checkers board that can be obtained from the current one by moving towards the
		/// given "pointer" position via the given "step". "Pointer" position must be on the same diagonal as the current position
		/// as well as be distinct from the current position, otherwise exception will be thrown.
		/// </summary>
		static PiecePosition move(const PiecePosition& start_pos, const int step, const PiecePosition& pointer);

		/// <summary>
		/// Returns "true" if the two given positions share same diagonal on the checkers board.
		/// </summary>
		static bool is_same_diagonal(const PiecePosition& start_pos, const PiecePosition& pos);

		/// <summary>
		/// Returns "true" if the given position is a valid position on the checkers board (a valid black field on the board).
		/// </summary>
		static bool is_valid(const PiecePosition& pos);

		/// <summary>
		/// Returns "true" if the given sub-move passes validation checks.
		/// </summary>
		static bool is_valid(const SubMove& sub_move);

		/// <summary>
		/// Returns "true" if the given collection of "sub-move"-s passes validation.
		/// </summary>
		static bool is_valid(const std::vector<SubMove>& sub_moves);

		/// <summary>
		/// "Applies" given move to the state represented by the given array.
		/// It is a responsibility of the caller to ensure that the given array represents a valid state (in terms of size, for example)
		/// as well as to ensure that the given move is valid.
		/// </summary>
		template <class T>
		static void make_move_internal(const CheckersMove& move, T* arr, const bool remove_captured);

		/// <summary>
		/// Internal implementation of a state inversion.
		/// </summary>
		/// <param name="state_array">State represented as an array.</param>
		/// <param name="size">Size of the array. It is assumed to be an even number.</param>
		template <class T>
		static void invert_internal(T* state_array, const std::size_t size);

		/// <summary>
		/// Piece position-to-plain ID conversion without check of validity for the argument.
		/// </summary>
		static long long piece_position_to_plain_id_unsafe(const PiecePosition& position);

		/// <summary>
		/// Returns score of the given state.
		/// </summary>
		template <class S>
		static StateScore calc_score_internal(const S& state);

		/// <summary>
		/// Returns "true" if the given collection of "sub-move"-s passes validation test in the context of the current state.
		/// </summary>
		[[nodiscard]] bool is_valid_move(const std::vector<SubMove>& sub_moves) const;

		/// <summary>
		///	Access the status of a single field through the general board position.
		///	The position must be valid, otherwise exception will be thrown
		/// </summary>
		[[nodiscard]] Piece get_piece(const PiecePosition& position) const;

		/// <summary>
		/// Access to field through the general board position.
		/// The position must be valid, otherwise an exception will be thrown.
		/// </summary>
		[[nodiscard]] Piece& get_piece(const PiecePosition& position);

		/// <summary>
		/// Converts the given ID of a field into a column-row index pair.
		/// </summary>
		static PiecePosition plain_id_to_piece_position(const long long plain_id);

		/// <summary>
		/// Converts the given position of a piece on the checkerboard into
		/// the corresponding field ID.
		/// </summary>
		static long long piece_position_to_plain_id(const PiecePosition& position);

		/// <summary>
		/// Returns 64 elements long representation of the given state vector.
		/// </summary>
		static std::vector<int> expand_to_64(const std::vector<int>& state_vec);

		/// <summary>
		/// "Applies" the given move to the current state. It is a responsibility of the caller to ensure validity of the move.
		/// </summary>
		/// <param name="move">Move to "apply",</param>
		/// <param name="remove_captured">If "false", the resulting state will contain auxiliary markers "illustrating" details of the move.</param>
		void make_move(const CheckersMove& move, const bool remove_captured);

	public:
		MSGPACK_DEFINE(MSGPACK_BASE(State_array), _inverted)

		using Move = CheckersMove;

		/// <summary>
		/// "Base" state.
		/// </summary>
		using BaseState = CheckersState;

		/// <summary>
		/// Constructor.
		/// </summary>
		CheckersState() : State_array() {}

		/// <summary>
		/// Constructor
		/// </summary>
		explicit CheckersState(const State_array& state_array, const bool inverted = false);

		/// <summary>
		/// Returns "true" if the current state is reversed with respect to its initial "orientation".
		/// </summary>
		[[nodiscard]] bool is_inverted() const;

		/// <summary>
		/// Returns "start state: for the checkers game.
		/// </summary>
		static CheckersState get_start_state();

		/// <summary>
		/// "Applies" the given move to the current state.
		/// </summary>
		/// <param name="move">Move to "apply".</param>
		void make_move(const CheckersMove& move);

		/// <summary>
		/// "Applies" the given move to the current state and inverts the state.
		/// </summary>
		/// <param name="move">Move to "apply".</param>
		virtual void make_move_and_invert(const CheckersMove& move);

		/// <summary>
		/// Returns "int-vector" representation of the current state after given `move` was "applied" to it.
		/// </summary>
		[[nodiscard]] std::vector<int> get_vector(const CheckersMove& move) const;

		/// <summary>
		/// Returns 64 element long "int-vector" representation of the current state after it
		/// was first "transformed" by the given `move` and then inverted.
		/// </summary>
		[[nodiscard]] std::vector<int> get_vector_inverted(const CheckersMove& move) const;

		/// <summary>
		/// Returns an "inverted" state, i.e., the current state, as it is seen by the opponent (an agent playing "anti" pieces).
		/// </summary>
		[[nodiscard]] CheckersState get_inverted() const;

		/// <summary>
		/// "Inverts" the current state.
		/// </summary>
		void invert();

		/// <summary>
		/// Returns an "inverted" state, i.e., the current state, as it is seen by the opponent (an agent playing "anti" pieces)
		/// in the form of "int-vector".
		///// </summary>
		[[nodiscard]] std::vector<int> get_vector_inverted() const;

		/// <summary>
		/// Calculates reward for the given pair of previous and next states represented with the given "int-vectors".
		///</summary>
		[[nodiscard]] static double calc_reward(const std::vector<int>& prev_state, const std::vector<int>& next_state);

		/// <summary>
		/// Returns "int-vector" representation of the state.
		/// </summary>
		[[nodiscard]] std::vector<int> to_vector() const;

		/// <summary>
		/// Returns size of the state represented as a vector of integers delivered by `to_vector()` method;
		/// </summary>
		static std::size_t state_size();

		/// <summary>
		/// Returns a 64 elements long "int-vector" representation of the state.
		/// </summary>
		[[nodiscard]] std::vector<int> to_vector_64() const;

		/// <summary>
		/// Returns a 64 elements long "int-vector" representation of the state inverted to the current one.
		/// </summary>
		[[nodiscard]] std::vector<int> to_vector_64_inverted() const;

		/// <summary>
		/// Returns collection of available moves for the current state.
		/// </summary>
		[[nodiscard]] std::vector<CheckersMove> get_moves() const;

		/// <summary>
		/// Fills the given collection with available moves for the current state.
		/// Returns "true" if the current state is a "draw".
		/// </summary>
		virtual bool get_moves(std::vector<CheckersMove>& out_result) const;

		/// <summary>
		/// Equality operator.
		/// </summary>
		bool operator ==(const CheckersState& another_state) const;

		/// <summary>
		/// Inequality operator.
		/// </summary>
		bool operator !=(const CheckersState& another_state) const;

		/// <summary>
		/// Returns "handle" to a copy of the current state <see cref="StateHandle"/>.
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
		/// Returns "anti"-piece for the given one.
		/// </summary>
		template <class P>
		static P get_anti_piece(const P& piece);

		/// <summary>
		/// Returns collections of "piece values" that can be "put"
		/// to the field with the given position by means of "editing".
		/// </summary>
		[[nodiscard]] std::vector<int> get_edit_options(const PiecePosition& pos) const;

		/// <summary>
		/// Applies "edit" option with the given ID to the field with the given position.
		/// The actual value of the edit option is resolved via its index in the context of
		/// the collection of options available for the given field <see cref="get_edit_options"/>.
		/// </summary>
		void apply_edit_option(const PiecePosition& pos, const int option_id);

		/// <summary>
		/// Resets state to its initial configuration.
		/// </summary>
		void reset();

		/// <summary>
		/// Removes all the pieces from the board.
		/// </summary>
		void clear();
	};
}

MSGPACK_ADD_ENUM(TrainingCell::Checkers::Piece)
