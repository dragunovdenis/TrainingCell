#pragma once
#include <array>
#include <vector>
#include <msgpack.hpp>

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
		StateScore diff(const StateScore& score_to_subtract) const;
	};


	/// <summary>
	///	Representation of possible game statuses
	/// </summary>
	enum class GameResult : int {
		Victory = 1,
		Loss = -1,
		Draw = 0,
		Canceled = 2,
	};

	/// <summary>
	/// A data structure representing row and column occupied by a piece on the board
	/// </summary>
	struct PiecePosition
	{
		/// <summary>
		///	Returns invalid position
		/// </summary>
		static PiecePosition create_invalid();

		/// <summary>
		/// Row occupied by a piece
		/// </summary>
		long long row {-1};

		/// <summary>
		/// Column occupied by a piece
		/// </summary>
		long long col {-1};

		/// <summary>
		///	Equality operator
		/// </summary>
		bool operator ==(const PiecePosition& pos) const;

		/// <summary>
		///	Inequality operator
		/// </summary>
		bool operator !=(const PiecePosition& pos) const;

		/// <summary>
		///	Returns true if the current position is a valid position on the checkers board (a valid black field on the board)
		/// </summary>
		[[nodiscard]] bool is_valid() const;

		/// <summary>
		///	Returns a piece position that is achieved from the current one by moving for the given (signed) number of steps
		///	in the "right" or "left" diagonals
		///	A move in the "right" diagonal is assumed to be the one when both row and column
		///	coordinates are either simultaneously increase (for positive step) or decrease (for negative step).
		///	A move in the "left" diagonal is when either row coordinate increases and column coordinate decreases (for positive step)
		///	of vice versa (for negative step)
		///	The returned position might be invalid (reside outside the board), use the corresponding method to check validity
		/// </summary>
		[[nodiscard]] PiecePosition move(const int step, bool rightDiagonal) const;

		/// <summary>
		///	Returns position on the checkers board that can be obtained from the current one by moving towards the
		///	given "pointer" position by the given "step". "Pointer" position must be on the same diagonal as the current position
		///	as well as be distinct from the current position
		///	one otherwise exception will be thrown
		/// </summary>
		[[nodiscard]] PiecePosition move(const int step, const PiecePosition& pointer) const;

		/// <summary>
		///	Returns true if the given and current positions share same diagonal on the checkers board
		/// </summary>
		[[nodiscard]] bool is_same_diagonal(const PiecePosition& pos) const;
	};

	/// <summary>
	///	The simplest move
	/// </summary>
	struct SubMove
	{
		/// <summary>
		///	Start position of the piece that "moves"
		/// </summary>
		PiecePosition start;
		/// <summary>
		///	End position of the piece that "moves"
		/// </summary>
		PiecePosition end;
		/// <summary>
		/// Position of a captured piece (if valid)
		/// </summary>
		PiecePosition capture;

		/// <summary>
		///	Returns "true" if the move passes validation checks
		/// </summary>
		[[nodiscard]] bool is_valid() const;

		/// <summary>
		///	"Inverts" the sub-move, i.e. aligns the sub-move with "inverted" state
		/// </summary>
		void invert();

		/// <summary>
		///	Returns "inverted" sub-move
		/// </summary>
		[[nodiscard]] SubMove get_inverted() const;
	};

	/// <summary>
	///	Representation of a compound "move" in the checkers game
	/// </summary>
	class Move
	{
	public:
		/// <summary>
		///	Component moves
		/// </summary>
		std::vector<SubMove> sub_moves {};

		/// <summary>
		///	Returns "true" if the current instance of a "move" has passed validation
		/// </summary>
		[[nodiscard]] bool is_valid() const;

		/// <summary>
		///	Appends the given move to the current one
		/// </summary>
		void append(const Move& move);

		/// <summary>
		///	Default constructor
		/// </summary>
		Move() = default;

		/// <summary>
		///	Constructs "move" from a single "sub-move"
		/// </summary>
		Move(const SubMove& sub_move);

		/// <summary>
		///	"Inverts" the move, i.e. aligns the sub-move with "inverted" state
		/// </summary>
		void invert();

		/// <summary>
		///	Returns "inverted" move
		/// </summary>
		[[nodiscard]] Move get_inverted() const;
	};

	constexpr int BoardRows = 8;
	constexpr int BoardColumns = 8;
	constexpr int FieldsInRow = BoardColumns / 2;
	constexpr int StateSize = BoardRows * FieldsInRow;
	
	/// <summary>
	///	Alias to make the messge-pack macros below happy
	/// </summary>
	using State_array = std::array<Piece, StateSize>;

	/// <summary>
	/// A data structure to represent state of the checkers game
	/// </summary>
	class State : public State_array
	{
	public:
		MSGPACK_DEFINE(MSGPACK_BASE(State_array));

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
		///	Calculates score of the state
		/// </summary>
		[[nodiscard]] StateScore calc_score() const;

		/// <summary>
		///	Converts the current state to tensor representation
		/// </summary>
		DeepLearning::Tensor to_tensor() const;

		/// <summary>
		///	Returns collection of available moves for the current state
		/// </summary>
		std::vector<Move> get_moves() const;
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
		///	Returns the "inverted" position with respect to the given one
		///	In the other words, this operation aligns the given position with the "inverted" state (see, State::invert())
		/// </summary>
		static PiecePosition invert(const PiecePosition& pos);

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
	};
}

MSGPACK_ADD_ENUM(TrainingCell::Checkers::Piece)
