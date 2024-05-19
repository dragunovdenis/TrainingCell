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

#pragma once
#include "IStateEditor.h"

namespace TrainingCell
{
	template <class S>
	class StateEditor : public IStateEditor
	{
		S _state{};

	public:

		/// <summary>
		/// Constructor.
		/// </summary>
		StateEditor();

		/// <summary>
		/// See summary of the base class.
		/// </summary>
		[[nodiscard]] std::unique_ptr<IState> yield(const bool initialize_recorder) const override;

		/// <summary>
		/// See summary of the base class.
		/// </summary>
		[[nodiscard]] StateTypeId state_type() const override;

		/// <summary>
		/// See summary of the base class.
		/// </summary>
		std::vector<int> to_vector() const override;

		/// <summary>
		/// See summary of the base class.
		/// </summary>
		std::vector<int> get_options(const PiecePosition& pos) const override;

		/// <summary>
		/// See summary of the base class.
		/// </summary>
		void apply_option(const PiecePosition& pos, const int option_id) override;

		/// <summary>
		/// See summary of the base class.
		/// </summary>
		void reset() override;

		/// <summary>
		/// See summary of the base class.
		/// </summary>
		void clear() override;

		/// <summary>
		/// See summary of the base class.
		/// </summary>
		[[nodiscard]] StateTypeId get_state_type() const override;
	};
}
