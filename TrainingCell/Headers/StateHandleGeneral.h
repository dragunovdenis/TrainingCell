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

#include "IState.h"

namespace TrainingCell
{
	/// <summary>
	/// Implementation "IState" interface for the case of checkers game
	/// </summary>
	template <class S>
	class StateHandleGeneral : public IState
	{
		/// <summary>
		/// The state.
		/// </summary>
		S _state{};

		/// <summary>
		/// Collection of "available" actions.
		/// </summary>
		std::vector<typename S::Move> _actions{};

	public:

		/// <summary>
		/// Deleted default constructor
		/// </summary>
		StateHandleGeneral() = delete;

		/// <summary>
		/// Constructor
		/// </summary>
		StateHandleGeneral(S state);

		/// <summary>
		/// See documentation of the base class
		/// </summary>
		[[nodiscard]] int get_moves_count() const override;

		/// <summary>
		/// See documentation of the base class
		/// </summary>
		[[nodiscard]] std::vector<int> evaluate(const int move_id) const override;

		/// <summary>
		/// See documentation of the base class
		/// </summary>
		[[nodiscard]] std::vector<int> evaluate() const override;

		/// <summary>
		/// See documentation of the base class
		/// </summary>
		[[nodiscard]] double calc_reward(const std::vector<int>& prev_state,
			const std::vector<int>& next_state) const override;

		/// <summary>
		/// See documentation of the base class
		/// </summary>
		[[nodiscard]] const IStateSeed& current_state_seed() const override;

		/// <summary>
		/// See documentation of the base class
		/// </summary>
		[[nodiscard]] const std::vector<Move> get_all_moves() const override;

		/// <summary>
		/// See documentation of the base class
		/// </summary>
		[[nodiscard]] bool is_capture_action(const int action_id) const override;

		/// <summary>
		/// See documentation of the base class
		/// </summary>
		[[nodiscard]] bool is_inverted() const override;

		/// <summary>
		/// See documentation of the base class
		/// </summary>
		void move_invert_reset(const int action_id) override;

		/// <summary>
		/// Returns copy of the underlying "state" structure (presumably, for diagnostics purposes; not a part of "general" interface)
		/// </summary>
		S get_state() const;

		/// <summary>
		/// See documentation of the base class
		/// </summary>
		[[nodiscard]] std::vector<int> evaluate_ui() const override;

		/// <summary>
		/// See documentation of the base class
		/// </summary>
		[[nodiscard]]  std::vector<int> evaluate_ui_inverted() const override;
	};
}
