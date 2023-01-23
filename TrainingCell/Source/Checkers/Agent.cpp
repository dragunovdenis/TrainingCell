#include "../../Headers/Checkers/Agent.h"
#include "../../../DeepLearning/DeepLearning/Utilities.h"
#include "../../../DeepLearning/DeepLearning/NeuralNet/Net.h"
#include "../../../DeepLearning/DeepLearning/MsgPackUtils.h"

namespace TrainingCell::Checkers
{
	int RandomAgent::make_move(const State& current_state, const std::vector<Move>& moves)
	{
		return DeepLearning::Utils::get_random_int(0, static_cast<int>(moves.size() - 1));
	}

	void RandomAgent::game_over(const State& final_state, const GameResult& result)
	{
		//Just do nothing because this agent can't improve its performance
	}

	void TdLambdaAgent::reset()
	{
		_new_game = true;
		_z.clear();
	}

	void TdLambdaAgent::game_over(const State& final_state, const GameResult& result)
	{
		update_z();
		const auto prev_state_with_move_value = _net.act(_prev_state_with_move.to_tensor())(0, 0, 0);
		const auto reward = static_cast<int>(result);
		const auto delta = reward - prev_state_with_move_value;
		_net.update(_z, -_alpha * delta, 0.0);

		reset();
	}

	void TdLambdaAgent::set_exploration_probability(double epsilon)
	{
		_exploration_epsilon = epsilon;
	}

	double TdLambdaAgent::get_exploratory_probability() const
	{
		return _exploration_epsilon;
	}

	void TdLambdaAgent::set_discount(double gamma)
	{
		_gamma = gamma;
	}

	double TdLambdaAgent::get_discount() const
	{
		return _gamma;
	}

	void TdLambdaAgent::set_training_mode(const bool training_mode)
	{
		_training_mode = training_mode;
	}

	bool TdLambdaAgent::get_training_mode() const
	{
		return _training_mode;
	}

	void TdLambdaAgent::set_lambda(const double lambda)
	{
		_lambda = lambda;
	}

	double TdLambdaAgent::get_lambda() const
	{
		return _lambda;
	}

	void TdLambdaAgent::set_learning_rate(const double alpha)
	{
		_alpha = alpha;
	}

	double TdLambdaAgent::get_learning_rate() const
	{
		return _alpha;
	}

	/// <summary>
	/// Calculates reward based on the given initial and final state of the game
	/// </summary>
	double calculate_reward(const State& init_state, const State& final_state)
	{
		const auto init_score = init_state.calc_score();
		const auto final_score = final_state.calc_score();
		const auto diff_score = final_score.diff(init_score);

		return (2.0 * diff_score[Piece::King] +
			diff_score[Piece::Man] -
			diff_score[Piece::AntiMan] -
			2.0 * diff_score[Piece::AntiKing]) / 50.0;
	}

	void TdLambdaAgent::update_z()
	{
		if (_z.empty())
			_z = _net.calc_gradient(_prev_state_with_move.to_tensor(),
				DeepLearning::Tensor(1, 1, 1, false), DeepLearning::CostFunctionId::LINEAR);
		else
		{
			auto gradients = _net.calc_gradient(_prev_state_with_move.to_tensor(),
				DeepLearning::Tensor(1, 1, 1, false), DeepLearning::CostFunctionId::LINEAR);
			if (gradients.size() != _z.size())
				throw std::exception("Incompatible data");

			for (auto layer_id = 0ull; layer_id < gradients.size(); ++layer_id)
			{
				gradients[layer_id] += _z[layer_id] * _lambda * _gamma;
				_z[layer_id] = std::move(gradients[layer_id]);
			}
		}
	}

	int TdLambdaAgent::make_move(const State& current_state, const std::vector<Move>& moves)
	{
		if (!_training_mode)
			return pick_move_id(current_state, moves);

		if (_new_game)
		{
			_prev_state_with_move = _prev_state = current_state;
			const auto move_id = pick_move_id(current_state, moves);
			_prev_state_with_move.make_move(moves[move_id], true, false);
			_new_game = false;
			return  move_id;
		}

		//update approximation function
		update_z();

		const auto move_to_take = pick_move_id(current_state, moves);
		auto current_state_with_move = current_state;
		current_state_with_move.make_move(moves[move_to_take], true, false);

		const auto current_state_with_move_value = _net.act(current_state_with_move.to_tensor())(0, 0, 0);
		const auto prev_state_with_move_value = _net.act(_prev_state_with_move.to_tensor())(0, 0, 0);
		const auto reward = calculate_reward(_prev_state, current_state);

		const auto delta = reward + _gamma * current_state_with_move_value - prev_state_with_move_value;

		_net.update(_z, -_alpha * delta, 0.0);

		_prev_state_with_move = current_state_with_move;
		_prev_state = current_state;

		return move_to_take;
	}

