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

#include "../Headers/NetWithConverter.h"
#include "../../DeepLearning/DeepLearning/MsgPackUtils.h"

namespace TrainingCell
{
	DeepLearning::Net<DeepLearning::CpuDC>& NetWithConverter::net()
	{
		return _net;
	}

	const DeepLearning::Net<DeepLearning::CpuDC>& NetWithConverter::net() const
	{
		return _net;
	}

	const StateConverter& NetWithConverter::converter() const
	{
		return _converter;
	}

	NetWithConverter::NetWithConverter(const DeepLearning::Net<DeepLearning::CpuDC>& net,
		StateConverter converter) : _converter(std::move(converter)), _net(net)
	{}

	void NetWithConverter::save_to_file(const std::filesystem::path& fileName) const
	{
		DeepLearning::MsgPack::save_to_file(*this, fileName);
	}

	NetWithConverter NetWithConverter::load_from_file(const std::filesystem::path& fileName)
	{
		return DeepLearning::MsgPack::load_from_file<NetWithConverter>(fileName);
	}

	bool NetWithConverter::operator==(const NetWithConverter& another_net) const
	{
		return _net.equal(another_net.net()) && _converter == another_net._converter;
	}

	bool NetWithConverter::operator!=(const NetWithConverter& another_net) const
	{
		return !(*this == another_net);
	}
}
