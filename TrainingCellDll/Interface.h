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
#include "../TrainingCell/Headers/Board.h"
#include "../TrainingCell/Headers/Agent.h"
#include "../TrainingCell/Headers/Move.h"

#ifdef TRAINING_CELL_EXPORTS
#define TRAINING_CELL_API __declspec(dllexport)
#else
#define TRAINING_CELL_API __declspec(dllexport)
#endif


namespace TrainingCell
{
	class TdLambdaAgent;
	class InteractiveAgent;
	class RandomAgent;
	class TdlEnsembleAgent;
	class AgentPack;
}

/// <summary>
/// Delegate to acquire array from the caller side
/// </summary>
typedef void (*GetArrayCallBack)(const int size, const unsigned int* arr);

extern "C"
{
	/// <summary>
	/// Runs training for the given number of episodes.
	/// Returns 0 if succeeded.
	/// </summary>
	TRAINING_CELL_API int RunTraining(
		TrainingCell::Agent* agent1,
		TrainingCell::Agent* agent2,
		int episodes, 
		TrainingCell::StateTypeId state_type_id,
		TrainingCell::PublishStateCallBack publishStateCallBack,
		TrainingCell::PublishEndEpisodeStatsCallBack publishStatsCallBack, TrainingCell::CancelCallBack cancellationCallBack,
		TrainingCell::ErrorMessageCallBack errorCallBack);

#pragma region Random agent
	/// <summary>
	/// Constructs a random agent on the heap and returns pointer to it
	/// </summary>
	TRAINING_CELL_API void* ConstructRandomAgent();

	/// <summary>
	/// Disposes the agent pointed by the given pointer
	/// </summary>
	TRAINING_CELL_API bool FreeRandomAgent(const TrainingCell::RandomAgent* agent_ptr);
#pragma endregion Random Agent
#pragma region Td(Lammbda)-Agent
	/// <summary>
	/// Constructs a TD(lambda) agent on the heap and returns pointer to it
	/// </summary>
	/// <param name="hidden_layer_dims">Dimensions of hidden layers (input and output layer dimensions
	/// will be deduced from the state type)</param>
	/// <param name="dims_count">Number of elements in "layer_dims" array</param>
	/// <param name="exploration_epsilon">Probability of making exploration move</param>
	/// <param name="lambda">The corresponding parameter of TD(lambda) method</param>
	/// <param name="gamma">Discount coefficient (used to evaluate value of a state)</param>
	/// <param name="alpha">Step of the gradient method (learning rate)</param>
	/// <param name="state_type_id">ID of the state type the agent will be specialized on.</param>
	TRAINING_CELL_API void* ConstructTdLambdaAgent(const unsigned int* hidden_layer_dims,
		const int dims_count, const double exploration_epsilon, const double lambda, const double gamma, const double alpha,
		const TrainingCell::StateTypeId state_type_id);

	/// <summary>
	/// Returns pointer to a copy of the TD(lambda) agent pointed by the given pointer
	/// Returns "null" if failed
	/// </summary>
	TRAINING_CELL_API void* TdLambdaAgentCreateCopy(const TrainingCell::TdLambdaAgent* agent_ptr);

	/// <summary>
	/// Returns true if the TD(lambda) agents pointed by the two given pointers are equal
	/// </summary>
	TRAINING_CELL_API bool TdLambdaAgentsAreEqual(const TrainingCell::TdLambdaAgent* agent0_ptr,
		const TrainingCell::TdLambdaAgent* agent1_ptr);

	/// <summary>
	/// Updates exploration probability parameter of TD(lambda) agent represented with its pointer
	///	Returns "true" if succeeded
	/// </summary>
	TRAINING_CELL_API bool TdLambdaAgentSetEpsilon(TrainingCell::TdLambdaAgent* agent_ptr, const double exploration_epsilon);

	/// <summary>
	/// Returns exploration probability of TD(lambda) agent represented with its pointer
	/// </summary>
	TRAINING_CELL_API double TdLambdaAgentGetEpsilon(const TrainingCell::TdLambdaAgent* agent_ptr);

	/// <summary>
	/// Updates "lambda" parameter of TD(lambda) agent represented with its pointer
	///	Returns "true" if succeeded
	/// </summary>
	TRAINING_CELL_API bool TdLambdaAgentSetLambda(TrainingCell::TdLambdaAgent* agent_ptr, const double lambda);

	/// <summary>
	/// Returns "lambda" parameter of TD(lambda) agent represented with its pointer
	/// </summary>
	TRAINING_CELL_API double TdLambdaAgentGetLambda(const TrainingCell::TdLambdaAgent* agent_ptr);

	/// <summary>
	/// Updates "gamma" (reward discount) parameter of TD(lambda) agent represented with its pointer
	///	Returns "true" if succeeded
	/// </summary>
	TRAINING_CELL_API bool TdLambdaAgentSetGamma(TrainingCell::TdLambdaAgent* agent_ptr, const double gamma);

	/// <summary>
	/// Returns "gamma" (reward discount) parameter of TD(lambda) agent represented with its pointer
	/// </summary>
	TRAINING_CELL_API double TdLambdaAgentGetGamma(const TrainingCell::TdLambdaAgent* agent_ptr);

	/// <summary>
	/// Updates learning rate parameter of TD(lambda) agent represented with its pointer
	///	Returns "true" if succeeded
	/// </summary>
	TRAINING_CELL_API bool TdLambdaAgentSetLearningRate(TrainingCell::TdLambdaAgent* agent_ptr, const double alpha);

	/// <summary>
	/// Returns learning rate parameter of TD(lambda) agent represented with its pointer
	/// </summary>
	TRAINING_CELL_API double TdLambdaAgentGetLearningRate(const TrainingCell::TdLambdaAgent* agent_ptr);

	/// <summary>
	/// Updates "reward factor" parameter of TD(lambda) agent represented with its pointer
	/// Returns "true" if succeeded
	/// "Reward factor" serves to scale value of internal reward function of the agent
	/// </summary>
	TRAINING_CELL_API bool TdLambdaAgentSetRewardFactor(TrainingCell::TdLambdaAgent* agent_ptr, const double reward_factor);

	/// <summary>
	/// Returns "reward factor" parameter of TD(lambda) agent represented with its pointer
	/// "Reward factor" serves to scale value of internal reward function of the agent
	/// Returns "NaN" if failed
	/// </summary>
	TRAINING_CELL_API double TdLambdaAgentGetRewardFactor(const TrainingCell::TdLambdaAgent* agent_ptr);

	/// <summary>
	/// Updates "search depth" parameter of TD(lambda) agent represented with its pointer
	/// Returns "true" if succeeded
	/// </summary>
	TRAINING_CELL_API bool TdLambdaAgentSetSearchDepth(TrainingCell::TdLambdaAgent* agent_ptr, const int search_depth);

	/// <summary>
	/// Returns "search depth" parameter of TD(lambda) agent represented with its pointer
	/// Returns "-1" if failed
	/// </summary>
	TRAINING_CELL_API int TdLambdaAgentGetSearchDepth(const TrainingCell::TdLambdaAgent* agent_ptr);

	/// <summary>
	/// Returns neural net dimensions of TD(lambda) agent represented with its pointer
	/// </summary>
	TRAINING_CELL_API bool TdLambdaAgentGetNetDimensions(const TrainingCell::TdLambdaAgent* agent_ptr,
		const GetArrayCallBack acquireDimensionsCallBack);

	/// <summary>
	/// Tries to load TD(lambda) agent from the given file on disk and returns pointer to it in case of success
	///	(otherwise null pointer is returned)
	/// </summary>
	TRAINING_CELL_API void* TdLambdaAgentLoadFromFile(const char* path);

	/// <summary>
	/// Returns "true" if the two agents represented with their pointers are equal
	/// </summary>
	TRAINING_CELL_API bool TdLambdaAgentEqual(const TrainingCell::TdLambdaAgent* agent_ptr1, const TrainingCell::TdLambdaAgent* agent_ptr2);

	/// <summary>
	/// Tries to save the given TD(lambda) agent represented with its pointer to the given file on disk
	/// Returns "true" if succeeded
	/// </summary>
	TRAINING_CELL_API bool TdLambdaAgentSaveToFile(const TrainingCell::TdLambdaAgent* agent_ptr, const char* path);

	/// <summary>
	/// Disposes the agent pointed by the given pointer
	/// </summary>
	TRAINING_CELL_API bool FreeTdLambdaAgent(const TrainingCell::TdLambdaAgent* agent_ptr);

	/// <summary>
	/// Allocates agent-pack for the given TD(lambda) agent and returns pointer to it (or null-pointer if something went wrong)
	/// </summary>
	TRAINING_CELL_API void* PackTdLambdaAgent(const TrainingCell::TdLambdaAgent* agent_ptr);

	/// <summary>
	/// Returns "1" if the given agent has its tree search functionality on, "0" if the search mode is off.
	/// Returned value other than "0" or "1" indicates an error.
	/// </summary>
	TRAINING_CELL_API char TdLambdaAgentGetSearchMode(const TrainingCell::TdLambdaAgent* agent_ptr);

	/// <summary>
	/// Sets search mode of the given agent, returns true is succeeded
	/// </summary>
	TRAINING_CELL_API bool TdLambdaAgentSetSearchMode(TrainingCell::TdLambdaAgent* agent_ptr, const bool search_mode);

	/// <summary>
	/// Returns number of search iterations to be done if tree search mode is on
	/// Negative returned number indicates an error
	/// </summary>
	TRAINING_CELL_API int TdLambdaAgentGetSearchModeIterations(const TrainingCell::TdLambdaAgent* agent_ptr);

	/// <summary>
	/// Sets number of search iterations to be done if tree search mode is on
	/// Returns "true" in case of success
	/// </summary>
	TRAINING_CELL_API bool TdLambdaAgentSetSearchModeIterations(TrainingCell::TdLambdaAgent* agent_ptr, const int search_iterations);

#pragma endregion Td(Lammbda)-Agent
#pragma region Interactive Agent
	/// <summary>
	/// Data transferring object to pass moves
	/// </summary>
	struct MoveDto
	{
		/// <summary>
		///	Pointer to the array of sub_moves
		/// </summary>
		TrainingCell::SubMove* sub_moves{};

		/// <summary>
		///	Number of sub-moves in the array
		/// </summary>
		int sub_moves_cnt{};
	};

	/// <summary>
	///	"Make move" delegate
	/// </summary>
	typedef int (*MakeMoveCallBack)(const int* state, const int state_size, const MoveDto* moves, const int moves_size);

	/// <summary>
	///	"Game over" delegate
	/// </summary>
	typedef void (*GameOverCallBack)(const int* state, const int state_size, const int game_result);

	/// <summary>
	/// Constructs "interactive" agent on the heap and returns pointer to it
	/// </summary>
	TRAINING_CELL_API void* ConstructInteractiveAgent(const MakeMoveCallBack make_move_callback, const GameOverCallBack game_over_callback, const bool play_for_whites);

	/// <summary>
	/// Disposes the agent pointed by the given pointer
	/// </summary>
	TRAINING_CELL_API bool FreeInteractiveAgent(const TrainingCell::InteractiveAgent* agent_ptr);

#pragma endregion Interactive Agent
#pragma region Agent
	/// <summary>
	/// Updates training mode parameter of an agent represented with its pointer
	/// Returns "true" if succeeded
	/// </summary>
	TRAINING_CELL_API bool AgentSetTrainingMode(TrainingCell::Agent* agent_ptr, const bool training_mode);

	/// <summary>
	/// Returns training mode parameter of an agent represented with its pointer
	/// </summary>
	TRAINING_CELL_API char AgentGetTrainingMode(const TrainingCell::Agent* agent_ptr);

	/// <summary>
	/// Returns "can train" flag of an agent represented with its pointer
	/// </summary>
	TRAINING_CELL_API char AgentGetCanTrainFlag(const TrainingCell::Agent* agent_ptr);

	/// <summary>
	/// Returns name of the agent represented with its pointer
	/// </summary>
	TRAINING_CELL_API const char* AgentGetName(const TrainingCell::Agent* agent_ptr);

	/// <summary>
	/// Sets name to the agent represented with its pointer
	/// </summary>
	TRAINING_CELL_API bool AgentSetName(TrainingCell::Agent* agent_ptr, const char* name);

	/// <summary>
	/// Returns string identifier of the agent represented with its pointer
	/// </summary>
	TRAINING_CELL_API const char* AgentGetId(const TrainingCell::Agent* agent_ptr);

	/// <summary>
	/// Returns state type ID of the agent represented with its pointer
	/// </summary>
	TRAINING_CELL_API TrainingCell::StateTypeId AgentGetStateTypeId(const TrainingCell::Agent* agent_ptr);

	/// <summary>
	/// Returns number of records in the record-book of the agent
	/// </summary>
	TRAINING_CELL_API int AgentGetRecordsCount(const TrainingCell::Agent* agent_ptr);

	/// <summary>
	/// Returns pointer to the string record (from the record book of the given agent) with the given id or null-pointer if
	/// the id is invalid
	/// </summary>
	TRAINING_CELL_API const char* AgentGetRecordById(const TrainingCell::Agent* agent_ptr, const int record_id);

	/// <summary>
	/// Adds given record to the record book of the given agent
	/// </summary>
	/// <returns>Index of the added record in the record book</returns>
	TRAINING_CELL_API int AgentAddRecord(TrainingCell::Agent* agent_ptr, const char* record);

	/// <summary>
	/// Returns "true" if there is a uniquely determined state type within which the two agent can play (the type is returned as `out_state_type_id` param).
	/// In case "false" is returned, the value of `out_state_type_id` parameter should be ignored by the caller.
	/// </summary>
	TRAINING_CELL_API bool CanPlay(const TrainingCell::Agent* agent0_ptr, const TrainingCell::Agent* agent1_ptr, TrainingCell::StateTypeId& out_state_type_id);
#pragma endregion Agent
#pragma region AgentPack
	/// <summary>
	/// Loads agent-pack from the given file on disk and returns pointer to it
	/// </summary>
	TRAINING_CELL_API void* AgentPackLoadFromFile(const char* path);

	/// <summary>
	/// Saves agent-pack pointed by the given pointer to the given file on disk
	/// </summary>
	TRAINING_CELL_API bool AgentPackSaveToFile(const TrainingCell::AgentPack* agent_pack_ptr, const char* path);

	/// <summary>
	/// Frees instance of the agent-pack pointed by the given pointer
	/// </summary>
	TRAINING_CELL_API bool AgentPackFree(const TrainingCell::AgentPack* agent_pack_ptr);

	/// <summary>
	/// Returns pointer to the "packed" agent (TrainingCell::Agent*)
	/// </summary>
	/// <param name="agent_pack_ptr"></param>
	/// <returns></returns>
	TRAINING_CELL_API void* AgentPackGetAgentPtr(TrainingCell::AgentPack* agent_pack_ptr);
#pragma endregion AgentPack
#pragma region TdlEnsembleAgent

	/// <summary>
	/// Returns pointer to an instance of TdlEnsembleAgent constructed fr4om the given collection of agents
	/// </summary>
	/// <param name="agent_count">Number of agents in the given collection</param>
	/// <param name="agent_collection">Collection of pointers to TdLambdaInstances</param>
	TRAINING_CELL_API void* ConstructTdlEnsembleAgent(const int agent_count, const TrainingCell::TdLambdaAgent** agent_collection);

	/// <summary>
	/// Frees the instance of ensemble collection represented with the given pointer
	///	Returns "true" if succeeded
	/// </summary>
	TRAINING_CELL_API bool FreeTdlEnsembleAgent(const TrainingCell::TdlEnsembleAgent* agent_ptr);

	/// <summary>
	/// Saves the given instance of ensemble agent to the given file on disc (returns "true" if succeeded)
	/// </summary>
	TRAINING_CELL_API bool SaveTdlEnsembleAgent(const TrainingCell::TdlEnsembleAgent* agent_ptr, const char* file_path);

	/// <summary>
	/// Loads ensemble agent from the given file on disk
	/// </summary>
	TRAINING_CELL_API void* LoadTdlEnsembleAgent(const char* file_path);

	/// <summary>
	/// Returns number of elements in the underlying collection of the given ensemble agent
	///	or "-1" if something went wrong
	/// </summary>
	/// <param name="agent_ptr">Pointer to an instance of TdlEnsembleAgent</param>
	/// <returns></returns>
	TRAINING_CELL_API int TdlEnsembleAgentGetSize(const TrainingCell::TdlEnsembleAgent* agent_ptr);

	/// <summary>
	/// Returns string identifier of a sub-agent of the given ensemble with the given index
	/// </summary>
	/// <param name="agent_ptr">Pointer to an ensemble agent</param>
	/// <param name="agent_id">Index of sub-agent to infer identifier from</param>
	TRAINING_CELL_API const char* TdlEnsembleAgentGetSubAgentId(const TrainingCell::TdlEnsembleAgent* agent_ptr, const int agent_id);

	/// <summary>
	/// Adds given agent to the given ensemble and returns index of the added agent in the underlying collection of agents
	///	or "-1" of something went wrong
	/// </summary>
	/// <param name="ensemble_agent_ptr">Ensemble to add agent to</param>
	/// <param name="agent_to_add_ptr">Agent, copy of which will be added to the ensemble</param>
	TRAINING_CELL_API int TdlEnsembleAgentAdd(TrainingCell::TdlEnsembleAgent* ensemble_agent_ptr,
		const TrainingCell::TdLambdaAgent* agent_to_add_ptr);

	/// <summary>
	/// Removes a sub-agent with the given index (in the underlying collection of agents in the ensemble)
	///	from the given ensemble
	/// </summary>
	/// <param name="ensemble_agent_ptr">Ensemble to be modified</param>
	/// <param name="sub_agent_id">Index of the sub-agent to be removed from the ensemble</param>
	TRAINING_CELL_API bool TdlEnsembleAgentRemove(TrainingCell::TdlEnsembleAgent* ensemble_agent_ptr,
		const int sub_agent_id);

	/// <summary>
	/// Sets "single agent model" for the given instance of ensemble agent
	/// and returns actual index of a "chosen" sub-agent (or "-1" depending on the input parameter)
	/// </summary>
	TRAINING_CELL_API int TdlEnsembleAgentSetSingleAgentMode(TrainingCell::TdlEnsembleAgent* ensemble_agent_ptr,
		const bool set_single_agent_mode);

	/// <summary>
	/// Returns index of the single agent in case "single agent mode" is on of "-2" otherwise
	/// </summary>
	TRAINING_CELL_API int TdlEnsembleAgentGetSingleAgentId(TrainingCell::TdlEnsembleAgent* ensemble_agent_ptr);

	/// <summary>
	/// Allocates agent-pack for the given TD(lambda)-ensemble agent and returns pointer to it (or null-pointer if something went wrong)
	/// </summary>
	TRAINING_CELL_API void* PackTdlEnsembleAgent(const TrainingCell::TdlEnsembleAgent* agent_ptr);

	/// <summary>
	/// Returns pointer to constant instance of the sub-agent with the given id
	/// </summary>
	TRAINING_CELL_API const void* TdlEnsembleAgentGetSubAgentPtr(const TrainingCell::TdlEnsembleAgent* agent_ptr, const int sub_agent_id);
#pragma endregion TdlEnsembleAgent
}
