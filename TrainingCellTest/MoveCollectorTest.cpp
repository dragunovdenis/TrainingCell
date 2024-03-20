//Copyright (c) 2024 Denys Dragunov, dragunovdenis@gmail.com
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
#include "../DeepLearning/DeepLearning/Utilities.h"
#include "../TrainingCell/Headers/MoveCollector.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace TrainingCell;

namespace TrainingCellTest
{
	TEST_CLASS(MoveCollectorTest)
	{
		/// <summary>
		/// Returns collection consisting of the
		/// given number of "move data" samples initialized with random values.
		/// </summary>
		static std::vector<MoveData> generate_samples(const int samples_count)
		{
			std::map<double, MoveData> temp_result;

			auto counter = 0;
			while (temp_result.size() < samples_count)
			{
				MoveData sample(counter++,
					DeepLearning::Utils::get_random(-10, 10), DeepLearning::Tensor(10, 3, 15));
				sample.after_state.standard_random_fill();
				temp_result[sample.value] = sample;
			}

			std::vector<MoveData> result(temp_result.size());
			std::ranges::transform(temp_result, result.begin(), [](const auto& val) { return val.second; });

			return result;
		}

		TEST_METHOD(ValueFilteringTest)
		{
			// Arrange
			constexpr auto samples_count = 1000;
			constexpr auto samples_to_collect = 10;
			MoveCollector collector(samples_to_collect);
			const auto samples = generate_samples(samples_count);
			auto samples_ordered_by_value = samples;
			std::ranges::sort(samples_ordered_by_value, [](const auto& x, const auto& y) { return x.value > y.value; });
			// Take `samples_to_collect` samples with the highest values, those should be accumulated by the collector.
			std::map<int, MoveData> reference_lookup;
			for (auto sample_id = 0; sample_id < samples_to_collect; ++sample_id)
			{
				const auto& sample = samples_ordered_by_value[sample_id];
				reference_lookup[sample.move_id] = sample;
			}

			// Act
			for (const auto& sample : samples)
				collector.add(sample.move_id, sample.value, sample.after_state);

			// Assert
			Assert::IsTrue(collector.get_elements_count() == samples_to_collect, L"Unexpected number of collected elements.");
			for (auto sample_id = 0; sample_id < collector.get_elements_count(); ++sample_id)
			{
				const auto& collected_sample = collector.get(sample_id);

				Assert::IsTrue(reference_lookup.contains(collected_sample.move_id),
					L"Sample not found in the reference collection.");

				Assert::IsTrue(collected_sample == reference_lookup[collected_sample.move_id],
					L"Data structures are not equal.");

				reference_lookup.erase(collected_sample.move_id);
			}

			Assert::IsTrue(reference_lookup.empty(), L"Reference lookup table is supposed to be empty at this point.");
		}
	};
}
