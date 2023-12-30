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
#include <array>
#include "ITrainableAgent.h"
#include "IStateSeed.h"
#include "IState.h"

namespace TrainingCell
{
	/// <summary>
	///	Callback to publish current state and move
	/// </summary>
	typedef void (*PublishStateCallBack)(const int* state, const int state_size,
	                                             const SubMove* sub_moves, const int sub_moves_count, const IMinimalAgent* agent_to_play);

	/// <summary>
	///	Callback to publish training statistics
	/// </summary>
	typedef void (*PublishEndEpisodeStatsCallBack)(const bool white_wins, const bool black_wins, const int total_games);

	/// <summary>
	/// Callback to report error messages
	/// </summary>
	typedef void (*ErrorMessageCallBack)(const char* message);

	/// <summary>
	///	Callback to cancel ongoing operation
	/// </summary>
	typedef bool (*CancelCallBack)();

	/// <summary>
	///	Environment to train checkers-agents
	/// </summary>
	class Board
	{
		IMinimalAgent* _white_agent_ptr{};
		IMinimalAgent* _black_agent_ptr{};

		/// <summary>
		/// Controller to work with a pair of agents.
		/// </summary>
		template <class A>
		class AgentManager
		{
		protected:
			std::array<A*, 2> _agents{};
			int _agent_to_move_id = 0;

		public:
			/// <summary>
			/// Default constructor - deleted.
			/// </summary>
			AgentManager() = delete;

			/// <summary>
			/// Copy constructor - deleted.
			/// </summary>
			AgentManager(const AgentManager&) = delete;

			/// <summary>
			/// Constructor.
			/// </summary>
			AgentManager(A* agent_white, A* agent_black);

			/// <summary>
			/// Returns reference to the "agent to move".
			/// </summary>
			A& agent_to_move();

			/// <summary>
			/// Returns reference to the "agent to weight".
			/// </summary>
			A& agent_to_wait();

			/// <summary>
			/// Returns "true" if the "agent to move" is the "white" agent.
			/// </summary>
			[[nodiscard]] bool is_agent_to_move_white() const;

			/// <summary>
			/// Swaps "agent to move" and "agent to weight".
			/// </summary>
			void take_turn();

			/// <summary>
			/// Returns ID of the agent that is supposed to move next.
			/// </summary>
			[[nodiscard]] int next_agent_id() const;

			/// <summary>
			/// Resets the controller to the init state.
			/// </summary>
			void reset();
		};

		/// <summary>
		/// Controller manager used to train agents on episode traces with certain outcome.
		/// </summary>
		template <class A>
		class AgentManagerAdv : public AgentManager<A>
		{
		public:
			/// <summary>
			/// Constructor.
			/// </summary>
			AgentManagerAdv(A* agent_white, A* agent_black);

			/// <summary>
			/// Toggles the two managed agents into an "exploration" (non-training) mode.
			/// </summary>
			void set_exploration_mode();

			/// <summary>
			/// Toggles the two managed agents into a "training" (non-exploration) mode.
			/// </summary>
			void set_training_mode();
		};

		/// <summary>
		/// Result of an episode.
		/// </summary>
		enum EpisodeResult : int
		{
			Draw = 0,
			StrongDraw = 1, // like a stalemate in chess
			WhiteVictory = 1 << 1,
			BlackVictory = 1 << 2,
		};

		/// <summary>
		/// Plays a single episode.
		/// </summary>
		/// <param name="state">State to "play on".</param>
		/// <param name="agent_manager">Access to the agents to play the episode.</param>
		/// <param name="max_moves_without_capture">Maximal number of moves without capture to be qualified as a "draw".</param>
		/// <param name="publish_state_callback">Callback to "publish" current state.</param>
		/// <param name="cancel">Callback to request cancellation.</param>
		/// <returns>Result of the episode.</returns>
		template <class A>
		static EpisodeResult play_episode(IState& state, AgentManager<A>& agent_manager,
			const int max_moves_without_capture, PublishStateCallBack publish_state_callback, CancelCallBack cancel);

		/// <summary>
		/// Retrieve move from the "current" agent and updates the "current" state accordingly.
		/// Returns "true" if the taken move was a "capture" one.
		/// It is a responsibility of the caller to ensure that it is possible to make a move
		/// in the "current state" before calling the method.
		/// </summary>
		template <class A>
		static bool make_move(IState& state_handle, AgentManager<A>& agent_manager, PublishStateCallBack publish);

	public:

		/// <summary>
		/// Data struct to represent playing statistics.
		/// </summary>
		class Stats
		{
			int _blacks_win_count{};
			int _whites_win_count{};
			int _total_episodes_count{};

