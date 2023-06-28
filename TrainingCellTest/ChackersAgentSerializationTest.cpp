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

#include "CppUnitTest.h"
#include "../TrainingCell/Headers/Checkers/Agent.h"
#include "../TrainingCell/Headers/Checkers/TdLambdaAgent.h"
#include "../TrainingCell/Headers/Checkers/TdlEnsembleAgent.h"
#include "../TrainingCell/Headers/Checkers/AgentPack.h"
#include "../DeepLearning/DeepLearning/MsgPackUtils.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace TrainingCell::Checkers;

namespace TrainingCellTest
{
	TEST_CLASS(CheckersAgentSerializationTest)
	{
		TEST_METHOD(TdLambdaAgentPackSerialization)
		{
			//Arrange
			const TdLambdaAgent agent({ 32, 64, 32, 16, 8, 1 }, 0.05, 0.1, 0.9, 0.11);
			Logger::WriteMessage(agent.to_script().c_str());
			const auto pack = AgentPack::make<TdLambdaAgent>(agent);

			//Act
			const auto pack_from_stream = DeepLearning::MsgPack::unpack<AgentPack>(DeepLearning::MsgPack::pack(pack));

			//Assert
			Assert::IsTrue(pack_from_stream.agent().equal(agent), L"agents are not equal");
		}

		TEST_METHOD(EnsembleAgentPackSerialization)
		{
			//Arrange
			TdlEnsembleAgent agent({
				{{ 32, 64, 32, 16, 8, 1 }, 0.05, 0.1, 0.9, 0.11},
				{{ 32, 21, 32, 1 }, 3.05, 0.3, 1.9, 2.1},
				});
			const auto pack = AgentPack::make<TdlEnsembleAgent>(agent);

			//Act
			const auto pack_from_stream = DeepLearning::MsgPack::unpack<AgentPack>(DeepLearning::MsgPack::pack(pack));

			//Assert
			Assert::IsTrue(pack_from_stream.agent().equal(agent), L"agents are not equal");
		}

		TEST_METHOD(TdLambdaAgentSerialization)
		{
			//Arrange
			TdLambdaAgent agent({ 32, 64, 32, 16, 8, 1 }, 0.05, 0.1, 0.9, 0.11, "some_name");
			//set some non-default values
			agent.set_reward_factor(0.375);
			auto state = State::get_start_state();

			//Do some number of iterations but not call "game over" method so that all
			//the auxiliary data of the agent is nontrivial
			constexpr int iter_count = 10;
			for (auto iter_id = 0; iter_id < iter_count; ++iter_id)
			{
				const auto moves = state.get_moves();
				const auto move_id = agent.make_move(state, moves, true);
				state.make_move(moves[move_id], true, false);
				state.invert();
			}
			//Set some none-trivial search mode after iterations to save time
			agent.set_tree_search_method(TreeSearchMethod::TD_SEARCH);
			agent.set_td_search_iterations(1234);
			agent.set_search_depth(321);

			//Act
			const auto agent_from_stream = DeepLearning::MsgPack::unpack<TdLambdaAgent>(DeepLearning::MsgPack::pack(agent));

			//Assert
			Assert::IsTrue(agent == agent_from_stream, L"Deserialized agent is not equal to the initial one");
		}

		TEST_METHOD(TdLambdaAgentScriptGeneration)
		{
			//Arrange
			TdLambdaAgent agent({ 32, 64, 32, 16, 8, 1 }, 0.05, 0.1, 0.9, 0.11, "Some Name");
			//set some non-default values
			agent.set_reward_factor(0.375);
			agent.set_tree_search_method(TreeSearchMethod::TD_SEARCH);
			agent.set_td_search_iterations(1234);
			agent.set_search_depth(321);

			//Act
			const auto script = agent.to_script();
			const TdLambdaAgent agent_reconstructed_from_script(script);
			const auto script_of_reconstructed_agent = agent_reconstructed_from_script.to_script();

			//Assert
			Assert::IsTrue(agent.equal_hyperparams(agent_reconstructed_from_script), L"Hyper-parameters are supposed to be equal");
		}
	};
}