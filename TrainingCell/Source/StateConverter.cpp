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

#include "../Headers/StateConverter.h"
#include "../Headers/Chess/PieceController.h"

namespace TrainingCell
{
	void StateConverter::activate()
	{
		switch (_type)
		{
		case StateConversionType::None:
			_operator = [](const std::vector<int>& in, DeepLearning::Tensor& out)
			{
				throw std::exception("Uninitialized converter.");
			};
			return;
		case StateConversionType::CheckersStandard:
			_expansion_factor = 1;
			_operator = [](const std::vector<int>& in, DeepLearning::Tensor& out)
			{
				out.resize(1, 1, in.size());
				std::ranges::copy(in, out.begin());
			};
			return;
		case StateConversionType::ChessStandard:
			_expansion_factor = Chess::PieceController::RankBitsCount;
			_operator = [](const std::vector<int>& in, DeepLearning::Tensor& out)
			{
				constexpr int channels = Chess::PieceController::RankBitsCount;
				out.resize(1, 1, in.size() * channels);
				const auto out_ptr = out.begin();

				auto out_item_id = 0;
				for (const auto piece_token : in)
				{
					const auto positive = !Chess::PieceController::is_rival_piece(piece_token);
					
					for (auto channel_id = 0; channel_id < channels; ++channel_id)
					{
						const auto bit_present = static_cast<int>((piece_token & (1 << channel_id)) != 0);
						out_ptr[out_item_id++] = positive ? bit_present : -bit_present;
					}
				}
			};
			return;
		}

		throw std::exception("Unknown conversion type.");
	}

	void StateConverter::msgpack_unpack(msgpack::object const& msgpack_o)
	{
		msgpack::type::make_define_array(_type).msgpack_unpack(msgpack_o);
		activate();
	}

	StateConverter::StateConverter(const StateConversionType type) : _type(type)
	{
		activate();
	}

	StateConverter::StateConverter() : StateConverter(StateConversionType::None)
	{}

	int StateConverter::get_expansion_factor() const
	{
		return _expansion_factor;
	}

	void StateConverter::convert(const std::vector<int>& in, DeepLearning::Tensor& out) const
	{
		_operator(in, out);
	}

	bool StateConverter::operator==(const StateConverter& anotherConverter) const
	{
		return _type == anotherConverter._type;
	}

	bool StateConverter::operator!=(const StateConverter& anotherConverter) const
	{
		return !(*this == anotherConverter);
	}
}
