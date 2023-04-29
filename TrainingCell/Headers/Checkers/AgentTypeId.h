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
#include <string>

namespace TrainingCell::Checkers
{
	/// <summary>
	///	Enumerates different agent types
	///	Used to handle message-pack serialization of agents through their base class (class Agent)
	/// </summary>
	enum class AgentTypeId : int
	{
		UNKNOWN = 0,
		RANDOM = 1,
		INTERACTIVE = 2,
		TDL = 3,
		TDL_ENSEMBLE = 4,
		TDL_AUTO = 5,
	};

	/// <summary>
	/// Converts given string to AgentTypeId
	/// </summary>
	AgentTypeId parse_agent_type_id(const std::string& str);

	/// <summary>
	/// Returns string representation of the given agent type ID
	/// </summary>
	std::string to_string(const AgentTypeId& agent_type_id);
}
