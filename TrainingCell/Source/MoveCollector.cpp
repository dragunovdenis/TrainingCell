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

#include "../Headers/MoveCollector.h"

namespace TrainingCell
{
	MoveCollector::MoveCollector(int capacity) : _capacity(capacity)
	{
		_collection.reserve(_capacity);
	}

	void MoveCollector::add(const int move_id, const double move_value, const DeepLearning::CpuDC::tensor_t& afterstate)
	{
		if (_collection.size() < _capacity)
			_collection.emplace_back(move_id, move_value, afterstate);
		else
		{
			const auto smallest_element = std::ranges::min_element(_collection,
				[](const auto& a, const auto& b) { return a.value < b.value; });

			if (smallest_element->value < move_value)
				*smallest_element = { move_id, move_value, afterstate };
		}
	}

	MoveData& MoveCollector::get(const int item_id)
	{
		return _collection[item_id];
	}

	int MoveCollector::get_capacity() const
	{
		return _capacity;
	}

	int MoveCollector::get_elements_count() const
	{
		return static_cast<int>(_collection.size());
	}
}
