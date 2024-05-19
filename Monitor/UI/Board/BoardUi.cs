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
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;
using System.Windows.Threading;
using Monitor.Agents;
using Monitor.DataStructures;
using Monitor.Dll;
using Monitor.State;
using Monitor.UI.Board.PieceControllers;
using Monitor.Utils;
using static Monitor.Dll.DllWrapper;

namespace Monitor.UI.Board
{
    /// <summary>
    /// Functionality to handle user interface of the checkers game
    /// </summary>
    class BoardUi : ITwoPlayerGameUi
    {
        /// <summary>
        /// Types of agents
        /// </summary>
        public enum AgentType : int
        {
            Random,
            TdLambda,
            Interactive,
            AgentPack,
        };

        /// <summary>
        /// Dispatcher of the UI thread
        /// </summary>
        private readonly Dispatcher _uiThreadDispatcher;

        /// <summary>
        /// Provides means to cancel ongoing playing
        /// </summary>
        private CancellationTokenSource _playTaskCancellation;

        private Task _playTask;

        /// <summary>
        /// The current state
        /// </summary>
        private State.State _state;

        /// <summary>
        /// Data structure that facilitates user move processing
        /// </summary>
        private class MoveRequest
        {
            /// <summary>
            /// Set of possible moves in the current state
            /// </summary>
            public readonly Move[] PossibleMoves;
            /// <summary>
            /// Channel to provide user input to the thread that is running training
            /// </summary>
            public readonly TaskCompletionSource<int> UserMoveResult = new TaskCompletionSource<int>();

            /// <summary>
            /// Constructor
            /// </summary>
            public MoveRequest(Move[] possibleMoves)
            {
                PossibleMoves = possibleMoves;
            }
        }

        /// <summary>
        /// Data structure that facilitates user move selection mechanism
        /// </summary>
        private MoveRequest _userMoveRequest;

        /// <summary>
        /// Initializes a state when we wait until used make a move
        /// </summary>
        private Task<int> RequestUserMove(int[] state, Move[] possibleMoves)
        {
            if (state.Length != Checkerboard.Fields)
                throw new Exception("Invalid size of the state");

            _userMoveRequest = new MoveRequest(possibleMoves);
            _uiThreadDispatcher.Invoke(UpdateMarkers, DispatcherPriority.ContextIdle);
            return _userMoveRequest.UserMoveResult.Task;
        }

        /// <summary>
        /// Factory method to create agent
        /// </summary>
        IAgent CreateAgent(AgentType agentType, bool playWhites)
        {
            switch (agentType)
            {
                case AgentType.Random: return new RandomAgent();
                case AgentType.TdLambda:
                case AgentType.AgentPack:
                    {
                        return AgentFileSystemManager.LoadAgent();
                    }
                case AgentType.Interactive:
                    return new InteractiveAgent((state, moves) =>
                    {
                        if (state.Length != Checkerboard.Fields)
                            throw new Exception("Invalid size of the state");

                        var movePromise = RequestUserMove(state, moves);
                        return movePromise.Result;
                    }, (state, result) =>
                    {
                        if (state.Length != Checkerboard.Fields)
                            throw new Exception("Invalid size of the state");
                    }, playWhites);
                default: throw new Exception("Unknown agent type");
            }
        }

        /// <summary>
        /// Event that is invoked each time we want to update training information on UI
        /// </summary>
        public event Action<IList<string>> InfoEvent;

        private bool _isPlaying;

        /// <summary>
        /// Returns "true" if playing session is ongoing
        /// </summary>
        public bool IsPlaying
        {
            get => _isPlaying;

            private set => SetField(ref _isPlaying, value);
        }

        private bool _showProgressBar;

        /// <summary>
        /// If "true" progress bar should be shown
        /// </summary>
        public bool ShowProgressBar
        {
            get => _showProgressBar;
            private set => _uiThreadDispatcher.Invoke(() => { SetField(ref _showProgressBar, value); });
        }

        private string _previousMoveTime = "";

