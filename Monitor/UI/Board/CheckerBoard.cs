//Copyright (c) 2024 Denys Dragunov, dragunovdenis@gmail.com
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
using System.Linq;
using System.Windows;
using System.Windows.Shapes;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using Monitor.DataStructures;
using Monitor.UI.Board.PieceControllers;
using Brushes = System.Windows.Media.Brushes;
using static Monitor.UI.Board.CheckerBoard;

namespace Monitor.UI.Board
{
    /// <summary>
    /// Interface for the class below.
    /// </summary>
    internal interface ICheckerBoard
    {
        /// <summary>
        /// Updates state collection.
        /// </summary>
        void UpdateState(State.State state);

        /// <summary>
        /// Updates markers on the board.
        /// </summary>
        void UpdateMarkers(IList<Marker> markers);

        /// <summary>
        /// Updates id of the preview piece and visualizes it.
        /// </summary>
        void UpdatePreviewPiece(int previewPieceId);

        /// <summary>
        /// Resets position of "selected field".
        /// </summary>
        void ResetSelectedField();

        /// <summary>
        /// Clears all the visual components except the bord itself.
        /// </summary>
        void Clear();

        /// <summary>
        /// Read-only access to the field under mouse pointer coordinate.
        /// </summary>
        PiecePosition FieldUnderMousePointer { get; }

        /// <summary>
        /// Read-only access to the coordinate of selected field.
        /// </summary>
        PiecePosition SelectedField { get; }

        /// <summary>
        /// Event that happens when user clicks on a field.
        /// </summary>
        event Action<PiecePosition> FieldSelected;

        /// <summary>
        /// Event that happens when mouse pointer enters a field.
        /// </summary>
        event Action<PiecePosition> FieldHovered;

        /// <summary>
        /// Event, that happens on mouse wheel change.
        /// </summary>
        event Action<int, PiecePosition> MouseWheelChanged;
    }

    /// <summary>
    /// Functionality to visualize and interact with checkerboard.
    /// </summary>
    internal class CheckerBoard : ICheckerBoard
    {
        /// <summary>
        /// Description of a marker on board
        /// </summary>
        internal struct Marker
        {
            /// <summary>
            /// Position of the marker on board.
            /// </summary>
            public PiecePosition Position { get; }
            
            /// <summary>
            /// Color of the marker.
            /// </summary>
            public Brush Color { get; }

            /// <summary>
            /// Constructor.
            /// </summary>
            public Marker(PiecePosition position, Brush color)
            {
                Position = position;
                Color = color;
            }
        }
        
        private double _boardSide;
        private double _fieldSide;
        const double MarkerOffset = 2.0;
        double _markerSide;
        private double _topLeftX;
        private double _topLeftY;
        private double _canvasHeight = -1;
        private double _canvasWidth = -1;

        private readonly Canvas _canvas;
        private IPieceController _pieceController;

        private readonly IList<UIElement> _boardElements = new List<UIElement>();
        private readonly IList<UIElement> _stateElements = new List<UIElement>();
        private readonly IList<UIElement> _markerElements = new List<UIElement>();
        private readonly IList<UIElement> _previewElements = new List<UIElement>();

        private int[] _state;
        private Marker[] _markers = Array.Empty<Marker>();
        private int _previewPieceId;

        /// <summary>
        /// Enumerates different drawing scopes.
        /// </summary>
        [Flags]
        private enum DrawScope
        {
            None = 0,
            DrawBoardElements = 1,
            DrawStateElements = 1 << 1,
            DrawMarkers = 1 << 2,
            DrawPreview = 1 << 3,
            DrawMarkersAndPreview = DrawMarkers | DrawPreview,
            DrawSateMarkersAndPreview = DrawStateElements | DrawMarkersAndPreview,
            All = DrawBoardElements | DrawSateMarkersAndPreview,
        }
        
        /// <summary>
        /// Updates state collection.
        /// </summary>
        public void UpdateState(State.State state)
        {
            _state = state?.Data.ToArray();
            _pieceController = state != null ? PieceControllerFactory.Create(state.Type) : null;
            
            Draw(DrawScope.DrawSateMarkersAndPreview);
        }

        /// <summary>
        /// Updates markers on the board.
        /// </summary>
        public void UpdateMarkers(IList<Marker> markers)
        {
            _markers = markers != null ? markers.ToArray() : Array.Empty<Marker>();
            Draw(DrawScope.DrawMarkersAndPreview);
        }

