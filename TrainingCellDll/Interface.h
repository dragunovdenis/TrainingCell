#pragma once
#include "../TrainingCell/Headers/Checkers/Board.h"

#ifdef TRAINING_CELL_EXPORTS
#define TRAINING_CELL_API __declspec(dllexport)
#else
#define TRAINING_CELL_API __declspec(dllexport)
#endif

extern "C"
{
	/// <summary>
	/// Run training for the given number of episodes
	/// </summary>
	TRAINING_CELL_API void RunCheckersTraining(
		TrainingCell::Checkers::Agent* agent1,
		TrainingCell::Checkers::Agent* agent2,
		int episodes,  TrainingCell::Checkers::PublishCheckersStateCallBack publishStateCallBack,
		TrainingCell::Checkers::PublishTrainingStatsCallBack publishStatsCallBack, TrainingCell::Checkers::CancelCallBack cancellationCallBack);

	/// <summary>
	/// Constructs a random checkers agent on the heap and returns pointer to it
	/// </summary>
	TRAINING_CELL_API void* ConstructCheckersRandomAgent();

	/// <summary>
	/// Disposes the agent pointed by the given pointer
	/// </summary>
	TRAINING_CELL_API bool FreeCheckersRandomAgent(const TrainingCell::Checkers::RandomAgent* agent_ptr);

	/// <summary>
	/// Constructs a TD(lambda) checkers agent on the heap and returns pointer to it
	/// </summary>
	/// <param name="layer_dims">Dimensions of layers (1 + number of actual neural layers,
	/// because input as such is also considered as a layer)</param>
	/// <param name="dims_count">Number of elements in "layer_dims" array</param>
	/// <param name="exploration_epsilon">Probability of making exploration move</param>
	/// <param name="lambda">The corresponding parameter of TD(lambda) method</param>
	/// <param name="gamma">Discount coefficient (used to evaluate value of a state)</param>
	/// <param name="alpha">Step of the gradient method (learning rate)</param>
	TRAINING_CELL_API void* ConstructCheckersTdLambdaAgent(const unsigned int* layer_dims,
		const int dims_count, const double exploration_epsilon, const double lambda, const double gamma, const double alpha);

	/// <summary>
	/// Updates exploration probability parameter of TD(lambda) agent represented with its pointer
	///	Returns "true" if succeeded
	/// </summary>
	TRAINING_CELL_API bool CheckersTdLambdaAgentSetEpsilon(TrainingCell::Checkers::TdLambdaAgent* agent_ptr, const double exploration_epsilon);

	/// <summary>
	/// Returns exploration probability of TD(lambda) agent represented with its pointer
	/// </summary>
	TRAINING_CELL_API double CheckersTdLambdaAgentGetEpsilon(const TrainingCell::Checkers::TdLambdaAgent* agent_ptr);

	/// <summary>
	/// Updates "lambda" parameter of TD(lambda) agent represented with its pointer
	///	Returns "true" if succeeded
	/// </summary>
	TRAINING_CELL_API bool CheckersTdLambdaAgentSetLambda(TrainingCell::Checkers::TdLambdaAgent* agent_ptr, const double lambda);

	/// <summary>
	/// Returns "lambda" parameter of TD(lambda) agent represented with its pointer
	/// </summary>
	TRAINING_CELL_API double CheckersTdLambdaAgentGetLambda(const TrainingCell::Checkers::TdLambdaAgent* agent_ptr);

	/// <summary>
	/// Updates "gamma" (reward discount) parameter of TD(lambda) agent represented with its pointer
	///	Returns "true" if succeeded
	/// </summary>
	TRAINING_CELL_API bool CheckersTdLambdaAgentSetGamma(TrainingCell::Checkers::TdLambdaAgent* agent_ptr, const double gamma);

	/// <summary>
	/// Returns "gamma" (reward discount) parameter of TD(lambda) agent represented with its pointer
	/// </summary>
	TRAINING_CELL_API double CheckersTdLambdaAgentGetGamma(const TrainingCell::Checkers::TdLambdaAgent* agent_ptr);

	/// <summary>
	/// Updates learning rate parameter of TD(lambda) agent represented with its pointer
	///	Returns "true" if succeeded
	/// </summary>
	TRAINING_CELL_API bool CheckersTdLambdaAgentSetLearningRate(TrainingCell::Checkers::TdLambdaAgent* agent_ptr, const double alpha);

	/// <summary>
	/// Returns learning parameter of TD(lambda) agent represented with its pointer
	/// </summary>
	TRAINING_CELL_API double CheckersTdLambdaAgentGetLearningRate(const TrainingCell::Checkers::TdLambdaAgent* agent_ptr);

	/// <summary>
	/// Updates training mode parameter of TD(lambda) agent represented with its pointer
	///	Returns "true" if succeeded
	/// </summary>
	TRAINING_CELL_API bool CheckersTdLambdaAgentSetTrainingMode(TrainingCell::Checkers::TdLambdaAgent* agent_ptr, const bool training_mode);

	/// <summary>
	/// Returns training mode parameter of TD(lambda) agent represented with its pointer
	/// </summary>
	TRAINING_CELL_API char CheckersTdLambdaAgentGetTrainingMode(const TrainingCell::Checkers::TdLambdaAgent* agent_ptr);

	/// <summary>
	/// Tries to load TD(lambda) agent from the given file on disk and returns pointer to it in case of success
	///	(otherwise null pointer is returned)
	/// </summary>
	TRAINING_CELL_API void* CheckersTdLambdaAgentLoadFromFile(const char* path);

	/// <summary>
	/// Returns "true" if the two agents represented with their pointers are equal
	/// </summary>
	TRAINING_CELL_API bool CheckersTdLambdaAgentEqual(const TrainingCell::Checkers::TdLambdaAgent* agent_ptr1, const TrainingCell::Checkers::TdLambdaAgent* agent_ptr2);

	/// <summary>
	/// Tries to save the given TD(lambda) agent represented with its pointer to the given file on disk
	///	Returns "true" if succeeded
	/// </summary>
	TRAINING_CELL_API bool CheckersTdLambdaAgentSaveToFile(const TrainingCell::Checkers::TdLambdaAgent* agent_ptr, const char* path);

	/// <summary>
	/// Disposes the agent pointed by the given pointer
	/// </summary>
	TRAINING_CELL_API bool FreeCheckersTdLambdaAgent(const TrainingCell::Checkers::TdLambdaAgent* agent_ptr);

	/// <summary>
	///	Data transferring object to pass checker moves
	/// </summary>
	struct CheckersMoveDto
	{
		/// <summary>
		///	Pointer to the array of sub_moves
		/// </summary>
		TrainingCell::Checkers::SubMove* sub_moves{};

		/// <summary>
		///	Number of sub-moves in the array
		/// </summary>
		int sub_moves_cnt{};
	};

	/// <summary>
	///	"Make move" delegate
	/// </summary>
	typedef int (*CheckersMakeMoveCallBack)(const int* state, const int state_size, const CheckersMoveDto* moves, const int moves_size);

	/// <summary>
	///	"Game over" delegate
	/// </summary>
	typedef void (*CheckersGameOverCallBack)(const int* state, const int state_size, const int game_result);

	/// <summary>
	/// Constructs "interactive" checkers agent on the heap and returns pointer to it
	/// </summary>
	TRAINING_CELL_API void* ConstructCheckersInteractiveAgent(const CheckersMakeMoveCallBack make_move_callback, const CheckersGameOverCallBack game_over_callback, const bool _play_for_whites);

	/// <summary>
	/// Disposes the agent pointed by the given pointer
	/// </summary>
	TRAINING_CELL_API bool FreeCheckersInteractiveAgent(const TrainingCell::Checkers::InteractiveAgent* agent_ptr);
}
