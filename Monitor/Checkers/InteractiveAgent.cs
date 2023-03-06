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
using System.Linq;
using Monitor.Checkers.UI;

namespace Monitor.Checkers
{
    /// <summary>
    /// Wrapper of the "native interactive agent"
    /// </summary>
    sealed class InteractiveAgent : Agent
    {
        /// <summary>
        /// "Make move" delegate
        /// </summary>
        internal delegate int MakeMoveDelegate(int[] state, CheckersMove[] moves);

        /// <summary>
        /// "Game over" delegate
        /// </summary>
        internal delegate void GameOverDelegate(int[] state, int result);

        // ReSharper disable once PrivateFieldCanBeConvertedToLocalVariable
        private readonly DllWrapper.CheckersMakeMoveCallBack _makeMoveAdapter;
        // ReSharper disable once PrivateFieldCanBeConvertedToLocalVariable
        private readonly DllWrapper.CheckersGameOverCallBack _gameOverAdapter;

        private IntPtr _ptr;

        /// <summary>
        /// Pointer to the native agent
        /// </summary>
        public override IntPtr Ptr => _ptr;

        /// <summary>
        /// Constructor
        /// </summary>
        internal InteractiveAgent(MakeMoveDelegate makeMove, GameOverDelegate gameOver, bool playForWhites)
        {
            _makeMoveAdapter =
                (state, stateSize, moves, movesSize) =>
                {
                    var movesAdapted = moves.Select((x, i) => new CheckersMove()
                    {
                        SubMoves = TrainingCellInterface.ReadArrayOfStructs<CheckersSubMove>(x.SubMoves, x.SubMovesCnt),
                        Index = i,
                    }).ToArray();

                    return makeMove(state, movesAdapted);
                };

            _gameOverAdapter = (state, stateSize, gameResult) => { gameOver(state, gameResult); };

            _ptr = DllWrapper.ConstructInteractiveAgent(_makeMoveAdapter, _gameOverAdapter, playForWhites);
        }

        /// <summary>
        /// Releases unmanaged resources
        /// </summary>
        public override void Dispose()
        {
            if (Ptr == IntPtr.Zero)
                return;

            if (!DllWrapper.FreeInteractiveAgent(Ptr))
                throw new Exception("Failed to release agent pointer");

            _ptr = IntPtr.Zero;
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Brief summary of the current agent instance
        /// </summary>
        public override string Summary => $"Interactive Agent: {Name} ({Id})";
    }
}