		public:
			/// <summary>
			/// Constructor.
			/// </summary>
			Stats(const int blacks_win_count, const int whites_win_count, const int total_episodes);

			/// <summary>
			/// Returns number of times "black" agent has won.
			/// </summary>
			[[nodiscard]] int blacks_win_count() const;

			/// <summary>
			/// Returns number of times "white" agent has won.
			/// </summary>
			[[nodiscard]] int whites_win_count() const;

			/// <summary>
			/// Returns total number of episodes played.
			/// </summary>
			[[nodiscard]] int total_episodes_count() const;
		};

		/// <summary>
		/// Constructor.
		/// </summary>
		Board(IMinimalAgent* white_agent_ptr, IMinimalAgent* black_agent_ptr);

		/// <summary>
		/// Swaps "white" and "black" agents.
		/// </summary>
		void swap_agents();

		/// <summary>
		///	Runs the given number of episodes (games)
		/// </summary>
		/// <param name="episodes">Number of episodes (games) to play</param>
		/// <param name="max_moves_without_capture">Defines maximal number of moves without a capture that will be qualified as a "draw"</param>
		/// <param name="start_state">State from which each episode (game) should be started</param>
		/// <param name="publish_state_callback">Callback to be called after each move. Allows caller to get some intermediate information about the process</param>
		/// <param name="publish_end_episode_stats_callback">Callback to be called after each episode (game). Allows caller to get some intermediate information about the process</param>
		/// <param name="cancel">Callback allowing caller to cancel the process</param>
		/// <param name="error">Callback allowing caller to get some information about errors encountered</param>
		Stats play(const int episodes, const IStateSeed& start_state, const int max_moves_without_capture = 200,
			PublishStateCallBack publish_state_callback = nullptr,
			PublishEndEpisodeStatsCallBack publish_end_episode_stats_callback = nullptr, CancelCallBack cancel = nullptr,
			ErrorMessageCallBack error = nullptr) const;


		/// <summary>
		///	Runs the given number of episodes (games) for the given pair of agents.
		/// </summary>
		/// <param name="agent_white_ptr">Pointer to the "white" agent.</param>
		/// <param name="agent_black_ptr">Pointer to the "black" agent.</param>
		/// <param name="episodes">Number of episodes (games) to play</param>
		/// <param name="max_moves_without_capture">Defines maximal number of moves without a capture that will be qualified as a "draw"</param>
		/// <param name="start_state">State from which each episode (game) should be started</param>
		/// <param name="publish_state_callback">Callback to be called after each move. Allows caller to get some intermediate information about the process</param>
		/// <param name="publish_end_episode_stats_callback">Callback to be called after each episode (game). Allows caller to get some intermediate information about the process</param>
		/// <param name="cancel">Callback allowing caller to cancel the process</param>
		/// <param name="error">Callback allowing caller to get some information about errors encountered</param>
		static Stats play(IMinimalAgent* const agent_white_ptr, IMinimalAgent* const agent_black_ptr,
				  const int episodes, const IStateSeed& start_state, const int max_moves_without_capture = 200,
		          PublishStateCallBack publish_state_callback = nullptr,
		          PublishEndEpisodeStatsCallBack publish_end_episode_stats_callback = nullptr, CancelCallBack cancel = nullptr,
		          ErrorMessageCallBack error = nullptr);

		/// <summary>
		/// Trains the pair of given agents for the given number of episodes with nontrivial outcome.
		/// </summary>
		/// <param name="agent_white_ptr">Pointer to the "white" agent.</param>
		/// <param name="agent_black_ptr">Pointer to the "black" agent.</param>
		/// <param name="episodes">Number of episodes (games) to play</param>
		/// <param name="max_moves_without_capture">Defines maximal number of moves without a capture that will be qualified as a "draw"</param>
		/// <param name="start_state">State from which each episode (game) should be started</param>
		/// <param name="publish_end_episode_stats_callback">Callback to be called after each episode (game). Allows caller to get some intermediate information about the process</param>
		/// <param name="cancel">Callback allowing caller to cancel the process</param>
		/// <param name="error">Callback allowing caller to get some information about errors encountered</param>
		static Stats train(ITrainableAgent* const agent_white_ptr, ITrainableAgent* const agent_black_ptr,
			const int episodes, const IStateSeed& start_state, const int max_moves_without_capture = 200,
			PublishEndEpisodeStatsCallBack publish_end_episode_stats_callback = nullptr, CancelCallBack cancel = nullptr,
			ErrorMessageCallBack error = nullptr);
	};
}
