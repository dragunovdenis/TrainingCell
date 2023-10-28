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

void RunCheckersTraining(TrainingCell::Agent* const agent1,
	TrainingCell::Agent* const agent2,
	int episodes, TrainingCell::PublishCheckersStateCallBack publishStateCallBack,
	TrainingCell::PublishTrainingStatsCallBack publishStatsCallBack,
	TrainingCell::CancelCallBack cancellationCallBack,
	TrainingCell::ErrorMessageCallBack errorCallBack)
{
	TrainingCell::Board board(agent1, agent2);
	board.play(episodes, TrainingCell::Checkers::CheckersState::get_start_state(),
		200, publishStateCallBack, publishStatsCallBack,
		cancellationCallBack, errorCallBack);
}
#pragma region Random Agent
void* ConstructCheckersRandomAgent()
{
	return new TrainingCell::RandomAgent();
}

bool FreeCheckersRandomAgent(const TrainingCell::RandomAgent* agent_ptr)
{
	if (!agent_ptr)
		return false;

	delete agent_ptr;
	return true;
}
#pragma endregion Random Agent
#pragma region Td(Lammbda)-Agent
void* ConstructCheckersTdLambdaAgent(const unsigned int* layer_dims,
	const int dims_count, const double exploration_epsilon, const double lambda, const double gamma, const double alpha)
{
	return new TrainingCell::TdLambdaAgent(convert(layer_dims, dims_count), exploration_epsilon, lambda, gamma, alpha);
}

void* CheckersTdLambdaAgentCreateCopy(const TrainingCell::TdLambdaAgent* agent_ptr)
{
	if (!agent_ptr)
		return nullptr;

	return new TrainingCell::TdLambdaAgent(*agent_ptr);
}