        /// <summary>
        /// String representation of the time the previous move has taken
        /// </summary>
        public string PreviousMoveTime
        {
            get => _previousMoveTime;
            set => _uiThreadDispatcher.Invoke(() => { SetField(ref _previousMoveTime, value); });
        }

        /// <summary>
        /// Handles loading of an agent to play for "whites" (so that user will play for "blacks")
        /// and starts the game
        /// </summary>
        public void LoadWhiteAgent()
        {
            Play(AgentType.AgentPack, AgentType.Interactive, 100);
        }

        /// <summary>
        /// Handles loading of an agent to play for "blacks" (so that user will play for "whites")
        /// and starts the game
        /// </summary>
        public void LoadBlackAgent()
        {
            Play(AgentType.Interactive, AgentType.AgentPack, 100);
        }

        /// <summary>
        /// Method to cancel playing. Returns after playing task is canceled
        /// </summary>
        public void TerminateGame()
        {
            if (IsPlaying)
            {
                if (_playTaskCancellation == null)
                    throw new Exception("The task can't be canceled: invalid cancellation token source");

                _playTaskCancellation.Cancel();

                if (_userMoveRequest != null)
                {
                    //set any result just to let the thread terminate
                    _userMoveRequest.UserMoveResult.SetResult(0);
                    _userMoveRequest = null;
                }
            }
        }

        private IAgent _inactiveAgent;

        /// <summary>
        /// Agent that is waiting for its turn to make a move
        /// </summary>
        private IAgent InactiveAgent
        {
            get => _inactiveAgent;
            set
            {
                _uiThreadDispatcher.Invoke(() =>
                {
                    SetField(ref _inactiveAgent, value);
                    OnPropertyChanged(nameof(CanEditAgent));
                    ShowProgressBar = _inactiveAgent is InteractiveAgent;
                });
            }
        }

        /// <summary>
        /// Invokes UI to edit inactive agent
        /// </summary>
        public void EditInactiveAgent()
        {
            if (InactiveAgent is TdLambdaAgent tdlInactive)
            {
                var dialog = new TdlAgentDialog(tdlInactive);
                dialog.ShowDialog();
            } else if (InactiveAgent is EnsembleAgent ensemble)
            {
                var dialog = new EnsembleAgentDialog(new List<ITdLambdaAgentReadOnly>(), ensemble);
                dialog.ShowDialog();
            }
            else
                throw new Exception("Invalid type of inactive agent. Can't edit.");
        }

        /// <summary>
        /// Flag indicating if agent editing is possible
        /// </summary>
        public bool CanEditAgent => InactiveAgent is TdLambdaAgent || InactiveAgent is EnsembleAgent;

        /// <summary>
        /// Adds extra symbols to the state to trace the series of given sub-moves (representing a move)
        /// </summary>
        private void AddMoveTrace(State.State state, SubMove[] subMoves)
        {
            if (subMoves == null)
                return;

            var pieceId = state.Data[subMoves.Last().End.LinearPosition];
            var pieceController = PieceControllerFactory.Create(state.Type);
            foreach (var move in subMoves)
            {
                if (move.Capture.IsValid)
                    state.Data[move.Capture.LinearPosition] = pieceController.GetCapturedPieceId(white: pieceId > 0);

                state.Data[move.Start.LinearPosition] = pieceController.GetPieceTraceId(pieceId);
            }
        }

        /// <summary>
        /// Returns state type compatible with the given state pair of agents.
        /// </summary>
        private StateTypeId ResolveStateType(IAgent agent0, IAgent agent1)
        {
            return CanPlay(agent0.Ptr, agent1.Ptr, out var stateTypeId) ?
                stateTypeId : StateTypeId.Invalid;
        }

        /// <summary>
        /// Returns an instance of state seed compatible with the given pair of agents.
        /// </summary>
        private IStateSeed ResolveStateSeed(IAgent agent0, IAgent agent1)
        {
            var stateType = ResolveStateType(agent0, agent1);

            switch (stateType)
            {
                case StateTypeId.Checkers: return _checkersSeed;
                case StateTypeId.Chess: return _chessSeed;
                default: throw new Exception("Can't resolve state seed");
            }
        }

