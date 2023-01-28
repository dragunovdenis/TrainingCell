#include "CppUnitTest.h"
#include "../TrainingCell/Headers/Checkers/Board.h"
#include "../TrainingCell/Headers/Checkers/Agent.h"
#include "../TrainingCell/Headers/Checkers/AgentPack.h"
#include "../DeepLearning/DeepLearning/MsgPackUtils.h"
#include <ppl.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace TrainingCell::Checkers;

namespace TrainingCellTest
{
	TEST_CLASS(TrainingCellTest)
	{
		/// <summary>
		/// Run "standard" training of a TD(lambda) agent
		/// </summary>
		static TdLambdaAgent train_agent_standard(const int episodes_with_exploration = 5000)
		{
			RandomAgent agent0;
			TdLambdaAgent result({ 32, 64, 32, 16, 8, 1 }, 0.05, 0.15, 0.9, 0.05);
			Board board(&agent0, &result);
			board.play(episodes_with_exploration);
			result.set_exploration_probability(-1);
			result.set_learning_rate(0.01);
			board.play(2000);
			result.set_learning_rate(0.001);
			board.play(2000);

			return result;
		}

		/// <summary>
		/// Runs "standard" performance test of the given agent and
		/// returns its success rate</summary>
		/// <param name="agent">Agent to test. Notice it is the caller who should make
		/// sure that the agent is not in the "training mode" before passing it to the method</param>
		/// <returns>Percentage of wins when playing with "random" agent</returns>
		static double standard_performance_test(Agent& agent)
		{
			RandomAgent r_agent;
			Board board(&r_agent, &agent);
			board.play(1000);

			return board.get_blacks_wins() * 1.0 / (board.get_whites_wins() + board.get_blacks_wins());
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
							result.Add(agent);
						}
					}
				});

			return result;
		}

	public:
		
		TEST_METHOD(TdLambdaAgentTraining)
		{
			TdlEnsembleAgent ensemble;

			auto smallest_win_percentage = 1.0;
			auto average_win_percentage = 0.0;
			constexpr auto epochs = 1;
			std::mutex m;
			Concurrency::parallel_for(0, epochs, [&](auto i)
				{
					auto agent = train_agent_standard();
					agent.set_training_mode(false);

					{
						const auto performance = standard_performance_test(agent);
						std::lock_guard lock(m);
						average_win_percentage += performance;

						if (smallest_win_percentage > performance)
							smallest_win_percentage = performance;

						Logger::WriteMessage((std::string("Black Win Percentage:") + std::to_string(performance) + "\n").c_str());
					}
				});

			average_win_percentage /= epochs;

			Logger::WriteMessage((std::string("Minimal percentage: ") + std::to_string(smallest_win_percentage) + "\n").c_str());
			Logger::WriteMessage((std::string("Average percentage: ") + std::to_string(average_win_percentage) + "\n").c_str());
			Assert::IsTrue(smallest_win_percentage > 0.9, L"Too low win percentage");
		}

		TEST_METHOD(EnsembleAgentTest)
		{
			//Act
			auto agent = train_ensemble_agent(0.5, 5, 100);

			//Assert
			const auto performance = standard_performance_test(agent);
			Logger::WriteMessage((std::string("Performance: ") + std::to_string(performance) + "\n").c_str());
			Assert::IsTrue(performance > 0.97, L"Too low performance");
		}

		TEST_METHOD(TdLambdaAgentPackSerialization)
		{
			//Arrange
			const TdLambdaAgent agent({ 32, 64, 32, 16, 8, 1 }, 0.05, 0.1, 0.9, 0.1);
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
				{{ 32, 64, 32, 16, 8, 1 }, 0.05, 0.1, 0.9, 0.1},
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
			TdLambdaAgent agent({ 32, 64, 32, 16, 8, 1 }, 0.05, 0.1, 0.9, 0.1);
			auto state = State::get_start_state();
			//Do some number of iterations but not call "game over" method so that all
			//the auxiliary data of the agent is nontrivial
			const int iter_count = 10;
			for (auto iter_id = 0; iter_id < iter_count; ++iter_id)
			{
				const auto moves = state.get_moves();
				const auto move_id = agent.make_move(state, moves);
				state.make_move(moves[move_id], true, false);
				state.invert();
			}

			//Act
			const auto agent_from_stream = DeepLearning::MsgPack::unpack<TdLambdaAgent>(DeepLearning::MsgPack::pack(agent));

			//Assert
			Assert::IsTrue(agent == agent_from_stream, L"Deserialized agent is not equal to the initial one");
		}
	};
}
