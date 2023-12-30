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
#include "IMinimalAgent.h"

namespace TrainingCell
{
	/// <summary>
	/// Interface to a "training" aspect of an agent
	/// </summary>
	class ITrainableAgent : public IMinimalAgent
	{
	public:

		/// <summary>
		/// Returns "true" if the agent can be trained otherwise returns "false"
		/// If "false" is returned one should avoid calling getter or setter of the "training mode"
		/// property because the later can throw exception (as not applicable).
		/// </summary>
		[[nodiscard]] virtual bool can_train() const = 0;

		/// <summary>
		/// Sets "training_mode" flag for the agent defining whether the agent trains while playing.
		/// </summary>
		virtual void set_training_mode(const bool training_mode) = 0;

		/// <summary>
		/// Returns actual value of training mode.
		/// </summary>
		[[nodiscard]] virtual bool get_training_mode() const = 0;
	};

}