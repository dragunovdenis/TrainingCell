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

#pragma once
#include "../../DeepLearning/DeepLearning/Math/Tensor.h"
#include <functional>

namespace TrainingCell
{
	/// <summary>
	/// Represents possible types of state conversion.
	/// </summary>
	enum class StateConversionType : int
	{
		None = 0,
		CheckersStandard = 1,
		ChessStandard = 2,
	};

	/// <summary>
	/// Functionality to do custom conversion of "standard" state representations into
	/// neural net "consumable" tensors.
	/// </summary>
	class StateConverter
	{
		StateConversionType _type{};

		/// <summary>
		/// Represents ratio between the output and input data dimensions.
		/// </summary>
		int _expansion_factor{ -1 };

		std::function<void(const std::vector<int>&, DeepLearning::Tensor&)> _operator;

		/// <summary>
		/// Activation method.
		/// </summary>
		void activate();

	public:

		/// <summary>
		/// Default constructor.
		/// </summary>
		StateConverter();

		/// <summary>
		/// Custom "packing" method
		/// </summary>
		template <typename Packer>
		void msgpack_pack(Packer& msgpack_pk) const
		{
			msgpack::type::make_define_array(_type).msgpack_pack(msgpack_pk);
		}

		/// <summary>
		/// Custom "unpacking" method
		/// </summary>
		void msgpack_unpack(msgpack::object const& msgpack_o);

		/// <summary>
		/// Constructor.
		/// </summary>
		StateConverter(const StateConversionType type);

		/// <summary>
		/// Returns ration between output and input data dimensions.
		/// </summary>
		int get_expansion_factor() const;

		/// <summary>
		/// Converts input vector and stores the result in the output tensor.
		/// </summary>
		void convert(const std::vector<int>& in, DeepLearning::Tensor& out) const;

		/// <summary>
		/// Equality operator.
		/// </summary>
		bool operator ==(const StateConverter& anotherConverter) const;

		/// <summary>
		/// Inequality operator.
		/// </summary>
		bool operator !=(const StateConverter& anotherConverter) const;
	};
}

MSGPACK_ADD_ENUM(TrainingCell::StateConversionType)

