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

#include "CheckersTestUtils.h"

#include "../TrainingCell/Headers/IMinimalAgent.h"
#include "../TrainingCell/Headers/RandomAgent.h"
#include "../TrainingCell/Headers/Checkers/StateHandle.h"

using namespace TrainingCell::Checkers;

namespace TrainingCellTest
{
	State CheckersTestUtils::get_random_state()
	{
		TrainingCell::RandomAgent random_agent;
		return play(15, random_agent);
	}

	State CheckersTestUtils::play(const int moves_count, TrainingCell::IMinimalAgent& agent)
	{
		auto state_handle = StateHandle(State::get_start_state());
		for (auto iter_id = 0; iter_id < moves_count; ++iter_id)
		{
			const auto move_id = agent.make_move(state_handle, state_handle.is_inverted());
			state_handle.move_invert_reset(move_id);
		}

		return state_handle.get_state();
	}
}
