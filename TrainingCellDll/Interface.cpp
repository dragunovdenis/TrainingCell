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

#include "Interface.h"
#include "../TrainingCell/Headers/AgentPack.h"
#include "../TrainingCell/Headers/Checkers/CheckersState.h"
#include "../TrainingCell/Headers/RandomAgent.h"
#include "../TrainingCell/Headers/InteractiveAgent.h"
#include "../TrainingCell/Headers/StateTypeController.h"

namespace
{
	/// <summary>
	///	Converts given raw array into a standard vector
	/// </summary>
	std::vector<std::size_t> convert(const unsigned int* arr, const int length)
	{
		std::vector<std::size_t> result(length);
		result.assign(arr, arr + length);
		return result;
	}
}

int PlayStateSeed(TrainingCell::Agent* const agent1,
	TrainingCell::Agent* const agent2,
	int episodes, const TrainingCell::IStateSeed* state_seed_ptr,
	TrainingCell::PublishStateCallBack publishStateCallBack,
	TrainingCell::PublishEndEpisodeStatsCallBack publishStatsCallBack,
	TrainingCell::CancelCallBack cancellationCallBack,
	TrainingCell::ErrorMessageCallBack errorCallBack, TrainingCell::Board::Stats& stats)
{
	try
	{
		stats = TrainingCell::Board::play(agent1, agent2, episodes, *state_seed_ptr,
			50, publishStateCallBack, publishStatsCallBack,
			cancellationCallBack, errorCallBack);

		return 0;
	}
	catch (...)
	{
		stats = { -1, -1, -1 };
		return -1;
	}
}

int Play(TrainingCell::Agent* const agent1,
	TrainingCell::Agent* const agent2,
	int episodes, TrainingCell::StateTypeId state_type_id,
	TrainingCell::PublishStateCallBack publishStateCallBack,
	TrainingCell::PublishEndEpisodeStatsCallBack publishStatsCallBack,
	TrainingCell::CancelCallBack cancellationCallBack,
	TrainingCell::ErrorMessageCallBack errorCallBack, TrainingCell::Board::Stats& stats)
{

	std::unique_ptr<TrainingCell::IStateSeed> state_seed_ptr = nullptr;

	try
	{
		state_seed_ptr = TrainingCell::StateTypeController::get_start_seed(state_type_id);
	}
	catch (...)
	{
		stats = { -1, -1, -1 };
		return -1;
	}

	return PlayStateSeed(agent1, agent2, episodes, state_seed_ptr.get(),
		publishStateCallBack, publishStatsCallBack, cancellationCallBack, errorCallBack, stats);
}

int Train(TrainingCell::Agent* agent1, TrainingCell::Agent* agent2, int episodes,
	TrainingCell::StateTypeId state_type_id, TrainingCell::PublishEndEpisodeStatsCallBack publishStatsCallBack,
	TrainingCell::CancelCallBack cancellationCallBack, TrainingCell::ErrorMessageCallBack errorCallBack, 
	TrainingCell::Board::Stats& stats)
{
	try
	{
		const auto seed_ptr = TrainingCell::StateTypeController::get_start_seed(state_type_id);
		stats = TrainingCell::Board::train(agent1, agent2, episodes, *seed_ptr,
			/*max moves without capture*/ 50, /*max consequent draw episodes*/ 100,
			publishStatsCallBack, cancellationCallBack, errorCallBack);

		return 0;
	}
	catch (...)
	{
		stats = { -1, -1, -1 };
		return -1;
	}
}

#pragma region Random Agent
void* ConstructRandomAgent()
{
	return new TrainingCell::RandomAgent();
}

bool FreeRandomAgent(const TrainingCell::RandomAgent* agent_ptr)
{
	if (!agent_ptr)
		return false;

	delete agent_ptr;
	return true;
}
#pragma endregion Random Agent
#pragma region Td(Lammbda)-Agent
void* ConstructTdLambdaAgent(const unsigned int* hidden_layer_dims,
	const int dims_count, const double exploration_epsilon, const double lambda,
	const double gamma, const double alpha, const TrainingCell::StateTypeId state_type_id)
{
	try
	{
		return new TrainingCell::TdLambdaAgent(convert(hidden_layer_dims, dims_count), exploration_epsilon,
			lambda, gamma, alpha, state_type_id);
	} catch(...)
	{
		return nullptr;
	}
}

