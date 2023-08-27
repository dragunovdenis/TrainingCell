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
#include "TdLambdaSubAgent.h"

namespace TrainingCell
{
	/// <summary>
	/// Functionality allowing to train a neural net using TD-lambda approach
	/// </summary>
	class TdlTrainingAdapter : public IMinimalAgent
	{
		/// <summary>
		/// Array of sub-agents (the first one "plays" black pieces and the second one "plays" white pieces)
		/// </summary>
		std::vector<TdLambdaSubAgent> _sub_agents{ TdLambdaSubAgent{false} , TdLambdaSubAgent{true} };

		/// <summary>
		/// Pointer to the net that needs to be trained
		/// </summary>
		DeepLearning::Net<DeepLearning::CpuDC>* _net_ptr{};

		/// <summary>
		/// Settings to be used during the training
		/// </summary>
		const TdlSettings _settings;
		
	public:

		/// <summary>
		/// Default constructor removed
		/// </summary>
		TdlTrainingAdapter() = delete;

		/// <summary>
		/// Constructor
		/// </summary>
		/// <param name="net_ptr">Pointer to a neural net to operate on</param>
		/// <param name="settings">Settings to be used in the TD-lambda training process</param>
		TdlTrainingAdapter(DeepLearning::Net<DeepLearning::CpuDC>* net_ptr, const TdlSettings& settings);

		/// <summary>
		/// See summary of the base class declaration
		/// </summary>
		int make_move(const IState& current_state, const std::vector<Move>& moves, const bool as_white) override;

		/// <summary>
		/// See summary of the base class declaration
		/// </summary>
		void game_over(const IState& final_state, const GameResult& result, const bool as_white) override;
	};
}
