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
#include "../TrainingCell/Headers/StateHandleGeneral.h"
#include "../TrainingCell/Headers/Chess/ChessState.h"
#include "../DeepLearning/DeepLearning/Utilities.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace TrainingCell;

namespace TrainingCellTest
{
	TEST_CLASS(StateTraceRecorderTest)
	{
		/// <summary>
		/// Plays an episode on the given state and returns collection
		/// of passed states that can be used to validate state trace recorder.
		/// </summary>
		static std::vector<std::vector<int>> play_episode(IState& state_handle)
		{
			std::vector<std::vector<int>> passed_states;
			passed_states.push_back(state_handle.evaluate());

			while (state_handle.get_moves_count() > 0 && !state_handle.is_draw() && passed_states.size() < 700)
			{
				const auto move_id = DeepLearning::Utils::get_random_int(
					0, static_cast<int>(state_handle.get_moves_count() - 1));

				state_handle.move_invert_reset(move_id);
				passed_states.push_back(state_handle.evaluate());
			}

			return passed_states;
		}

		TEST_METHOD(ValidateRecordedState)
		{
			constexpr auto episodes_count = 1000;
			auto stalemates_counter = 0;
			auto checkmates_counter = 0;

			for (auto iter_id = 0; iter_id < episodes_count; ++iter_id)
			{
				// Arrange
				StateHandleGeneral state_handle(Chess::ChessState::get_start_state(), true /*initialize recorder*/);

				// Act
				const auto passed_sates = play_episode(state_handle);

				// Assert
				auto recorded_state_ptr = state_handle.get_recorded_state();
				Assert::IsNotNull(recorded_state_ptr.get(), L"State trace was not recorded");

				const auto double_passed_states = play_episode(*recorded_state_ptr);

				Assert::IsTrue(passed_sates == double_passed_states, L"Passed and double-passed states are not the same.");
				Assert::IsTrue(recorded_state_ptr->is_draw() == state_handle.is_draw(), L"Final draw flags are not the same.");
				Assert::IsNull(recorded_state_ptr->get_recorded_state().get(), L"Recorder of the recorded state should not be initialized.");

				stalemates_counter += static_cast<int>(state_handle.is_draw() && state_handle.get_moves_count() <= 0);
				checkmates_counter += static_cast<int>(!state_handle.is_draw() && state_handle.get_moves_count() <= 0);
			}

			Logger::WriteMessage((std::string("Stalemates: ") + std::to_string(stalemates_counter) + "\n").c_str());
			Logger::WriteMessage((std::string("Checkmates: ") + std::to_string(checkmates_counter)+ "\n").c_str());

			Assert::IsTrue(stalemates_counter > 0.03 * episodes_count, L"too low percentage of stalemates.");
			Assert::IsTrue(checkmates_counter > 0.06 * episodes_count, L"too low percentage of checkmates.");
		}
	};
}