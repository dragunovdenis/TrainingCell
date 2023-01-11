using System;
using System.Runtime.InteropServices;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Shapes;

namespace Monitor.Checkers
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
    /// Functionality to handle user interface of the checkers game
    /// </summary>
    class Ui
    {
        private const int CheckerRows = 8;
        private const int CheckerColumns = 8;
        private const int BlackFieldsInRow = 4;
        private const int StateSize = CheckerRows * BlackFieldsInRow;

        double _boardSide;
        double _fieldSide;
        double _topLeftX;
        double _topLeftY;

        private readonly Canvas _canvas = null;

        /// <summary>
        /// Draws a "shape" (ellipse, rectangle...) with the given position and radius on the given canvas
        /// </summary>
        public static void DrawShape<S>(double x, double y, double width, double height,
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
        /// Draws the given state
        /// </summary>
        public void DrawState(int[] state)
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

                var fieldTopLeftX = colId * _fieldSide + _topLeftX + shrink;
                var fieldTopLeftY = rowId * _fieldSide + _topLeftY + shrink;

                DrawShape<Ellipse>(fieldTopLeftX, fieldTopLeftY, fieldSizeShrunk, fieldSizeShrunk,
                    Brushes.BlueViolet, PieceIdToColor(pieceId), _canvas);

                if (!IsKingPiece(pieceId) && ! IsManPiece(pieceId))
                    continue;

                var shift = fieldSizeShrunk / 4;

                DrawShape<Ellipse>(fieldTopLeftX + shift, fieldTopLeftY + shift,
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
        public void DrawBoard()
        {
            _canvas.Children.Clear();

            UpdateBoardDimensions();

            for (var rowId = 0; rowId < CheckerRows; rowId++)
            for (var colId = 0; colId < CheckerColumns; colId++)
            {
                var fieldTopLeftX = colId * _fieldSide + _topLeftX;
                var fieldTopLeftY = rowId * _fieldSide + _topLeftY;

                var isDarkField = (rowId % 2 == 0) ? (colId % 2 == 1) : (colId % 2 == 0);
                DrawShape<Rectangle>(fieldTopLeftX, fieldTopLeftY, _fieldSide, _fieldSide,
                    Brushes.Black, isDarkField ? Brushes.SaddleBrown : Brushes.BurlyWood, _canvas);
            }

        }

        /// <summary>
        /// Constructor
        /// </summary>
        public Ui(Canvas canvas)
        {
            _canvas = canvas ?? throw new Exception("Invalid input");
            DrawBoard();
        }
    }
}
