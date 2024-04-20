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

#include "OptimizationMode.h"

#include <iostream>

#include "ArgumentsOptimization.h"
#include "ConsoleUtils.h"
#include "TrainingState.h"
#include "Headers/TrainingEngine.h"
#include "../DeepLearning/DeepLearning/Math/Optimization/NelderMeadOptimizer.h"
#include "../DeepLearning/DeepLearning/MsgPackUtils.h"
#include "../DeepLearning/DeepLearning/StopWatch.h"

using namespace TrainingCell;

namespace Training::Modes
{
	/// <summary>
	/// Returns "true" if succeeds to load optimizer from the given path
	/// </summary>
	/// <param name="optimizer_path">Path to load from</param>
	/// <param name="optimizer">Container which will be assigned with the loaded optimizer.
	/// Should be regarded as valid only if "True" is returned</param>
	template <int N>
	static bool try_load_optimizer_silent(const std::filesystem::path& optimizer_path, DeepLearning::NelderMeadOptimizer<N>& optimizer)
	{
		try
		{
			optimizer = DeepLearning::MsgPack::load_from_file<DeepLearning::NelderMeadOptimizer<N>>(optimizer_path);
			return true;
		} catch (...)
		{
			return false;
		}
	}

	/// <summary>
	/// Tries to load optimized dump with the corresponding dialog
	/// </summary>
	template <int N>
	static bool try_load_optimizer(const std::filesystem::path& optimizer_path, DeepLearning::NelderMeadOptimizer<N>& optimizer)
	{
		if (try_load_optimizer_silent(optimizer_path, optimizer))
		{
			ConsoleUtils::horizontal_console_separator();
			ConsoleUtils::print_to_console("Optimizer dump was successfully loaded");

			if (ConsoleUtils::decision_prompt("Discard? (y/n):"))
			{
				return false;
			}
		}
		else
			return false;

		return true;
	}


	/// <summary>
	/// Data structure to hold values of parameters that take part in the optimization process and restrictions on their values
	/// </summary>
	struct PointAndConstraints
	{
		/// <summary>
		///	A point
		/// </summary>
		std::vector<DeepLearning::Real> point;

		/// <summary>
		///	Upper boundary for the parameters
		/// </summary>
		std::vector<DeepLearning::Real> upper_bound;

		/// <summary>
		/// Lower boundary on the parameters
		/// </summary>
		std::vector<DeepLearning::Real> lower_bound;
	};

	/// <summary>
	/// Returns initial value of parameters that will take part in the optimization process based on the optimization settings and
	/// the given "sample agent"
	/// </summary>
	static PointAndConstraints get_init_point_and_restrictions(const ParameterFlags& args, const TrainingCell::TdLambdaAgent& sample_agent)
	{
		PointAndConstraints result;
		auto& point = result.point;
		auto& lower_bound = result.lower_bound;
		auto& upper_bound = result.upper_bound;

		if (args.get_lambda_flag())
		{
			point.push_back(static_cast<DeepLearning::Real>(sample_agent.get_lambda()));
			lower_bound.push_back(0.0);
			upper_bound.push_back(1.0);
		}

		if (args.get_discount_flag())
		{
			point.push_back(static_cast<DeepLearning::Real>(sample_agent.get_discount()));
			lower_bound.push_back(0.0);
			upper_bound.push_back(1.0);
		}

		if (args.get_learning_rate_flag())
		{
			point.push_back(static_cast<DeepLearning::Real>(sample_agent.get_learning_rate()));
			lower_bound.push_back(0.0);
			upper_bound.push_back(2.0);
		}

		if (args.get_exploration_flag())
		{
			point.push_back(static_cast<DeepLearning::Real>(sample_agent.get_exploration_probability()));
			lower_bound.push_back(0.0);
			upper_bound.push_back(1.0);
		}

		return result;
	}

