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
		TEST_METHOD(ToVectorConversionComplexTest)
		{
			// Arrange
			constexpr auto rounds_to_do = 25;
			auto state = ChessState::create_start_state();

			// Act
			for (auto round_id = 0; round_id < rounds_to_do; ++round_id)
			{
				const auto available_moves = state.get_moves();
				const auto move_id = DeepLearning::Utils::get_random_int(0,
					static_cast<int>(available_moves.size() - 1));
				const auto& move = available_moves[move_id];

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
			}
		}
	};
}
