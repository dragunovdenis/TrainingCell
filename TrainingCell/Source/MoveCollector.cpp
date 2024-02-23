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
	MoveCollector::MoveCollector(std::size_t capacity) : _capacity(capacity)
	{
		_collection.reserve(_capacity);
	}

	void MoveCollector::add(const int move_id, const double move_value, const DeepLearning::Tensor& afterstate)
	{
		if (_collection.size() < _capacity)
			_collection.emplace_back(move_id, move_value, afterstate);
		else
		{
			double least_value = std::numeric_limits<double>::max();
			int least_item_id = -1;
			for (auto item_id = 0ull; item_id < _capacity; ++item_id)
			{
				if (_collection[item_id].value < least_value)
				{
					least_value = _collection[item_id].value;
					least_item_id = item_id;
				}
			}

			if (least_value < move_value)
				_collection[least_item_id] = { move_id, move_value, afterstate };
		}
	}

	MoveData& MoveCollector::get(const int item_id)
	{
		return _collection[item_id];
	}

	std::size_t MoveCollector::capacity() const
	{
		return _capacity;
	}
}
