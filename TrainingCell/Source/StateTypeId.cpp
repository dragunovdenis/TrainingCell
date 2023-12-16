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

#include "../Headers/StateTypeId.h"
#include "../../DeepLearning/DeepLearning/Utilities.h"

namespace TrainingCell
{
	StateTypeId parse_state_type_id(const std::string& str)
	{
		const auto str_normalized = DeepLearning::Utils::normalize_string(str);

		for (auto id = static_cast<int>(StateTypeId::ALL);
			id <= static_cast<int>(StateTypeId::CHESS); ++id)
		{
			const auto agent_id = static_cast<StateTypeId>(id);
			if (to_string(agent_id) == str_normalized)
				return agent_id;
		}

		return StateTypeId::INVALID;
	}

	std::string to_string(const StateTypeId& state_type_id)
	{
		switch (state_type_id)
		{
			case StateTypeId::ALL:      return "ALL";
			case StateTypeId::INVALID:  return "INVALID";
			case StateTypeId::CHECKERS: return "CHECKERS";
			case StateTypeId::CHESS:    return "CHESS";
			default:                    return "UNKNOWN";
		}
	}
}
