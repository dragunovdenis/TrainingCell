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

#include <concrt.h>
#include "CppUnitTest.h"
#include "../TrainingCell/Headers/StateConverter.h"
#include "../../DeepLearning/DeepLearning/Utilities.h"
#include "../DeepLearning/DeepLearning/MsgPackUtils.h"
#include "../TrainingCell/Headers/Chess/ChessMove.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace TrainingCell;

namespace TrainingCellTest
{
	TEST_CLASS(StateConverterTest)
	{
		/// <summary>
		/// Returns random vector of integer values of the given size.
		/// </summary>
		static std::vector<int> get_random_state_vector(const int size)
		{
			std::vector<double> temp(size);
			DeepLearning::Utils::fill_with_random_values(temp.begin(), temp.end(), 0, 1 << Chess::PieceController::TotalBitsCount);
			std::vector<int> result;
			std::ranges::transform(temp, std::back_inserter(result), [](const auto x) { return static_cast<int>(x); });
			return result;
		}

		/// <summary>
		/// General method to test serialization of state convertors of different type.
		/// </summary>
		static void test_serialization(const StateConversionType type)
		{
			// Arrange
			const StateConverter converter_original(type);
			constexpr int state_size = 125;
			const auto input = get_random_state_vector(state_size);
			DeepLearning::Tensor reference_out;
			converter_original.convert(input, reference_out);
			// Sanity check
			Assert::IsTrue(reference_out.size() == state_size * converter_original.get_expansion_factor(),
				L"Unexpected size of the conversion result.");

			// Act
			const auto converter_restored = DeepLearning::MsgPack::unpack<StateConverter>(DeepLearning::MsgPack::pack(converter_original));

			// Assert
			DeepLearning::Tensor trial_out;
			converter_restored.convert(input, trial_out);
			Assert::IsTrue(reference_out == trial_out, L"Outputs must be the same");
			Assert::AreEqual(converter_original.get_expansion_factor(), converter_restored.get_expansion_factor(),
				L"Expansion factors must be the same.");
		}

		TEST_METHOD(SerializationChessStandardTest)
		{
			test_serialization(StateConversionType::ChessStandard);
		}

		TEST_METHOD(SerializationChechersStandardTest)
		{
			test_serialization(StateConversionType::CheckersStandard);
		}

		TEST_METHOD(TypeNoneConverterTest)
		{
			// Arrange
			const StateConverter converter(StateConversionType::None);

			// Assert
			Assert::AreEqual(-1, converter.get_expansion_factor(), L"Unexpected value of expansion factor.");

			Assert::ExpectException<std::exception>([&converter]()
				{
					const auto input = get_random_state_vector(123);
			        DeepLearning::Tensor out;
			        converter.convert(input, out);
				}, L"Converter of `none` type should throw an exception on conversion.");
		}

		TEST_METHOD(CheckersStandardConversionTest)
		{
			// Arrange
			const StateConverter converter(StateConversionType::CheckersStandard);
			constexpr int state_size = 125;
			const auto input = get_random_state_vector(state_size);

			// Act
			DeepLearning::Tensor reference_out;
			converter.convert(input, reference_out);

			// Assert
			Assert::AreEqual(converter.get_expansion_factor(), 1,
				L"Unexpected expansion factor for the given conversion type.");
			const auto input_double = std::vector<double>(input.begin(), input.end());
			Assert::IsTrue(reference_out.to_stdvector() == input_double, L"Unexpected result of conversion.");
		}

		TEST_METHOD(ChecssStandardConversionTest)
		{
			// Arrange
			const StateConverter converter(StateConversionType::ChessStandard);
			constexpr int state_size = 125;
			const auto input = get_random_state_vector(state_size);

			// Act
			DeepLearning::Tensor out;
			converter.convert(input, out);

			// Assert
			Assert::AreEqual(converter.get_expansion_factor(), Chess::PieceController::RankBitsCount,
				L"Unexpected expansion factor for the given conversion type.");
			Assert::AreEqual(out.size(), input.size() * converter.get_expansion_factor(),
				L"Unexpected size of the conversion output.");

			const auto out_check = out.to_stdvector();

			Assert::IsTrue(std::ranges::all_of(out, [](const auto x)
				{
					return x == 0 || x == 1 || x == -1;
				}), L"Unexpected value in the output collection.");

			const auto out_ptr = out.begin();

			for (auto input_item_id = 0ull; input_item_id < input.size(); ++input_item_id)
			{
				const auto original_piece_token = input[input_item_id];
				const auto base_id = input_item_id * Chess::PieceController::RankBitsCount;
				const auto rank_restored = static_cast<int>(out_ptr[base_id]) +
					2 * static_cast<int>(out_ptr[base_id + 1]) +
					4 * static_cast<int>(out_ptr[base_id + 2]);

				const auto rank_abs_restored = static_cast<int>(std::abs(out_ptr[base_id])) +
					2 * static_cast<int>(std::abs(out_ptr[base_id + 1])) +
					4 * static_cast<int>(std::abs(out_ptr[base_id + 2]));

				Assert::AreEqual(rank_abs_restored, std::abs(rank_restored), L"Values must be equal.");

				Assert::IsTrue(rank_restored == 0 && Chess::PieceController::extract_min_piece_rank(original_piece_token) == 0 ||
					rank_restored > 0 && Chess::PieceController::is_ally_piece(original_piece_token) ||
					rank_restored < 0 && Chess::PieceController::is_rival_piece(original_piece_token),
					L"Unexpected sign of the restored rank.");

				Assert::AreEqual(rank_abs_restored,
					Chess::PieceController::extract_min_piece_rank(original_piece_token),
					L"Unexpected value of the restored rank");
			}
		}
	};
}
