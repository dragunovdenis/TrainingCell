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
using System.Runtime.InteropServices;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Shapes;
using System.Windows.Threading;
using Microsoft.Win32;

namespace Monitor.Checkers.UI
{
    /// <summary>
    /// Data transferring structure to contain checkers piece position
    /// Must be kept in sync with the corresponding twin structure on the c++ side
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    internal struct CheckersPiecePosition
    {
        /// <summary>
        /// Index of a row
        /// </summary>
        public long Row;

        /// <summary>
        /// Index of a column
        /// </summary>
        public long Col;

        /// <summary>
        /// Returns invalid position
        /// </summary>
        /// <returns></returns>
        public static CheckersPiecePosition Invalid()
        {
            return new CheckersPiecePosition() { Row = -1, Col = -1 };
        }

        /// <summary>
        /// Returns true if the current instance represents a valid checkers piece position
        /// </summary>
        public bool IsValid()
        {
            return Row >= 0 && Row < BoardUi.CheckerRows && Col >= 0 && Col < BoardUi.CheckerRows;
        }

        /// <summary>
        /// Returns "true" if the current position is equal to the given one
        /// </summary>
        public bool IsEqualTo(CheckersPiecePosition pos)
        {
            return Row == pos.Row && Col == pos.Col;
        }
    }

    /// <summary>
    /// Data transferring structure to contain checkers sub-move information
    /// Must be kept in sync with the corresponding twin structure on the c++ side
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    internal struct CheckersSubMove
    {
        /// <summary>
        /// Position on the checkers board where the sub-move started
        /// </summary>
        public CheckersPiecePosition Start;

        /// <summary>
        /// Position on the checkers board where the sub-move ended
        /// </summary>
        public CheckersPiecePosition End;

        /// <summary>
        /// Position on the checkers board where an opponent piece was captured during the sub-move
        /// </summary>
        public CheckersPiecePosition Capture;
    }

    /// <summary>
    /// Representation of a checkers move (collection of sub-moves)
    /// </summary>
    internal struct CheckersMove
    {
        /// <summary>
        /// Collection of sub-moves
        /// </summary>
        public CheckersSubMove[] SubMoves;

        /// <summary>
        /// Index of the move
        /// </summary>
        public int Index;
    };

    /// <summary>
    /// Functionality to handle user interface of the checkers game
    /// </summary>
    class BoardUi : ITwoPlayerGameUi
    {
        public const int CheckerRows = 8;
        public const int CheckerColumns = 8;
        private const int BlackFieldsInRow = 4;
        private const int StateSize = CheckerRows * BlackFieldsInRow;

        double _boardSide;
        double _fieldSide;
        double _topLeftX;
        double _topLeftY;

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
        private int[] _state;

        /// <summary>
        /// Data structure that facilitates user move processing
        /// </summary>
        private class MoveRequest
        {
            /// <summary>
            /// Set of possible moves in the current state
            /// </summary>
            public readonly CheckersMove[] PossibleMoves;
            /// <summary>
            /// Channel to provide user input to the thread that is running training
            /// </summary>
            public readonly TaskCompletionSource<int> UserMoveResult = new TaskCompletionSource<int>();