        /// <summary>
        /// Returns a string with a message summarising the game.  
        /// </summary>
        string GetConclusionMessage(bool whiteWonFlag, bool blackWonFlag)
        {
            if (whiteWonFlag & !blackWonFlag)
                return "The Whites have won!";
            
            if (!whiteWonFlag & blackWonFlag)
                return "The Blacks have won!";

            if (whiteWonFlag & blackWonFlag)
                return "Stalemate!";

            return "It is a draw!";
        }

        /// <summary>
        /// Starts checkers game with the agents of the two given types (asynchronously) and returns immediately
        /// Returns true if the previous playing task is complete and the current one is successfully started
        /// </summary>
        /// <param name="agentTypeWhite">Type of the "white" agent.</param>
        /// <param name="agentTypeBlack">Type of the "black" agent.</param>
        /// <param name="episodes">Number of episodes to play</param>
        /// <param name="movePause">Number of milliseconds to wait between two successive moves</param>
        public bool Play(AgentType agentTypeWhite, AgentType agentTypeBlack, int episodes, int movePause = 100)
        {
            if (IsPlaying)
                return false;

            IsPlaying = true;
            _playTaskCancellation = new CancellationTokenSource();

            _playTask = new Task(() =>
            {
                using (var agentWhite = CreateAgent(agentTypeWhite, true))
                {
                    using (var agentBlack = CreateAgent(agentTypeBlack, false))
                    {
                        if (agentWhite == null || agentBlack == null)
                            return;

                        int whiteWinsCounter = 0;
                        int blackWinsCounter = 0;
                        int staleMatesCounter = 0;

                        var stateSeed = ResolveStateSeed(agentWhite, agentBlack);
                        var stateType = stateSeed.Type;
                        _uiThreadDispatcher.Invoke(ConnectToBoard);

                        var timePoint = DateTime.Now;
                        var res = DllWrapper.Play(
                            agentWhite.Ptr, agentBlack.Ptr, episodes, stateSeed.Ptr,
                            (stateData, size, subMoves, subMovesCount, agentToMovePtr) =>
                            {
                                if (size != Checkerboard.Fields)
                                    throw new Exception("Invalid size of the state");

                                var timePointNext = DateTime.Now;
                                var elapsedTime = timePointNext - timePoint;
                                PreviousMoveTime = elapsedTime.ToString(@"hh\:mm\:ss");
                                timePoint = timePointNext;

                                InactiveAgent = agentToMovePtr == agentWhite.Ptr ? agentBlack : agentWhite;

                                var state = new State.State(stateData, stateType);
                                AddMoveTrace(state, subMoves);
                                _state = state;
                                _uiThreadDispatcher.Invoke(() => _board.UpdateState(state), DispatcherPriority.ContextIdle);
                                if (movePause > 0) Thread.Sleep(movePause);
                            },
                            (whiteWon, blackWon, totalGamers) =>
                            {
                                InactiveAgent = null;
                                _uiThreadDispatcher.Invoke(() =>
                                {
                                    whiteWinsCounter += (whiteWon & !blackWon).ToInt();
                                    blackWinsCounter += (blackWon & !whiteWon).ToInt();
                                    staleMatesCounter += (whiteWon & blackWon).ToInt();

                                    if (!_playTaskCancellation.IsCancellationRequested &&
                                        (Application.Current.MainWindow == null ||
                                        MessageBox.Show(Application.Current.MainWindow,
                                            $"{GetConclusionMessage(whiteWon, blackWon)}. Continue?", "Episode is over",
                                            MessageBoxButton.YesNo, MessageBoxImage.Information) == MessageBoxResult.No))
                                            TerminateGame();

                                    _state = null;
                                    _board.UpdateState(null);
                                    InfoEvent?.Invoke(new List<string>()
                                    {
                                        "Whites Won: " + whiteWinsCounter,
                                        "Blacks Won: " + blackWinsCounter,
                                        "Stalemates: " + staleMatesCounter,
                                        "Total Games: " + totalGamers,
                                    });
                                }, DispatcherPriority.ContextIdle);
                            }, () => _playTaskCancellation.IsCancellationRequested,
                            (errorMessage) => _uiThreadDispatcher.Invoke(
                                () =>
                                {
                                    InactiveAgent = null;
                                    MessageBox.Show(errorMessage, "Error", MessageBoxButton.OK);
                                }), out _);

                        if (res != 0)
                            throw new Exception("Playing failed");

                        InactiveAgent = null;
                        PreviousMoveTime = "";
                    }
                }
            });
            _playTask.ContinueWith((task) =>
            {
                _playTask = null;
                _playTaskCancellation = null;
                IsPlaying = false;
                InfoEvent?.Invoke(null);
                ResetUserInput();
                DisconnectFromBoard();
            }, TaskScheduler.FromCurrentSynchronizationContext());
            _playTask.Start();

            return true;
        }

