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
#include "MoveData.h"
#include "TdlSettings.h"
#include "../../DeepLearning/DeepLearning/NeuralNet/Net.h"

namespace TrainingCell
{
	class AfterStateValueFunction;

	/// <summary>
	/// TD(lambda) sub-agent
	/// </summary>
	class TdLambdaSubAgent
	{
		/// <summary>
		/// A flag that should be set depending on the "color of pieces" the agent is playing
		/// </summary>
		const bool _is_white{false};

		/// <summary>
		/// A flag indicating that we are about to start new game
		/// </summary>
		bool _new_game{true};

		/// <summary>
		///	Auxiliary data structure that is used during training process
		/// </summary>
		std::vector<DeepLearning::LayerGradient<DeepLearning::CpuDC>> _z{};

		/// <summary>
		/// Computation context (serves optimization purposes)
		/// </summary>
		DeepLearning::Net<DeepLearning::CpuDC>::Context _context{};

		/// <summary>
		/// Auxiliary data structure used when computing gradient of the neural net (serves optimization purposes)
		/// </summary>
		std::vector<DeepLearning::LayerGradient<DeepLearning::CpuDC>> _gradient_cache{};

		/// <summary>
		/// Auxiliary data structure used when computing afterstate value (serves optimization purposes)
		/// </summary>
		DeepLearning::CpuDC::tensor_t _value_cache{};

		/// <summary>
		/// Previous state
		/// </summary>
		std::vector<int> _prev_state{};

		/// <summary>
		/// Previous afterstate
		/// </summary>
		DeepLearning::CpuDC::tensor_t _prev_after_state{};

		/// <summary>
		/// Number of moves taken by the sub-agent in the current episode
		/// </summary>
		int _move_counter{ 0 };

		/// <summary>
		/// Returns index of the picked move and the related data
		/// </summary>
		[[nodiscard]] MoveData pick_move(const IMinimalStateReadonly& state,
		                                 const ITdlSettingsReadOnly& settings, const DeepLearning::Net<DeepLearning::CpuDC>& net) const;

		/// <summary>
		/// Calculates afterstate and its value
		/// </summary>
		[[nodiscard]] static MoveData evaluate(const IMinimalStateReadonly& state, const int move_id,
		                                       const DeepLearning::Net<DeepLearning::CpuDC>& net);

		/// <summary>
		/// Updates "z" field and returns value of the afterstate value function at the "previous afterstate"
		/// </summary>
		double update_z_and_evaluate_prev_after_state(const ITdlSettingsReadOnly& settings, DeepLearning::Net<DeepLearning::CpuDC>& net);

		/// <summary>
		///	Resets training state of the object which is an obligatory procedure to start new episode
		/// </summary>
		void reset();

	public:
		/// <summary>
		/// Default constructor
		/// </summary>
		TdLambdaSubAgent() = delete;

		/// <summary>
		/// Constructor
		/// </summary>
		TdLambdaSubAgent(const bool is_white);

		/// <summary>
		/// Returns index of a move from the given collection of available moves
		/// that the agent wants to take given the current state
		/// </summary>
		int make_move(const IMinimalStateReadonly& state,
		              const ITdlSettingsReadOnly& settings, DeepLearning::Net<DeepLearning::CpuDC>& net);

		/// <summary>
		/// Returns index of a move from the given "move data" 
		/// and performs all the training related actions (if those are required)
		/// </summary>
		int make_move(const IMinimalStateReadonly& state, MoveData&& move_data,
		              const ITdlSettingsReadOnly& settings, DeepLearning::Net<DeepLearning::CpuDC>& net);

		/// <summary>
		/// The method is supposed to be called by the "training environment" when the current training episode is over
		/// to notify the agent about the "final" state and the result of entire game (episode)
		/// </summary>
		void game_over(const IMinimalStateReadonly& final_state, const GameResult& result,
			const ITdlSettingsReadOnly& settings, DeepLearning::Net<DeepLearning::CpuDC>& net);

		/// <summary>
		/// Returns index of the picked move and the related data
		/// </summary>
		[[nodiscard]] int pick_move_id(const IMinimalStateReadonly& state,
		                               const ITdlSettingsReadOnly& settings, const DeepLearning::Net<DeepLearning::CpuDC>& net) const;

		/// <summary>
		/// Returns index of the "best score" move (out of the collection of given ones) and the related data
		/// </summary>
		[[nodiscard]] static MoveData pick_move(const IMinimalStateReadonly& state,
		                                        const DeepLearning::Net<DeepLearning::CpuDC>& net);

		/// <summary>
		/// Returns true if the current sub-agent is equal to the given sub-agent
		/// </summary>
		[[nodiscard]] bool equal(const TdLambdaSubAgent& another_sub_agent) const;
	};
}
