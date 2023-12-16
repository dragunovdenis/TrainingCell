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
#include "../TrainingCell/Headers/TdLambdaAgent.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace TrainingCell;

namespace TrainingCellTest
{
	TEST_CLASS(TdLambdaStateSpecializationTest)
	{

		/// <summary>
		/// General method to test augmentation of hidden neural net layers in the constructor of TDL agent.
		/// </summary>
		static void test_hidden_layer_augmentation(const StateTypeId state_type_id)
		{
			// Arrange
			const std::vector<std::size_t> hidden_layer_dimensions_original{ 64, 32, 16, 8 };

			// Act
			const TdLambdaAgent agent(hidden_layer_dimensions_original, 0.05, 0.1, 0.9, 0.11, state_type_id);

			// Assert
			const auto net_dimensions = agent.get_net_dimensions();
			Assert::IsTrue(agent.get_state_type_id() == state_type_id, L"Unexpected state type ID of the constructed agent.");
			Assert::IsTrue(net_dimensions.size() == hidden_layer_dimensions_original.size() + 2, L"Unexpected number of neural net layers.");
			Assert::IsTrue(*net_dimensions.rbegin() == 1, L"Unexpected number of neurons in the output layer.");
			std::vector<std::size_t> hidden_layer_dimensions(hidden_layer_dimensions_original.size());
			std::copy(net_dimensions.begin() + 1, net_dimensions.end() - 1, hidden_layer_dimensions.begin());
			Assert::IsTrue(hidden_layer_dimensions == hidden_layer_dimensions_original, L"Dimensions of hidden net layers must remain the same.");
		}

		TEST_METHOD(TdLambdaAgentHiddenLayerAugmentationCheckersTest)
		{
			test_hidden_layer_augmentation(StateTypeId::CHECKERS);
		}

		TEST_METHOD(TdLambdaAgentHiddenLayerAugmentationChessTest)
		{
			test_hidden_layer_augmentation(StateTypeId::CHESS);
		}

		TEST_METHOD(TdLambdaAgentConstructionWithInvalidStateTypeTest)
		{
			Assert::ExpectException<std::exception>([]() {TdLambdaAgent({ 64, 32, 16, 8 },
				0.05, 0.1, 0.9, 0.11, StateTypeId::INVALID); }, L"Exception must be thrown");
		}

		TEST_METHOD(TdLambdaAgentConstructionWithAllStateTypeTest)
		{
			Assert::ExpectException<std::exception>([]() {TdLambdaAgent({ 64, 32, 16, 8 },
				0.05, 0.1, 0.9, 0.11, StateTypeId::ALL); }, L"Exception must be thrown");
		}
	};
}