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

#include "../Headers/AgentTypeId.h"
#include "../../DeepLearning/DeepLearning/Utilities.h"

namespace TrainingCell
{
	AgentTypeId parse_agent_type_id(const std::string& str)
	{
		const auto str_normalized = DeepLearning::Utils::normalize_string(str);

		for (auto id = static_cast<unsigned int>(AgentTypeId::UNKNOWN);
			id <= static_cast<unsigned int>(AgentTypeId::TDL_ENSEMBLE); ++id)
		{
			const auto agent_id = static_cast<AgentTypeId>(id);
			if (to_string(agent_id) == str_normalized)
				return agent_id;
		}

		return AgentTypeId::UNKNOWN;
	}

	std::string to_string(const AgentTypeId& agent_type_id)
	{
		switch (agent_type_id)
		{
			case AgentTypeId::TDL_ENSEMBLE : return "TDL_ENSEMBLE";
			case AgentTypeId::INTERACTIVE  : return "INTERACTIVE";
			case AgentTypeId::RANDOM       : return "RANDOM";
			case AgentTypeId::TDL   	   : return "TDL";
			case AgentTypeId::UNKNOWN      :
			default                        : return "UNKNOWN";
		}
	}

}