	int TdLambdaAgent::pick_move_id(const State state, const std::vector<Move>& moves) const
	{
		if (moves.empty())
			return -1;

		if (moves.size() == 1)
			return 0;

		if (DeepLearning::Utils::get_random(0, 1.0) <= _exploration_epsilon)
			//Exploration move
			return DeepLearning::Utils::get_random_int(0, static_cast<int>(moves.size()) - 1);

		//Exploitation move
		int best_move_id = -1;
		auto best_value = -std::numeric_limits<double>::max();

		for (auto move_id = 0ull; move_id < moves.size(); ++move_id)
		{
			auto state_copy = state;
			state_copy.make_move(moves[move_id], true, false);
			const auto eval_result = _net.act(state_copy.to_tensor())(0, 0, 0);

			if (eval_result > best_value)
			{
				best_value = eval_result;
				best_move_id = static_cast<int>(move_id);
			}
		}

		return best_move_id;
	}

	TdLambdaAgent::TdLambdaAgent(
		const std::vector<std::size_t>& layer_dimensions, const double exploration_epsilon,
		const double lambda, const double gamma, const double alpha) :
		_new_game(true), _exploration_epsilon(exploration_epsilon), _lambda(lambda), _gamma(gamma), _alpha(alpha)
	{
		if (layer_dimensions.empty() || layer_dimensions[0] != StateSize || layer_dimensions.rbegin()[0] != 1)
			throw std::exception("Invalid Net configuration");

		std::vector activ_func_ids(layer_dimensions.size() - 1, DeepLearning::ActivationFunctionId::RELU);
		activ_func_ids.rbegin()[0] = DeepLearning::ActivationFunctionId::LINEAR;

		_net = DeepLearning::Net(layer_dimensions, activ_func_ids);
	}

	bool TdLambdaAgent::operator ==(const TdLambdaAgent& anotherAgent) const
	{
		return _net.equal(anotherAgent._net) &&
			_z == anotherAgent._z &&
			_prev_state == anotherAgent._prev_state &&
			_prev_state_with_move == anotherAgent._prev_state_with_move &&
			_new_game == anotherAgent._new_game &&
			_exploration_epsilon == anotherAgent._exploration_epsilon &&
			_training_mode == anotherAgent._training_mode &&
			_lambda == anotherAgent._lambda &&
			_gamma == anotherAgent._gamma &&
			_alpha == anotherAgent._alpha;

}

	bool TdLambdaAgent::operator !=(const TdLambdaAgent& anotherAgent) const
	{
		return !(*this == anotherAgent);
	}

	void TdLambdaAgent::save_to_file(const std::filesystem::path& file_path) const
	{
		DeepLearning::MsgPack::save_to_file(*this, file_path);
	}

	TdLambdaAgent TdLambdaAgent::load_from_file(const std::filesystem::path& file_path)
	{
		return DeepLearning::MsgPack::load_from_file<TdLambdaAgent>(file_path);
	}

	InteractiveAgent::InteractiveAgent(const MakeMoveCallback& make_move_callback, const GameOverCallback& game_over_callback,
		const bool play_for_whites): _make_move_callback(make_move_callback), _game_over_callback(game_over_callback), _play_for_whites(play_for_whites)
	{
		if (_make_move_callback == nullptr || _game_over_callback == nullptr)
			throw std::exception("Invalid callback pointers");
	}

	int InteractiveAgent::make_move(const State& current_state, const std::vector<Move>& moves)
	{
		if (_play_for_whites)
			return _make_move_callback(current_state, moves);

		std::vector<Move> moves_inverted(moves.size());

		std::ranges::transform(moves, moves_inverted.begin(), [](const auto m)
			{
				return m.get_inverted();
			});

		return _make_move_callback(current_state.get_inverted(), moves_inverted);
	}

	void InteractiveAgent::game_over(const State& final_state, const GameResult& result)
	{
		if (_play_for_whites)
			_game_over_callback(final_state, result);

		_game_over_callback(final_state.get_inverted(), result);
	}
}