        /// <summary>
        /// Updates id of the preview piece and visualizes it.
        /// </summary>
        public void UpdatePreviewPiece(int previewPieceId)
        {
            _previewPieceId = previewPieceId;
            Draw(DrawScope.DrawPreview);
        }

        /// <summary>
        /// Subscribes event handlers to the canvas.
        /// </summary>
        private void ConnectToCanvas()
        {
            _canvas.SizeChanged += CanvasSizeChangedHandler;
            _canvas.MouseDown += CanvasOnMouseDown;
            _canvas.MouseMove += CanvasOnMouseMove;
            _canvas.MouseWheel += CanvasOnMouseWheel;
        }

        /// <summary>
        /// Constructor.
        /// </summary>
        public CheckerBoard(Canvas canvas, IPieceController pieceController = null)
        {
            _pieceController = pieceController;
            _canvas = canvas;
            ConnectToCanvas();
            Draw();
        }

        public event Action<int, PiecePosition> MouseWheelChanged;
        
        /// <summary>
        /// Handles mouse wheel activity on the canvas.
        /// </summary>
        private void CanvasOnMouseWheel(object sender, MouseWheelEventArgs e)
        {
            MouseWheelChanged?.Invoke(e.Delta, _selectedField);
        }

        /// <summary>
        /// Position of the board field under the mouse pointer (if valid).
        /// </summary>
        private PiecePosition _fieldUnderMousePointer = PiecePosition.Invalid();

        /// <summary>
        /// Read-only access to the field under mouse pointer coordinate.
        /// </summary>
        public PiecePosition FieldUnderMousePointer => _fieldUnderMousePointer;

        /// <summary>
        /// For the given mouse position on the canvas returns coordinates of the corresponding checkers field
        /// or (-1, -1), if the given point does not belong to any valid field
        /// </summary>
        private PiecePosition CanvasCoordinateToFieldPosition(Point pt)
        {
            var colIdFloat = (pt.X - _topLeftX) / _fieldSide;
            if (colIdFloat < 0 || colIdFloat > Checkerboard.Columns)
                return PiecePosition.Invalid();

            var rowIdFloat = (pt.Y - _topLeftY) / _fieldSide;
            if (rowIdFloat < 0 || rowIdFloat > Checkerboard.Rows)
                return PiecePosition.Invalid();

            return new PiecePosition() { Row = (int)rowIdFloat, Col = (int)colIdFloat };
        }

        public event Action<PiecePosition> FieldHovered;
        
        /// <summary>
        /// Handles mouse move activity on the canvas.
        /// </summary>
        private void CanvasOnMouseMove(object sender, MouseEventArgs e)
        {
            var fieldPosition = CanvasCoordinateToFieldPosition(e.GetPosition(_canvas));

            if (_fieldUnderMousePointer.IsEqualTo(fieldPosition))
                return;

            _fieldUnderMousePointer = fieldPosition;
            FieldHovered?.Invoke(_fieldUnderMousePointer);
        }

        private PiecePosition _selectedField = PiecePosition.Invalid();
        
        /// <summary>
        /// Read-only access to the coordinate of selected field.
        /// </summary>
        public PiecePosition SelectedField => _selectedField;
        
        public event Action<PiecePosition> FieldSelected;

        /// <summary>
        /// Handles mouse down activity on the canvas.
        /// </summary>
        private void CanvasOnMouseDown(object sender, MouseButtonEventArgs e)
        {
            var selectedField = CanvasCoordinateToFieldPosition(e.GetPosition(_canvas));

            _selectedField = selectedField;
            FieldSelected?.Invoke(_selectedField);
        }

        /// <summary>
        /// Handles changing of the size of the canvas.
        /// </summary>
        private void CanvasSizeChangedHandler(object sender, SizeChangedEventArgs e)
        {
            Draw();
        }

        /// <summary>
        /// Draws all the UI elements of the board.
        /// </summary>
        private void Draw(DrawScope scope = DrawScope.All)
        {
            if (scope.HasFlag(DrawScope.DrawBoardElements))
                DrawBoardElements();
            
            if (scope.HasFlag(DrawScope.DrawStateElements))
                DrawStateElements();

            if (scope.HasFlag(DrawScope.DrawMarkers))
                DrawMarkers();

            if (scope.HasFlag(DrawScope.DrawPreview))
                DrawPreview();
        }

