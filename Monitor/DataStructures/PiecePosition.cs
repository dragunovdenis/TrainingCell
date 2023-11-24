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

using System.Runtime.InteropServices;

namespace Monitor.DataStructures
{
    /// <summary>
    /// Data transferring structure to contain checkers piece position
    /// Must be kept in sync with the corresponding twin structure on the c++ side
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    internal struct PiecePosition
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
        public static PiecePosition Invalid()
        {
            return new PiecePosition() { Row = -1, Col = -1 };
        }

        /// <summary>
        /// Returns true if the current instance represents a valid checkers piece position
        /// </summary>
        public bool IsValid => Row >= 0 && Row < Checkerboard.Rows && Col >= 0 && Col < Checkerboard.Rows;

        /// <summary>
        /// Returns "true" if the current position is equal to the given one
        /// </summary>
        public bool IsEqualTo(PiecePosition pos)
        {
            return Row == pos.Row && Col == pos.Col;
        }

        /// <summary>
        /// Outputs row and column representation for the state item given by its ID
        /// </summary>
        public static PiecePosition GetPosition(int linearPosition)
        {
            return new PiecePosition()
            {
                Row = linearPosition / Checkerboard.Columns,
                Col = linearPosition % Checkerboard.Columns,
            };
        }

        /// <summary>
        /// Converts given position to linear representation.
        /// </summary>
        public int LinearPosition => (int)(Row * Checkerboard.Columns + Col);
    }
}
