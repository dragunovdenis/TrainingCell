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
#include "../TrainingCell/Headers/Checkers/Board.h"
#include "../TrainingCell/Headers/Checkers/Agent.h"
#include "../TrainingCell/Headers/Checkers/RandomAgent.h"
#include "../TrainingCell/Headers/Checkers/TdLambdaAgent.h"
#include "../TrainingCell/Headers/Checkers/TdlEnsembleAgent.h"
#include "../DeepLearning/DeepLearning/MsgPackUtils.h"
#include <ppl.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace TrainingCell::Checkers;

namespace TrainingCellTest
{
	TEST_CLASS(CheckersAgentTrainingTest)
	{
		/// <summary>
		/// Enumerates different training modes
		/// </summary>
		enum class TrainingMode : int
		{
			WHITE = 0,
			BLACK = 1,
			BOTH = 2,
			
		};

		/// <summary>
		/// Run "standard" training of a TD(lambda) agent
		/// </summary>
		static TdLambdaAgent train_agent_standard(const int episodes_with_exploration = 5000, const TrainingMode mode = TrainingMode::BLACK,
			const AutoTrainingSubMode training_sub_mode = AutoTrainingSubMode::FULL )
		{
			RandomAgent agent0;
			TdLambdaAgent result({ 32, 64, 32, 16, 8, 1 }, 0.05, 0.15, 0.97, 0.025);
			result.set_training_sub_mode(training_sub_mode);
			train_agent_standard(result, episodes_with_exploration, mode);

			return result;
		}

		/// <summary>
		/// Run "standard" training of a TD(lambda) agent
		/// </summary>
		static void train_agent_standard(TdLambdaAgent& agent_to_train, const int episodes_with_exploration, const TrainingMode mode)
		{
			RandomAgent r_agent;
			auto board = mode == TrainingMode::WHITE ? Board(&agent_to_train, &r_agent) : ((mode == TrainingMode::BLACK) ?
				Board(&r_agent, &agent_to_train) : Board(&agent_to_train, &agent_to_train));
			board.play(episodes_with_exploration);
			agent_to_train.set_exploration_probability(-1);
			agent_to_train.set_learning_rate(0.01);
			board.play(2000);
			agent_to_train.set_learning_rate(0.001);
			board.play(2000);
		}

		/// <summary>
		/// Runs "standard" performance test of the given agent and
		/// returns its success rate</summary>
		/// <param name="agent">Agent to test. Notice it is the caller who should make
		/// sure that the agent is not in the "training mode" before passing it to the method</param>
		/// <param name="as_white">If "true" the agent will be tested while "playing white" pieces, otherwise, it will play for "black"</param>
		/// <returns>Percentage of wins when playing with "random" agent</returns>
		static double standard_performance_test(Agent& agent, const bool as_white = false)
		{
			RandomAgent r_agent;
			auto board = as_white ? Board(&agent , &r_agent) : Board(&r_agent, &agent);
			constexpr auto episodes = 1000;
			board.play(episodes);

			return (as_white ? board.get_whites_wins() : board.get_blacks_wins()) * 1.0 / episodes;
		}

		/// <summary>
		/// Returns ensemble agent consisting of the given number of TdlAgents with performance above the given level;
		/// If fails to gather the desired number of agents with the given minimal performance for the given number of iterations
		/// an exception is thrown
		/// </summary>
		/// <param name="min_perf_of_agent">Minimal success rate of a trained agent to be included into the ensemble</param>
		/// <param name="number_of_agents_in_ensemble">Desired number of agents in the ensemble</param>
		/// <param name="max_iterations">Maximum number of training iterations to do when gathering agents</param>
		/// <param name="episodes_with_exploration">Number of episodes with exploration component when training agents</param>
		/// <returns></returns>
		static TdlEnsembleAgent train_ensemble_agent(const double min_perf_of_agent, 
		                                             const int number_of_agents_in_ensemble,
			const int max_iterations = 100, const int episodes_with_exploration = 5000)
		{
			TdlEnsembleAgent result;
			std::mutex m;
			Concurrency::parallel_for(0, max_iterations, [&](auto i)
				{
					if (result.size() >= number_of_agents_in_ensemble)
						return;

					auto agent = train_agent_standard(episodes_with_exploration);
					agent.set_training_mode(false);

					if (result.size() >= number_of_agents_in_ensemble)
						return;

					{
						const auto blacks_win_percentage = standard_performance_test(agent);
						Logger::WriteMessage((std::string("Black Win Percentage:") + std::to_string(blacks_win_percentage) + "\n").c_str());

						if (blacks_win_percentage > min_perf_of_agent && result.size() < number_of_agents_in_ensemble)
						{
							std::lock_guard lock(m);
							result.add(agent);
						}
					}
				});

			return result;
		}

		/// <summary>
		/// General method to assess performance of the given agent
		/// </summary>
		template <class A>
		static void assess_performance(A& agent, const double min_acceptable_performance)
		{
			const auto performance_as_black = standard_performance_test(agent, false);
			Logger::WriteMessage((std::string("Score as black: ") + std::to_string(performance_as_black) + "\n").c_str());
			const auto performance_as_white = standard_performance_test(agent, true);
			Logger::WriteMessage((std::string("Score as white: ") + std::to_string(performance_as_white) + "\n").c_str());
			const auto smallest_win_percentage = std::min(performance_as_black, performance_as_white);
			Assert::IsTrue(smallest_win_percentage > min_acceptable_performance, L"Too low win percentage");
		}

	public:

		TEST_METHOD(TdLambdaAgentAsBlackTraining)
		{
			auto agent = train_agent_standard(5000, TrainingMode::BLACK);
			agent.set_training_mode(false);

			//Assert
			assess_performance(agent, 0.95);
		}

		TEST_METHOD(TdLambdaAgentAsWhiteTraining)
		{
			auto agent = train_agent_standard(5000, TrainingMode::WHITE);
			agent.set_training_mode(false);

			//Assert
			assess_performance(agent, 0.95);
		}

		TEST_METHOD(TdLambdaAgentAutoTraining)
		{
			auto agent = train_agent_standard(5000, TrainingMode::BOTH);
			agent.set_training_mode(false);

			//Assert
			assess_performance(agent, 0.935);
		}

		TEST_METHOD(TdLambdaAgentAutoTrainingWhiteOnly)
		{
			auto agent = train_agent_standard(5000, TrainingMode::BOTH, AutoTrainingSubMode::WHITE_ONLY);
			agent.set_training_mode(false);

			//Assert
			assess_performance(agent, 0.89);
		}

		TEST_METHOD(TdLambdaAgentAutoTrainingBlackOnly)
		{
			auto agent = train_agent_standard(5000, TrainingMode::BOTH, AutoTrainingSubMode::BLACK_ONLY);
			agent.set_training_mode(false);

			//Assert
			assess_performance(agent, 0.87);
		}

		TEST_METHOD(EnsembleAgentTest)
		{
			//Act
			auto agent = train_ensemble_agent(0.5, 5, 100);

			//Assert
			assess_performance(agent, 0.97);
		}

		TEST_METHOD(TdLambdaAgentTrainingRegression)
		{
			//Arrange
			auto agent0 = TdLambdaAgent::load_from_file("TestData/TdlTrainingRegression/agent0.tda");
			auto agent1 = TdLambdaAgent::load_from_file("TestData/TdlTrainingRegression/agent1.tda");
			const auto agent0_trained = TdLambdaAgent::load_from_file("TestData/TdlTrainingRegression/agent0_trained.tda");
			const auto agent1_trained = TdLambdaAgent::load_from_file("TestData/TdlTrainingRegression/agent1_trained.tda");

			//Act
			Board board(&agent0, &agent1);
			board.play(200);

			//Assert
			Assert::IsTrue(agent0 == agent0_trained, L"0th agent does not coincide with the reference");
			Assert::IsTrue(agent1 == agent1_trained, L"1st agent does not coincide with the reference");
		}
	};
}
