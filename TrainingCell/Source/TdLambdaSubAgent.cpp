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

#include "../Headers/TdLambdaSubAgent.h"
#include "../../DeepLearning/DeepLearning/Utilities.h"
#include "../Headers/MoveCollector.h"
#include <cmath>

namespace TrainingCell
{
	thread_local DeepLearning::RandomGenerator TdLambdaSubAgent::Explorer::_generator{};
	thread_local DeepLearning::Net<DeepLearning::CpuDC>::Context TdLambdaSubAgent::_context{};
	thread_local DeepLearning::Tensor TdLambdaSubAgent::_tensor_shared{};
	thread_local std::vector<DeepLearning::LayerGradient<DeepLearning::CpuDC>> TdLambdaSubAgent::_gradient_cache{};

	bool TdLambdaSubAgent::Explorer::should_explore(const double exploration_probability)
	{
		return exploration_probability > 0 && (exploration_probability >= 1 || _generator.next() < exploration_probability);
	}

	int TdLambdaSubAgent::Explorer::pick(const int options_count)
	{
		return _generator.get_int(0, options_count);
	}

	void TdLambdaSubAgent::Explorer::reset(const unsigned seed)
	{
		_generator = DeepLearning::RandomGenerator(seed);
	}

	MoveData TdLambdaSubAgent::pick_move(const IMinimalStateReadonly& state,
	                                     const ITdlSettingsReadOnly& settings, const INet& net) const
	{
		if (state.get_moves_count() == 1)
			return evaluate(state, 0, net);

		if (should_do_exploration(settings))
			return explore(state, net, settings.get_exploration_volume());

		return pick_move(state, net);
	}

	bool TdLambdaSubAgent::should_do_exploration(const ITdlSettingsReadOnly& settings) const
	{
		const auto explore_probability = settings.get_exploratory_probability();
		return _move_counter < settings.get_exploration_depth() && settings.get_exploration_volume() > 1 &&
			Explorer::should_explore(explore_probability);
	}

	MoveData TdLambdaSubAgent::explore(const IMinimalStateReadonly& state, const INet& net,
		const int exploration_volume)
	{
		const auto actual_exploration_volume = std::min(exploration_volume, state.get_moves_count());
		const auto picked_move_id = Explorer::pick(actual_exploration_volume);

		if (actual_exploration_volume == state.get_moves_count())
			return evaluate(state, picked_move_id, net);

		MoveCollector collector(actual_exploration_volume);

		const auto actions_count = state.get_moves_count();
		for (auto move_id = 0; move_id < actions_count; ++move_id)
		{
			const auto value = evaluate(state, move_id, net, _tensor_shared, _context);
			collector.add(move_id, value, _tensor_shared);
		}

		return std::move(collector.get(picked_move_id));
	}

	MoveData TdLambdaSubAgent::pick_move(const IMinimalStateReadonly& state, const INet& net)
	{
		MoveData best_move_data{ -1, -std::numeric_limits<double>::max() };

		const auto actions_count = state.get_moves_count();
		for (auto move_id = 0; move_id < actions_count; ++move_id)
		{
			const auto value = evaluate(state, move_id, net, _tensor_shared, _context);

			if (value > best_move_data.value)
			{
				best_move_data.move_id = move_id;
				best_move_data.value = value;
				best_move_data.after_state = _tensor_shared;
			}
		}

		if (best_move_data.move_id < 0)
			throw std::exception("Neural network is NaN. Try decreasing learning rate parameter.");

		return best_move_data;
	}

	MoveData TdLambdaSubAgent::evaluate(const IMinimalStateReadonly& state,
	                                    const int move_id, const INet& net)
	{
		const auto value = evaluate(state, move_id, net, _tensor_shared, _context);
		return { move_id,  value, _tensor_shared };
	}

	double TdLambdaSubAgent::evaluate(const IMinimalStateReadonly& state, const int move_id,
		const INet& net, DeepLearning::CpuDC::tensor_t& afterstate,
		DeepLearning::Net<DeepLearning::CpuDC>::Context& comp_context)
	{
		const auto afterstate_std = state.evaluate(move_id);
		net.evaluate(afterstate_std, afterstate, comp_context);
		return comp_context.get_out()(0, 0, 0);
	}

