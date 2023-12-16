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
    /// Data transferring structure to contain checkers sub-move information
    /// Must be kept in sync with the corresponding twin structure on the c++ side
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct SubMove
    {
        /// <summary>
        /// Position on the checkers board where the sub-move started
        /// </summary>
        public PiecePosition Start;

        /// <summary>
        /// Position on the checkers board where the sub-move ended
        /// </summary>
        public PiecePosition End;

        /// <summary>
        /// Position on the checkers board where an opponent piece was captured during the sub-move
        /// </summary>
        public PiecePosition Capture;
    }
}
