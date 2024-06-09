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

using System.Collections.Generic;
using System.Collections.Immutable;
using System.Linq;
using Monitor.DataStructures;
using Monitor.Dll;
using Monitor.UI.Board.PieceControllers;

namespace Monitor.State
{
    /// <summary>
    /// Representation of a checkerboard state.
    /// </summary>
    internal class State
    {
        private readonly int[] _dataOriginal;

        /// <summary>
        /// Values of the checkerboard fields (representing pieces on them).
        /// </summary>
        public ImmutableArray<int> Data { get; private set; }
        
        /// <summary>
        /// Type of the state.
        /// </summary>
        public DllWrapper.StateTypeId Type { get; }

        /// <summary>
        /// Adds extra symbols to the state to trace the series of given sub-moves (representing a move)
        /// </summary>
        public void AddMoveTrace(SubMove[] subMoves, bool afterMoveDone = true)
        {
            if (subMoves == null || subMoves.Length == 0)
            {
                Data = _dataOriginal.ToImmutableArray();
                return;
            }

            var rawData = _dataOriginal.ToArray();
            var pieceId = rawData[afterMoveDone ? subMoves.Last().End.LinearPosition :
                subMoves.First().Start.LinearPosition];
            var pieceController = PieceControllerFactory.Create(Type);
            
            foreach (var move in subMoves)
            {
                if (move.Capture.IsValid)
                    rawData[move.Capture.LinearPosition] = pieceController.GetCapturedPieceId(white: pieceId > 0);

                rawData[move.Start.LinearPosition] = pieceController.GetPieceTraceId(pieceId);
            }

            rawData[subMoves.Last().End.LinearPosition] = pieceId;

            Data = rawData.ToImmutableArray();
        }

        /// <summary>
        /// Constructor.
        /// </summary>
        public State(IList<int> data, DllWrapper.StateTypeId type)
        {
            _dataOriginal = data.ToArray();
            Data = data.ToImmutableArray();
            Type = type;
        }
    }
}