void* TdLambdaAgentCreateCopy(const TrainingCell::TdLambdaAgent* agent_ptr)
{
	if (!agent_ptr)
		return nullptr;

	return new TrainingCell::TdLambdaAgent(*agent_ptr);
}

bool TdLambdaAgentsAreEqual(const TrainingCell::TdLambdaAgent* agent0_ptr,
	const TrainingCell::TdLambdaAgent* agent1_ptr)
{
	if ((agent0_ptr == nullptr) ^ (agent1_ptr == nullptr))
		return false;

	if (agent0_ptr == agent1_ptr)//this accounts for the case when both pointers are null
		return true;

	try
	{
		return agent0_ptr->equal(*agent1_ptr);
	} catch (...)
	{
		return false;
	}
}

bool TdLambdaAgentSetEpsilon(TrainingCell::TdLambdaAgent* agent_ptr, const double exploration_epsilon)
{
	if (!agent_ptr)
		return false;

	agent_ptr->set_exploration_probability(exploration_epsilon);
	return true;
}

double TdLambdaAgentGetEpsilon(const TrainingCell::TdLambdaAgent* agent_ptr)
{
	if (!agent_ptr)
		return std::numeric_limits<double>::quiet_NaN();

	return agent_ptr->get_exploration_probability();
}

bool TdLambdaAgentSetLambda(TrainingCell::TdLambdaAgent* agent_ptr, const double lambda)
{
	if (!agent_ptr)
		return false;

	agent_ptr->set_lambda(lambda);
	return true;
}

double TdLambdaAgentGetLambda(const TrainingCell::TdLambdaAgent* agent_ptr)
{
	if (!agent_ptr)
		return std::numeric_limits<double>::quiet_NaN();

	return agent_ptr->get_lambda();
}

bool TdLambdaAgentSetGamma(TrainingCell::TdLambdaAgent* agent_ptr, const double gamma)
{
	if (!agent_ptr)
		return false;

	agent_ptr->set_discount(gamma);
	return true;
}

double TdLambdaAgentGetGamma(const TrainingCell::TdLambdaAgent* agent_ptr)
{
	if (!agent_ptr)
		return std::numeric_limits<double>::quiet_NaN();

	return agent_ptr->get_discount();
}

bool TdLambdaAgentSetLearningRate(TrainingCell::TdLambdaAgent* agent_ptr, const double alpha)
{
	if (!agent_ptr)
		return false;

	agent_ptr->set_learning_rate(alpha);
	return true;
}

double TdLambdaAgentGetLearningRate(const TrainingCell::TdLambdaAgent* agent_ptr)
{
	if (!agent_ptr)
		return std::numeric_limits<double>::quiet_NaN();

	return agent_ptr->get_learning_rate();
}

bool TdLambdaAgentGetNetDimensions(const TrainingCell::TdLambdaAgent* agent_ptr, const GetArrayCallBack acquireDimensionsCallBack)
{
	if (!agent_ptr || !acquireDimensionsCallBack)
		return false;

	const auto dims = agent_ptr->get_net_dimensions();
	acquireDimensionsCallBack(static_cast<int>(dims.size()), dims.data());

	return true;
}


void* TdLambdaAgentLoadFromFile(const char* path)
{
	try
	{
		return new TrainingCell::TdLambdaAgent(TrainingCell::TdLambdaAgent::load_from_file(path));
	} catch (...)
	{
		return nullptr;
	}
}

bool TdLambdaAgentSaveToFile(const TrainingCell::TdLambdaAgent* agent_ptr, const char* path)
{
	if (!agent_ptr)
		return false;

	try
	{
		agent_ptr->save_to_file(path);
	} catch (...)
	{
		return false;
	}

	return true;
}

bool TdLambdaAgentEqual(const TrainingCell::TdLambdaAgent* agent_ptr1, const TrainingCell::TdLambdaAgent* agent_ptr2)
{
	if (!agent_ptr1 || !agent_ptr2)
		return false;

	return *agent_ptr1 == *agent_ptr2;
}

