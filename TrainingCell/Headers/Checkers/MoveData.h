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
#include "../../../DeepLearning/DeepLearning/NeuralNet/DataContext.h"

namespace TrainingCell::Checkers
{
	/// <summary>
	/// Holds data related to a picked move
	/// </summary>
	struct MoveData
	{
		/// <summary>
		/// ID of the move
		/// </summary>
		int move_id{};

		/// <summary>
		/// Value of the after-state
		/// </summary>
		double value{};

		/// <summary>
		/// After-sate resulted from the move (in a form of tensor, see 'State::to_tensor()')
		/// </summary>
		DeepLearning::CpuDC::tensor_t after_state{};
	};
}
