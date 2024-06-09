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
using Monitor.Utils;
using static Monitor.Dll.DllWrapper;

namespace Monitor.UI.Board
{
    /// <summary>
    /// Functionality to handle user interface of the checkers game
    /// </summary>
    class GameEngine : IGameEngine
    {
        /// <summary>
        /// Types of agents
        /// </summary>
        public enum AgentType
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
        /// Collection of evaluated moves.
        /// </summary>
        public ObservableCollection<EvaluatedMove> EvaluatedOptions { get; } = new ObservableCollection<EvaluatedMove>();

        private EvaluatedMove _selectedOption;

        /// <summary>
        /// Pointer to the selected evaluated move from the collection above.
        /// </summary>
        public EvaluatedMove SelectedOption
        {
            get => _selectedOption;
            set {
                if (SetField(ref _selectedOption, value))
                {
                    _state.AddMoveTrace(_selectedOption?.Move.SubMoves, afterMoveDone: false);
                    _board.UpdateState(_state);
                }
            }
        }

        /// <summary>
        /// Updates collection of evaluated moves according to the given date.
        /// </summary>
        private void UpdateEvaluatedMoves(Move[] moves, double[] values)
        {
            EvaluatedOptions.Clear();
            if (moves == null || values == null)
                return;

            if (moves.Length != values.Length)
                throw new Exception("Inconsistent data");
            
            var optionsSorted = moves.Select((m, i) => new EvaluatedMove(m, values[i], i)).ToArray();
            optionsSorted = optionsSorted.OrderByDescending(x => x.Value).ToArray();

            foreach (var option in optionsSorted)
                EvaluatedOptions.Add(option);

            SelectedOption = EvaluatedOptions.FirstOrDefault();
        }

        private TaskCompletionSource<bool> _optionEvaluationTask;
        private bool _inspectOptions;

        /// <summary>
        /// Flag enabling the functionality allowing user to inspect moves and their values before actual move is taken.
        /// </summary>
        public bool InspectOptions
        {
            get => _inspectOptions;
            set
            {
                if (SetField(ref _inspectOptions, value))
                {
                    if (_inspectOptions)
                        _optionEvaluationTask = new TaskCompletionSource<bool>();
                    else
                    {
                        _optionEvaluationTask.TrySetResult(true);
                        _optionEvaluationTask = null;
                    }
                }
            }
        }
        
        /// <summary>
        /// Method to resume game flow in case it was interrupted by the option evaluation procedure.
        /// </summary>
        public void CompleteOptionSelection()
        {
            if (_optionEvaluationTask == null) return;
            _optionEvaluationTask.TrySetResult(true);
            _optionEvaluationTask = new TaskCompletionSource<bool>();
        }

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
            private set
            {
                if (SetField(ref _isPlaying, value))
                    OnPropertyChanged(nameof(CanCancelPlaying));
            }
        }

        private bool _inspectionOngoing;

        /// <summary>
        /// Flag indicating thea move inspection is ongoing at the moment.
        /// </summary>
        public bool InspectionOngoing
        {
            get => _inspectionOngoing;
            private set
            {
                if (SetField(ref _inspectionOngoing, value))
                    OnPropertyChanged(nameof(CanCancelPlaying));
            }
        }

        /// <summary>
        /// Flag indicating that at the moment it is possible to cancel playing.
        /// </summary>
        public bool CanCancelPlaying => IsPlaying && !InspectionOngoing;

        private bool _tdlIsPlayng;

        /// <summary>
        /// If "true" if it is a turn for TD(lambda) agent to make a move.
        /// </summary>
        public bool TdlAgentIsPlaying
        {
            get => _tdlIsPlayng;
            private set => SetField(ref _tdlIsPlayng, value);
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

                CompleteOptionSelection();
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
                if (SetField(ref _inactiveAgent, value))
                {
                    OnPropertyChanged(nameof(CanEditAgent));
                    TdlAgentIsPlaying = _inactiveAgent is InteractiveAgent;
                }
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
        public bool Play(AgentType agentTypeWhite, AgentType agentTypeBlack, int episodes)
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

                        var optionEvaluator = agentBlack as IOptionEvaluator ?? agentWhite as IOptionEvaluator;

                        var timePoint = DateTime.Now;
                        var res = DllWrapper.Play(
                            agentWhite.Ptr, agentBlack.Ptr, episodes, stateSeed.Ptr,
                            (statePtr, subMoves, subMovesCount, agentToMovePtr) =>
                            {
                                var state = IStateGetState(statePtr);

                                if (state.Data.Length != Checkerboard.Fields || state.Type != stateType)
                                    throw new Exception("Incompatible data");

                                var timePointNext = DateTime.Now;
                                var elapsedTime = timePointNext - timePoint;
                                PreviousMoveTime = elapsedTime.ToString(@"hh\:mm\:ss");
                                timePoint = timePointNext;

                                state.AddMoveTrace(subMoves);
                                _state = state;
                                var tdlPlaying = _uiThreadDispatcher.Invoke(() =>
                                {
                                    _board.UpdateState(state);
                                    InactiveAgent = agentToMovePtr == agentWhite.Ptr ? agentBlack : agentWhite;
                                    return TdlAgentIsPlaying;
                                });

                                var evaluationTask = _uiThreadDispatcher.Invoke(() => _optionEvaluationTask);
                                if (optionEvaluator != null && evaluationTask != null &&
                                    !evaluationTask.Task.IsCompleted && !_uiThreadDispatcher.Invoke(() =>
                                        _playTaskCancellation.IsCancellationRequested))
                                {
                                    var options = IStateGetMoves(statePtr);
                                    
                                    if (options.Moves.Length == 0)
                                        return;
                                    
                                    var optValues = optionEvaluator.EvaluateOptions(statePtr);
                                    _uiThreadDispatcher.Invoke(() =>
                                    {
                                        UpdateEvaluatedMoves(options.Moves, optValues);
                                        _board.UpdateMarkers(null);
                                        InspectionOngoing = true;
                                        TdlAgentIsPlaying = false;
                                    });

                                    evaluationTask.Task.Wait();

                                    _uiThreadDispatcher.Invoke(() =>
                                    {
                                        UpdateEvaluatedMoves(null, null);
                                        InspectionOngoing = false;
                                        TdlAgentIsPlaying = tdlPlaying;
                                    });
                                }
                            },
                            (whiteWon, blackWon, totalGamers) =>
                            {
                                _uiThreadDispatcher.Invoke(() =>
                                {
                                    InactiveAgent = null;
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
                InfoEvent?.Invoke(null);
                ResetUserInput();
                EvaluatedOptions.Clear();
                DisconnectFromBoard();
                _board.Clear();
                IsPlaying = false;
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
        public GameEngine(ICheckerBoard board, Dispatcher uiThreadDispatcher, IStateSeed chessSeed, IStateSeed checkersSeed)
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
