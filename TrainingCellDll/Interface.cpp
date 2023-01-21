#include "Interface.h"

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

bool CheckersTdLambdaAgentSetTrainingMode(TrainingCell::Checkers::TdLambdaAgent* agent_ptr, const bool training_mode)
{
	if (agent_ptr == nullptr)
		return false;

	agent_ptr->set_training_mode(training_mode);
	return true;
}

char CheckersTdLambdaAgentGetTrainingMode(const TrainingCell::Checkers::TdLambdaAgent* agent_ptr)
{
	if (agent_ptr == nullptr)
		return static_cast<char>(2);

	return static_cast<char>(agent_ptr->get_training_mode());
}

void* CheckersTdLambdaAgentLoadFromFile(const char* path)
{
	const auto agent_ptr = new TrainingCell::Checkers::TdLambdaAgent();
	try
	{
		*agent_ptr = TrainingCell::Checkers::TdLambdaAgent::load_from_file(path);
	} catch (...)
	{
		delete agent_ptr;
		return nullptr;
	}

	return agent_ptr;
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
