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

#include "NetWithConverterAbstract.h"
#include <msgpack.hpp>

namespace TrainingCell
{
	/// <summary>
	/// Neural net with stater converter.
	/// </summary>
	class NetWithConverter : public NetWithConverterAbstract
	{
		StateConverter _converter;
		DeepLearning::Net<DeepLearning::CpuDC> _net;

	protected:

		/// <summary>
		/// See summary in the base class.
		/// </summary>
		DeepLearning::Net<DeepLearning::CpuDC>& net() override;

		/// <summary>
		/// See summary in the base class.
		/// </summary>
		const StateConverter& converter() const override;

		/// <summary>
		/// See summary in the base class.
		/// </summary>
		const DeepLearning::Net<DeepLearning::CpuDC>& net() const override;
	public:

		MSGPACK_DEFINE(_converter, _net)

		/// <summary>
		/// Default constructor.
		/// </summary>
		NetWithConverter() = default;

		/// <summary>
		/// Constructor.
		/// </summary>
		NetWithConverter(const DeepLearning::Net<DeepLearning::CpuDC>& net, StateConverter converter);

		/// <summary>
		/// Saves the instance to the given file on disk.
		/// </summary>
		void save_to_file(const std::filesystem::path& fileName) const;

		/// <summary>
		/// Loads instance of the class from the given file on disk.
		/// </summary>
		static NetWithConverter load_from_file(const std::filesystem::path& fileName);

		/// <summary>
		/// Equality operator
		/// </summary>
		bool operator == (const NetWithConverter& another_net) const;

		/// <summary>
		/// Inequality operator
		/// </summary>
		bool operator != (const NetWithConverter& another_net) const;
	};
}
