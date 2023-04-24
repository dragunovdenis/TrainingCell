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
#include "../TrainingCell/Headers/Checkers/State.h"
#include "../DeepLearning/DeepLearning/MsgPackUtils.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace TrainingCell::Checkers;

namespace TrainingCellTest
{
	TEST_CLASS(CheckersStateTest)
	{
		TEST_METHOD(StartStateTest)
		{
			//Act
			const auto start_state = State::get_start_state();

			//Assert
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
			//Arrange
			const auto state = State::get_start_state();

			//Act
			const auto inverted_state = state.get_inverted();
			const auto double_inverted = state.get_inverted().get_inverted();

			//Assert
			for (auto pos_id = 0ull; pos_id < state.size(); ++pos_id)
			{
				Assert::IsTrue(state[pos_id] == Utils::get_anti_piece(inverted_state[state.size() - 1 - pos_id]),
					L"Unexpected piece at the current position");
			}

			Assert::IsTrue(state.is_inverted() != inverted_state.is_inverted(), L"Unexpected value of the flag");
			Assert::IsTrue(state == double_inverted, L"Double inverted state must be equal to the initial one");
		}

		TEST_METHOD(StateSerializationTest)
		{
			//Arrange
			const auto state = State::get_start_state().get_inverted();
			Assert::IsTrue(state.is_inverted(), L"State is supposed to be inverted");

			//Act
			const auto state_from_stream = DeepLearning::MsgPack::unpack<State>(DeepLearning::MsgPack::pack(state));

			//Assert
			Assert::IsTrue(state == state_from_stream, L"States are supposed to be equal");
		}
	};
}
