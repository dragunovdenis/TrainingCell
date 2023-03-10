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
using System.Runtime.InteropServices;
using Monitor.Checkers.UI;

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

        /// <summary>
        /// Checkers move report delegate
        /// </summary>
        public delegate void PublishCheckersStateCallBack(
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)]
            int[] state, int stateSize,
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)]
            CheckersSubMove[] subMoves, int subMovesCount);

        /// <summary>
        /// Checkers statistics report delegate
        /// </summary>
        public delegate void PublishCheckersGameStatsCallBack(
            int whiteWins, int blackWins, int totalGames);

        /// <summary>
        /// Cancellation delegate
        /// </summary>
        [return: MarshalAs(UnmanagedType.U1)]
        public delegate bool CancelCallBack();

        /// <summary>
        /// Error message delegate
        /// </summary>
        public delegate void ErrorMessageCallBack(
            [MarshalAs(UnmanagedType.LPStr)]
            string message);

        /// <summary>
        /// Delegate to acquire array of unsigned integers from the managed side
        /// </summary>
        public delegate void AcquireArrayCallBack(
            int size, 
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 0)]
            System.UInt32[] array);

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
            CancelCallBack cancel,
            [MarshalAs(UnmanagedType.FunctionPtr)]
            ErrorMessageCallBack error);

        #region Random Agent
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
        #endregion

        #region TD(Lambda)-Agent
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
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersTdLambdaAgentGetNetDimensions")]
        public static extern bool TdLambdaAgentGetNetDimensions(IntPtr agentPtr, AcquireArrayCallBack catchArray);

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
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "PackCheckersTdLambdaAgent")]
        public static extern IntPtr PackTdLambdaAgent(IntPtr agentPtr);
        #endregion

        #region Interactive Agent

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
        #endregion

        #region Agent
        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersAgentSetTrainingMode")]
        public static extern bool AgentSetTrainingMode(IntPtr agentPtr,
            [MarshalAs(UnmanagedType.U1)]
            bool trainingMode);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersAgentGetTrainingMode")]
        public static extern byte AgentGetTrainingMode(IntPtr agentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersAgentGetCanTrainFlag")]
        public static extern byte AgentGetCanTrainFlag(IntPtr agentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersAgentGetId")]
        private static extern IntPtr AgentGetIdInternal(IntPtr agentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersAgentGetName")]
        private static extern IntPtr AgentGetNameInternal(IntPtr agentPtr);

        /// <summary>
        /// Converts given pointer to ANSI atring
        /// </summary>
        private static string PtrToAnsiString(IntPtr ptr)
        {
            if (ptr == IntPtr.Zero)
                return null;

            return Marshal.PtrToStringAnsi(ptr);
        }

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        public static string AgentGetId(IntPtr agentPtr)
        {
            return PtrToAnsiString(AgentGetIdInternal(agentPtr));
        }

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        public static string AgentGetName(IntPtr agentPtr)
        {
            return PtrToAnsiString(AgentGetNameInternal(agentPtr));
        }

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersAgentSetName", CharSet = CharSet.Ansi)]
        public static extern bool AgentSetName(IntPtr agentPtr, string id);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersAgentGetRecordsCount")]
        public static extern int AgentGetRecordsCount(IntPtr agentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersAgentGetRecordById")]
        private static extern IntPtr AgentGetRecordByIdInternal(IntPtr agentPtr, int recordId);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        public static string AgentGetRecordById(IntPtr agentPtr, int recordId)
        {
            return PtrToAnsiString(AgentGetRecordByIdInternal(agentPtr, recordId));
        }

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersAgentAddRecord", CharSet = CharSet.Ansi)]
        public static extern int AgentAddRecord(IntPtr agentPtr, string record);
        #endregion

        #region AgentPack
        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersAgentPackLoadFromFile", CharSet = CharSet.Ansi)]
        public static extern IntPtr AgentPackLoadFromFile(string path);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersAgentPackSaveToFile", CharSet = CharSet.Ansi)]
        public static extern bool AgentPackSaveToFile(IntPtr agentPackPtr, string path);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersAgentPackFree")]
        public static extern bool AgentPackFree(IntPtr agentPackPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersAgentPackGetAgentPtr")]
        public static extern IntPtr AgentPackGetAgentPtr(IntPtr agentPackPtr);
        #endregion

        #region EnsembleAgent
        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "ConstructCheckersTdlEnsembleAgent")]
        public static extern IntPtr ConstructTdlEnsembleAgent(int subAgentCount, 
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 0)]
            IntPtr[] tdlAgents);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "FreeCheckersTdlEnsembleAgent")]
        public static extern bool FreeTdlEnsembleAgent(IntPtr ensembleAgentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "SaveCheckersTdlEnsembleAgent", CharSet = CharSet.Ansi)]
        public static extern bool SaveTdlEnsembleAgent(IntPtr ensembleAgentPtr, string filePath);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "LoadCheckersTdlEnsembleAgent", CharSet = CharSet.Ansi)]
        public static extern IntPtr LoadTdlEnsembleAgent(string filePath);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersTdlEnsembleAgentGetSize")]
        public static extern int TdlEnsembleAgentGetSize(IntPtr ensembleAgentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersTdlEnsembleAgentGetSubAgentId")]
        private static extern IntPtr TdlEnsembleAgentGetSubAgentIdInternal(IntPtr ensembleAgentPtr, int subAgentId);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        public static string TdlEnsembleAgentGetSubAgentId(IntPtr ensembleAgentPtr, int subAgentId)
        {
            var strPtr = TdlEnsembleAgentGetSubAgentIdInternal(ensembleAgentPtr, subAgentId);

            if (strPtr == IntPtr.Zero)
                return null;

            return Marshal.PtrToStringAnsi(strPtr);
        }

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersTdlEnsembleAgentAdd")]
        public static extern int TdlEnsembleAgentAdd(IntPtr ensembleAgentPtr, IntPtr tdlAgentPtr);


        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersTdlEnsembleAgentRemove")]
        public static extern bool TdlEnsembleAgentRemove(IntPtr ensembleAgentPtr, int subAgentId);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersTdlEnsembleAgentSetSingleAgentMode")]
        public static extern int TdlEnsembleAgentSetSingleAgentMode(IntPtr ensembleAgentPtr, bool setSingleAgentMode);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersTdlEnsembleAgentGetSingleAgentId")]
        public static extern int TdlEnsembleAgentGetSingleAgentId(IntPtr ensembleAgentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "PackCheckersTdlEnsembleAgent")]
        public static extern IntPtr PackTdlEnsembleAgent(IntPtr ensembleAgentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: TrainingCellInterface.DllName, EntryPoint = "CheckersTdlEnsembleAgentGetSubAgentPtr")]
        public static extern IntPtr TdlEnsembleAgentGetSubAgentPtr(IntPtr ensembleAgentPtr, int subAgentId);
        #endregion
    }
}
