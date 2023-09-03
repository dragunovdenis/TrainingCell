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
#include <functional>
#include "Agent.h"

namespace TrainingCell
{
	using MakeMoveCallback = std::function<int(const std::vector<int>&, const std::vector<Move>&)>;
	using GameOverCallback = std::function<void(const std::vector<int>&, const GameResult&)>;

	/// <summary>
	///	Interface for a human player
	/// </summary>
	class InteractiveAgent : public Agent
	{
		const MakeMoveCallback _make_move_callback{};
		const GameOverCallback _game_over_callback{};
		const bool _play_for_whites{};
	public:
		/// <summary>
		/// Constructor
		/// </summary>
		/// <param name="make_move_callback">Pointer to a method that
		/// will be called when "make_move" method of the agent is called</param>
		/// <param name="game_over_callback">Pointer to a method that
		/// will be called when "game_over" method of the agent is called</param>
		/// <param name="play_for_whites">Determines whether state and move parameters of the callback methods
		/// should be inverted (if "false") or not (if "true")</param>
		InteractiveAgent(const MakeMoveCallback& make_move_callback, const GameOverCallback& game_over_callback,
			const bool play_for_whites);

		/// <summary>
		/// Returns index of a move from the given collection of available moves
		/// that the agent wants to take given the current state
		/// </summary>
		int make_move(const IActionEvaluator& evaluator, const bool as_white) override;

		/// <summary>
		/// The method is supposed to be called by the "training environment" when the current training episode is over
		/// to notify the agent about the "final" state and the result of entire game (episode)
		/// </summary>
		void game_over(const IState& final_state, const GameResult& result, const bool as_white) override;

		/// <summary>
		/// Returns type ID
		/// </summary>
		static AgentTypeId TYPE_ID();

		/// <summary>
		/// Returns type identifier of the current instance
		/// </summary>
		[[nodiscard]] AgentTypeId get_type_id() const override;

		/// <summary>
		/// Returns "true" if the agent can be trained otherwise returns "false"
		/// If "false" is returned one should avoid calling getter or setter of the "training mode"
		/// property because the later can throw exception (as not applicable)
		/// </summary>
		[[nodiscard]] bool can_train() const override;

		/// <summary>
		/// Returns true if the current agent is equal to the given one
		/// </summary>
		bool equal(const Agent& agent) const override;
	};

}

