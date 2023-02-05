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
#include "../TrainingCell/Headers/Checkers/AgentPack.h"

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

void RunCheckersTraining(TrainingCell::Checkers::Agent* const agent1,
	TrainingCell::Checkers::Agent* const agent2,
	int episodes, TrainingCell::Checkers::PublishCheckersStateCallBack publishStateCallBack,
	TrainingCell::Checkers::PublishTrainingStatsCallBack publishStatsCallBack, TrainingCell::Checkers::CancelCallBack cancellationCallBack)
{
	TrainingCell::Checkers::Board board(agent1, agent2);
	board.play(episodes, 200, publishStateCallBack, publishStatsCallBack, cancellationCallBack);
}
#pragma region Random Agent
void* ConstructCheckersRandomAgent()
{
	return new TrainingCell::Checkers::RandomAgent();
}

bool FreeCheckersRandomAgent(const TrainingCell::Checkers::RandomAgent* agent_ptr)
{
	if (agent_ptr == nullptr)
		return false;

	delete agent_ptr;
	return true;
}
#pragma endregion Random Agent
#pragma region Td(Lammbda)-Agent
void* ConstructCheckersTdLambdaAgent(const unsigned int* layer_dims,
	const int dims_count, const double exploration_epsilon, const double lambda, const double gamma, const double alpha)
{
	return new TrainingCell::Checkers::TdLambdaAgent(convert(layer_dims, dims_count), exploration_epsilon, lambda, gamma, alpha);
}

bool CheckersTdLambdaAgentSetEpsilon(TrainingCell::Checkers::TdLambdaAgent* agent_ptr, const double exploration_epsilon)
{
	if (agent_ptr == nullptr)
		return false;

	agent_ptr->set_exploration_probability(exploration_epsilon);
	return true;
}

double CheckersTdLambdaAgentGetEpsilon(const TrainingCell::Checkers::TdLambdaAgent* agent_ptr)
{
	if (agent_ptr == nullptr)
		return std::numeric_limits<double>::quiet_NaN();

	return agent_ptr->get_exploratory_probability();
}

bool CheckersTdLambdaAgentSetLambda(TrainingCell::Checkers::TdLambdaAgent* agent_ptr, const double lambda)
{
	if (agent_ptr == nullptr)
		return false;

	agent_ptr->set_lambda(lambda);
	return true;
}

double CheckersTdLambdaAgentGetLambda(const TrainingCell::Checkers::TdLambdaAgent* agent_ptr)
{
	if (agent_ptr == nullptr)
		return std::numeric_limits<double>::quiet_NaN();

	return agent_ptr->get_lambda();
}

bool CheckersTdLambdaAgentSetGamma(TrainingCell::Checkers::TdLambdaAgent* agent_ptr, const double gamma)
{
	if (agent_ptr == nullptr)
		return false;

	agent_ptr->set_discount(gamma);
	return true;
}

double CheckersTdLambdaAgentGetGamma(const TrainingCell::Checkers::TdLambdaAgent* agent_ptr)
{
	if (agent_ptr == nullptr)
		return std::numeric_limits<double>::quiet_NaN();

	return agent_ptr->get_discount();
}

bool CheckersTdLambdaAgentSetLearningRate(TrainingCell::Checkers::TdLambdaAgent* agent_ptr, const double alpha)
{
	if (agent_ptr == nullptr)
		return false;

	agent_ptr->set_learning_rate(alpha);
	return true;
}

double CheckersTdLambdaAgentGetLearningRate(const TrainingCell::Checkers::TdLambdaAgent* agent_ptr)
{
	if (agent_ptr == nullptr)
		return std::numeric_limits<double>::quiet_NaN();

	return agent_ptr->get_learning_rate();
}

bool CheckersTdLambdaAgentGetNetDimensions(const TrainingCell::Checkers::TdLambdaAgent* agent_ptr, const GetArrayCallBack acquireDimensionsCallBack)
{
	if (agent_ptr == nullptr || acquireDimensionsCallBack == nullptr)
		return false;

	const auto dims = agent_ptr->get_net_dimensions();
	acquireDimensionsCallBack(static_cast<int>(dims.size()), dims.data());

	return true;
}


void* CheckersTdLambdaAgentLoadFromFile(const char* path)
{
	try
	{
		return new TrainingCell::Checkers::TdLambdaAgent(TrainingCell::Checkers::TdLambdaAgent::load_from_file(path));
	} catch (...)
	{
		return nullptr;
	}
}

