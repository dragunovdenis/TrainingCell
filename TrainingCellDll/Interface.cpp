
#include "Interface.h"
void RunCheckersTraining(int epochs, TrainingCell::Checkers::PublishCheckersStateCallBack publishStateCallBack,
	TrainingCell::Checkers::PublishTrainingStatsCallBack publishStatsCallBack)
{
	TrainingCell::Checkers::RandomAgent agent0{};
	//TrainingCell::Checkers::RandomAgent agent1{};
	TrainingCell::Checkers::TdLambdaAgent agent1({ 32, 64, 128, 128, 1 }, 0.1);
	TrainingCell::Checkers::Board board(&agent0, &agent1);

	board.play(epochs, 200, publishStateCallBack, publishStatsCallBack);
}