	/// <summary>
	/// Updates values of the corresponding parameters in the given training state
	/// </summary>
	static void assign_params(const std::vector<DeepLearning::Real>& parameter_values, const ParameterFlags& args, TrainingState& state)
	{
		auto current_param_id = 0;

		if (args.get_lambda_flag())
		{
			state.set_lambda(parameter_values[current_param_id++]);
		}

		if (args.get_discount_flag())
		{
			state.set_discount(parameter_values[current_param_id++]);
		}

		if (args.get_learning_rate_flag())
		{
			state.set_learning_rate(parameter_values[current_param_id++]);
		}

		if (args.get_exploration_flag())
		{
			state.set_exploration(parameter_values[current_param_id]);
		}
	}

	/// <summary>
	/// Writes values of parameters that are present in the given collection,
	/// supplemented with the values of parameters taken from the given agent, to the given stream
	/// </summary>
	static void dump_parameters( std::ostream& stream, const std::vector<DeepLearning::Real>& parameter_values, const ParameterFlags& args,
		const TrainingCell::TdLambdaAgent& sample_agent)
	{
		auto current_param_id = 0;

		stream << "Lambda = " << (args.get_lambda_flag() ? parameter_values[current_param_id++] : sample_agent.get_lambda()) << std::endl;
		stream << "Discount = " << (args.get_discount_flag() ? parameter_values[current_param_id++] : sample_agent.get_discount()) << std::endl;
		stream << "Learning rate = " << (args.get_learning_rate_flag() ? parameter_values[current_param_id++] : sample_agent.get_learning_rate()) << std::endl;
		stream << "Exploration = " << (args.get_exploration_flag() ? parameter_values[current_param_id] : sample_agent.get_exploration_probability()) << std::endl;
	}

	/// <summary>
	/// Returns value of the cost function
	/// </summary>
	/// <param name="params">Parameter vector</param>
	/// <param name="args">Parameter flags. Needed to map values of the "params"
	/// vector to to the corresponding parameters of agents</param>
	/// <param name="init_state">Initial state that we start with when evaluating the cost function</param>
	/// <param name="episodes_to_train">Number of episodes to train agents</param>
	/// <param name="episodes_to_evaluate">Number of episodes to evaluate performance of the trained agents</param>
	template <int N>
	double calculate_cost(const DeepLearning::VectorNdReal<N>& params, const ParameterFlags& args, const TrainingState& init_state,
		const int episodes_to_train, const int episodes_to_evaluate)
	{
		auto state = init_state;//copy initial state

		assign_params(params.to_std_vector(), args, state);

		std::vector<TdLambdaAgent*> agent_ptrs;

		for (auto agent_id = 0ull; agent_id < state.agents_count(); ++agent_id)
			agent_ptrs.push_back(&state[agent_id]);

		TrainingEngine engine(agent_ptrs);

		auto result = -1.0;
		engine.run(0, 1 /*one round*/, episodes_to_train, [&result](const auto& time, const auto& performance)
			{
				result = 0.0;
				for (const auto& perf_item : performance)
					result += perf_item.get_score();

				result /= performance.size();
			}, true /*fixed pairs*/, episodes_to_evaluate);

		return result;
	}