bool CheckersTdLambdaAgentSaveToFile(const TrainingCell::Checkers::TdLambdaAgent* agent_ptr, const char* path)
{
	if (agent_ptr == nullptr)
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

bool CheckersTdLambdaAgentEqual(const TrainingCell::Checkers::TdLambdaAgent* agent_ptr1, const TrainingCell::Checkers::TdLambdaAgent* agent_ptr2)
{
	if (agent_ptr1 == nullptr || agent_ptr2 == nullptr)
		return false;

	return *agent_ptr1 == *agent_ptr2;
}

bool FreeCheckersTdLambdaAgent(const TrainingCell::Checkers::TdLambdaAgent* agent_ptr)
{
	if (agent_ptr == nullptr)
		return false;

	delete agent_ptr;
	return true;
}

void* PackCheckersTdLambdaAgent(const TrainingCell::Checkers::TdLambdaAgent* agent_ptr)
{
	if (agent_ptr == nullptr)
		return nullptr;

	try
	{
		return new TrainingCell::Checkers::AgentPack(TrainingCell::Checkers::AgentPack::make<TrainingCell::Checkers::TdLambdaAgent>(*agent_ptr));
	}
	catch (...)
	{
		return nullptr;
	}
}

#pragma endregion Td(Lammbda)-Agent
#pragma region Interactive Agent
void* ConstructCheckersInteractiveAgent(const CheckersMakeMoveCallBack make_move_callback, const CheckersGameOverCallBack game_over_callback, const bool _play_for_whites)
{
	try
	{
		if (make_move_callback == nullptr || game_over_callback == nullptr)
			return nullptr;

		return new TrainingCell::Checkers::InteractiveAgent(
			[=](const TrainingCell::Checkers::State& state, std::vector<TrainingCell::Checkers::Move> moves)
			{
				std::vector<CheckersMoveDto> moves_dto;
				for (auto& move : moves)
					moves_dto.emplace_back(move.sub_moves.data(), static_cast<int>(move.sub_moves.size()));

				return make_move_callback(reinterpret_cast<const int*>(state.data()), static_cast<int>(state.size()), moves_dto.data(), static_cast<int>(moves_dto.size()));
			},
			[=](const TrainingCell::Checkers::State& state, const TrainingCell::Checkers::GameResult& result)
			{
				game_over_callback(reinterpret_cast<const int*>(state.data()), static_cast<int>(state.size()), static_cast<int>(result));
				
			}, _play_for_whites);
	}
	catch (...)
	{
		return nullptr;
	}
}

bool FreeCheckersInteractiveAgent(const TrainingCell::Checkers::InteractiveAgent* agent_ptr)
{
	if (agent_ptr == nullptr)
		return false;

	delete agent_ptr;
	return true;
}
#pragma endregion Interactive Agent
#pragma region Agent
bool CheckersAgentSetTrainingMode(TrainingCell::Checkers::Agent* agent_ptr, const bool training_mode)
{
	if (agent_ptr == nullptr)
		return false;

	agent_ptr->set_training_mode(training_mode);
	return true;
}

char CheckersAgentGetTrainingMode(const TrainingCell::Checkers::Agent* agent_ptr)
{
	if (agent_ptr == nullptr)
		return static_cast<char>(2);

	return agent_ptr->get_training_mode();
}

char CheckersAgentGetCanTrainFlag(const TrainingCell::Checkers::Agent* agent_ptr)
{
	if (agent_ptr == nullptr)
		return static_cast<char>(2);

	return agent_ptr->can_train();
}

const char* CheckersAgentGetId(const TrainingCell::Checkers::Agent* agent_ptr)
{
	if (agent_ptr == nullptr)
		return nullptr;

	return agent_ptr->get_id().c_str();
}

bool CheckersAgentSetId(TrainingCell::Checkers::Agent* agent_ptr, const char* id)
{
	if (agent_ptr == nullptr)
		return false;

	agent_ptr->set_id(id);

	return true;
}

#pragma endregion Agent
#pragma region AgentPack
void* CheckersAgentPackLoadFromFile(const char* path)
{
	try
	{
		return new TrainingCell::Checkers::AgentPack(TrainingCell::Checkers::AgentPack::load_from_file(path));
	}
	catch (...)
	{
		return nullptr;
	}
}

bool CheckersAgentPackSaveToFile(const TrainingCell::Checkers::AgentPack* agent_pack_ptr, const char* path)
{
	if (agent_pack_ptr == nullptr)
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

bool CheckersAgentPackFree(const TrainingCell::Checkers::AgentPack* agent_pack_ptr)
{
	if (agent_pack_ptr == nullptr)
		return false;

	delete agent_pack_ptr;
	return true;
}

void* CheckersAgentPackGetAgentPtr(TrainingCell::Checkers::AgentPack* agent_pack_ptr)
{
	if (agent_pack_ptr == nullptr)
		return nullptr;

	return  &(agent_pack_ptr->agent());
}
#pragma endregion AgentPack