bool FreeTdLambdaAgent(const TrainingCell::TdLambdaAgent* agent_ptr)
{
	if (!agent_ptr)
		return false;

	delete agent_ptr;
	return true;
}

void* PackTdLambdaAgent(const TrainingCell::TdLambdaAgent* agent_ptr)
{
	if (!agent_ptr)
		return nullptr;

	try
	{
		return new TrainingCell::AgentPack(TrainingCell::AgentPack::make<TrainingCell::TdLambdaAgent>(*agent_ptr));
	}
	catch (...)
	{
		return nullptr;
	}
}

char TdLambdaAgentGetSearchMode(const TrainingCell::TdLambdaAgent* agent_ptr)
{
	if (!agent_ptr)
		return static_cast<char>(2);

	return static_cast<char>(agent_ptr->get_tree_search_method() != TrainingCell::TreeSearchMethod::NONE);
}

bool TdLambdaAgentSetSearchMode(TrainingCell::TdLambdaAgent* agent_ptr, const bool search_mode)
{
	if (!agent_ptr)
		return false;

	//Currently dll interface allows to use only TD-tree search (TDTS)
	agent_ptr->set_tree_search_method(search_mode ?
		TrainingCell::TreeSearchMethod::TD_SEARCH :
		TrainingCell::TreeSearchMethod::NONE);

	return true;
}

int TdLambdaAgentGetSearchModeIterations(const TrainingCell::TdLambdaAgent* agent_ptr)
{
	if (!agent_ptr)
		return -1;

	return agent_ptr->get_td_search_iterations();
}

bool TdLambdaAgentSetSearchModeIterations(TrainingCell::TdLambdaAgent* agent_ptr, const int search_iterations)
{
	if (!agent_ptr)
		return false;

	agent_ptr->set_td_search_iterations(search_iterations);

	return true;
}

char TdLambdaAgentGetPerformanceEvaluationMode(TrainingCell::TdLambdaAgent* agent_ptr)
{
	if (!agent_ptr)
		return static_cast<char>(2);

	return static_cast<char>(agent_ptr->get_performance_evaluation_mode());
}

bool TdLambdaAgentSetPerformanceEvaluationMode(TrainingCell::TdLambdaAgent* agent_ptr, const bool mode)
{
	if (!agent_ptr)
		return false;

	agent_ptr->set_performance_evaluation_mode(mode);

	return true;
}

bool TdLambdaAgentSetRewardFactor(TrainingCell::TdLambdaAgent* agent_ptr, const double reward_factor)
{
	if (!agent_ptr)
		return false;

	agent_ptr->set_reward_factor(reward_factor);

	return true;
}

double TdLambdaAgentGetRewardFactor(const TrainingCell::TdLambdaAgent* agent_ptr)
{
	if (!agent_ptr)
		return std::numeric_limits<double>::quiet_NaN();

	return agent_ptr->get_reward_factor();
}

bool TdLambdaAgentSetSearchDepth(TrainingCell::TdLambdaAgent* agent_ptr, const int search_depth)
{
	if (!agent_ptr)
		return false;

	agent_ptr->set_search_depth(search_depth);

	return true;
}

int TdLambdaAgentGetSearchDepth(const TrainingCell::TdLambdaAgent* agent_ptr)
{
	if (!agent_ptr)
		return -1;

	return agent_ptr->get_search_depth();
}

int TdLambdaAgentGetSearchExplorationDepth(const TrainingCell::TdLambdaAgent* agent_ptr)
{
	if (!agent_ptr)
		return -1;

	return agent_ptr->get_search_exploration_depth();
}

bool TdLambdaAgentSetSearchExplorationDepth(TrainingCell::TdLambdaAgent* agent_ptr, const int depth)
{
	if (!agent_ptr)
		return false;

	agent_ptr->set_search_exploration_depth(depth);

	return true;
}

int TdLambdaAgentGetSearchExplorationVolume(const TrainingCell::TdLambdaAgent* agent_ptr)
{
	if (!agent_ptr)
		return -1;

	return agent_ptr->get_search_exploration_volume();
}

bool TdLambdaAgentSetSearchExplorationVolume(TrainingCell::TdLambdaAgent* agent_ptr, const int volume)
{
	if (!agent_ptr)
		return false;

	agent_ptr->set_search_exploration_volume(volume);

	return true;
}