	double TdLambdaSubAgent::update_z_and_evaluate_prev_after_state(const ITdlSettingsReadOnly& settings, INet& net)
	{
		const auto lambda_times_gamma = settings.get_lambda() * settings.get_discount();
		const auto long_evaluation = lambda_times_gamma > 0 && !_z.empty();

		auto& gradient_alias = long_evaluation ? _gradient_cache : _z;

		net.calc_gradient_and_value(_prev_after_state,
			_tensor_shared, DeepLearning::CostFunctionId::LINEAR,
			gradient_alias, _tensor_shared, _context);

		if (long_evaluation)
		{
			for (auto layer_id = 0ull; layer_id < gradient_alias.size(); ++layer_id)
				_z[layer_id].scale_and_add(lambda_times_gamma, gradient_alias[layer_id]);
		}

		return _tensor_shared(0, 0, 0);
	}

	void TdLambdaSubAgent::reset()
	{
		_new_game = true;
		_move_counter = 0;
		_z.clear();
	}

	TdLambdaSubAgent::TdLambdaSubAgent(const bool is_white) : _is_white(is_white)
	{}

	int TdLambdaSubAgent::make_move(const IMinimalStateReadonly& state,
	                                const ITdlSettingsReadOnly& settings, INet& net)
	{
		auto move_data = pick_move(state, settings, net);
		return make_move(state, std::move(move_data), settings, net);
	}

	int TdLambdaSubAgent::make_move(const IMinimalStateReadonly& state,
	                                MoveData&& move_data,
	                                const ITdlSettingsReadOnly& settings, INet& net)
	{
		_move_counter++;

		if (!settings.get_training_mode(_is_white) || settings.get_train_depth() < _move_counter)
			return move_data.move_id;

		if (_new_game)
		{
			_prev_after_state = std::move(move_data.after_state);
			_prev_state = state.evaluate();
			_new_game = false;
			return move_data.move_id;
		}

		auto current_state_tensor = state.evaluate();
		const auto reward = settings.get_reward_factor() <= 0.0 ? 0.0 : settings.get_reward_factor() *
			state.calc_reward(_prev_state, current_state_tensor);

		const auto prev_afterstate_value = update_z_and_evaluate_prev_after_state(settings, net);
		const auto delta = reward + settings.get_discount() * move_data.value - prev_afterstate_value;

		net.update(_z, -settings.get_learning_rate() * delta, 0.0);

		_prev_after_state = std::move(move_data.after_state);
		_prev_state = std::move(current_state_tensor);

		return move_data.move_id;
	}

	void TdLambdaSubAgent::game_over(const IMinimalStateReadonly& final_state, const GameResult& result,
		const ITdlSettingsReadOnly& settings, INet& net)
	{
		if (settings.get_training_mode(_is_white) && !_new_game)
		{
			const auto moves_to_discount = _move_counter - settings.get_train_depth();
			const auto discount_factor = moves_to_discount <= 0 ? 1.0 : pow(settings.get_discount(), moves_to_discount);

			const auto reward = 2 * static_cast<int>(result) * discount_factor;
			const auto delta = reward - update_z_and_evaluate_prev_after_state(settings, net);
			net.update(_z, -settings.get_learning_rate() * delta, 0.0);
		}

		reset();
	}

	int TdLambdaSubAgent::pick_move_id(const IMinimalStateReadonly& state,
	                                   const ITdlSettingsReadOnly& settings, const INet& net) const
	{
		return pick_move(state, settings, net).move_id;
	}

	bool TdLambdaSubAgent::equal(const TdLambdaSubAgent& another_sub_agent) const
	{
		return _z == another_sub_agent._z &&
			_prev_state == another_sub_agent._prev_state &&
			_prev_after_state == another_sub_agent._prev_after_state;
	}

	void TdLambdaSubAgent::reset_explorer(const unsigned seed)
	{
		Explorer::reset(seed);
	}
}
