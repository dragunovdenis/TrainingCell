﻿using System;
using System.Runtime.InteropServices;

namespace Monitor.Checkers
{
    /// <summary>
    /// Wrapping functionality to communicate with the code inside native dll
    /// </summary>
    internal static class DllWrapper
    {
        /// <summary>
        /// Byte to boolean conversion
        /// </summary>
        public static bool ToBool(this byte val)
        {
            switch (val)
            {
                case 0: return false;
                case 1: return true;
                default: throw new Exception("Invalid input");
            }
        }

        public delegate void PublishCheckersStateCallBack(
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)]
            int[] state, int stateSize,
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)]
            CheckersSubMove[] subMoves, int subMovesCount);

        public delegate void PublishCheckersGameStatsCallBack(
            int whiteWins, int blackWins, int totalGames);

        [return: MarshalAs(UnmanagedType.U1)]
        public delegate bool CancelCallBack();

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName)]
        public static extern void RunCheckersTraining(IntPtr agent1, IntPtr agent2, int episodes,
            [MarshalAs(UnmanagedType.FunctionPtr)]
            PublishCheckersStateCallBack publishStateCallBack,
            [MarshalAs(UnmanagedType.FunctionPtr)]
            PublishCheckersGameStatsCallBack publishStats,
            [MarshalAs(UnmanagedType.FunctionPtr)]
            CancelCallBack cancel);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "ConstructCheckersRandomAgent")]
        public static extern IntPtr ConstructRandomAgent();

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "FreeCheckersRandomAgent")]
        public static extern bool FreeRandomAgent(IntPtr agentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "ConstructCheckersTdLambdaAgent")]
        public static extern IntPtr ConstructTdLambdaAgent(
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)]
            UInt32[] layerDims, 
            int layerDimsSize, double explorationEpsilon, double lambda, double gamma, double alpha);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersTdLambdaAgentLoadFromFile", CharSet = CharSet.Ansi)]
        public static extern IntPtr LoadTdLambdaAgent(string path);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersTdLambdaAgentSaveToFile", CharSet = CharSet.Ansi)]
        public static extern bool SaveTdLambdaAgent(IntPtr agentPtr, string path);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersTdLambdaAgentSetEpsilon")]
        public static extern bool TdLambdaAgentSetEpsilon(IntPtr agentPtr, double explorationEpsilon);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersTdLambdaAgentGetEpsilon")]
        public static extern double TdLambdaAgentGetEpsilon(IntPtr agentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersTdLambdaAgentSetLambda")]
        public static extern bool TdLambdaAgentSetLambda(IntPtr agentPtr, double lambda);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersTdLambdaAgentGetLambda")]
        public static extern double TdLambdaAgentGetLambda(IntPtr agentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersTdLambdaAgentSetGamma")]
        public static extern bool TdLambdaAgentSetGamma(IntPtr agentPtr, double gamma);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersTdLambdaAgentGetGamma")]
        public static extern double TdLambdaAgentGetGamma(IntPtr agentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersTdLambdaAgentSetLearningRate")]
        public static extern bool TdLambdaAgentSetLearningRate(IntPtr agentPtr, double learningRater);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersTdLambdaAgentGetLearningRate")]
        public static extern double TdLambdaAgentGetLearningRate(IntPtr agentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersTdLambdaAgentSetTrainingMode")]
        public static extern bool TdLambdaAgentSetTrainingMode(IntPtr agentPtr,
            [MarshalAs(UnmanagedType.U1)]
            bool trainingMode);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersTdLambdaAgentGetTrainingMode")]
        public static extern byte TdLambdaAgentGetTrainingMode(IntPtr agentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersTdLambdaAgentEqual")]
        public static extern bool TdLambdaAgentEqual(IntPtr agentPtr1, IntPtr agentPtr2);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "FreeCheckersTdLambdaAgent")]
        public static extern bool FreeTdLambdaAgent(IntPtr agentPtr);

        /// <summary>
        ///	Data transferring object to pass checker moves
        /// </summary>
        [StructLayout(LayoutKind.Sequential)]
        public struct CheckersMoveDto
        {
            /// <summary>
            ///	Pointer to the array of sub_moves
            /// </summary>
            public IntPtr SubMoves;

            /// <summary>
            ///	Number of sub-moves in the array
            /// </summary>
            public int SubMovesCnt;
        };

        /// <summary>
        /// Checkers "make move" delegate
        /// </summary>
        public delegate int CheckersMakeMoveCallBack(
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)]
            int[] state, int stateSize,
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)]
            CheckersMoveDto[] moves, int movesSize);

        /// <summary>
        /// Checkers "game over" delegate
        /// </summary>
        public delegate void CheckersGameOverCallBack(
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)]
            int[] state, int stateSize, int gameResult);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "ConstructCheckersInteractiveAgent")]
        public static extern IntPtr ConstructInteractiveAgent(
            [MarshalAs(UnmanagedType.FunctionPtr)]
            CheckersMakeMoveCallBack makeMoveCallBack,
            [MarshalAs(UnmanagedType.FunctionPtr)]
            CheckersGameOverCallBack gameOverCallBack,
            [MarshalAs(UnmanagedType.U1)]
            bool playForWhites);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "FreeCheckersInteractiveAgent")]
        public static extern bool FreeInteractiveAgent(IntPtr agentPtr);
    }
}