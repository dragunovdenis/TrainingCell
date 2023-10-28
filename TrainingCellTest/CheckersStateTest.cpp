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
#include "../TrainingCell/Headers/Checkers/CheckersState.h"
#include "../TrainingCell/Headers/Move.h"
#include "../TrainingCell/Headers/Checkers/CheckersMove.h"
#include "../DeepLearning/DeepLearning/MsgPackUtils.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace TrainingCell;
using namespace TrainingCell::Checkers;

namespace TrainingCellTest
{
	TEST_CLASS(CheckersStateTest)
	{
		TEST_METHOD(StartStateTest)
		{
			// Act
			const auto start_state = CheckersState::get_start_state();

			// Assert
			Assert::IsTrue(std::all_of(start_state.begin(), start_state.begin() + 12, [](const Piece& pc)
				{
					return pc == Piece::Man;
				}), L"Man piece was expected");

			Assert::IsTrue(std::all_of(start_state.begin() + 12, start_state.begin() + 20, [](const Piece& pc)
				{
					return pc == Piece::Space;
				}), L"Space was expected");

			Assert::IsTrue(std::all_of(start_state.begin() + 20, start_state.end(), [](const Piece& pc)
				{
					return pc == Piece::AntiMan;
				}), L"Anti-man was expected");

			Assert::IsFalse(start_state.is_inverted(), L"Start state should not be inverted");
		}

		TEST_METHOD(InversionTest)
		{
			// Arrange
			const auto state = CheckersState::get_start_state();

			// Act
			const auto inverted_state = state.get_inverted();
			const auto double_inverted = state.get_inverted().get_inverted();

			// Assert
			for (auto pos_id = 0ull; pos_id < state.size(); ++pos_id)
			{
				Assert::IsTrue(state[pos_id] == CheckersState::get_anti_piece(inverted_state[state.size() - 1 - pos_id]),
					L"Unexpected piece at the current position");
			}

			Assert::IsTrue(state.is_inverted() != inverted_state.is_inverted(), L"Unexpected value of the flag");
			Assert::IsTrue(state == double_inverted, L"Double inverted state must be equal to the initial one");
		}

		TEST_METHOD(StateSerializationTest)
		{
			// Arrange
			const auto state = CheckersState::get_start_state().get_inverted();
			Assert::IsTrue(state.is_inverted(), L"State is supposed to be inverted");

			// Act
			const auto state_from_stream = DeepLearning::MsgPack::unpack<CheckersState>(DeepLearning::MsgPack::pack(state));

			// Assert
			Assert::IsTrue(state == state_from_stream, L"States are supposed to be equal");
		}

		TEST_METHOD(GetVectorTest)
		{
			// Arrange
			const auto random_state = CheckersTestUtils::get_random_state();

			const auto moves = random_state.get_moves();

			// Sanity check
			Assert::IsTrue(moves.size() > 0, L"Empty collection of available moves");

			for (const auto& move : moves)
			{
				// Act
				const auto state_vector = random_state.get_vector(move);

				// Assert
				auto state_copy = random_state;
				state_copy.make_move(move);
				const auto reference_vector = state_copy.to_vector();

				Assert::IsTrue(state_vector == reference_vector, L"Vectors are not the same");
			}
		}

		TEST_METHOD(GetVectorInvertedTest)
		{
			// Arrange
			const auto random_state = CheckersTestUtils::get_random_state();

			const auto moves = random_state.get_moves();

			// Sanity check
			Assert::IsTrue(moves.size() > 0, L"Empty collection of available moves");

			for (const auto& move : moves)
			{
				// Act
				const auto state_vector = random_state.get_vector_inverted(move);

				// Assert
				auto state_copy = random_state;
				state_copy.make_move(move);
				state_copy.invert();
				const auto reference_vector = state_copy.to_vector();

				Assert::IsTrue(state_vector == reference_vector, L"Vectors are not the same");
			}
		}

		/// <summary>
		/// General method to run move conversion test.
		/// </summary>
		/// <param name="gen_move"></param>
		static void run_checkers_move_to_general_move_conversion_test(const Move& gen_move)
		{
			// Act
			const CheckersMove conversion_to_checkers_move(gen_move);
			const auto conversion_back_to_gen_move = conversion_to_checkers_move.to_move();

			// Assert
			Assert::IsTrue(gen_move.sub_moves == conversion_back_to_gen_move.sub_moves,
				L"Initial and restored moves are not the same");
		}

		TEST_METHOD(CheckersMoveToGeneralMoveMultipleCapturesConversionTest)
		{
			// Arrange
			const auto gen_move = Move{
				{
					{ {0, 0}, {3, 3}, {2, 2}},
					{ {3, 3}, {5, 5}, {4, 4}},
					{ {5, 5}, {7, 3}, {6, 4}},
					{ {7, 3}, {4, 0}, {5, 1}},
					{ {4, 0}, {2, 2}, {3, 1}},
					{ {2, 2}, {0, 4}, {1, 3}},
					{ {0, 4}, {3, 7}, {2, 6}},
				} };

			// Act and assert
			run_checkers_move_to_general_move_conversion_test(gen_move);
		}

		TEST_METHOD(CheckersMoveToGeneralMoveNoCaptureConversionTest)
		{
			// Arrange
			const auto gen_move = Move{
				{
					{ {0, 0}, {3, 3}},
				} };

			// Act and assert
			run_checkers_move_to_general_move_conversion_test(gen_move);
		}

		TEST_METHOD(CheckersMoveToGeneralMoveSingleCaptureConversionTest)
		{
			// Arrange
			const auto gen_move = Move{
				{
					{ {0, 0}, {3, 3}, {2, 2}},
				} };

			// Act and assert
			run_checkers_move_to_general_move_conversion_test(gen_move);
		}

		TEST_METHOD(CheckersMoveToGeneralMoveDoubleInlineCaptureConversionTest)
		{
			// Arrange
			const auto gen_move = Move{
				{
					{ {7, 2}, {5, 4}, {6, 3}},
					{ {5, 4}, {2, 7}, {3, 6}},
				} };

			// Act and assert
			run_checkers_move_to_general_move_conversion_test(gen_move);
		}

	};
}