            /// <summary>
            /// Constructor
            /// </summary>
            public MoveRequest(CheckersMove[] possibleMoves)
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
        private Task<int> RequestUserMove(int[] state, CheckersMove[] possibleMoves)
        {
            if (_state == null) _state = state;
            _userMoveRequest = new MoveRequest(possibleMoves);
            Draw();
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
                        var movePromise = _uiThreadDispatcher.Invoke(() => RequestUserMove(state, moves));
                        return movePromise.Result;
                    }, (state, result) => { }, playWhites);
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
                _isPlaying = value;
                OnPropertyChanged(nameof(IsPlaying));
            }
        }

        /// <summary>
        /// Handles loading of an agent to play for "whites" (so that user will play for "blacks")
        /// and starts the game
        /// </summary>
        public void LoadWhiteAgent()
        {
            Play(BoardUi.AgentType.AgentPack, BoardUi.AgentType.Interactive, 100);
        }

        /// <summary>
        /// Handles loading of an agent to play for "blacks" (so that user will play for "whites")
        /// and starts the game
        /// </summary>
        public void LoadBlackAgent()
        {
            Play(BoardUi.AgentType.Interactive, BoardUi.AgentType.AgentPack, 100);
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

        /// <summary>
        /// Starts checkers game with the agents of the two given types (asynchronously) and returns immediately
        /// Returns true if the previous playing task is complete and the current one is successfully started
        /// </summary>
        /// <param name="agentTypeWhite"></param>
        /// <param name="agentTypeBlack"></param>
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

                        DllWrapper.RunCheckersTraining(
                            agentWhite.Ptr, agentBlack.Ptr, episodes,
                            (state, size, subMoves, subMovesCount) =>
                            {
                                if (movePause > 0) Thread.Sleep(movePause);

                                _uiThreadDispatcher.Invoke(() =>
                                {
                                    _state = state;
                                    Draw();
                                });
                            },
                            (whiteWins, blackWins, totalGamers) =>
                            {
                                _uiThreadDispatcher.Invoke(() =>
                                {
                                    _state = null;
                                    InfoEvent?.Invoke(new List<string>()
                                    {
                                        "Whites Won: " + whiteWins,
                                        "Blacks Won: " + blackWins,
                                        "Total Games: " + totalGamers,
                                    });
                                    Draw();
                                });
                            }, () => _playTaskCancellation.IsCancellationRequested);
                    }
                }
            });
            _playTask.ContinueWith((task) =>
            {
                _playTask = null;
                _playTaskCancellation = null;
                IsPlaying = false;
                InfoEvent ?.Invoke(null);
            }, TaskScheduler.FromCurrentSynchronizationContext());
            _playTask.Start();

            return true;
        }

        private readonly Canvas _canvas = null;

        /// <summary>
        /// Draws a "shape" (ellipse, rectangle...) with the given position and radius on the given canvas
        /// </summary>
        private static void DrawShape<S>(double x, double y, double width, double height,
            Brush borderColor, Brush fillColor, Canvas cv)
            where S : Shape, new()
        {
            var shape = new S()
            {
                Width = width,
                Height = height,
                Stroke = borderColor,
                Fill = fillColor,
                StrokeThickness = fillColor == null ? 4 : 1,
            };

            cv.Children.Add(shape);

            shape.SetValue(Canvas.LeftProperty, x);
            shape.SetValue(Canvas.TopProperty, y);
        }

        /// <summary>
        /// Outputs row and column representation for the state item given by its ID
        /// </summary>
        void StateItemIdToRowAndCol(int stateItemId, out int rowId, out int colId)
        {
            rowId = stateItemId / BlackFieldsInRow;
            colId = 2 * (stateItemId % BlackFieldsInRow);

            if (rowId % 2 == 0)
                colId++;
        }

        /// <summary>
        /// Converts index of a checkers piece (that we receive in the "state" collection) into the fill color that should be
        /// used when drawing the piece on the board
        /// </summary>
        Brush PieceIdToColor(int pieceId)
        {
            switch (pieceId)
            {
                case 1: return Brushes.Wheat;
                case 2: return Brushes.Wheat;
                case 3: return Brushes.Wheat;
                case 4: return null;
                case -1: return Brushes.Black;
                case -2: return Brushes.Black;
                case -3: return Brushes.Black;
                case -4: return null;
                default: throw new Exception("Unknown piece identifier");
            }
        }

        /// <summary>
        /// Returns "true" is the given identifier represents a King piece
        /// </summary>
        bool IsKingPiece(int pieceId)
        {
            return pieceId == 2 || pieceId == -2;
        }

        /// <summary>
        /// Returns "true" if the given identifier represents a Man piece
        /// </summary>
        bool IsManPiece(int pieceId)
        {
            return pieceId == 1 || pieceId == -1;
        }

        /// <summary>
        /// For the given mouse position on the canvas returns coordinates of the corresponding checkers field
        /// or (-1, -1), if the given point does not belong to any valid field
        /// </summary>
        CheckersPiecePosition CanvasCoordinateToFieldPosition(Point pt)
        {
            var result = new CheckersPiecePosition() { Col = -1, Row = -1 };

            var colIdFloat = (pt.X - _topLeftX) / _fieldSide;
            if (colIdFloat < 0 || colIdFloat > CheckerColumns)
                return result;

            var rowIdFloat = (pt.Y - _topLeftY) / _fieldSide;
            if (rowIdFloat < 0 || rowIdFloat > CheckerRows)
                return result;

            return new CheckersPiecePosition() { Row = (int)rowIdFloat, Col = (int)colIdFloat };
        }

        private CheckersPiecePosition _selectedField = CheckersPiecePosition.Invalid();

        /// <summary>
        /// Returns sub-set of possible moves that start from the given position
        /// </summary>
        private CheckersMove[] GetPossibleMovesStartingFrom(CheckersPiecePosition startPos)
        {
            return GetPossibleMovesStartingFrom(_userMoveRequest?.PossibleMoves, startPos);
        }

        /// <summary>
        /// Returns sub-set of possible moves that start from the given position
        /// </summary>
        private static CheckersMove[] GetPossibleMovesStartingFrom(CheckersMove[]  possibleMoves, CheckersPiecePosition startPos)
        {
            if (possibleMoves == null || possibleMoves.Length == 0)
                return Array.Empty<CheckersMove>();

            return possibleMoves.Where(x => x.SubMoves[0].Start.IsEqualTo(startPos)).ToArray();
        }

        /// <summary>
        /// Returns sub-set of possible moves that end at the given position
        /// </summary>
        private static CheckersMove[] GetPossibleMovesEndingAt(CheckersMove[] possibleMoves, CheckersPiecePosition endPos)
        {
            if (possibleMoves == null || possibleMoves.Length == 0)
                return Array.Empty<CheckersMove>();

            return possibleMoves.Where(x => x.SubMoves.Last().End.IsEqualTo(endPos)).ToArray();
        }

        /// <summary>
        /// Returns coordinates of the top left corner of the corresponding field on the canvas
        /// </summary>
        /// <param name="colId">Field column</param>
        /// <param name="rowId">Field row</param>
        /// <param name="offset">Offset that should added to booth coordinates of the returned corner</param>
        /// <returns></returns>
        Point GetTopLeftCorner(long colId, long rowId, double offset)
        {
            return new Point(colId * _fieldSide + _topLeftX + offset, rowId * _fieldSide + _topLeftY + offset);
        }

        /// <summary>
        /// Draws the given state
        /// </summary>
        private void DrawState(int[] state)
        {
            if (state == null || state.Length != StateSize)
                throw new Exception("Invalid state parameter");

            UpdateBoardDimensions();

            var shrink = 0.1 * _fieldSide;
            var fieldSizeShrunk = _fieldSide - 2 * shrink;

            for (var stateItemId = 0; stateItemId < state.Length; stateItemId++)
            {
                var pieceId = state[stateItemId];

                if (pieceId == 0)
                    continue;

                StateItemIdToRowAndCol(stateItemId, out var rowId, out var colId);
                var topLeft = GetTopLeftCorner(colId, rowId, shrink);

                DrawShape<Ellipse>(topLeft.X, topLeft.Y, fieldSizeShrunk, fieldSizeShrunk,
                    Brushes.BlueViolet, PieceIdToColor(pieceId), _canvas);

                if (!IsKingPiece(pieceId) && ! IsManPiece(pieceId))
                    continue;

                var shift = fieldSizeShrunk / 4;

                DrawShape<Ellipse>(topLeft.X + shift, topLeft.Y + shift,
                    fieldSizeShrunk / 2, fieldSizeShrunk / 2,
                    Brushes.BlueViolet, IsKingPiece(pieceId) ? Brushes.Red : Brushes.Green, _canvas);
            }
        }

        /// <summary>
        /// Updates board size parameters from the current size of the canvas constrol
        /// </summary>
        void UpdateBoardDimensions()
        {
            _boardSide = Math.Max(0, Math.Min(_canvas.ActualHeight, _canvas.ActualWidth) - 10.0);
            _fieldSide = _boardSide / CheckerRows;
            _topLeftX = (_canvas.ActualWidth - _boardSide) / 2;
            _topLeftY = (_canvas.ActualHeight - _boardSide) / 2;
        }

        /// <summary>
        /// Draws checkers board
        /// </summary>
        private void DrawBoard()
        {
            _canvas.Children.Clear();

            UpdateBoardDimensions();

            for (var rowId = 0; rowId < CheckerRows; rowId++)
            for (var colId = 0; colId < CheckerColumns; colId++)
            {
                var topLeft = GetTopLeftCorner(colId, rowId, 0);
                var isDarkField = (rowId % 2 == 0) ? (colId % 2 == 1) : (colId % 2 == 0);
                DrawShape<Rectangle>(topLeft.X, topLeft.Y, _fieldSide, _fieldSide,
                    Brushes.Black, isDarkField ? Brushes.SaddleBrown : Brushes.BurlyWood, _canvas);
            }

            if (_userMoveRequest != null)
            {
                foreach (var move in _userMoveRequest.PossibleMoves)
                {
                    var start = move.SubMoves[0].Start;
                    var topLeftStart = GetTopLeftCorner(start.Col, start.Row, 0);
                    DrawShape<Rectangle>(topLeftStart.X, topLeftStart.Y, _fieldSide, _fieldSide,
                        _selectedField.IsEqualTo(start) ? Brushes.Red : Brushes.Yellow, null, _canvas);
                }

                var possibleMovesForCurrentPosition = GetPossibleMovesStartingFrom(_selectedField);
                foreach (var move in possibleMovesForCurrentPosition)
                {
                    foreach (var subMove in move.SubMoves)
                    {
                        var startCenter = GetTopLeftCorner(subMove.Start.Col, subMove.Start.Row, _fieldSide / 2);
                        var endCenter = GetTopLeftCorner(subMove.End.Col, subMove.End.Row, _fieldSide / 2);

                        var line = new Line() { X1 = startCenter.X, Y1 = startCenter.Y,
                            X2 = endCenter.X, Y2 = endCenter.Y, Stroke = Brushes.Black, StrokeThickness = 1 };
                        _canvas.Children.Add(line);
                    }

                    var end = move.SubMoves.Last().End;
                    var topLeftEnd = GetTopLeftCorner(end.Col, end.Row, 0);
                    DrawShape<Rectangle>(topLeftEnd.X, topLeftEnd.Y, _fieldSide, _fieldSide,
                        Brushes.GreenYellow, null, _canvas);
                }
            }
        }

        /// <summary>
        /// General method to update UI
        /// </summary>
        public void Draw()
        {
            DrawBoard();
            if (_state != null)
                DrawState(_state);
        }

        /// <summary>
        /// Constructor
        /// </summary>
        public BoardUi(Canvas canvas, Dispatcher uiThreadDispatcher)
        {
            _uiThreadDispatcher = uiThreadDispatcher ?? throw new Exception("Dispatcher must be not null");
            _canvas = canvas ?? throw new Exception("Invalid input");
            _canvas.MouseDown += CanvasOnMouseDown;
            Draw();
        }

        /// <summary>
        /// Event handler
        /// </summary>
        private void CanvasOnMouseDown(object sender, MouseButtonEventArgs e)
        {

            if (_userMoveRequest != null)
            {
                var oldSelectedField = _selectedField;
                _selectedField = CanvasCoordinateToFieldPosition(e.GetPosition(_canvas));
                var possibleMoves = GetPossibleMovesEndingAt(GetPossibleMovesStartingFrom(oldSelectedField), _selectedField);

                if (possibleMoves.Length > 0)
                {
                    //TODO: 
                    //in principle there can be a couple of moves that have the same start and end position but this is only for the case of capturing moves
                    //For now we go for a simplified solution of the problem we choose index of the move that captures the most
                    //It would be nice to come up with a mechanism that would allow user to deliberately choose between the possible moves in such a (rare) cases
                    var moveId =  possibleMoves.OrderByDescending(x => x.SubMoves.Length).First().Index;
                    _userMoveRequest.UserMoveResult.SetResult(moveId);
                    _userMoveRequest = null;
                }
            }
            else
                _selectedField = CanvasCoordinateToFieldPosition(e.GetPosition(_canvas));

            Draw();
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
    }
}