bool CheckersTdLambdaAgentsAreEqual(const TrainingCell::TdLambdaAgent* agent0_ptr,
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

bool CheckersTdLambdaAgentSetEpsilon(TrainingCell::TdLambdaAgent* agent_ptr, const double exploration_epsilon)
{
	if (!agent_ptr)
		return false;

	agent_ptr->set_exploration_probability(exploration_epsilon);
	return true;
}

double CheckersTdLambdaAgentGetEpsilon(const TrainingCell::TdLambdaAgent* agent_ptr)
{
	if (!agent_ptr)
		return std::numeric_limits<double>::quiet_NaN();

	return agent_ptr->get_exploratory_probability();
}

bool CheckersTdLambdaAgentSetLambda(TrainingCell::TdLambdaAgent* agent_ptr, const double lambda)
{
	if (!agent_ptr)
		return false;

	agent_ptr->set_lambda(lambda);
	return true;
}

double CheckersTdLambdaAgentGetLambda(const TrainingCell::TdLambdaAgent* agent_ptr)
{
	if (!agent_ptr)
		return std::numeric_limits<double>::quiet_NaN();

	return agent_ptr->get_lambda();
}

bool CheckersTdLambdaAgentSetGamma(TrainingCell::TdLambdaAgent* agent_ptr, const double gamma)
{
	if (!agent_ptr)
		return false;

	agent_ptr->set_discount(gamma);
	return true;
}

double CheckersTdLambdaAgentGetGamma(const TrainingCell::TdLambdaAgent* agent_ptr)
{
	if (!agent_ptr)
		return std::numeric_limits<double>::quiet_NaN();

	return agent_ptr->get_discount();
}

bool CheckersTdLambdaAgentSetLearningRate(TrainingCell::TdLambdaAgent* agent_ptr, const double alpha)
{
	if (!agent_ptr)
		return false;

	agent_ptr->set_learning_rate(alpha);
	return true;
}

double CheckersTdLambdaAgentGetLearningRate(const TrainingCell::TdLambdaAgent* agent_ptr)
{
	if (!agent_ptr)
		return std::numeric_limits<double>::quiet_NaN();

	return agent_ptr->get_learning_rate();
}

bool CheckersTdLambdaAgentGetNetDimensions(const TrainingCell::TdLambdaAgent* agent_ptr, const GetArrayCallBack acquireDimensionsCallBack)
{
	if (!agent_ptr || !acquireDimensionsCallBack)
		return false;

	const auto dims = agent_ptr->get_net_dimensions();
	acquireDimensionsCallBack(static_cast<int>(dims.size()), dims.data());

	return true;
}


void* CheckersTdLambdaAgentLoadFromFile(const char* path)
{
	try
	{
		return new TrainingCell::TdLambdaAgent(TrainingCell::TdLambdaAgent::load_from_file(path));
	} catch (...)
	{
		return nullptr;
	}
}

bool CheckersTdLambdaAgentSaveToFile(const TrainingCell::TdLambdaAgent* agent_ptr, const char* path)
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

bool CheckersTdLambdaAgentEqual(const TrainingCell::TdLambdaAgent* agent_ptr1, const TrainingCell::TdLambdaAgent* agent_ptr2)
{
	if (!agent_ptr1 || !agent_ptr2)
		return false;

	return *agent_ptr1 == *agent_ptr2;
}

bool FreeCheckersTdLambdaAgent(const TrainingCell::TdLambdaAgent* agent_ptr)
{
	if (!agent_ptr)
		return false;

	delete agent_ptr;
	return true;
}

void* PackCheckersTdLambdaAgent(const TrainingCell::TdLambdaAgent* agent_ptr)
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

char CheckersTdLambdaAgentGetSearchMode(const TrainingCell::TdLambdaAgent* agent_ptr)
{
	if (!agent_ptr)
		return static_cast<char>(2);

	return static_cast<char>(agent_ptr->get_tree_search_method() != TrainingCell::TreeSearchMethod::NONE);
}

bool CheckersTdLambdaAgentSetSearchMode(TrainingCell::TdLambdaAgent* agent_ptr, const bool search_mode)
{
	if (!agent_ptr)
		return false;

	//Currently dll interface allows to use only TD-tree search (TDTS)
	agent_ptr->set_tree_search_method(search_mode ?
		TrainingCell::TreeSearchMethod::TD_SEARCH :
		TrainingCell::TreeSearchMethod::NONE);

	return true;
}

int CheckersTdLambdaAgentGetSearchModeIterations(const TrainingCell::TdLambdaAgent* agent_ptr)
{
	if (!agent_ptr)
		return -1;

	return agent_ptr->get_td_search_iterations();
}

bool CheckersTdLambdaAgentSetSearchModeIterations(TrainingCell::TdLambdaAgent* agent_ptr, const int search_iterations)
{
	if (!agent_ptr)
		return false;

	agent_ptr->set_td_search_iterations(search_iterations);

	return true;
}

#pragma endregion Td(Lammbda)-Agent
#pragma region Interactive Agent
void* ConstructCheckersInteractiveAgent(const CheckersMakeMoveCallBack make_move_callback,
	const CheckersGameOverCallBack game_over_callback, const bool play_for_whites)
{
	try
	{
		if (!make_move_callback || !game_over_callback)
			return nullptr;

		return new TrainingCell::InteractiveAgent(
			[=](const std::vector<int>& state, std::vector<TrainingCell::Move> moves)
			{
				std::vector<CheckersMoveDto> moves_dto;
				moves_dto.reserve(moves.size());
				for (auto& move : moves)
					moves_dto.push_back(CheckersMoveDto{ move.sub_moves.data(), static_cast<int>(move.sub_moves.size()) });

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

bool FreeCheckersInteractiveAgent(const TrainingCell::InteractiveAgent* agent_ptr)
{
	if (!agent_ptr)
		return false;

	delete agent_ptr;
	return true;
}
#pragma endregion Interactive Agent
#pragma region Agent
bool CheckersAgentSetTrainingMode(TrainingCell::Agent* agent_ptr, const bool training_mode)
{
	if (!agent_ptr)
		return false;

	agent_ptr->set_training_mode(training_mode);
	return true;
}

char CheckersAgentGetTrainingMode(const TrainingCell::Agent* agent_ptr)
{
	if (!agent_ptr)
		return static_cast<char>(2);

	return static_cast<char>(agent_ptr->get_training_mode());
}

char CheckersAgentGetCanTrainFlag(const TrainingCell::Agent* agent_ptr)
{
	if (!agent_ptr)
		return static_cast<char>(2);

	return static_cast<char>(agent_ptr->can_train());
}

const char* CheckersAgentGetName(const TrainingCell::Agent* agent_ptr)
{
	if (!agent_ptr)
		return nullptr;

	return agent_ptr->get_name().c_str();
}

bool CheckersAgentSetName(TrainingCell::Agent* agent_ptr, const char* name)
{
	if (!agent_ptr)
		return false;

	agent_ptr->set_name(name);

	return true;
}

const char* CheckersAgentGetId(const TrainingCell::Agent* agent_ptr)
{
	if (!agent_ptr)
		return nullptr;

	return agent_ptr->get_id().c_str();
}

int CheckersAgentGetRecordsCount(const TrainingCell::Agent* agent_ptr)
{
	if (agent_ptr == nullptr)
		return -1;

	return static_cast<int>(agent_ptr->get_records_count());
}

const char* CheckersAgentGetRecordById(const TrainingCell::Agent* agent_ptr, const int record_id)
{
	if (agent_ptr == nullptr || record_id < 0 || record_id >= agent_ptr->get_records_count())
		return nullptr;

	return agent_ptr->get_record(record_id).c_str();
}

int CheckersAgentAddRecord(TrainingCell::Agent* agent_ptr, const char* record)
{
	if (agent_ptr == nullptr || record == nullptr)
		return -1;

	return static_cast<int>(agent_ptr->add_record(record));
}

#pragma endregion Agent
#pragma region AgentPack
void* CheckersAgentPackLoadFromFile(const char* path)
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

bool CheckersAgentPackSaveToFile(const TrainingCell::AgentPack* agent_pack_ptr, const char* path)
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

bool CheckersAgentPackFree(const TrainingCell::AgentPack* agent_pack_ptr)
{
	if (!agent_pack_ptr)
		return false;

	delete agent_pack_ptr;
	return true;
}

void* CheckersAgentPackGetAgentPtr(TrainingCell::AgentPack* agent_pack_ptr)
{
	if (!agent_pack_ptr)
		return nullptr;

	return  &(agent_pack_ptr->agent());
}
#pragma endregion AgentPack
#pragma region TdlEnsembleAgent
void* ConstructCheckersTdlEnsembleAgent(const int agent_count, const TrainingCell::TdLambdaAgent** agent_collection)
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

bool FreeCheckersTdlEnsembleAgent(const TrainingCell::TdlEnsembleAgent* agent_ptr)
{
	if (!agent_ptr)
		return false;

	delete agent_ptr;
	return true;
}

bool SaveCheckersTdlEnsembleAgent(const TrainingCell::TdlEnsembleAgent* agent_ptr, const char* file_path)
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

void* LoadCheckersTdlEnsembleAgent(const char* file_path)
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

int CheckersTdlEnsembleAgentGetSize(const TrainingCell::TdlEnsembleAgent* agent_ptr)
{
	if (!agent_ptr)
		return -1;

	return static_cast<int>(agent_ptr->size());
}

const char* CheckersTdlEnsembleAgentGetSubAgentId(const TrainingCell::TdlEnsembleAgent* agent_ptr, const int agent_id)
{
	if (!agent_ptr)
		return nullptr;

	return (*agent_ptr)[agent_id].get_name().c_str();
}

int CheckersTdlEnsembleAgentAdd(TrainingCell::TdlEnsembleAgent* ensemble_agent_ptr,
	const TrainingCell::TdLambdaAgent* agent_to_add_ptr)
{
	if (!ensemble_agent_ptr || !agent_to_add_ptr)
		return -1;

	return static_cast<int>(ensemble_agent_ptr->add(*agent_to_add_ptr));
}

bool CheckersTdlEnsembleAgentRemove(TrainingCell::TdlEnsembleAgent* ensemble_agent_ptr,
	const int sub_agent_id)
{
	if (!ensemble_agent_ptr)
		return false;

	return ensemble_agent_ptr->remove_agent(sub_agent_id);
}

int CheckersTdlEnsembleAgentSetSingleAgentMode(TrainingCell::TdlEnsembleAgent* ensemble_agent_ptr,
	const bool set_single_agent_mode)
{
	if (!ensemble_agent_ptr)
		return -1;

	return static_cast<int>(ensemble_agent_ptr->set_single_agent_mode(set_single_agent_mode));
}

int CheckersTdlEnsembleAgentGetSingleAgentId(TrainingCell::TdlEnsembleAgent* ensemble_agent_ptr)
{
	if (!ensemble_agent_ptr)
		return -2;

	return static_cast<int>(ensemble_agent_ptr->get_current_random_agent_id());
}

void* PackCheckersTdlEnsembleAgent(const TrainingCell::TdlEnsembleAgent* agent_ptr)
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

const void* CheckersTdlEnsembleAgentGetSubAgentPtr(const TrainingCell::TdlEnsembleAgent* agent_ptr,
	const int sub_agent_id)
{
	if (agent_ptr == nullptr || sub_agent_id < 0 || sub_agent_id >= agent_ptr->size())
		return nullptr;

	return &(*agent_ptr)[sub_agent_id];
}
#pragma endregion TdlEnsembleAgent


bool CheckersTdLambdaAgentSetRewardFactor(TrainingCell::TdLambdaAgent* agent_ptr, const double reward_factor)
{
	if (!agent_ptr)
		return false;

	agent_ptr->set_reward_factor(reward_factor);

	return true;
}

double CheckersTdLambdaAgentGetRewardFactor(const TrainingCell::TdLambdaAgent* agent_ptr)
{
	if (!agent_ptr)
		return std::numeric_limits<double>::quiet_NaN();

	return agent_ptr->get_reward_factor();
}

bool CheckersTdLambdaAgentSetSearchDepth(TrainingCell::TdLambdaAgent* agent_ptr, const int search_depth)
{
	if (!agent_ptr)
		return false;

	agent_ptr->set_search_depth(search_depth);

	return true;
}

int CheckersTdLambdaAgentGetSearchDepth(const TrainingCell::TdLambdaAgent* agent_ptr)
{
	if (!agent_ptr)
		return -1;

	return agent_ptr->get_search_depth();
}
