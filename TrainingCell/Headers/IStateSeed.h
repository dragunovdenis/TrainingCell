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
#include <memory>
#include "StateTypeId.h"

namespace TrainingCell
{
	class IState;

	/// <summary>
	/// An auxiliary interface allowing to get a copy of an "IState" instance 
	/// </summary>
	class IStateSeed
	{
	public:
		/// <summary>
		/// Destructor
		/// </summary>
		virtual ~IStateSeed() = default;

		/// <summary>
		/// Returns an instance of "IState", with or without initialized
		/// recorder (depending on the value of the input parameter).
		/// </summary>
		[[nodiscard]] virtual std::unique_ptr<IState> yield(const bool initialize_recorder) const = 0;

		/// <summary>
		/// Returns type identifier of the state that can be yielded by the "seed".
		/// </summary>
		[[nodiscard]] virtual StateTypeId state_type() const = 0;
	};

}
