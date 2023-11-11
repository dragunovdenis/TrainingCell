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

namespace TrainingCell
{
	/// <summary>
	/// Position of a piece on a checkerboard
	/// </summary>
	struct PiecePosition
	{
		/// <summary>
		/// Row occupied by a piece
		/// </summary>
		long long row{ -1 };

		/// <summary>
		/// Column occupied by a piece
		/// </summary>
		long long col{ -1 };

		/// <summary>
		/// Equality operator
		/// </summary>
		bool operator ==(const PiecePosition& pos) const;

		/// <summary>
		/// Inequality operator
		/// </summary>
		bool operator !=(const PiecePosition& pos) const;

		/// <summary>
		/// Returns the "inverted" position with respect to the current one
		/// In the other words, this operation aligns the given position with the "inverted" state (see, State::invert())
		/// </summary>
		PiecePosition invert() const;

		/// <summary>
		/// Returns "true" if the current instance is valid.
		/// </summary>
		bool is_valid() const;

		/// <summary>
		/// Compound addition operator.
		/// </summary>
		PiecePosition& operator +=(const PiecePosition& pos);

		/// <summary>
		/// Compound subtraction operator.
		/// </summary>
		PiecePosition& operator -=(const PiecePosition& pos);

		/// <summary>
		/// Compound division by integer number operator.
		/// </summary>
		PiecePosition& operator /= (const long long divisor);

		/// <summary>
		/// Negation operator.
		/// </summary>
		PiecePosition operator -() const;
	};

	/// <summary>
	/// Addition operator.
	/// </summary>
	PiecePosition operator+(PiecePosition pos1, const PiecePosition& pos2);

	/// <summary>
	/// Subtraction operator.
	/// </summary>
	PiecePosition operator-(PiecePosition pos1, const PiecePosition& pos2);

	/// <summary>
	/// Division by integer number operator.
	/// </summary>
	PiecePosition operator / (PiecePosition pos, const long long divisor);


}