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

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using Monitor.State;

namespace Monitor.UI
{
    /// <summary>
    /// General user interface for 2 players games
    /// </summary>
    public interface IGameEngine : INotifyPropertyChanged
    {
        /// <summary>
        /// Flag to check if the game is ongoing
        /// </summary>
        bool IsPlaying { get; }

        /// <summary>
        /// Flag indicating whether agent can be edited
        /// </summary>
        bool CanEditAgent { get; }

        /// <summary>
        /// String representation of the time the previous move has taken
        /// </summary>
        string PreviousMoveTime { get; }

        /// <summary>
        /// Invokes UI to edit inactive agent
        /// </summary>
        void EditInactiveAgent();

        /// <summary>
        /// If "true" progress bar should be shown
        /// </summary>
        bool TdlAgentIsPlaying { get; }

        /// <summary>
        /// Flag enabling the functionality allowing user to inspect moves and their values before actual move is taken.
        /// </summary>
        bool InspectOptions { get; }

        /// <summary>
        /// Flag indicating that at the moment it is possible to cancel playing.
        /// </summary>
        bool CanCancelPlaying { get; }

        /// <summary>
        /// Flag indicating that move inspection is ongoing at the moment.
        /// </summary>
        bool InspectionOngoing { get; }

        /// <summary>
        /// Method to resume game flow in case it was interrupted by the option evaluation procedure.
        /// </summary>
        void CompleteOptionSelection();

        /// <summary>
        /// Collection of evaluated moves.
        /// </summary>
        ObservableCollection<EvaluatedMove> EvaluatedOptions { get; }

        /// <summary>
        /// Pointer to the selected evaluated move from the collection above.
        /// </summary>
        EvaluatedMove SelectedOption { get; set; }

        /// <summary>
        /// Handles loading of an agent to play for "whites" (so that user will play for "blacks")
        /// and starts the game
        /// </summary>
        void LoadWhiteAgent();

        /// <summary>
        /// Handles loading of an agent to play for "blacks" (so that user will play for "whites")
        /// and starts the game
        /// </summary>
        void LoadBlackAgent();

        /// <summary>
        /// Terminates the current game session
        /// </summary>
        void TerminateGame();

        /// <summary>
        /// Event that is invoked each time we want to update training information on UI
        /// </summary>
        event Action<IList<string>> InfoEvent;
    }
}