        private PiecePosition SelectedField => _board.SelectedField;

        /// <summary>
        /// Returns sub-set of possible moves that start from the given position
        /// </summary>
        private Move[] GetPossibleMovesStartingFrom(PiecePosition startPos)
        {
            return GetPossibleMovesStartingFrom(_userMoveRequest?.PossibleMoves, startPos);
        }

        /// <summary>
        /// Returns sub-set of possible moves that start from the given position
        /// </summary>
        private static Move[] GetPossibleMovesStartingFrom(Move[] possibleMoves, PiecePosition startPos)
        {
            if (possibleMoves == null || possibleMoves.Length == 0)
                return Array.Empty<Move>();

            return possibleMoves.Where(x => x.SubMoves[0].Start.IsEqualTo(startPos)).ToArray();
        }

        /// <summary>
        /// Returns sub-set of possible moves that end at the given position
        /// </summary>
        private static Move[] GetPossibleMovesEndingAt(Move[] possibleMoves, PiecePosition endPos)
        {
            if (possibleMoves == null || possibleMoves.Length == 0)
                return Array.Empty<Move>();

            return possibleMoves.Where(x => x.SubMoves.Last().End.IsEqualTo(endPos)).ToArray();
        }

        /// <summary>
        /// Adds guiding marker controls generated for the given selected field to the corresponding collections. 
        /// </summary>
        private void AppendMarkers(PiecePosition selectedField, List<CheckerBoard.Marker> markers = null)
        {
            var possibleMovesForCurrentPosition = GetPossibleMovesStartingFrom(selectedField);
            foreach (var move in possibleMovesForCurrentPosition)
            {
                var end = move.SubMoves.Last().End;
                markers?.Add(new CheckerBoard.Marker(end, Brushes.GreenYellow));
            }
        }

        /// <summary>
        /// Updates guiding markers on the board.
        /// </summary>
        void UpdateMarkers()
        {
            if (_userMoveRequest != null)
            {
                var markers = new List<CheckerBoard.Marker>();
                
                foreach (var move in _userMoveRequest.PossibleMoves)
                {
                    var start = move.SubMoves[0].Start;
                    markers.Add(new CheckerBoard.Marker(start, FieldUnderMousePointer.IsEqualTo(start) ?
                        Brushes.Red : Brushes.BlueViolet));
                }

                AppendMarkers(SelectedField, markers);
                AppendMarkers(FieldUnderMousePointer, markers);
                
                _board.UpdateMarkers(markers);
            }
        }

        private readonly ICheckerBoard _board;
        private readonly IStateSeed _chessSeed;
        private readonly IStateSeed _checkersSeed;

        /// <summary>
        /// Constructor
        /// </summary>
        public BoardUi(ICheckerBoard board, Dispatcher uiThreadDispatcher, IStateSeed chessSeed, IStateSeed checkersSeed)
        {
            _uiThreadDispatcher = uiThreadDispatcher ?? throw new Exception("Dispatcher must be not null");
            _board = board;

            if (chessSeed == null || chessSeed.Type != StateTypeId.Chess)
                throw new Exception("Invalid chess seed");

            _chessSeed = chessSeed;
            
            if (checkersSeed == null || checkersSeed.Type != StateTypeId.Checkers)
                throw new Exception("Invalid checkers seed");

            _checkersSeed = checkersSeed;
        }

