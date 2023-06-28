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

#include "../../Headers/Checkers/TdLambdaSubAgent.h"
#include "../../Headers/Checkers/TdLambdaAgent.h"
#include "../../../DeepLearning/DeepLearning/Utilities.h"
#include <cmath>

namespace TrainingCell::Checkers
{
	MoveData TdLambdaSubAgent::pick_move(const State& state, const std::vector<Move>& moves,
		const ITdlSettingsReadOnly& settings, const DeepLearning::Net<DeepLearning::CpuDC>& net) const
	{
		if (moves.empty())
			return { -1 };

		if (settings.get_training_mode(_is_white) &&
			DeepLearning::Utils::get_random(0, 1.0) <= settings.get_exploratory_probability())
			return evaluate(state, moves, DeepLearning::Utils::get_random_int(0, static_cast<int>(moves.size()) - 1), net);

		return pick_move(state, moves, net);
	}

	MoveData TdLambdaSubAgent::pick_move(const State& state, const std::vector<Move>& moves,
		const DeepLearning::Net<DeepLearning::CpuDC>& net)
	{
		MoveData best_move_data{ -1, -std::numeric_limits<double>::max() };

		for (auto move_id = 0ull; move_id < moves.size(); ++move_id)
		{
			const auto trial_move_data = evaluate(state, moves, static_cast<int>(move_id), net);

			if (trial_move_data.value > best_move_data.value)
				best_move_data = trial_move_data;
		}

		if (best_move_data.move_id < 0)
			throw std::exception("Neural network is NaN. Try decreasing learning rate parameter.");

		return best_move_data;
	}

	MoveData TdLambdaSubAgent::evaluate(const State& state, const std::vector<Move>& moves,
		const int move_id, const DeepLearning::Net<DeepLearning::CpuDC>& net)
	{
		auto afterstate = state;
		afterstate.make_move(moves[move_id], true, false);
		const auto value = net.act(afterstate.to_tensor())(0, 0, 0);
		return { move_id,  value, afterstate };
	}

	double TdLambdaSubAgent::update_z_and_evaluate_prev_after_state(const ITdlSettingsReadOnly& settings, DeepLearning::Net<DeepLearning::CpuDC>& net)
	{
		const auto lambda_times_gamma = settings.get_lambda() * settings.get_discount();
		const auto long_evaluation = lambda_times_gamma > 0 && !_z.empty();

		auto& gradient_alias = long_evaluation ? _gradient_cache : _z;

		net.calc_gradient_and_value(_prev_afterstate.to_tensor(),
			_value_cache, DeepLearning::CostFunctionId::LINEAR,
			gradient_alias, _value_cache, _context);

		if (long_evaluation)
		{
			for (auto layer_id = 0ull; layer_id < gradient_alias.size(); ++layer_id)
				_z[layer_id].scale_and_add(lambda_times_gamma, gradient_alias[layer_id]);
		}

		return _value_cache(0, 0, 0);
	}

	void TdLambdaSubAgent::reset()
	{
		_new_game = true;
		_move_counter = 0;
		_z.clear();
	}

	TdLambdaSubAgent::TdLambdaSubAgent(const bool is_white) : _is_white(is_white)
	{}

	int TdLambdaSubAgent::make_move(const State& current_state, const std::vector<Move>& moves,
	                                const ITdlSettingsReadOnly& settings, DeepLearning::Net<DeepLearning::CpuDC>& net)
	{
		const auto move_data = pick_move(current_state, moves, settings, net);
		return make_move(current_state, move_data, settings, net);
	}

	int TdLambdaSubAgent::make_move(const State& current_state, 
		const MoveData& move_data,
		const ITdlSettingsReadOnly& settings,
		DeepLearning::Net<DeepLearning::CpuDC>& net)
	{
		_move_counter++;

		if (!settings.get_training_mode(_is_white) || settings.get_train_depth() < _move_counter)
			return move_data.move_id;

		if (_new_game)
		{
			_prev_afterstate = move_data.after_state;
			_prev_state = current_state;
			_new_game = false;
			return move_data.move_id;
		}

		const auto reward = settings.get_reward_factor() <= 0.0 ? 0.0 : settings.get_reward_factor() *
			Utils::calculate_reward(_prev_state, current_state);

		const auto prev_afterstate_value = update_z_and_evaluate_prev_after_state(settings, net);
		const auto delta = reward + settings.get_discount() * move_data.value - prev_afterstate_value;

		net.update(_z, -settings.get_learning_rate() * delta, 0.0);

		_prev_afterstate = move_data.after_state;
		_prev_state = current_state;

		return move_data.move_id;
	}

	void TdLambdaSubAgent::game_over(const State& final_state, const GameResult& result, 
		const ITdlSettingsReadOnly& settings, DeepLearning::Net<DeepLearning::CpuDC>& net)
	{
		if (settings.get_training_mode(_is_white))
		{
			const auto moves_to_discount = _move_counter - settings.get_train_depth();
			const auto discount_factor = moves_to_discount <= 0 ? 1.0 : pow(settings.get_discount(), moves_to_discount);

			const auto reward = 2 * static_cast<int>(result) * discount_factor;
			const auto delta = reward - update_z_and_evaluate_prev_after_state(settings, net);
			net.update(_z, -settings.get_learning_rate() * delta, 0.0);
		}

		reset();
	}

	int TdLambdaSubAgent::pick_move_id(const State& state, const std::vector<Move>& moves,
		const ITdlSettingsReadOnly& settings, const DeepLearning::Net<DeepLearning::CpuDC>& net) const
	{
		return pick_move(state, moves, settings, net).move_id;
	}

	bool TdLambdaSubAgent::equal(const TdLambdaSubAgent& another_sub_agent) const
	{
		return _z == another_sub_agent._z &&
			_prev_state == another_sub_agent._prev_state &&
			_prev_afterstate == another_sub_agent._prev_afterstate;
	}
}
