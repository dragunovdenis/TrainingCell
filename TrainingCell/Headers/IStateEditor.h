//Copyright (c) 2024 Denys Dragunov, dragunovdenis@gmail.com
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
#include "IStateSeed.h"
#include "PiecePosition.h"

namespace TrainingCell
{
	/// <summary>
	/// Interface for a state editor.
	/// </summary>
	class IStateEditor : public IStateSeed
	{
	public:
		/// <summary>
		/// Returns edited state represented as an array of integers.
		/// </summary>
		virtual std::vector<int> to_vector() const = 0;

		/// <summary>
		/// Returns collection of "values" (i.e., pieces)
		/// the given field on the board can get via editing. Each of the returned options
		/// then can be applied by calling the "apply option" method.
		/// </summary>
		virtual std::vector<int> get_options(const PiecePosition& pos) const = 0;

		/// <summary>
		/// Applies editing option with the given ID to the field with the given position.
		/// The actual value of the option to apply will be resolved in the context of
		/// the collection of options returned by "get option" method.
		/// Throws exception if ID of the option exceeds range of the corresponding collection
		/// of editing options available for the given position on the board
		/// </summary>
		virtual void apply_option(const PiecePosition& pos, const int option_id) = 0;

		/// <summary>
		/// Resets edited state to "initial configuration".
		/// </summary>
		virtual void reset() = 0;

		/// <summary>
		/// Removes (almost) all the pieces from the board.
		/// </summary>
		virtual void clear() = 0;

		/// <summary>
		/// Returns type ID of the edited state.
		/// </summary>
		virtual StateTypeId get_state_type() const = 0;
	};
}
