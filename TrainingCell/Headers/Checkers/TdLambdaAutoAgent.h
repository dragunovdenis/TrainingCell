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
#include "TdLambdaSubAgent.h"

namespace TrainingCell::Checkers
{
	/// <summary>
	/// Interface to access afterstate function
	/// </summary>
	class AfterStateValueFunction
	{
	public:
		/// <summary>
		///	Virtual destructor
		/// </summary>
		virtual ~AfterStateValueFunction() = default;

	private:
		template <bool WHITE>
		friend class TdLambdaSubAgent;
		/// <summary>
		/// Access to the neural net 
		/// </summary>
		virtual DeepLearning::Net<DeepLearning::CpuDC>& net() = 0;
	};

	/// <summary>
	/// Self-training capable TD(lambda) agent
	/// </summary>
	class TdLambdaAutoAgent : virtual public TdlAbstractAgent, virtual public AfterStateValueFunction
	{
		/// <summary>
		/// Access to the neural net 
		/// </summary>
		DeepLearning::Net<DeepLearning::CpuDC>& net() override;

		/// <summary>
		/// Pointer to the "white" sub-agent
		/// </summary>
		std::unique_ptr<TdLambdaSubAgent<true>> _white_sub_agent_ptr{};

		/// <summary>
		/// Pointer to the "black" sub-agent
		/// </summary>
		std::unique_ptr<TdLambdaSubAgent<false>> _black_sub_agent_ptr{};

		/// <summary>
		///	Initializes sub-agents
		/// </summary>
		void init_sub_agents();

	public:

		MSGPACK_DEFINE(MSGPACK_BASE(TdlAbstractAgent))

		/// <summary>
		/// Default constructor
		/// </summary>
		TdLambdaAutoAgent();

		/// <summary>Constructor</summary>
		/// <param name="layer_dimensions">Dimensions of the layers of the neural net that will serve as "afterstate value function".
		/// Must start with "32" and end with "1"</param>
		/// <param name="exploration_epsilon">Probability of taking an exploratory move during the training process, (0,1)</param>
		/// <param name="lambda">Lambda parameters used to determine "strength" of eligibility traces, (0,1)</param>
		/// <param name="gamma">Discount reward determining how reward decay with each next move, (0,1) </param>
		/// <param name="alpha">Learning rate</param>
		/// <param name="name">Name of the agent</param>
		TdLambdaAutoAgent(const std::vector<std::size_t>& layer_dimensions, const double exploration_epsilon,
			const double lambda, const double gamma, const double alpha, const std::string& name);

		/// <summary>
		/// Returns index of a move from the given collection of available moves
		/// that the agent wants to take given the current state
		/// </summary>
		int make_move(const State& current_state, const std::vector<Move>& moves, const bool as_white) override;

		/// <summary>
		/// The method is supposed to be called by the "training environment" when the current training episode is over
		/// to notify the agent about the "final" state and the result of entire game (episode)
		/// </summary>
		void game_over(const State& final_state, const GameResult& result, const bool as_white) override;

		/// <summary>
		/// Returns type ID
		/// </summary>
		static AgentTypeId TYPE_ID();

		/// <summary>
		/// Returns type identifier of the current instance
		/// </summary>
		[[nodiscard]] AgentTypeId get_type_id() const override;

		/// <summary>
		/// Returns "true" if the agent can train otherwise returns "false"
		/// If "false" is returned one should avoid calling getter or setter of the "training mode"
		/// property because the later can throw exception (as not applicable)
		/// </summary>
		[[nodiscard]] bool can_train() const override;

		/// <summary>
		/// Returns true if the current agent is equal to the given one
		/// </summary>
		[[nodiscard]] bool equal(const Agent& agent) const override;
	};

}