double TdLambdaAgentGetSearchExplorationProbability(const TrainingCell::TdLambdaAgent* agent_ptr)
{
	if (!agent_ptr)
		return -1;

	return agent_ptr->get_search_exploration_probability();
}

bool TdLambdaAgentSetSearchExplorationProbability(
	TrainingCell::TdLambdaAgent* agent_ptr, const double probability)
{
	if (!agent_ptr)
		return false;

	agent_ptr->set_search_exploration_probability(probability);

	return true;
}

bool TdLambdaAgentGetScriptString(const TrainingCell::TdLambdaAgent* agent_ptr, char* buffer, int buffer_capacity)
{
	if (!agent_ptr)
		return false;

	const auto script = agent_ptr->to_script();
	strncpy_s(buffer, buffer_capacity, script.c_str(), _TRUNCATE);

	return true;
}

bool TdLambdaAgentEvaluateOptions(const TrainingCell::TdLambdaAgent* agent_ptr,
	const TrainingCell::IStateReadOnly* state_ptr, const GetDoubleArrayCallBack get_rewards_callback)
{
	if (!agent_ptr || !state_ptr || !get_rewards_callback ||
		!TrainingCell::StateTypeController::states_are_compatible(agent_ptr->get_state_type_id(),
			state_ptr->current_state_seed().state_type()))
		return false;

	const auto option_rewards = agent_ptr->evaluate_options(*state_ptr);
	get_rewards_callback(static_cast<int>(option_rewards.size()), option_rewards.data());

	return true;
}

#pragma endregion Td(Lammbda)-Agent
#pragma region Interactive Agent
void* ConstructInteractiveAgent(const MakeMoveCallBack make_move_callback,
	const GameOverCallBack game_over_callback, const bool play_for_whites)
{
	try
	{
		if (!make_move_callback || !game_over_callback)
			return nullptr;

		return new TrainingCell::InteractiveAgent(
			[=](const std::vector<int>& state, std::vector<TrainingCell::Move> moves)
			{
				std::vector<MoveDto> moves_dto;
				moves_dto.reserve(moves.size());
				for (auto& move : moves)
					moves_dto.push_back(MoveDto{ move.sub_moves.data(), static_cast<int>(move.sub_moves.size()), move.final_rank });

				return make_move_callback(state.data(), static_cast<int>(state.size()), moves_dto.data(), static_cast<int>(moves_dto.size()));
			},
			[=](const std::vector<int>& state, const TrainingCell::GameResult& result)
			{
				game_over_callback(state.data(), static_cast<int>(state.size()), static_cast<int>(result));
				
			}, play_for_whites);
	}
	catch (...)
	{
		return nullptr;
	}
}

bool FreeInteractiveAgent(const TrainingCell::InteractiveAgent* agent_ptr)
{
	if (!agent_ptr)
		return false;

	delete agent_ptr;
	return true;
}
#pragma endregion Interactive Agent
#pragma region Agent
bool AgentSetTrainingMode(TrainingCell::Agent* agent_ptr, const bool training_mode)
{
	if (!agent_ptr)
		return false;

	agent_ptr->set_training_mode(training_mode);
	return true;
}

char AgentGetTrainingMode(const TrainingCell::Agent* agent_ptr)
{
	if (!agent_ptr)
		return static_cast<char>(2);

	return static_cast<char>(agent_ptr->get_training_mode());
}

char AgentGetCanTrainFlag(const TrainingCell::Agent* agent_ptr)
{
	if (!agent_ptr)
		return static_cast<char>(2);

	return static_cast<char>(agent_ptr->can_train());
}

const char* AgentGetName(const TrainingCell::Agent* agent_ptr)
{
	if (!agent_ptr)
		return nullptr;

	return agent_ptr->get_name().c_str();
}

bool AgentSetName(TrainingCell::Agent* agent_ptr, const char* name)
{
	if (!agent_ptr)
		return false;

	agent_ptr->set_name(name);

	return true;
}

const char* AgentGetId(const TrainingCell::Agent* agent_ptr)
{
	if (!agent_ptr)
		return nullptr;

	return agent_ptr->get_id().c_str();
}

