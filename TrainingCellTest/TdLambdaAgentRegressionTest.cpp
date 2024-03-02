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
#include "../TrainingCell/Headers/Board.h"
#include "../TrainingCell/Headers/TdLambdaAgent.h"
#include "../TrainingCell/Headers/Checkers/StateHandle.h"
#include "../TrainingCell/Headers/Checkers/CheckersState.h"
#include "../DeepLearning/DeepLearning/MsgPackUtils.h"
#include <ppl.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace TrainingCell::Checkers;
using namespace TrainingCell;

namespace TrainingCellTest
{
	TEST_CLASS(TdlambdaAgentRegressionTest)
	{
		TEST_METHOD(TdLambdaAgentTrainingRegression)
		{
			//Arrange
			auto agent0 = TdLambdaAgent::load_from_file("TestData/TdlTrainingRegression/agent0.tda");
			auto agent1 = TdLambdaAgent::load_from_file("TestData/TdlTrainingRegression/agent1.tda");
			const auto agent0_trained = TdLambdaAgent::load_from_file("TestData/TdlTrainingRegression/agent0_trained.tda");
			const auto agent1_trained = TdLambdaAgent::load_from_file("TestData/TdlTrainingRegression/agent1_trained.tda");

			//Act
			const Board board(&agent0, &agent1);
			board.play(200, CheckersState::get_start_state());

			//Assert
			Assert::IsTrue(agent0 == agent0_trained, L"0th agent does not coincide with the reference");
			Assert::IsTrue(agent1 == agent1_trained, L"1st agent does not coincide with the reference");
		}

		TEST_METHOD(TdLambdaAgentAutoTrainingRegression)
		{
			//Arrange
			auto agent = TdLambdaAgent::load_from_file("TestData/TdlTrainingRegression/agent0.tda");
			agent.set_exploration_probability(0.1);
			const auto agent_trained = TdLambdaAgent::load_from_file("TestData/TdlTrainingRegression/agent0_auto_trained.tda");

			//Act
			const Board board(&agent, &agent);
			TdLambdaAgent::reset_explorer(0); // to ensure reproducible exploration
			board.play(200, CheckersState::get_start_state());
			TdLambdaAgent::reset_explorer();

			//Assert
			Assert::IsTrue(agent == agent_trained, L"Agent does not coincide with the reference");
		}

		/// <summary>
		/// General method to run search regression tests.
		/// </summary>
		static void tdlambda_agent_search_regression_general(const std::filesystem::path& input_agent_file_name,
		                                                     const std::filesystem::path& reference_net_file_name,
															 const double exploration, const int volume, const int depth)
		{
			//Arrange
			auto agent = TdLambdaAgent::load_from_file("TestData/TdlTrainingRegression" / input_agent_file_name);
			agent.set_exploration_probability(0.1);
			agent.set_search_exploration_probability(exploration);
			agent.set_search_exploration_depth(depth);
			agent.set_search_exploration_volume(volume);
			agent.set_td_search_iterations(100);
			agent.set_tree_search_method(TreeSearchMethod::TD_SEARCH);
			auto state_handle = StateHandle(CheckersState::get_start_state());
			const auto reference_search_net = NetWithConverter::load_from_file(
				"TestData/TdlTrainingRegression" / reference_net_file_name);

			//Act
			TdLambdaAgent::reset_explorer(0);// to ensure reproducible exploration
			agent.set_search_depth(5);
			const auto move_id = agent.make_move(state_handle, true);
			state_handle.move_invert_reset(move_id);
			agent.set_search_depth(1000);
			agent.make_move(state_handle, false);
			TdLambdaAgent::reset_explorer();

			//Assert
			Assert::IsTrue(agent._search_net.has_value(), L"Search net is not initialized");
			Assert::IsTrue(agent._search_net.value() == reference_search_net, L"Nets are supposed to be equal");
		}

		TEST_METHOD(TdLambdaAgentSearchRegression)
		{
			tdlambda_agent_search_regression_general("agent0.tda", "search_net.dat", 0.1, 10000, 10000);
		}

		TEST_METHOD(TdLambdaAgentVolumeSearchRegression)
		{
			tdlambda_agent_search_regression_general("agent0.tda", "volume_search_net.dat", 1.0, 5, 3);
		}

		TEST_METHOD(TdLambdaAgentSearchSettingsTest)
		{
			// Arrange
			TdLambdaAgent agent({ 64, 32, 16, 8 }, 0.0517, 0.15, 0.97, 0.025, StateTypeId::CHECKERS);
			agent.set_reward_factor(0.1234);
			agent.set_training_mode(false);
			agent.set_search_depth(3216);
			agent.set_search_exploration_probability(0.354);
			agent.set_search_exploration_volume(13);
			agent.set_search_exploration_depth(11);
			Assert::IsFalse(agent.get_training_mode(), L"Training mode is supposed to be off.");

			// Act
			const auto search_settings_default = agent.get_search_settings();

			// Assert
			Assert::IsTrue(search_settings_default.get_training_mode(false) &&
				search_settings_default.get_training_mode(true), L"Search settings should have training mode on.");

			Assert::IsTrue(search_settings_default.get_discount() == agent.get_discount(),
				L"Search settings should have the same reward discount as the source agent.");
			Assert::IsTrue(search_settings_default.get_lambda() == agent.get_lambda(),
				L"Search settings should have the same lambda factor as the source agent.");
			Assert::IsTrue(search_settings_default.get_learning_rate() == agent.get_learning_rate(),
				L"Search settings should have the same learning rate as the source agent.");
			Assert::IsTrue(search_settings_default.get_reward_factor() == agent.get_reward_factor(),
				L"Search settings should have the same reward factor as the source agent.");
			Assert::IsTrue(search_settings_default.get_train_depth() == agent.get_search_depth(),
				L"Search settings should have training depth equal to the search depth of the source agent.");
			Assert::IsTrue(search_settings_default.get_exploration_depth() == agent.get_search_exploration_depth(),
				L"Unexpected value of exploration depth in search settings.");
			Assert::IsTrue(search_settings_default.get_exploration_volume() == agent.get_search_exploration_volume(),
				L"Unexpected value of exploration volume in search settings.");
			Assert::IsTrue(search_settings_default.get_exploration_probability() == agent.get_search_exploration_probability(),
				L"Unexpected value of exploration probability in search settings.");
		}

		/// <summary>
		/// General method to test performance evaluation mode.
		/// </summary>
		static void test_performance_evaluation_mode(const TdLambdaAgent& agent)
		{
			// Arrange
			auto test_agent = agent;
			// Sanity check.
			Assert::IsTrue(test_agent.get_exploration_probability() > 0,
				L"Exploration probability is supposed to be positive.");
			Assert::IsFalse(test_agent.get_performance_evaluation_mode(),
				L"Performance evaluation mode is supposed to be disabled by default.");

			// Act
			test_agent.set_performance_evaluation_mode(true);

			// Assert
			Assert::IsTrue(test_agent.get_performance_evaluation_mode(),
				L"Performance evaluation mode is supposed to be enabled.");
			Assert::IsFalse(test_agent.get_training_mode(),
				L"Training mode is supposed to be of in the performance evaluation mode.");
			Assert::IsTrue(test_agent.get_exploration_probability() == 0.0,
				L"Exploration probability is supposed to be 0 in the performance evaluation mode.");

			// Act
			test_agent.set_performance_evaluation_mode(false);

			// Assert
			Assert::IsFalse(test_agent.get_performance_evaluation_mode(),
				L"Performance evaluation mode is supposed to be disabled.");
			Assert::IsTrue(test_agent.get_training_mode() == agent.get_training_mode(),
				L"Training mode is supposed to be restored to the original value when performance evaluation mode is off.");
			Assert::IsTrue(agent.get_exploration_probability() == test_agent.get_exploration_probability(),
				L"Exploration probability should be restored when performance evaluation mode is off.");
		}

		TEST_METHOD(TdLambdaAgentPerformanceEvaluationModeTrainingOnTest)
		{
			const TdLambdaAgent agent({ 64, 32, 16, 8 },
				0.0517, 0.15, 0.97, 0.025, StateTypeId::CHECKERS);
			Assert::IsTrue(agent.get_training_mode(), L"Training mode is supposed to be on by default.");
			test_performance_evaluation_mode(agent);
		}

		TEST_METHOD(TdLambdaAgentPerformanceEvaluationModeTrainingOffTest)
		{
			TdLambdaAgent agent({ 64, 32, 16, 8 },
				0.0517, 0.15, 0.97, 0.025, StateTypeId::CHECKERS);
			agent.set_training_mode(false);
			Assert::IsFalse(agent.get_training_mode(), L"Training mode is supposed to be off.");
			test_performance_evaluation_mode(agent);
		}

		TEST_METHOD(TdLambdaAgentSearchSettingsInPerformanceEvaluationModeTest)
		{
			// Arrange
			TdLambdaAgent agent({ 64, 32, 16, 8 }, 0.0517, 0.15, 0.97, 0.025, StateTypeId::CHECKERS);
			const auto search_settings_default = agent.get_search_settings();

			// Act
			agent.set_performance_evaluation_mode(true);

			// Assert
			const auto search_settings_performance_evaluation_mode = agent.get_search_settings();
			Assert::IsTrue(search_settings_default == search_settings_performance_evaluation_mode,
				L"Search settings should be independent on the performance evaluation mode of the agent.");
		}
	};
}