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

#pragma once
#include "IStateSeed.h"
#include "Move.h"
#include "../../DeepLearning/DeepLearning/Math/Tensor.h"

namespace TrainingCell
{
	/// <summary>
	/// General interface for an "environment state"
	///</summary>
	class IState : public IStateSeed
	{
	public:
		/// <summary>
		/// Size of the state.
		/// Literally, number of elements in the "tensor" representation of the state
		///</summary>
		[[nodiscard]] virtual std::size_t dim() const = 0;

		/// <summary>
		///	Returns tensor representation of the current state after `cached move` with the given ID was applied to it
		/// </summary>
		[[nodiscard]] virtual DeepLearning::Tensor get_state(const Move& move) const = 0;

		/// <summary>
		/// Makes the move
		/// </summary>
		/// <param name="move">Move to "make"</param>
		/// <param name="remove_extra_markers">If "false", state will contain auxiliary markers "illustrating" details of the move</param>
		virtual void make_move(const Move& move, const bool remove_extra_markers) = 0;

		/// <summary>
		///	Returns tensor representation of the current state
		/// </summary>
		[[nodiscard]] virtual DeepLearning::Tensor to_tensor() const = 0;

		/// <summary>
		///	Returns int vector representation of the state
		/// </summary>
		[[nodiscard]] virtual std::vector<int> to_std_vector() const = 0;

		/// <summary>
		/// Returns an "inverted" state, i.e. a state that it is seen by the opponent (an agent playing "anti" pieces)
		/// in the form of integer vector
		///// </summary>
		[[nodiscard]] virtual std::vector<int> get_inverted_std() const = 0;

		/// <summary>
		/// Returns collection of available moves for the current state
		/// </summary>
		[[nodiscard]] virtual std::vector<Move> get_moves() const = 0;

		/// <summary>
		/// "Inverts" the state
		/// </summary>
		void virtual invert() = 0;

		/// <summary>
		/// Returns "true" if the state is reversed with respect to its initial "orientation" (the one it was created)
		/// </summary>
		[[nodiscard]] virtual bool is_inverted() const = 0;

		/// <summary>
		/// Calculates for the given pair of previous and next after-states represented with "raw" tensors
		/// (those that can be obtained by calling `to_tensor` method below)
		///</summary>
		[[nodiscard]] virtual double calc_reward(const DeepLearning::Tensor& prev_after_state,
			const DeepLearning::Tensor& next_after_state) const = 0;

		/// <summary>
		/// Returns copy of the current instance
		/// </summary>
		[[nodiscard]] virtual std::unique_ptr<IState> copy() const = 0;
	};
}