TrainingCell::StateTypeId AgentGetStateTypeId(const TrainingCell::Agent* agent_ptr)
{
	if (!agent_ptr)
		return TrainingCell::StateTypeId::INVALID;

	return agent_ptr->get_state_type_id();
}

int AgentGetRecordsCount(const TrainingCell::Agent* agent_ptr)
{
	if (agent_ptr == nullptr)
		return -1;

	return static_cast<int>(agent_ptr->get_records_count());
}

const char* AgentGetRecordById(const TrainingCell::Agent* agent_ptr, const int record_id)
{
	if (agent_ptr == nullptr || record_id < 0 || record_id >= agent_ptr->get_records_count())
		return nullptr;

	return agent_ptr->get_record(record_id).c_str();
}

int AgentAddRecord(TrainingCell::Agent* agent_ptr, const char* record)
{
	if (agent_ptr == nullptr || record == nullptr)
		return -1;

	return static_cast<int>(agent_ptr->add_record(record));
}

bool CanPlay(const TrainingCell::Agent* agent0_ptr, const TrainingCell::Agent* agent1_ptr,
	TrainingCell::StateTypeId& out_state_type_id)
{
	if (agent0_ptr == nullptr || agent1_ptr == nullptr)
		return false;

	return TrainingCell::StateTypeController::can_play(*agent0_ptr, *agent1_ptr, out_state_type_id);
}

#pragma endregion Agent
#pragma region AgentPack
void* AgentPackLoadFromFile(const char* path)
{
	try
	{
		return new TrainingCell::AgentPack(TrainingCell::AgentPack::load_from_file(path));
	}
	catch (...)
	{
		return nullptr;
	}
}

bool AgentPackSaveToFile(const TrainingCell::AgentPack* agent_pack_ptr, const char* path)
{
	if (!agent_pack_ptr)
		return false;

	try
	{
		agent_pack_ptr->save_to_file(path);
	}
	catch (...)
	{
		return false;
	}

	return true;
}

bool AgentPackFree(const TrainingCell::AgentPack* agent_pack_ptr)
{
	if (!agent_pack_ptr)
		return false;

	delete agent_pack_ptr;
	return true;
}

void* AgentPackGetAgentPtr(TrainingCell::AgentPack* agent_pack_ptr)
{
	if (!agent_pack_ptr)
		return nullptr;

	return  &(agent_pack_ptr->agent());
}
#pragma endregion AgentPack
#pragma region TdlEnsembleAgent
void* ConstructTdlEnsembleAgent(const int agent_count, const TrainingCell::TdLambdaAgent** agent_collection)
{
	if (!agent_collection && agent_count != 0)
		return nullptr;

	const auto result = new TrainingCell::TdlEnsembleAgent();

	try
	{
		for (auto sub_agent_id = 0; sub_agent_id < agent_count; ++sub_agent_id)
			result->add(*agent_collection[sub_agent_id]);
	} catch(...)
	{
		delete result;
		return nullptr;
	}

	return result;
}

bool FreeTdlEnsembleAgent(const TrainingCell::TdlEnsembleAgent* agent_ptr)
{
	if (!agent_ptr)
		return false;

	delete agent_ptr;
	return true;
}

bool SaveTdlEnsembleAgent(const TrainingCell::TdlEnsembleAgent* agent_ptr, const char* file_path)
{
	if (!agent_ptr)
		return false;

	try
	{
		agent_ptr->save_to_file(file_path);
	} catch (...)
	{
		return false;
	}

	return true;
}

void* LoadTdlEnsembleAgent(const char* file_path)
{
	try
	{
		return new TrainingCell::TdlEnsembleAgent(
			TrainingCell::TdlEnsembleAgent::load_from_file(file_path));
	}
	catch (...)
	{
		return nullptr;
	}
}

int TdlEnsembleAgentGetSize(const TrainingCell::TdlEnsembleAgent* agent_ptr)
{
	if (!agent_ptr)
		return -1;

	return static_cast<int>(agent_ptr->size());
}

const char* TdlEnsembleAgentGetSubAgentId(const TrainingCell::TdlEnsembleAgent* agent_ptr, const int agent_id)
{
	if (!agent_ptr)
		return nullptr;

	return (*agent_ptr)[agent_id].get_name().c_str();
}

