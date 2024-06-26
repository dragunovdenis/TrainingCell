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
#include "AgentTypeId.h"
#include "ITrainableAgent.h"

namespace TrainingCell
{
	/// <summary>
	///	Abstract checkers agent (interface)
	/// </summary>
	class Agent : public ITrainableAgent
	{
		std::string _id{};
		std::string _name{};
		std::vector<std::string> _record_book{};

	public:
		/// <summary>
		/// Constructor
		/// </summary>
		Agent();

		/// <summary>
		/// Returns type identifier of the current instance
		/// </summary>
		[[nodiscard]] virtual AgentTypeId get_type_id() const = 0;

		/// <summary>
		/// Sets "training_mode" flag for the agent defining whether the agent trains while playing
		/// </summary>
		void set_training_mode(const bool training_mode) override;

		/// <summary>
		/// Returns actual value of training mode
		/// </summary>
		[[nodiscard]] bool get_training_mode() const override;

		/// <summary>
		/// Returns true if the current agent is equal to the given one
		/// </summary>
		[[nodiscard]] virtual bool equal(const Agent& agent) const;

		MSGPACK_DEFINE(_name, _id, _record_book);

		/// <summary>
		/// Getter for a string name of the agent
		/// </summary>
		[[nodiscard]] const std::string& get_name() const;

		/// <summary>
		/// Getter for a string identifier of the agent
		/// </summary>
		void set_name(const std::string& name);

		/// <summary>
		/// Read-only access to the unique identifier of the object
		/// </summary>
		[[nodiscard]] const std::string& get_id() const;

		/// <summary>
		/// Adds the given record to the record book
		/// Returns index of the added record in the record-book
		/// </summary>
		std::size_t add_record(const std::string& record);

		/// <summary>
		/// Returns number of records in the record book
		/// </summary>
		[[nodiscard]] std::size_t get_records_count() const;

		/// <summary>
		/// Read-only access to the record string with the given ID (throws exception if ID is invalid)
		/// </summary>
		[[nodiscard]] const std::string& get_record(const std::size_t record_id) const;

		/// <summary>
		/// Returns constant reference to the entire record book of the agent
		/// </summary>
		[[nodiscard]] const std::vector<std::string>& get_record_book() const;

		/// <summary>
		/// See documentation of the base class.
		/// </summary>
		[[nodiscard]] StateTypeId get_state_type_id() const override;
	};
}

