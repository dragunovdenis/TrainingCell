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
#include "TdlAbstractAgent.h"

namespace TrainingCell::Checkers
{
	class AfterStateValueFunction;

	/// <summary>
	/// TD(lambda) sub-agent
	/// </summary>
	class TdLambdaSubAgent
	{
		const TdlSettingsReadOnly* _settings_ptr{};

		DeepLearning::Net<DeepLearning::CpuDC>* _net_ptr{};

		/// <summary>
		/// A flag indicating that we are about to start new game
		/// </summary>
		bool _new_game{true};

		/// <summary>
		///	Auxiliary data structure that is used during training process
		/// </summary>
		std::vector<DeepLearning::LayerGradient<DeepLearning::CpuDC>> _z{};

		/// <summary>
		/// Previous state
		/// </summary>
		State _prev_state{};

		/// <summary>
		/// Previous afterstate
		/// </summary>
		State _prev_afterstate{};

		/// <summary>
		/// Returns id of a move to take
		/// </summary>
		[[nodiscard]] int pick_move_id(const State& state, const std::vector<Move>& moves) const;

		/// <summary>
		/// Returns index of the picked move and the related data
		/// </summary>
		[[nodiscard]] MoveData pick_move(const State& state, const std::vector<Move>& moves) const;

		/// <summary>
		/// Calculates afterstate and its value
		/// </summary>
		[[nodiscard]] MoveData evaluate(const State& state, const std::vector<Move>& moves, const int move_id) const;

		/// <summary>
		/// Updates "z" field and returns value of the afterstate value function at the "previous afterstate"
		/// </summary>
		double update_z_and_evaluate_prev_after_state();

		/// <summary>
		///	Resets training state of the object which is an obligatory procedure to start new episode
		/// </summary>
		void reset();

		/// <summary>
		/// Assigns the given pointers to the corresponding fields
		/// </summary>
		void activate(const TdlSettingsReadOnly* settings_ptr, DeepLearning::Net<DeepLearning::CpuDC>* const net_ptr);

		/// <summary>
		/// Resets the corresponding pointer fields
		/// </summary>
		void deactivate();

	public:
		/// <summary>
		/// Default constructor
		/// </summary>
		TdLambdaSubAgent() = default;

		/// <summary>
		/// Returns index of a move from the given collection of available moves
		/// that the agent wants to take given the current state
		/// </summary>
		int make_move(const State& current_state, const std::vector<Move>& moves,
			const TdlSettingsReadOnly* settings_ptr, DeepLearning::Net<DeepLearning::CpuDC>* const net_ptr);

		/// <summary>
		/// The method is supposed to be called by the "training environment" when the current training episode is over
		/// to notify the agent about the "final" state and the result of entire game (episode)
		/// </summary>
		void game_over(const State& final_state, const GameResult& result,
			const TdlSettingsReadOnly* settings_ptr, DeepLearning::Net<DeepLearning::CpuDC>* const net_ptr);

		/// <summary>
		/// Returns true if the current sub-agent is equal to the given sub-agent
		/// disregarding their pointer fields
		/// </summary>
		[[nodiscard]] bool semi_equal(const TdLambdaSubAgent& another_sub_agent) const;
	};
}