int TdlEnsembleAgentAdd(TrainingCell::TdlEnsembleAgent* ensemble_agent_ptr,
	const TrainingCell::TdLambdaAgent* agent_to_add_ptr)
{
	if (!ensemble_agent_ptr || !agent_to_add_ptr)
		return -1;

	return static_cast<int>(ensemble_agent_ptr->add(*agent_to_add_ptr));
}

bool TdlEnsembleAgentRemove(TrainingCell::TdlEnsembleAgent* ensemble_agent_ptr,
	const int sub_agent_id)
{
	if (!ensemble_agent_ptr)
		return false;

	return ensemble_agent_ptr->remove_agent(sub_agent_id);
}

int TdlEnsembleAgentSetSingleAgentMode(TrainingCell::TdlEnsembleAgent* ensemble_agent_ptr,
	const bool set_single_agent_mode)
{
	if (!ensemble_agent_ptr)
		return -1;

	return static_cast<int>(ensemble_agent_ptr->set_single_agent_mode(set_single_agent_mode));
}

int TdlEnsembleAgentGetSingleAgentId(TrainingCell::TdlEnsembleAgent* ensemble_agent_ptr)
{
	if (!ensemble_agent_ptr)
		return -2;

	return static_cast<int>(ensemble_agent_ptr->get_current_random_agent_id());
}

void* PackTdlEnsembleAgent(const TrainingCell::TdlEnsembleAgent* agent_ptr)
{
	if (!agent_ptr)
		return nullptr;

	try
	{
		return new TrainingCell::AgentPack(
			TrainingCell::AgentPack::make<TrainingCell::TdlEnsembleAgent>(*agent_ptr));
	}
	catch (...)
	{
		return nullptr;
	}
}

const void* TdlEnsembleAgentGetSubAgentPtr(const TrainingCell::TdlEnsembleAgent* agent_ptr,
	const int sub_agent_id)
{
	if (agent_ptr == nullptr || sub_agent_id < 0 || sub_agent_id >= agent_ptr->size())
		return nullptr;

	return &(*agent_ptr)[sub_agent_id];
}

char TdlEnsembleAgentGetSearchMode(const TrainingCell::TdlEnsembleAgent* agent_ptr)
{
	if (!agent_ptr)
		return 2;

	return static_cast<char>(agent_ptr->get_search_method() != TrainingCell::TreeSearchMethod::NONE);
}

TRAINING_CELL_API bool TdlEnsembleAgentSetSearchMode(TrainingCell::TdlEnsembleAgent* agent_ptr, const bool search_mode)
{
	if (!agent_ptr)
		return false;

	agent_ptr->set_search_method(search_mode ? TrainingCell::TreeSearchMethod::TD_SEARCH : TrainingCell::TreeSearchMethod::NONE);

	return true;
}

TRAINING_CELL_API int TdlEnsembleAgentGetSearchIterations(const TrainingCell::TdlEnsembleAgent* agent_ptr)
{
	if (!agent_ptr)
		return -1;

	return agent_ptr->get_search_iterations();
}

bool TdlEnsembleAgentSetSearchIterations(TrainingCell::TdlEnsembleAgent* agent_ptr, const int search_iterations)
{
	if (!agent_ptr)
		return false;

	agent_ptr->set_search_iterations(search_iterations);

	return true;
}

int TdlEnsembleAgentGetSearchDepth(const TrainingCell::TdlEnsembleAgent* agent_ptr)
{
	if (!agent_ptr)
		return -1;

	return agent_ptr->get_search_depth();
}

TRAINING_CELL_API bool TdlEnsembleAgentSetSearchDepth(TrainingCell::TdlEnsembleAgent* agent_ptr, const int search_depth)
{
	if (!agent_ptr)
		return false;

	agent_ptr->set_search_depth(search_depth);

	return true;
}

char TdlEnsembleAgentGetRunMultiThreaded(const TrainingCell::TdlEnsembleAgent* agent_ptr)
{
	if (!agent_ptr)
		return 2;


	return static_cast<char>(agent_ptr->get_run_multi_threaded());
}

