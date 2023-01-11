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
	/// Run training for the given number of epochs
	/// </summary>
	TRAINING_CELL_API void RunCheckersTraining(int epochs, TrainingCell::Checkers::PublishCheckersStateCallBack publishStateCallBack,
		TrainingCell::Checkers::PublishTrainingStatsCallBack publishStatsCallBack);
}
