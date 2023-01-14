
#include "Interface.h"
void RunCheckersTraining(int epochs, TrainingCell::Checkers::PublishCheckersStateCallBack publishStateCallBack,
	TrainingCell::Checkers::PublishTrainingStatsCallBack publishStatsCallBack)
{
	TrainingCell::Checkers::RandomAgent agent0{};
	TrainingCell::Checkers::TdLambdaAgent agent1({ 32, 64, 32, 16, 8, 1 }, 0.1, 0, 0.8, 0.01);
	TrainingCell::Checkers::Board board(&agent0, &agent1);
	board.play(epochs, 200, publishStateCallBack, publishStatsCallBack);
}