        /// <summary>
        /// Subscribes to the events raised by the checkerboard.
        /// </summary>
        private void ConnectToBoard()
        {
            _board.FieldHovered += OnFieldHovered;
            _board.FieldSelected += OnFieldSelected;
            _board.MouseWheelChanged += OnMouseWheelChanged;
        }

        /// <summary>
        /// Unsubscribes from the events raised by checkerboard.
        /// </summary>
        private void DisconnectFromBoard()
        {
            _board.FieldHovered -= OnFieldHovered;
            _board.FieldSelected -= OnFieldSelected;
            _board.MouseWheelChanged -= OnMouseWheelChanged;
        }

        /// <summary>
        /// Position of the board field under the mouse pointer (if valid).
        /// </summary>
        private PiecePosition FieldUnderMousePointer => _board.FieldUnderMousePointer;

        private OptionSelector _optionSelector = null;

        /// <summary>
        /// MouseMove handler.
        /// </summary>
        private void OnFieldHovered(PiecePosition pos)
        {
            _optionSelector = null;
            ClearMovePreview();
            UpdateMarkers();

            if (FieldUnderMousePointer.IsValid && SelectedField.IsValid && _userMoveRequest != null)
            {
                var moves =
                    GetPossibleMovesEndingAt(GetPossibleMovesStartingFrom(SelectedField), FieldUnderMousePointer);

                if (moves.Length != 0)
                {
                    _optionSelector = new OptionSelector(moves.Select(x => x.FinalPieceRank).ToList(),
                        moves.Select(x => x.Index).ToList());
                    UpdateMovePreview();
                }
            }
        }

        /// <summary>
        /// Removes "move preview" element from the canvas.
        /// </summary>
        private void ClearMovePreview()
        {
            _board.UpdatePreviewPiece(0);
        }

        /// <summary>
        /// Updates "move preview" visualization.
        /// </summary>
        private void UpdateMovePreview()
        {
            if (_optionSelector == null)
                throw new InvalidOperationException("Invalid input data");

            var selectedOption = _optionSelector.SelectedOption;
            var selectedPiece = selectedOption != 0 ? selectedOption : _state.Data[SelectedField.LinearPosition];
            _board.UpdatePreviewPiece(selectedPiece);
        }

        /// <summary>
        /// MouseWheel handler.
        /// </summary>
        private void OnMouseWheelChanged(int delta, PiecePosition pos)
        {
            if (_optionSelector == null || delta == 0)
                return;

            if (delta > 0)
                _optionSelector.SelectNextMove();
            else
                _optionSelector.SelectPreviousMove();

            UpdateMovePreview();
        }

        /// <summary>
        /// Resets all the user input.
        /// </summary>
        private void ResetUserInput()
        {
            _userMoveRequest = null;
            _optionSelector = null;
            _board.ResetSelectedField();
        }

        /// <summary>
        /// Mouse down handler.
        /// </summary>
        private void OnFieldSelected(PiecePosition pos)
        {
            if (_userMoveRequest != null && _optionSelector != null)
            {
                _userMoveRequest.UserMoveResult.SetResult(_optionSelector.SelectedOptionId);
                ResetUserInput();
            }

            UpdateMarkers();
        }

        /// <summary>
        /// Property changed event
        /// </summary>
        public event PropertyChangedEventHandler PropertyChanged;

        /// <summary>
        /// Notifies about change of the corresponding property
        /// </summary>
        protected void OnPropertyChanged(string name) =>
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));

        /// <summary>
        /// Property setter with notification
        /// </summary>
        protected bool SetField<T>(ref T field, T value, [CallerMemberName] string propertyName = null)
        {
            if (EqualityComparer<T>.Default.Equals(field, value)) return false;
            field = value;
            OnPropertyChanged(propertyName);
            return true;
        }
    }
}
