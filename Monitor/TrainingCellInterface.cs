using System.Runtime.InteropServices;
using Monitor.Checkers;

namespace Monitor
{
    /// <summary>
    /// Wrapper for the methods of the corresponding dynamic link library
    /// </summary>
    class TrainingCellInterface
    {
        public const string DllName = "TrainingCellDll.dll";

        public delegate void PublishCheckersStateCallBack(
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)]
            int[] state, int stateSize,
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)]
            CheckersSubMove[] subMoves, int subMovesCount);

        public delegate void PublishCheckersGameStatsCallBack(
            int whiteWins, int blackWins, int totalGames);

        [DllImport(dllName: DllName)]
        public static extern void RunCheckersTraining(int epochs, 
            [MarshalAs(UnmanagedType.FunctionPtr)]
            PublishCheckersStateCallBack publishStateCallBack,
            [MarshalAs(UnmanagedType.FunctionPtr)]
            PublishCheckersGameStatsCallBack publishStats);

        [DllImport(dllName: DllName)]
        public static extern void test();
    }
}
