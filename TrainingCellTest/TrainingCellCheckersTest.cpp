#include "CppUnitTest.h"
#include "../TrainingCell/Headers/Checkers/Board.h"
#include "../TrainingCell/Headers/Checkers/Agent.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TrainingCellTest
{
	TEST_CLASS(TrainingCellTest)
	{
	public:
		
		TEST_METHOD(TdLambdaAgentTraining)
		{
			auto smallest_win_percentage = 1.0;
			auto average_win_percentage = 0.0;
			constexpr auto epochs = 1;
			for (int i = 0; i < epochs; i++)
			{
				TrainingCell::Checkers::RandomAgent agent0{};
				TrainingCell::Checkers::TdLambdaAgent agent1({ 32, 64, 32, 16, 8, 1 }, 0.05, 0.1, 0.9, 0.1);
				TrainingCell::Checkers::Board board(&agent0, &agent1);
				board.play(5000, 200, nullptr, nullptr);
				agent1.set_exploration_probability(-1);
				board.play(500, 200, nullptr, nullptr);

				board.reset_wins();
				agent1.set_training_mode(false);
				board.play(1000, 200, nullptr, nullptr);

				const auto blacks_win_percentage = board.get_blacks_wins() * 1.0 / (board.get_whites_wins() + board.get_blacks_wins());
				average_win_percentage += blacks_win_percentage;

				if (smallest_win_percentage > blacks_win_percentage)
					smallest_win_percentage = blacks_win_percentage;

				Logger::WriteMessage((std::string("Black Win Percentage:") + std::to_string(blacks_win_percentage) + "\n").c_str());
			}

			average_win_percentage /= epochs;

			Logger::WriteMessage((std::string("Minimal percentage: ") + std::to_string(smallest_win_percentage) + "\n").c_str());
			Logger::WriteMessage((std::string("Average percentage: ") + std::to_string(average_win_percentage) + "\n").c_str());
			Assert::IsTrue(smallest_win_percentage >= 0.9, L"Too low win percentage");
		}
	};
}
