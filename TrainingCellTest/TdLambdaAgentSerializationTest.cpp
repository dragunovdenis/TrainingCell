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

#include "CheckersTestUtils.h"
#include "CppUnitTest.h"
#include "../TrainingCell/Headers/Agent.h"
#include "../TrainingCell/Headers/Checkers/CheckersState.h"
#include "../TrainingCell/Headers/TdLambdaAgent.h"
#include "../TrainingCell/Headers/TdlEnsembleAgent.h"
#include "../TrainingCell/Headers/AgentPack.h"
#include "../DeepLearning/DeepLearning/MsgPackUtils.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace TrainingCell::Checkers;
using namespace TrainingCell;

namespace TrainingCellTest
{
	TEST_CLASS(TdLambdaAgentSerializationTest)
	{
		TEST_METHOD(TdLambdaAgentPackSerialization)
		{
			//Arrange
			const TdLambdaAgent agent({ 64, 32, 16, 8 }, 0.05, 0.1, 0.9, 0.11, StateTypeId::CHESS);
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
				{{ 64, 32, 16, 8 }, 0.05, 0.1, 0.9, 0.11, StateTypeId::CHESS},
				{{ 21, 32 }, 3.05, 0.3, 1.9, 2.1, StateTypeId::CHESS},
				});

			agent.set_search_depth(123);
			agent.set_search_iterations(4321);
			agent.set_search_method(TreeSearchMethod::TD_SEARCH);
			agent.set_run_multi_threaded(true);

			const auto pack = AgentPack::make<TdlEnsembleAgent>(agent);

			//Act
			const auto pack_from_stream = DeepLearning::MsgPack::unpack<AgentPack>(DeepLearning::MsgPack::pack(pack));

			//Assert
			Assert::IsTrue(pack_from_stream.agent().equal(agent), L"agents are not equal");
		}

		/// <summary>
		/// Returns instance of an agent assigned in a "standard" way.
		/// </summary>
		static TdLambdaAgent create_standard_agent()
		{
			TdLambdaAgent result({ 64, 32, 16, 8 }, 0.05, 0.1, 0.9,
				0.11, StateTypeId::CHECKERS, 0.73, 11, 13, "some_name");
			//set some non-default values
			result.set_reward_factor(0.375);

			CheckersTestUtils::play(15, result);

			//Set some none-trivial search mode after iterations to save time
			result.set_tree_search_method(TreeSearchMethod::TD_SEARCH);
			result.set_td_search_iterations(1234);
			result.set_search_depth(321);
			result.set_performance_evaluation_mode(true);

			return result;
		}
		TEST_METHOD(TdLambdaAgentSerialization)
		{
			//Arrange
			const auto agent = create_standard_agent();

			//Act
			const auto agent_from_stream = DeepLearning::MsgPack::unpack<TdLambdaAgent>(DeepLearning::MsgPack::pack(agent));

			//Assert
			Assert::IsTrue(agent == agent_from_stream, L"Deserialized agent is not equal to the initial one");
		}

		TEST_METHOD(TdLambdaAgentScriptGeneration)
		{
			//Arrange
			const auto agent = create_standard_agent();

			//Act
			const auto script = agent.to_script();
			const TdLambdaAgent agent_reconstructed_from_script(script);
			const auto script_of_reconstructed_agent = agent_reconstructed_from_script.to_script();

			//Assert
			Assert::IsTrue(agent.equal_hyperparams(agent_reconstructed_from_script), L"Hyper-parameters are supposed to be equal");
		}
	};
}