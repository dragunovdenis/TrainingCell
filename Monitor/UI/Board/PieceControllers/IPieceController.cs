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

using System.Windows;
using System.Windows.Controls;
using Monitor.Dll;

namespace Monitor.UI.Board.PieceControllers
{
    /// <summary>
    /// Interface to a piece visualization controller
    /// </summary>
    internal interface IPieceController
    {
        /// <summary>
        /// Method to visualize a piece that corresponds to the given ID on a field defined with the coordinates of its top left corner and the size.
        /// </summary>
        void DrawPiece(Canvas canvas, Point topLeft, double fieldSize, int pieceId);

        /// <summary>
        /// Returns "piece ID" that the current instance of controller will visualize as a "captured" piece of the corresponding color.
        /// </summary>
        int GetCapturedPieceId(bool white);

        /// <summary>
        /// Returns "piece ID" that the current instance of controller will visualize as a "piece trace" of the corresponding rank.
        /// </summary>
        int GetPieceTraceId(int pieceId);

        /// <summary>
        /// Returns type ID of the state the controller is compatible with.
        /// </summary>
        DllWrapper.StateTypeId StateTypeId { get; }
    }
}
