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
#include "MoveData.h"

namespace TrainingCell
{
	/// <summary>
	/// A container that can be used to collect certain number
	/// of move data instances filtering out instances of lower value.
	/// </summary>
	class MoveCollector
	{
		std::size_t _capacity{};
		std::vector<MoveData> _collection{};

	public:

		/// <summary>
		/// Constructor.
		/// </summary>
		MoveCollector(std::size_t capacity);

		/// <summary>
		/// Adds move consisting of the given data to the underlying collection
		///	iff either the number of elements in the collection is below the current capacity or
		/// the value of the given move is higher than the value of at least one move in the
		/// collection (in which case the move with the least value will be substituted with the given one).
		/// </summary>
		/// <param name="move_id">ID of the move to add.</param>
		/// <param name="move_value">Value of the move to add (takes part in the filtering process).</param>
		/// <param name="afterstate">Afterstate tensor of the move to add.</param>
		void add(const int move_id, const double move_value, const DeepLearning::Tensor& afterstate);

		/// <summary>
		/// Returns reference to the element of the underlying collection with the given id.
		/// </summary>
		MoveData& get(const int item_id);

		/// <summary>
		/// Returns capacity of the collector.
		/// </summary>
		std::size_t capacity() const;
	};
}
