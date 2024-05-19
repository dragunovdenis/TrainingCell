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
#include "IStateEditor.h"
#include "IStateSeed.h"

namespace TrainingCell
{
	/// <summary>
	/// Provides means to retrieve different pieces of information about a state by its type identifier.
	/// </summary>
	class StateTypeController
	{
	public:

		/// <summary>
		/// Returns size of the state represented by the given type identifier.
		/// </summary>
		static std::size_t get_state_size(const StateTypeId type);

		/// <summary>
		/// Returns "seed" of the "start" state represented with the given type identifier.
		/// </summary>
		static std::unique_ptr<IStateSeed> get_start_seed(const StateTypeId type);

		/// <summary>
		/// Instantiates state editor and returns pointer to it. It is a responsibility
		/// of the caller to make sure that the instance is properly disposed.
		/// </summary>
		static IStateEditor* instantiate_editor(const StateTypeId type);

		/// <summary>
		/// Returns "true" if the given pair of agents can "play" with each other in the context of the given state.
		/// </summary>
		static bool validate(const IMinimalAgent& agent0, const IMinimalAgent& agent1, const IStateSeed& state_seed);

		/// <summary>
		/// Returns "true" if there is a uniquely determined state type within which the two agent can play (the type is returned as `out_state_type_id` param).
		/// In case "false" is returned, the value of `out_state_type_id` parameter should be ignored by the caller.
		/// </summary>
		static bool can_play(const IMinimalAgent& agent0, const IMinimalAgent& agent1, StateTypeId& out_state_type_id);

		/// <summary>
		/// Returns type ID of a state that is "common" for the given pair of state type IDs.
		/// </summary>
		static StateTypeId get_common_state(const StateTypeId st0, const StateTypeId st1);

		/// <summary>
		/// Returns "true" if the given pair of state type IDs is "compatible".
		/// </summary>
		static bool states_are_compatible(const StateTypeId st0, const StateTypeId st1);

		/// <summary>
		/// Returns "true" if the given triplet of state type IDs is "compatible".
		/// </summary>
		static bool states_are_compatible(const StateTypeId st0, const StateTypeId st1, const StateTypeId st2);
	};
}