        /// <summary>
        /// Returns coordinates of the top left corner of the corresponding field on the canvas
        /// </summary>
        /// <param name="colId">Field column</param>
        /// <param name="rowId">Field row</param>
        /// <param name="offset">Offset that should be added to booth coordinates of the returned corner</param>
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

        /// <summary>
        /// Removes board elements from the canvas.
        /// </summary>
        private void ClearBoardElements()
        {
            foreach (var element in _boardElements)
                _canvas.Children.Remove(element);

            _boardElements.Clear();
        }

        /// <summary>
        /// Updates board size parameters from the current size of the canvas control.
        /// Returns "true" if any of the board dimensions has been updated.
        /// </summary>
        private bool TryUpdateBoardDimensions()
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
            _markerSide = _fieldSide - 2 * MarkerOffset;

            return true;
        }

        /// <summary>
        /// Draws only board elements on the canvas.
        /// </summary>
        private void DrawBoardElements()
        {
            if (!TryUpdateBoardDimensions())
                return;

            ClearBoardElements();

            var boardTopLeft = GetTopLeftCorner(0, 0, 0);
            _boardElements.Add(CanvasDrawingUtils.CreateShape<Rectangle>(
                boardTopLeft.X, boardTopLeft.Y, _boardSide, _boardSide, 1,
                Brushes.Black, Brushes.BurlyWood));

            for (var rowId = 0; rowId < Checkerboard.Rows; rowId++)
            for (var colId = 0; colId < Checkerboard.Columns; colId++)
            {
                var topLeft = GetTopLeftCorner(colId, rowId, 0);
                var isDarkField = rowId % 2 == 0 ? colId % 2 == 1 : colId % 2 == 0;
                if (!isDarkField)
                    continue;

                _boardElements.Add(CanvasDrawingUtils.CreateShape<Rectangle>(
                    topLeft.X, topLeft.Y, _fieldSide, _fieldSide, 1,
                    Brushes.Black, Brushes.SaddleBrown));
            }

            foreach (var element in _boardElements)
                _canvas.Children.Add(element);
        }

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
        private void DrawStateElements()
        {
            ClearStateElements();
            
            if (_state == null || _pieceController == null)
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
        /// Clears guiding markers on the canvas.
        /// </summary>
        private void ClearMarkers()
        {
            foreach (var marker in _markerElements)
                _canvas.Children.Remove(marker);

            _markerElements.Clear();
        }

        /// <summary>
        /// Draws markers on the board.
        /// </summary>
        private void DrawMarkers()
        {
            ClearMarkers();

            foreach (var marker in _markers)
            {
                var topLeftCorner = GetTopLeftCorner(marker.Position.Col, marker.Position.Row, MarkerOffset);
                _markerElements.Add(CanvasDrawingUtils.CreateShape<Rectangle>(topLeftCorner.X, topLeftCorner.Y,
                    _markerSide, _markerSide, 3, marker.Color, null));
            }

            foreach (var marker in _markerElements)
                _canvas.Children.Add(marker);
        }

        /// <summary>
        /// Removes "piece preview" element from the canvas.
        /// </summary>
        private void ClearPreview()
        {
            foreach (var element in _previewElements)
                _canvas.Children.Remove(element);

            _previewElements.Clear();
        }

        /// <summary>
        /// Updates "piece preview" visualization.
        /// </summary>
        private void DrawPreview()
        {
            ClearPreview();

            if (!_fieldUnderMousePointer.IsValid || _pieceController == null)
                return;

            var tracePieceId = _pieceController.GetPieceTraceId(_previewPieceId);
            var topLeftCorner = GetTopLeftCorner(_fieldUnderMousePointer);
            foreach (var element in _pieceController.CreatePieceElements(topLeftCorner, _fieldSide, tracePieceId))
                _previewElements.Add(element);

            foreach (var element in _previewElements)
                _canvas.Children.Add(element);
        }

        /// <summary>
        /// Resets position of "selected field".
        /// </summary>
        public void ResetSelectedField()
        {
            _selectedField = PiecePosition.Invalid();
        }

        /// <summary>
        /// Clears all the visual components except the bord itself.
        /// </summary>
        public void Clear()
        {
            UpdateMarkers(null);
            UpdatePreviewPiece(0);
            UpdateState(null);
        }
    }
}
