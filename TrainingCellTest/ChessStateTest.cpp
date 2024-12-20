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
#include "../TrainingCell/Headers/Chess/ChessState.h"
#include "../DeepLearning/DeepLearning/Utilities.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace TrainingCell::Chess;

namespace TrainingCellTest
{
	TEST_CLASS(ChessStateTest)
	{
		template <class F>
		static void run_standard_game_play_test(const F& functor)
		{
			// Arrange
			constexpr auto episodes_to_play = 500;

			// Act
			std::vector<ChessMove> available_moves;
			int castling_moves_executed = 0;
			int stalemates = 0;
			int checkmates = 0;
			int promotions = 0;
			int total_options_count = 0;
			int total_moves_played = 0;

			for (auto episode_id = 0; episode_id < episodes_to_play; ++episode_id)
			{
				auto state = ChessState::get_start_state();
				int round_id = 0;
				while (round_id < 1000 && !state.get_moves(available_moves) && !available_moves.empty())
				{
					total_options_count += static_cast<int>(available_moves.size());
					total_moves_played++;

					const auto move_id = DeepLearning::Utils::get_random_int(0,
							static_cast<int>(available_moves.size() - 1));

					const auto& move = available_moves[move_id];

					// count some exotic moves to be sure that test covers them
					castling_moves_executed += state.is_castling_move(move);
					promotions += state.is_promotion(move);

					functor(state, move);

					round_id++;
				}

				const auto draw = state.get_moves(available_moves);
				stalemates += draw && available_moves.empty();
				checkmates += !draw && available_moves.empty();

				Assert::IsTrue(round_id >= 4, 
					(std::wstring(L"It is impossible to play a game for less than four moves: ") +
						std::to_wstring(round_id)).c_str());
			}

			const auto average_options_per_move = total_options_count / static_cast<double>(total_moves_played);

			Logger::WriteMessage((std::string("Castling moves : ") + std::to_string(castling_moves_executed) + "\n").c_str());
			Logger::WriteMessage((std::string("Stalemates : ") + std::to_string(stalemates) + "\n").c_str());
			Logger::WriteMessage((std::string("Checkmates : ") + std::to_string(checkmates) + "\n").c_str());
			Logger::WriteMessage((std::string("Promotions : ") + std::to_string(promotions) + "\n").c_str());
			Logger::WriteMessage((std::string("Average options per move : ") + std::to_string(average_options_per_move) + "\n").c_str());
			Assert::IsTrue(castling_moves_executed >= episodes_to_play * 0.03, L"Too few castling moves..");
			Assert::IsTrue(stalemates >= episodes_to_play * 0.03, L"Too few stalemates.");
			Assert::IsTrue(checkmates >= episodes_to_play * 0.1, L"Too few checkmates.");
			Assert::IsTrue(promotions >= episodes_to_play, L"Too few promotions.");
		}

		TEST_METHOD(ToVectorConversionComplexTest)
		{
			run_standard_game_play_test([](ChessState& state, const ChessMove& move)
				{
					const auto vector_with_move = state.get_vector(move);
					const auto vector_with_move_inverted = state.get_vector_inverted(move);

					state.make_move(move);
					const auto vector_with_move_reference = state.to_vector();
					state.invert();
					const auto vector_with_move_inverted_reference = state.to_vector();

					Assert::IsTrue(vector_with_move == vector_with_move_reference,
						L"Vectors must be the same");
					Assert::IsTrue(vector_with_move_inverted == vector_with_move_inverted_reference,
						L"Vectors must be the same (inverted case)");
				});
		}

		TEST_METHOD(AttackFieldValidationTest)
		{
			run_standard_game_play_test([](ChessState& state, const ChessMove& move)
				{
					state.make_move(move);
					const ChessState check_state(state.to_vector(), state.is_inverted());

					Assert::IsTrue(state == check_state,
						L"States are supposed to be the same");

					state.invert();
				});
		}
	};
}