	/// <summary>
	/// Internal method to run parameter optimization
	/// </summary>
	template <int N>
	static void run_parameter_optimization_internal(const ArgumentsOptimization& args, TrainingState& state,
		const PointAndConstraints& init_pt_and_constraints, const bool can_continue)
	{
		DeepLearning::NelderMeadOptimizer<N> optimizer;
		bool optimizer_loaded = false;
		if (!try_load_optimizer<N>(args.get_optimizer_dump_path(), optimizer))
		{
			optimizer.set_constraints_lower(init_pt_and_constraints.lower_bound);
			optimizer.set_constraints_upper(init_pt_and_constraints.upper_bound);
			optimizer.set_min_simplex_size(static_cast<DeepLearning::Real>(args.get_min_simplex_size()));
		}
		else
			optimizer_loaded = true;

		const auto skip_initialization = can_continue && optimizer_loaded;

		int round_counter = 0;
		int evaluation_counter = 0;
		DeepLearning::StopWatch sw_total;
		DeepLearning::StopWatch sw_round;
		optimizer.optimize([&args, &state, &evaluation_counter](const auto& params)
			{
				const DeepLearning::StopWatch sw_evaluation;
				std::cout << "Evaluation : " << evaluation_counter++ << std::endl;
				dump_parameters(std::cout, params.to_std_vector(), args, state[0]);
				const auto score = calculate_cost(params, args, state,
					args.get_num_episodes(), args.get_num_eval_episodes());
				std::cout << "Score :" << score << std::endl;
				std::cout << "Evaluation time : " << sw_evaluation.elapsed_time_hh_mm_ss() << std::endl;
				ConsoleUtils::horizontal_console_separator();
				return -static_cast<DeepLearning::Real>(score);
			}, static_cast<DeepLearning::Real>(0.1), init_pt_and_constraints.point, skip_initialization,
			true /*regular simplex*/,
				[&round_counter, &args, &state, &optimizer, &sw_round, &sw_total]
				(const std::array<DeepLearning::VectorNdReal<N>, N + 1>& simplex,
					const int min_vertex_id, const DeepLearning::Real& simplex_size)

			{
				if (++round_counter % args.get_dump_rounds() == 0)
					DeepLearning::MsgPack::save_to_file(optimizer, args.get_optimizer_dump_path());

				ConsoleUtils::horizontal_console_separator();
				std::cout << "Round : " << round_counter << std::endl;
				std::cout << "Simplex size : " << simplex_size << std::endl;
				dump_parameters(std::cout, simplex[min_vertex_id].to_std_vector(), args, state[0]);
				std::cout << "Max score : " << -optimizer.get_min_value() << std::endl;
				std::cout << "Round time : " << sw_round.elapsed_time_hh_mm_ss() << std::endl;
				std::cout << "Total time time : " << sw_total.elapsed_time_hh_mm_ss() << std::endl;
				sw_round.reset();
				ConsoleUtils::horizontal_console_separator();
				ConsoleUtils::horizontal_console_separator();
			});

		std::ofstream result_file(args.get_output_folder() / (args.get_hash() + ".txt"));
		result_file << args.to_string() << std::endl;
		result_file << std::endl;
		result_file << "Score = " << -optimizer.get_min_value() << std::endl;
		dump_parameters(result_file, optimizer.get_min_vertex().to_std_vector(), args, state[0]);
	}

	/// <summary>
	/// Synchronizes training parameters of the agents in the given state
	/// </summary>
	void sync_training_parameters(TrainingState& state)
	{
		if (state.agents_count() < 2)
			return;

		state.set_lambda(state[0].get_lambda());
		state.set_discount(state[0].get_discount());
		state.set_learning_rate(state[0].get_learning_rate());
		state.set_exploration(state[0].get_exploration_probability());
	}

	void run_parameter_optimization(int argc, char** argv)
	{
		const ArgumentsOptimization args(argc, argv);
		ConsoleUtils::print_to_console(args.to_string());

		TrainingState state;
		bool continue_optimization = false;
		if (!ConsoleUtils::try_load_state(args.get_state_dump_path(), state))
		{
			state = TrainingState(args.get_source_path());
			sync_training_parameters(state);
			ConsoleUtils::print_to_console("State was constructed from script: ");
			ConsoleUtils::print_to_console(state.get_agents_script());
			state.save_to_file(args.get_state_dump_path(), false);
		if (!ConsoleUtils::decision_prompt())
			return;
		}
		else
			continue_optimization = true;

		const auto point_and_bounds = get_init_point_and_restrictions(args, state[0]);

		switch (point_and_bounds.point.size())
		{
			case 1: run_parameter_optimization_internal<1>(args, state, point_and_bounds, continue_optimization);
				break;
			case 2: run_parameter_optimization_internal<2>(args, state, point_and_bounds, continue_optimization);
				break;
			case 3: run_parameter_optimization_internal<3>(args, state, point_and_bounds, continue_optimization);
				break;
			case 4: run_parameter_optimization_internal<4>(args, state, point_and_bounds, continue_optimization);
				break;
			default: throw std::exception("Unexpected number of parameters");
		}

	}
}
