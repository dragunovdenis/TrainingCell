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
#include "IStateReadOnly.h"

namespace TrainingCell
{
	/// <summary>
	/// An interface that is supposed to be a "middleman" between actual environment's
	/// state on the one side and the "board" with agents on the other side, allowing us to hide
	/// moves (as explicit instructions telling how exactly a state should be transformed
	/// during the "move") "under the hood" and work with their indices instead.
	/// </summary>
	class IState : public IStateReadOnly
	{
	public:

		/// <summary>
		/// "Applies" move with the given ID to the state,
		/// inverts the state and updates cache of available moves.
		/// It is a responsibility of the caller to ensure validness of the provided "move ID".
		/// </summary>
		virtual void move_invert_reset(const int move_id) = 0;

		/// <summary>
		/// Returns pointer to an instance of state recorder.
		/// If state recorder is not initialized null is returned.
		/// </summary>
		[[nodiscard]] virtual std::unique_ptr<IState> get_recorded_state() const = 0;
	};
}
