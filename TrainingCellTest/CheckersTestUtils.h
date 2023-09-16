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
	class IMinimalAgent;
}

namespace TrainingCell::Checkers
{
	class State;
}

namespace TrainingCellTest
{
	/// <summary>
	/// Utility methods to test checkers related functionality.
	/// </summary>
	class CheckersTestUtils
	{
	public:

		/// <summary>
		/// Returns "random" state.
		/// </summary>
		static TrainingCell::Checkers::State get_random_state();

		/// <summary>
		/// Makes the given agent to play the given number of moves starting from the "initial" state of the checkers board.
		/// Returns resulting state.
		/// </summary>
		static TrainingCell::Checkers::State play(const int moves_count, TrainingCell::IMinimalAgent& agent);
	};
}

