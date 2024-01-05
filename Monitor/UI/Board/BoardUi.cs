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
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Shapes;
using System.Windows.Threading;
using Monitor.Agents;
using Monitor.DataStructures;
using Monitor.Dll;
using Monitor.UI.Board.PieceControllers;

namespace Monitor.UI.Board
{
    /// <summary>
    /// Functionality to handle user interface of the checkers game
    /// </summary>
    class BoardUi : ITwoPlayerGameUi
    {
        private double _boardSide;
        private double _fieldSide;
        private double _topLeftX;
        private double _topLeftY;
        private double _canvasHeight = -1;
        private double _canvasWidth = -1;

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

        private IPieceController _pieceController;

        /// <summary>
        /// Provides means to cancel ongoing playing
        /// </summary>
        private CancellationTokenSource _playTaskCancellation;

        private Task _playTask;

        /// <summary>
        /// The current state
        /// </summary>
        private int[] _state;

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
            }
            else
                throw new Exception("Invalid type of inactive agent. Can't edit.");
        }

        /// <summary>
        /// Flag indicating if agent editing is possible
        /// </summary>
        public bool CanEditAgent => InactiveAgent is TdLambdaAgent;

        /// <summary>
        /// Adds extra symbols to the state to trace the series of given sub-moves (representing a move)
        /// </summary>
        private void AddMoveTrace(int[] state, SubMove[] subMoves)
        {
            if (subMoves == null)
                return;

            var pieceId = state[subMoves.Last().End.LinearPosition];
            foreach (var move in subMoves)
            {
                if (move.Capture.IsValid)
                    state[move.Capture.LinearPosition] = _pieceController.GetCapturedPieceId(white: pieceId > 0);

                state[move.Start.LinearPosition] = _pieceController.GetPieceTraceId(pieceId);
            }
        }

        /// <summary>
        /// Returns instance of piece controller compatible with the given state type ID.
        /// </summary>
        IPieceController ResolvePieceController(IAgent agent0, IAgent agent1)
        {
            if (!DllWrapper.CanPlay(agent0.Ptr, agent1.Ptr, out var stateTypeId))
                return null;

            switch (stateTypeId)
            {
                case DllWrapper.StateTypeId.Checkers : return new CheckersPieceController();
                case DllWrapper.StateTypeId.Chess: return new ChessPieceController();
                default: return null;
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

                        _pieceController = ResolvePieceController(agentWhite, agentBlack);

                        var timePoint = DateTime.Now;
                        var res = DllWrapper.Play(
                            agentWhite.Ptr, agentBlack.Ptr, episodes, _pieceController.StateTypeId,
                            (state, size, subMoves, subMovesCount, agentToMovePtr) =>
                            {
                                if (size != Checkerboard.Fields)
                                    throw new Exception("Invalid size of the state");

                                var timePointNext = DateTime.Now;
                                var elapsedTime = timePointNext - timePoint;
                                PreviousMoveTime = elapsedTime.ToString(@"hh\:mm\:ss");
                                timePoint = timePointNext;

                                InactiveAgent = agentToMovePtr == agentWhite.Ptr ? agentBlack : agentWhite;

                                AddMoveTrace(state, subMoves);
                                _state = state;
                                _uiThreadDispatcher.Invoke(Draw, DispatcherPriority.ContextIdle);
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

                                    MessageBox.Show(Application.Current.MainWindow,
                                        GetConclusionMessage(whiteWon, blackWon), "Episode is over",
                                        MessageBoxButton.OK, MessageBoxImage.Information);

                                    _state = null;
                                    InfoEvent?.Invoke(new List<string>()
                                    {
                                        "Whites Won: " + whiteWinsCounter,
                                        "Blacks Won: " + blackWinsCounter,
                                        "Stalemates: " + staleMatesCounter,
                                        "Total Games: " + totalGamers,
                                    });
                                    Draw();
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
            }, TaskScheduler.FromCurrentSynchronizationContext());
            _playTask.Start();

            return true;
        }

        private readonly Canvas _canvas = null;

        /// <summary>
        /// For the given mouse position on the canvas returns coordinates of the corresponding checkers field
        /// or (-1, -1), if the given point does not belong to any valid field
        /// </summary>
        PiecePosition CanvasCoordinateToFieldPosition(Point pt)
        {
            var colIdFloat = (pt.X - _topLeftX) / _fieldSide;
            if (colIdFloat < 0 || colIdFloat > Checkerboard.Columns)
                return PiecePosition.Invalid();

            var rowIdFloat = (pt.Y - _topLeftY) / _fieldSide;
            if (rowIdFloat < 0 || rowIdFloat > Checkerboard.Rows)
                return PiecePosition.Invalid();

            return new PiecePosition() { Row = (int)rowIdFloat, Col = (int)colIdFloat };
        }

        private PiecePosition _selectedField = PiecePosition.Invalid();

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
        /// Returns coordinates of the top left corner of the corresponding field on the canvas
        /// </summary>
        /// <param name="colId">Field column</param>
        /// <param name="rowId">Field row</param>
        /// <param name="offset">Offset that should be added to booth coordinates of the returned corner</param>
        /// <returns></returns>
        private Point GetTopLeftCorner(long colId, long rowId, double offset)
        {
            return new Point(colId * _fieldSide + _topLeftX + offset, rowId * _fieldSide + _topLeftY + offset);
        }

        /// <summary>
        /// Returns coordinates of the top left corner of the corresponding field on the canvas.
        /// </summary>
        private Point GetTopLeftCorner(PiecePosition fieldCoordinate)
        {
            return GetTopLeftCorner(fieldCoordinate.Col, fieldCoordinate.Row, 0);
        }

        private readonly IList<UIElement> _stateElements = new List<UIElement>();

        /// <summary>
        /// Removes state visualization elements from the canvas.
        /// </summary>
        private void ClearStateElements()
        {
            foreach (var element in _stateElements)
                _canvas.Children.Remove(element);
            
            _stateElements.Clear();
        }
        
        /// <summary>
        /// Updates visualization elements the current state.
        /// </summary>
        private void UpdateStateElements()
        {
            ClearStateElements();

            if (_state == null)
                return;

            for (var stateItemId = 0; stateItemId < _state.Length; stateItemId++)
            {
                var pieceId = _state[stateItemId];

                var position = PiecePosition.GetPosition(stateItemId);
                var topLeft = GetTopLeftCorner(position.Col, position.Row, 0);
                foreach (var element in _pieceController.CreatePieceElements(topLeft, _fieldSide, pieceId))
                    _stateElements.Add(element);
            }

            foreach (var element in _stateElements)
                _canvas.Children.Add(element);
        }

        /// <summary>
        /// Updates board size parameters from the current size of the canvas control.
        /// Returns "true" if any of the board dimensions has been updated.
        /// </summary>
        bool UpdateBoardDimensions()
        {
            if (Math.Abs(_canvasHeight - _canvas.ActualHeight) < 1e-10 &&
                Math.Abs(_canvasWidth - _canvas.ActualWidth) < 1e-10)
                return false;

            _canvasHeight = _canvas.ActualHeight;
            _canvasWidth = _canvas.ActualWidth;

            _boardSide = Math.Max(0, Math.Min(_canvasHeight, _canvasWidth) - 10.0);
            _fieldSide = _boardSide / Checkerboard.Rows;
            _topLeftX = (_canvasWidth - _boardSide) / 2;
            _topLeftY = (_canvasHeight - _boardSide) / 2;

            return true;
        }

        private readonly IList<UIElement> _boardElements = new List<UIElement>();

        /// <summary>
        /// Removes board elements from canvas.
        /// </summary>
        private void ClearBoardElements()
        {
            foreach (var element in _boardElements)
                _canvas.Children.Remove(element);
            
            _boardElements.Clear();
        }
        
        /// <summary>
        /// Updates checkers board visualization elements.
        /// </summary>
        private void UpdateBoardElements()
        {
            if (!UpdateBoardDimensions())
                return;

            ClearBoardElements();

            for (var rowId = 0; rowId < Checkerboard.Rows; rowId++)
                for (var colId = 0; colId < Checkerboard.Columns; colId++)
                {
                    var topLeft = GetTopLeftCorner(colId, rowId, 0);
                    var isDarkField = rowId % 2 == 0 ? colId % 2 == 1 : colId % 2 == 0;
                    _boardElements.Add(CanvasDrawingUtils.CreateShape<Rectangle>(topLeft.X, topLeft.Y, _fieldSide, _fieldSide,
                        Brushes.Black, isDarkField ? Brushes.SaddleBrown : Brushes.BurlyWood));
                }

            foreach (var element in _boardElements)
                _canvas.Children.Add(element);
        }

        private readonly IList<UIElement> _markers = new List<UIElement>();

        /// <summary>
        /// Clears guiding markers on the canvas.
        /// </summary>
        void ClearMarkers()
        {
            foreach (var marker in _markers)
                _canvas.Children.Remove(marker);

            _markers.Clear();
        }

        /// <summary>
        /// Updates guiding markers on the board.
        /// </summary>
        void UpdateMarkers()
        {
            ClearMarkers();

            if (_userMoveRequest != null)
            {
                foreach (var move in _userMoveRequest.PossibleMoves)
                {
                    var start = move.SubMoves[0].Start;
                    var topLeftCorner = GetTopLeftCorner(start.Col, start.Row, 0);
                    _markers.Add(CanvasDrawingUtils.CreateShape<Rectangle>(topLeftCorner.X, topLeftCorner.Y, _fieldSide, _fieldSide,
                        _selectedField.IsEqualTo(start) ? Brushes.Red : Brushes.BlueViolet, null));
                }

                var possibleMovesForCurrentPosition = GetPossibleMovesStartingFrom(_selectedField);
                foreach (var move in possibleMovesForCurrentPosition)
                {
                    var end = move.SubMoves.Last().End;
                    var topLeftCorner = GetTopLeftCorner(end.Col, end.Row, 0);
                    _markers.Add(CanvasDrawingUtils.CreateShape<Rectangle>(topLeftCorner.X, topLeftCorner.Y, _fieldSide, _fieldSide,
                        Brushes.GreenYellow, null));
                }

                foreach (var marker in _markers)
                    _canvas.Children.Add(marker);
            }
        }

        /// <summary>
        /// General method to update UI
        /// </summary>
        public void Draw()
        {
            UpdateBoardElements();
            UpdateStateElements();
            UpdateMarkers();
        }

        /// <summary>
        /// Constructor
        /// </summary>
        public BoardUi(Canvas canvas, Dispatcher uiThreadDispatcher)
        {
            _uiThreadDispatcher = uiThreadDispatcher ?? throw new Exception("Dispatcher must be not null");
            _canvas = canvas ?? throw new Exception("Invalid input");
            _canvas.MouseDown += CanvasOnMouseDown;
            _canvas.MouseMove += CanvasOnMouseMove;
            _canvas.MouseWheel += CanvasOnMouseWheel;
            _canvas.CacheMode = new BitmapCache();
            Draw();
        }

        /// <summary>
        /// Position of the board field under the mouse pointer (if valid).
        /// </summary>
        PiecePosition _fieldUnderMousePointer = PiecePosition.Invalid();

        /// <summary>
        /// Functionality to facilitate selection of moves (especially
        /// in case there are multiple moves with the same end position).
        /// </summary>
        private class MoveSelector
        {
            private readonly IList<Move> _moves;
            private int _selectedMoveId = 0;

            /// <summary>
            /// Constructor.
            /// </summary>
            public MoveSelector(IList<Move> moves)
            {
                if (moves == null || moves.Count == 0)
                    throw new ArgumentException("Invalid collection of moves.");
                    
                _moves = moves.ToArray();
            }

            /// <summary>
            /// Selects next move in the collection of moves.
            /// </summary>
            public void SelectNextMove()
            {
                _selectedMoveId = (_selectedMoveId + 1) % _moves.Count;
            }

            /// <summary>
            /// Selects previous move in the collection of moves.
            /// </summary>
            public void SelectPreviousMove()
            {
                _selectedMoveId = (_selectedMoveId + _moves.Count - 1) % _moves.Count;
            }

            /// <summary>
            /// Returns the selected move.
            /// </summary>
            public Move SelectedMove => _moves[_selectedMoveId];
        }

        private MoveSelector _moveSelector = null;

        /// <summary>
        /// MouseMove handler.
        /// </summary>
        private void CanvasOnMouseMove(object sender, MouseEventArgs e)
        {
            var fieldPositions = CanvasCoordinateToFieldPosition(e.GetPosition(_canvas));

            if (_fieldUnderMousePointer.IsEqualTo(fieldPositions))
                return;

            _fieldUnderMousePointer = fieldPositions;
            _moveSelector = null;
            ClearMovePreview();

            if (_fieldUnderMousePointer.IsValid && _selectedField.IsValid && _userMoveRequest != null)
            {
                var possibleMovesForCurrentPosition =
                    GetPossibleMovesEndingAt(GetPossibleMovesStartingFrom(_selectedField), _fieldUnderMousePointer);

                if (possibleMovesForCurrentPosition.Length != 0)
                {
                    _moveSelector = new MoveSelector(possibleMovesForCurrentPosition);
                    UpdateMovePreview();
                }
            }
        }

        private readonly IList<UIElement> _previewElements = new List<UIElement>();

        /// <summary>
        /// Removes "move preview" element from the canvas.
        /// </summary>
        private void ClearMovePreview()
        {
            foreach (var element in _previewElements)
                _canvas.Children.Remove(element);

            _previewElements.Clear();
        }

        /// <summary>
        /// Updates "move preview" visualization.
        /// </summary>
        private void UpdateMovePreview()
        {
            if (_moveSelector == null)
                throw new InvalidOperationException("Invalid input data");

            ClearMovePreview();

            var selectedMove = _moveSelector.SelectedMove;
            var selectedPiece = selectedMove.FinalPieceRank != 0 ?
                selectedMove.FinalPieceRank : _state[_selectedField.LinearPosition];
            var tracePieceId = _pieceController.GetPieceTraceId(selectedPiece);
            var topLeftCorner = GetTopLeftCorner(_fieldUnderMousePointer);
            foreach (var element in _pieceController.CreatePieceElements(topLeftCorner, _fieldSide, tracePieceId))
                _previewElements.Add(element);

            foreach (var element in _previewElements)
                _canvas.Children.Add(element);
        }

        /// <summary>
        /// MouseWheel handler.
        /// </summary>
        private void CanvasOnMouseWheel(object sender, MouseWheelEventArgs e)
        {
            if (_moveSelector == null || e.Delta == 0)
                return;

            if (e.Delta > 0)
                _moveSelector.SelectNextMove();
            else
                _moveSelector.SelectPreviousMove();
            
            UpdateMovePreview();
        }

        /// <summary>
        /// Resets all the user input.
        /// </summary>
        private void ResetUserInput()
        {
            _userMoveRequest = null;
            _moveSelector = null;
            _selectedField = PiecePosition.Invalid();
        }

        /// <summary>
        /// Event handler
        /// </summary>
        private void CanvasOnMouseDown(object sender, MouseButtonEventArgs e)
        {
            _selectedField = CanvasCoordinateToFieldPosition(e.GetPosition(_canvas));

            if (_userMoveRequest != null && _moveSelector != null)
            {
                var moveId = _moveSelector.SelectedMove.Index;
                _userMoveRequest.UserMoveResult.SetResult(moveId);
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
        protected void OnPropertyChanged(string name)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(name));
            }
        }

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
