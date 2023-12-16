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
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Shapes;
using Monitor.Dll;

namespace Monitor.UI.Board.PieceControllers
{
    /// <summary>
    /// Checkers specific implementation of the corresponding interface.
    /// </summary>
    internal class CheckersPieceController : IPieceController
    {
        private const int WhiteCapturedPieceId = 3;
        private const int BlackCapturedPieceId = -3;
        private const int WhitePieceTraceId = 4;
        private const int BlackPieceTraceId = -4;

        /// <summary>
        /// See the summary of the interface method.
        /// </summary>
        public DllWrapper.StateTypeId StateTypeId => DllWrapper.StateTypeId.Checkers;


        /// <summary>
        /// Converts index of a checkers piece (that we receive in the "state" collection) into the fill color that should be
        /// used when drawing the piece on the board
        /// </summary>
        private static Brush PieceIdToColor(int pieceId)
        {
            switch (pieceId)
            {
                case 1: return Brushes.Wheat;
                case 2: return Brushes.Wheat;
                case WhiteCapturedPieceId: return Brushes.Wheat;
                case WhitePieceTraceId: return null;
                case -1: return Brushes.Black;
                case -2: return Brushes.Black;
                case BlackCapturedPieceId: return Brushes.Black;
                case BlackPieceTraceId: return null;
                default: throw new Exception("Unknown piece identifier");
            }
        }

        /// <summary>
        /// Returns "true" is the given identifier represents a King piece
        /// </summary>
        private static bool IsKingPiece(int pieceId)
        {
            return pieceId == 2 || pieceId == -2;
        }

        /// <summary>
        /// Returns "true" if the given identifier represents a Man piece
        /// </summary>
        private static bool IsManPiece(int pieceId)
        {
            return pieceId == 1 || pieceId == -1;
        }

        /// <summary>
        /// Draws a piece that corresponds to the given ID on a field defined with the coordinates of its top left corner and the size.
        /// </summary>
        private static void DrawPieceInternal(Canvas canvas, Point topLeft, double fieldSize, int pieceId)
        {
            if (pieceId == 0)
                return;

            var shrink = 0.1 * fieldSize;
            var fieldSizeShrunk = fieldSize - 2 * shrink;
            var topLeftAdjusted = new Point(topLeft.X + shrink, topLeft.Y + shrink);

            CanvasDrawingUtils.DrawShape<Ellipse>(topLeftAdjusted.X, topLeftAdjusted.Y, fieldSizeShrunk, fieldSizeShrunk,
                Brushes.BlueViolet, PieceIdToColor(pieceId), canvas);

            if (!IsKingPiece(pieceId) && !IsManPiece(pieceId))
                return;

            var shift = fieldSizeShrunk / 4;

            CanvasDrawingUtils.DrawShape<Ellipse>(topLeftAdjusted.X + shift, topLeftAdjusted.Y + shift,
                fieldSizeShrunk / 2, fieldSizeShrunk / 2,
                Brushes.BlueViolet, IsKingPiece(pieceId) ? Brushes.Red : Brushes.Green, canvas);
        }


        /// <summary>
        /// See the summary of the interface method.
        /// </summary>
        public void DrawPiece(Canvas canvas, Point topLeft, double fieldSize, int pieceId)
        {
            DrawPieceInternal(canvas, topLeft, fieldSize, pieceId);
        }

        /// <summary>
        /// See the summary of the interface method.
        /// </summary>
        public int GetCapturedPieceId(bool white)
        {
            return white ? BlackCapturedPieceId : WhiteCapturedPieceId;
        }

        /// <summary>
        /// See the summary of the interface method.
        /// </summary>
        public int GetPieceTraceId(bool white)
        {
            return white ? WhitePieceTraceId : BlackPieceTraceId;
        }
    }
}
