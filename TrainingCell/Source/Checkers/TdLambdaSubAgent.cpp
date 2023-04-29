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
#include "../../Headers/Checkers/TdLambdaAutoAgent.h"
#include "../../../DeepLearning/DeepLearning/Utilities.h"

namespace TrainingCell::Checkers
{
	template <bool WHITE>
	int TdLambdaSubAgent<WHITE>::pick_move_id(const State& state, const std::vector<Move>& moves) const
	{
		return pick_move(state, moves).move_id;
	}

	template <bool WHITE>
	MoveData TdLambdaSubAgent<WHITE>::pick_move(const State& state, const std::vector<Move>& moves) const
	{
		if (moves.empty())
			return { -1 };

		if (_settings_ptr->get_training_mode() &&
			DeepLearning::Utils::get_random(0, 1.0) <= _settings_ptr->get_exploratory_probability())
			return evaluate(state, moves, DeepLearning::Utils::get_random_int(0, static_cast<int>(moves.size()) - 1));

		MoveData best_move_data{ -1, -std::numeric_limits<double>::max() * color_factor() };

		for (auto move_id = 0ull; move_id < moves.size(); ++move_id)
		{
			const auto trial_move_data = evaluate(state, moves, static_cast<int>(move_id));

			if (color_factor() * trial_move_data.value > color_factor() * best_move_data.value)
				best_move_data = trial_move_data;
		}

		if (best_move_data.move_id < 0)
			throw std::exception("Neural network is NaN. Try decreasing learning rate parameter.");

		return best_move_data;
	}

	template <bool WHITE>
	MoveData TdLambdaSubAgent<WHITE>::evaluate(const State& state, const std::vector<Move>& moves,
		const int move_id) const
	{
		auto afterstate = state;
		afterstate.make_move(moves[move_id], true, false);
		const auto value = _func_ptr->net().act(
			(WHITE ? afterstate : afterstate.get_inverted()).to_tensor())(0, 0, 0);
		return { move_id,  value, afterstate };
	}

	template <bool WHITE>
	double TdLambdaSubAgent<WHITE>::update_z_and_evaluate_prev_after_state()
	{
		auto calc_result = _func_ptr->net().calc_gradient_and_value(
			(WHITE ? _prev_afterstate : _prev_afterstate.get_inverted()).to_tensor(),
			DeepLearning::Tensor(1, 1, 1, false), DeepLearning::CostFunctionId::LINEAR);

		auto& gradient = std::get<0>(calc_result);

		if (_z.empty())
		{
			_z = std::move(gradient);
		}
		else
		{
			if (gradient.size() != _z.size())
				throw std::exception("Incompatible data");

			const auto lambda_times_gamma = _settings_ptr->get_lambda() * _settings_ptr->get_discount();

			for (auto layer_id = 0ull; layer_id < gradient.size(); ++layer_id)
			{
				gradient[layer_id] += _z[layer_id] * lambda_times_gamma;
				_z[layer_id] = std::move(gradient[layer_id]);
			}
		}

		return std::get<1>(calc_result)(0, 0, 0);
	}

	template <bool WHITE>
	void TdLambdaSubAgent<WHITE>::reset()
	{
		_new_game = true;
		_z.clear();
	}

	template <bool WHITE>
	TdLambdaSubAgent<WHITE>::TdLambdaSubAgent(const TdlSettingsReadOnly* settings_ptr,
		AfterStateValueFunction* const func_ptr) : _settings_ptr(settings_ptr), _func_ptr(func_ptr)
	{}

	template <bool WHITE>
	int TdLambdaSubAgent<WHITE>::make_move(const State& current_state, const std::vector<Move>& moves)
	{
		//Sanity check
		if (WHITE == current_state.is_inverted())
			throw std::exception("Unexpected state");

		if (!_settings_ptr->get_training_mode())
			return pick_move_id(current_state, moves);

		const auto move_data = pick_move(current_state, moves);

		if (_new_game)
		{
			_prev_afterstate = move_data.after_state;
			_prev_state = current_state;
			_new_game = false;
			return move_data.move_id;
		}

		const auto reward = _settings_ptr->get_reward_factor() *
			Utils::calculate_reward(_prev_state, current_state);

		const auto prev_afterstate_value = update_z_and_evaluate_prev_after_state();
		const auto delta = reward + _settings_ptr->get_discount() * move_data.value - prev_afterstate_value;

		_func_ptr->net().update(_z, -_settings_ptr->get_learning_rate() * delta, 0.0);

		_prev_afterstate = move_data.after_state;
		_prev_state = current_state;

		return move_data.move_id;
	}

	template <bool WHITE>
	void TdLambdaSubAgent<WHITE>::game_over(const State& final_state, const GameResult& result)
	{
		if (_settings_ptr->get_training_mode())
		{
			const auto reward = 2 * static_cast<int>(result) * color_factor();
			const auto delta = reward - update_z_and_evaluate_prev_after_state();
			_func_ptr->net().update(_z, -_settings_ptr->get_learning_rate() * delta, 0.0);
		}

		reset();
	}

	template <bool WHITE>
	constexpr int TdLambdaSubAgent<WHITE>::color_factor()
	{
		return WHITE ? 1 : -1;
	}

	template class TdLambdaSubAgent<true>;
	template class TdLambdaSubAgent<false>;
}
