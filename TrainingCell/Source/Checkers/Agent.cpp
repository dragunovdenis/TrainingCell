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

#include "../../Headers/Checkers/Agent.h"
#include "../../../DeepLearning/DeepLearning/Utilities.h"

namespace TrainingCell::Checkers
{
	void Agent::set_training_mode(const bool training_mode) { throw std::exception("Not implemented"); }

	bool Agent::get_training_mode() const { throw std::exception("Not implemented"); }

	bool Agent::equal(const Agent& agent) const
	{
		return _id == agent._id &&
			_name == agent._name &&
			_record_book == agent._record_book;
	}

	const std::string& Agent::get_name() const
	{
		return _name;
	}

	void Agent::set_name(const std::string& name)
	{
		_name = name;
	}

	const std::string& Agent::get_id() const
	{
		return _id;
	}

	Agent::Agent()
	{
		_id = DeepLearning::Utils::create_guid_string();
	}

	std::size_t Agent::add_record(const std::string& record)
	{
		_record_book.push_back(record);
		return _record_book.size() - 1;
	}

	std::size_t Agent::get_records_count() const
	{
		return _record_book.size();
	}

	const std::string& Agent::get_record(const std::size_t record_id) const
	{
		if (record_id >= _record_book.size())
			throw std::exception("Invalid record ID");

		return _record_book[record_id];
	}

	const std::vector<std::string>& Agent::get_record_book() const
	{
		return _record_book;
	}
}
