using System;
using System.Linq;

namespace Monitor.Checkers
{
    /// <summary>
    /// Wrapper of the "native interactive agent"
    /// </summary>
    class InteractiveAgent : Agent
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
        /// Finalizer
        /// </summary>
        ~InteractiveAgent()
        {
            Dispose();
        }

        /// <summary>
        /// Releases unmanaged resources
        /// </summary>
        public override void Dispose()
        {
            if (_ptr == IntPtr.Zero)
                return;

            if (!DllWrapper.FreeInteractiveAgent(_ptr))
                throw new Exception("Failed to release agent pointer");

            _ptr = IntPtr.Zero;
            GC.SuppressFinalize(this);
        }
    }
}