TRAINING_CELL_API bool TdlEnsembleAgentSetRunMultiThreaded(TrainingCell::TdlEnsembleAgent* agent_ptr, const bool run_multi_threaded)
{
	if (!agent_ptr)
		return false;

	agent_ptr->set_run_multi_threaded(run_multi_threaded);

	return true;
}
#pragma endregion TdlEnsembleAgent

#pragma region StateEditor

void* ConstructStateEditor(TrainingCell::StateTypeId state_type_id)
{
	try
	{
		return TrainingCell::StateTypeController::instantiate_editor(state_type_id);
	} catch(...)
	{
		return nullptr;
	}
}

bool FreeStateEditor(const TrainingCell::IStateEditor* editor_ptr)
{
	if (!editor_ptr)
		return false;

	delete editor_ptr;

	return true;
}

bool StateEditorGetState(const TrainingCell::IStateEditor* editor_ptr, const GetSignedArrayCallBack acquireStateCallBack)
{
	if (!editor_ptr || !acquireStateCallBack)
		return false;

	const auto state_vec = editor_ptr->to_vector();
	acquireStateCallBack(static_cast<int>(state_vec.size()), state_vec.data());

	return true;
}

bool StateEditorGetOptions(const TrainingCell::IStateEditor* editor_ptr, const TrainingCell::PiecePosition pos,
	const GetSignedArrayCallBack acquireEditOptionsCallBack)
{
	if (!editor_ptr || !acquireEditOptionsCallBack)
		return false;

	const auto options = editor_ptr->get_options(pos);
	acquireEditOptionsCallBack(static_cast<int>(options.size()), options.data());

	return true;
}

bool StateEditorApplyOption(TrainingCell::IStateEditor* editor_ptr, const TrainingCell::PiecePosition pos,
	const int option_id)
{
	if (!editor_ptr)
		return false;

	try
	{
		editor_ptr->apply_option(pos, option_id);
	} catch (...)
	{
		return false;
	}

	return true;
}

bool StateEditorReset(TrainingCell::IStateEditor* editor_ptr)
{
	if (!editor_ptr)
		return false;

	editor_ptr->reset();

	return true;
}

bool StateEditorClear(TrainingCell::IStateEditor* editor_ptr)
{
	if (!editor_ptr)
		return false;

	editor_ptr->clear();

	return true;
}

TrainingCell::StateTypeId StateEditorGetTypeId(const TrainingCell::IStateEditor* editor_ptr)
{
	if (!editor_ptr)
		return TrainingCell::StateTypeId::INVALID;

	return editor_ptr->get_state_type();
}

#pragma endregion StateEditor

#pragma region IState

TrainingCell::StateTypeId IStateGetState(const TrainingCell::IStateReadOnly* state_ptr, const GetSignedArrayCallBack get_moves_callback)
{
	if (!state_ptr || !get_moves_callback)
		return TrainingCell::StateTypeId::INVALID;

	const auto state_std = state_ptr->is_inverted() ?
		state_ptr->evaluate_ui_inverted() : state_ptr->evaluate_ui();

	get_moves_callback(static_cast<int>(state_std.size()), state_std.data());

	return state_ptr->current_state_seed().state_type();
}

TrainingCell::StateTypeId IStateGetMoves(const TrainingCell::IStateReadOnly* state_ptr, const GetMovesArrayCallBack get_moves_callback)
{
	if (!state_ptr || !get_moves_callback)
		return TrainingCell::StateTypeId::INVALID;

	const auto moves = state_ptr->get_all_moves();
	std::vector<MoveDto> moves_dto(moves.size());
	std::vector<TrainingCell::Move> moves_adjusted = moves;

	if (state_ptr->is_inverted())
		std::ranges::transform(moves_adjusted, moves_adjusted.begin(), [](const auto& m) { return m.get_inverted(); });

	std::ranges::transform(moves_adjusted, moves_dto.begin(), [](auto& m)
		{

			return MoveDto{ m.sub_moves.data(),
				static_cast<int>(m.sub_moves.size()),
				m.final_rank };
		});

	get_moves_callback(static_cast<int>(moves_dto.size()), moves_dto.data());

	return state_ptr->current_state_seed().state_type();
}
#pragma endregion IState