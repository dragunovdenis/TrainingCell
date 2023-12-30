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
#include <vector>

namespace TrainingCell
{
	/// <summary>
	/// Represents a state which offers fixed sequence ("path" or "trace") of moves.
	/// </summary>
	template <class S>
	class StateTraceRecorder : public S
	{
		int _moves_counter{ 0 };
		std::vector<typename S::Move> _moves{};
		std::vector<bool> _draw_flags{};
	public:

		/// <summary>
		/// "Base" state.
		/// </summary>
		using BaseState = S;

		/// <summary>
		/// Constructor.
		/// </summary>
		StateTraceRecorder(const S& init_state);

		/// <summary>
		/// Appends move to the collection of "recorded" moves.
		/// </summary>
		void add_record(const typename S::Move& move, const bool draw_flag);

		/// <summary>
		/// Adds final record (the one that does not have a move, only draw state flag)
		/// </summary>
		void add_final_record(const bool draw_flag);

		/// <summary>
		/// Substitutes the latest added move with the given one.
		void adjust_last_move(const typename S::Move& move);

		/// <summary>
		/// Fills the given array with all the possible moves "in the current state".
		/// </summary>
		bool get_moves(std::vector<typename S::Move>& out_result) const override;

		/// <summary>
		/// "Applies" the given move to the current state and inverts the state.
		/// Increments "moves counter".
		/// Notice that override version below, throws an exception if the given move does not coincide with the recorder one;
		/// </summary>
		/// <param name="move">Move to "apply".</param>
		void make_move_and_invert(const typename S::Move& move) override;
	};
}
