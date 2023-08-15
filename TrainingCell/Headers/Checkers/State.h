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
#include "../Move.h"
#include "../IState.h"

namespace DeepLearning
{
	class Tensor;
}

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
	class State : public State_array, public IState
	{
	private:
		bool _inverted{};

		/// <summary>
		/// Calculates score of the state
		/// </summary>
		[[nodiscard]] StateScore calc_score() const;

	public:
		MSGPACK_DEFINE(MSGPACK_BASE(State_array), _inverted)

		/// <summary>
		/// Constructor
		/// </summary>
		State() : State_array() {}

		/// <summary>
		/// Constructor
		/// </summary>
		explicit State(const State_array& state_array, const bool inverted = false);

		/// <summary>
		/// Size of the state.
		/// Literally, number of elements in the "tensor" representation of the state
		///</summary>
		[[nodiscard]] std::size_t dim() const override;

		/// <summary>
		/// Returns "true" if the state is reversed with respect to its initial "orientation"
		/// </summary>
		[[nodiscard]] bool is_inverted() const;

		/// <summary>
		///	Returns state that corresponds to the beginning of the game
		/// </summary>
		static State get_start_state();

		/// <summary>
		/// Method to remove captured pieces from the state (board)
		/// </summary>
		void remove_captured_pieces();

		/// <summary>
		///	Access the status of a single field through the general board position.
		///	The position must be valid, otherwise exception will be thrown
		/// </summary>
		[[nodiscard]] Piece get_piece(const PiecePosition& position) const;

		/// <summary>
		///	Access the status of a single field through the general board position.
		///	The position must be valid, otherwise exception will be thrown
		/// </summary>
		[[nodiscard]] Piece& get_piece(const PiecePosition& position);

		/// <summary>
		///	Converts the given id of a field in the "sense" of CheckersState structure into a column-row index pair
		/// </summary>
		static PiecePosition plain_id_to_piece_position(const long long plain_id);

		/// <summary>
		///	Converts the given position of a piece on the checkers board into
		///	the corresponding field index of CheckersState structure
		/// </summary>
		static long long piece_position_to_plain_id(const PiecePosition& position);

		/// <summary>
		///	Makes the move
		/// </summary>
		void make_move(const Move& move, const bool remove_captured, const bool mark_trace = false);

		/// <summary>
		/// Returns tensor representation of the current state after given `move` was applied to it
		/// </summary>
		[[nodiscard]] DeepLearning::Tensor get_state(const Move& move) const override;

		/// <summary>
		///	Makes the move
		/// </summary>
		void make_move(const SubMove& sub_move, const bool remove_captured);

		/// <summary>
		///	Returns "true" if the given move is valid in the context of the current state
		/// </summary>
		[[nodiscard]] bool is_valid_move(const Move& move) const;

		/// <summary>
		///	Returns an "inverted" state, i.e. a state that it is seen by the opponent (an agent playing "anti" pieces)
		/// </summary>
		[[nodiscard]] State get_inverted() const;

		/// <summary>
		///	"Inverts" the state
		/// </summary>
		void invert();

		/// <summary>
		/// Returns an "inverted" state, i.e. a state that it is seen by the opponent (an agent playing "anti" pieces)
		/// in the form of integer vector
		///// </summary>
		[[nodiscard]] std::vector<int> get_inverted_std() const override;

		/// <summary>
		/// Calculates reward for the given pair of previous and next after-states represented with "raw" tensors
		/// (those that can be obtained by calling `to_tensor` method below)
		///</summary>
		[[nodiscard]] double calc_reward(const DeepLearning::Tensor& prev_after_state, const DeepLearning::Tensor& next_after_state) const override;

		/// <summary>
		///	Converts the current state to tensor representation
		/// </summary>
		[[nodiscard]] DeepLearning::Tensor to_tensor() const override;

		/// <summary>
		/// Returns int vector representation of the state
		/// </summary>
		[[nodiscard]] std::vector<int> to_std_vector() const override;

		/// <summary>
		/// Assigns the current instance of state with the given vector representation
		/// </summary>
		void assign(const std::vector<int>& state_vect);

		/// <summary>
		///	Returns collection of available moves for the current state
		/// </summary>
		[[nodiscard]] std::vector<Move> get_moves() const;

		/// <summary>
		/// Equality operator
		/// </summary>
		bool operator ==(const State& another_state) const;

		/// <summary>
		/// Inequality operator
		/// </summary>
		bool operator !=(const State& another_state) const;
	};

	/// <summary>
	/// Functionality to handle training of checkers agent
	/// </summary>
	class Utils
	{
		/// <summary>
		///	Returns collection of capturing moves that we can get for the given state,
		///	the given position of a piece on the board, along the given diagonal and in the given direction
		///	Only "single capture" moves are considered
		/// </summary>
		/// <param name="current_state">State</param>
		/// <param name="pos">Position of "Man" or "King" on the board (in the state indexing system)</param>
		/// <param name="right_diagonal">Direction along which we count capturing moves</param>
		/// <param name="positive_direction">Direction on the diagonal in which we search for capturing moves</param>
		static std::vector<SubMove> get_capturing_moves(const State& current_state, const PiecePosition& pos,
		                                 const bool right_diagonal, const bool positive_direction);

		/// <summary>
		///	Returns all the possible capturing moves for the given "ally" piece (represented with its position in the given state structure)
		///	and the current state
		/// </summary>
		static std::vector<Move> get_capturing_moves(const State& current_state, const PiecePosition& start_pos);

	public:
		/// <summary>
		///	Returns "true" if the given piece represents either "Man" or "King"
		/// </summary>
		static bool is_allay_piece(const Piece piece);

		/// <summary>
		///	Returns "true" if the given piece is not "space" or (anti-)captured
		/// </summary>
		static bool is_alive(const Piece piece);

		/// <summary>
		///	Returns "true" if the given piece is (anti-)captured
		/// </summary>
		static bool is_dead(const Piece piece);

		/// <summary>
		///	Returns "true" if the given piece is (anti-)king
		/// </summary>
		static bool is_king(const Piece piece);

		/// <summary>
		///	Returns "true" if the given piece represents an opponent's piece ("AntiMan" or "AntiKing")
		/// </summary>
		static bool is_opponent_piece(const Piece piece);

		/// <summary>
		///	Returns "true" if the given piece represents (anti-)trace marker
		/// </summary>
		static bool is_trace_marker(const Piece piece);

		/// <summary>
		///	Returns "anti"-piece for the given one
		/// </summary>
		static Piece get_anti_piece(const Piece& piece);

		/// <summary>
		///	Returns collection of all the possible non-capturing moves starting from the given position along the given diagonal
		///	and the given direction
		/// </summary>
		static std::vector<SubMove> get_non_capturing_moves(const State& current_state, const PiecePosition& pos,
			const bool right_diagonal, const bool positive_direction);

		/// <summary>
		///	Returns collection of all the possible non-capturing moves starting from the given position
		/// </summary>
		static std::vector<Move> get_non_capturing_moves(const State& current_state, const PiecePosition& start_pos);

		/// <summary>
		///	Returns collection of all the capturing moves available for the given state
		/// </summary>
		static std::vector<Move> get_capturing_moves(const State& current_state);

		/// <summary>
		///	Returns collection of all the n0n-capturing moves available for the given state
		/// </summary>
		static std::vector<Move> get_non_capturing_moves(const State& current_state);

		/// <summary>
		///	Returns all the available moves for the given state
		/// </summary>
		static std::vector<Move> get_moves(const State& current_state);

		/// <summary>
		/// Returns a piece position that is achieved from the given one by moving for the given (signed) number of steps
		/// in the "right" or "left" diagonals
		/// A move in the "right" diagonal is assumed to be the one when both row and column
		/// coordinates are either simultaneously increase (for positive step) or decrease (for negative step).
		/// A move in the "left" diagonal is when either row coordinate increases and column coordinate decreases (for positive step)
		/// of vice versa (for negative step)
		/// The returned position might be invalid (reside outside the board), use the corresponding method to check validity
		/// </summary>
		static PiecePosition move(const PiecePosition& start_pos, const int step, bool rightDiagonal);

		/// <summary>
		/// Returns position on the checkers board that can be obtained from the current one by moving towards the
		/// given "pointer" position by the given "step". "Pointer" position must be on the same diagonal as the current position
		/// as well as be distinct from the current position
		/// one otherwise exception will be thrown
		/// </summary>
		static PiecePosition move(const PiecePosition& start_pos, const int step, const PiecePosition& pointer);

		/// <summary>
		/// Returns true if the two given positions share same diagonal on the checkers board
		/// </summary>
		static bool is_same_diagonal(const PiecePosition& start_pos, const PiecePosition& pos);

		/// <summary>
		/// Returns true if the given position is a valid position on the checkers board (a valid black field on the board)
		/// </summary>
		static bool is_valid(const PiecePosition& pos);

		/// <summary>
		/// Returns "true" if the given sub-move passes validation checks
		/// </summary>
		static bool is_valid(const SubMove& sub_move);

		/// <summary>
		/// Returns "true" if the given instance of a "move" has passed validation
		/// </summary>
		static bool is_valid(const Move& move);

		/// <summary>
		/// Appends one of the given moves to another one
		/// </summary>
		static void append(Move& move_to_append_to, const Move& move);
	};
}

MSGPACK_ADD_ENUM(TrainingCell::Checkers::Piece)
