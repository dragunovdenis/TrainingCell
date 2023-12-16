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
#include "TdlAbstractAgent.h"

namespace TrainingCell
{
	class TdlLegacyMsgPackAdapter;

	/// <summary>
	/// Self-training capable TD(lambda) agent
	/// </summary>
	class TdLambdaAgent : public TdlAbstractAgent
	{
		/// <summary>
		/// Field to track version of the container and facilitate backward compatibility if needed
		/// </summary>
		int _msg_pack_version = 1;

	public:

		MSGPACK_DEFINE(_msg_pack_version, MSGPACK_BASE(TdlAbstractAgent))

		/// <summary>
		/// Construction from a legacy adapter container
		/// </summary>
		TdLambdaAgent(const TdlLegacyMsgPackAdapter& legacyContainer);

		/// <summary>
		/// Default constructor
		/// </summary>
		TdLambdaAgent() = default;

		/// <summary>
		///	Construction from script
		/// </summary>
		TdLambdaAgent(const std::string& script_str);

		/// <summary>Constructor</summary>
		/// <param name="hidden_layer_dimensions">Dimensions of the hidden(!) layers of the neural net that will serve as "afterstate value function";
		/// They will be automatically augmented with "input" layer compatible with the suggested state type and the "output" layer of dimension "1".</param>
		/// <param name="exploration_epsilon">Probability of taking an exploratory move during the training process, (0,1)</param>
		/// <param name="lambda">Lambda parameters used to determine "strength" of eligibility traces, (0,1)</param>
		/// <param name="gamma">Discount reward determining how reward decay with each next move, (0,1) </param>
		/// <param name="alpha">Learning rate</param>
		/// <param name="state_type_id">Type ID of the state the agent will be "compatible" with.</param>
		/// <param name="name">Name of the agent</param>
		TdLambdaAgent(const std::vector<std::size_t>& hidden_layer_dimensions, const double exploration_epsilon,
			const double lambda, const double gamma, const double alpha, const StateTypeId state_type_id, const std::string& name = "AutoAgent");

		/// <summary>
		/// Returns type ID
		/// </summary>
		static AgentTypeId TYPE_ID();

		/// <summary>
		/// Returns type identifier of the current instance
		/// </summary>
		[[nodiscard]] AgentTypeId get_type_id() const override;

		/// <summary>
		/// Returns "true" if the agent can train otherwise returns "false"
		/// If "false" is returned one should avoid calling getter or setter of the "training mode"
		/// property because the later can throw exception (as not applicable)
		/// </summary>
		[[nodiscard]] bool can_train() const override;

		/// <summary>
		/// Returns true if the current agent is equal to the given one
		/// </summary>
		[[nodiscard]] bool equal(const Agent& agent) const override;

		/// <summary>
		/// Equality operator
		/// </summary>
		bool operator == (const TdLambdaAgent& another_agent) const;

		/// <summary>
		/// Inequality operator
		/// </summary>
		bool operator != (const TdLambdaAgent& another_agent) const;

		/// <summary>
		/// Serializes the current instance of the agent into the "message-pack" format and saves it
		/// to the given file. Trows exception if fails.
		/// </summary>
		void save_to_file(const std::filesystem::path& file_path) const;

		/// <summary>
		/// Instantiates agent from the given "message-pack" file.
		/// Throws exception if fails.
		/// </summary>
		static TdLambdaAgent load_from_file(const std::filesystem::path& file_path);

		/// <summary>
		/// Returns smart pointer to a clone of the current instance
		/// </summary>
		[[nodiscard]] std::unique_ptr<Agent> clone() const override;

	};

}
