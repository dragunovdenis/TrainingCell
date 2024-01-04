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
using Monitor.DataStructures;

namespace Monitor.Dll
{
    /// <summary>
    /// Wrapping functionality to communicate with the code inside native dll
    /// </summary>
    public static class DllWrapper
    {
        private const string DllName = "TrainingCellDll.dll";

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
        /// Boolean to integer conversion.
        /// </summary>
        public static int ToInt(this bool val)
        {
            return val ? 1 : 0;
        }
        
        /// <summary>
        /// Checkers move report delegate
        /// </summary>
        public delegate void PublishStateCallBack(
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)]
            int[] state, int stateSize,
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)]
            SubMove[] subMoves, int subMovesCount, IntPtr agentToPlayPtr);

        /// <summary>
        /// Checkers statistics report delegate
        /// </summary>
        public delegate void PublishGameStatsCallBack(
            [MarshalAs(UnmanagedType.U1)]
            bool whiteWon,
            [MarshalAs(UnmanagedType.U1)]
            bool blackWon,
            int totalGames);

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
            uint[] array);

        /// <summary>
        /// Represents state type identifier.
        /// Should be kept in sync with the corresponding data type on C++ side.
        /// </summary>
        [Flags]
        public enum StateTypeId : int
        {
            All = -1,
            Invalid = 0,
            Checkers = 1 << 0,
            Chess = 1 << 1,
        }

        /// <summary>
        /// Playing/training statistics. Should match the corresponding data structure on C++ side.
        /// </summary>
        [StructLayout(LayoutKind.Sequential)]
        public struct Stats
        {
            /// <summary>
            /// Number of "black" wins.
            /// </summary>
            public int BlacksWinCount;
            
            /// <summary>
            /// Number of "white" wins.
            /// </summary>
            public int WhitesWinCount;
            
            /// <summary>
            /// Total number of games played.
            /// </summary>
            public int TotalEpisodesCount;
        }

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        public static extern int Play(IntPtr agent1, IntPtr agent2, int episodes,
            [MarshalAs(UnmanagedType.I4)]
            StateTypeId stateType,
            [MarshalAs(UnmanagedType.FunctionPtr)]
            PublishStateCallBack publishStateCallBack,
            [MarshalAs(UnmanagedType.FunctionPtr)]
            PublishGameStatsCallBack publishStats,
            [MarshalAs(UnmanagedType.FunctionPtr)]
            CancelCallBack cancel,
            [MarshalAs(UnmanagedType.FunctionPtr)]
            ErrorMessageCallBack error, out Stats stats);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        public static extern int Train(IntPtr agent1, IntPtr agent2, int episodes,
            [MarshalAs(UnmanagedType.I4)]
            StateTypeId stateType,
            [MarshalAs(UnmanagedType.FunctionPtr)]
            PublishGameStatsCallBack publishStats,
            [MarshalAs(UnmanagedType.FunctionPtr)]
            CancelCallBack cancel,
            [MarshalAs(UnmanagedType.FunctionPtr)]
            ErrorMessageCallBack error, out Stats stats);

        /// <summary>
        /// Compound play-or-train operation.
        /// </summary>
        public static int PlayOrTrain(IntPtr agent1, IntPtr agent2, int episodes,
            StateTypeId stateType,
            PublishStateCallBack publishStateCallBack,
            PublishGameStatsCallBack publishStats,
            CancelCallBack cancel,
            ErrorMessageCallBack error, out Stats stats, bool train = false)
        {
            if (train)
                return Train(agent1, agent2, episodes, stateType, publishStats, cancel, error, out stats);

            return Play(agent1, agent2, episodes, stateType, publishStateCallBack, publishStats, cancel, error, out stats);
        }
        
        #region Random Agent
        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        public static extern IntPtr ConstructRandomAgent();

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool FreeRandomAgent(IntPtr agentPtr);
        #endregion

        #region TD(Lambda)-Agent
        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        public static extern IntPtr ConstructTdLambdaAgent(
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)]
            uint[] layerDims,
            int layerDimsSize, double explorationEpsilon, double lambda, double gamma, double alpha,
            [MarshalAs(UnmanagedType.I4)] StateTypeId stateTypeId);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName, EntryPoint = "TdLambdaAgentCreateCopy")]
        public static extern IntPtr CopyTdLambdaAgent(IntPtr agentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool TdLambdaAgentsAreEqual(IntPtr agentPtr0, IntPtr agentPtr1);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName, EntryPoint = "TdLambdaAgentLoadFromFile", CharSet = CharSet.Ansi)]
        public static extern IntPtr LoadTdLambdaAgent(string path);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName, EntryPoint = "TdLambdaAgentSaveToFile", CharSet = CharSet.Ansi)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool SaveTdLambdaAgent(IntPtr agentPtr, string path);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool TdLambdaAgentSetEpsilon(IntPtr agentPtr, double explorationEpsilon);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        public static extern double TdLambdaAgentGetEpsilon(IntPtr agentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool TdLambdaAgentSetLambda(IntPtr agentPtr, double lambda);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        public static extern double TdLambdaAgentGetLambda(IntPtr agentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool TdLambdaAgentSetGamma(IntPtr agentPtr, double gamma);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        public static extern double TdLambdaAgentGetGamma(IntPtr agentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool TdLambdaAgentSetLearningRate(IntPtr agentPtr, double learningRater);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        public static extern double TdLambdaAgentGetLearningRate(IntPtr agentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool TdLambdaAgentSetRewardFactor(IntPtr agentPtr, double rewardFactor);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        public static extern double TdLambdaAgentGetRewardFactor(IntPtr agentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool TdLambdaAgentSetSearchDepth(IntPtr agentPtr, int searchDepth);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        public static extern int TdLambdaAgentGetSearchDepth(IntPtr agentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool TdLambdaAgentGetNetDimensions(IntPtr agentPtr, AcquireArrayCallBack catchArray);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool TdLambdaAgentEqual(IntPtr agentPtr1, IntPtr agentPtr2);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool FreeTdLambdaAgent(IntPtr agentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        public static extern byte TdLambdaAgentGetSearchMode(IntPtr agentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool TdLambdaAgentSetSearchMode(IntPtr agentPtr,
            [MarshalAs(UnmanagedType.U1)]
            bool searchMode);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        public static extern int TdLambdaAgentGetSearchModeIterations(IntPtr agentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool TdLambdaAgentSetSearchModeIterations(IntPtr agentPtr, int searchIterations);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        public static extern IntPtr PackTdLambdaAgent(IntPtr agentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        public static extern byte TdLambdaAgentGetPerformanceEvaluationMode(IntPtr agentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool TdLambdaAgentSetPerformanceEvaluationMode(IntPtr agentPtr,
            [MarshalAs(UnmanagedType.U1)] bool mode);

        #endregion

        #region Interactive Agent

        /// <summary>
        /// Data transferring object to pass moves
        /// </summary>
        [StructLayout(LayoutKind.Sequential)]
        public struct MoveDto
        {
            /// <summary>
            ///	Pointer to the array of sub_moves
            /// </summary>
            public IntPtr SubMoves;

            /// <summary>
            ///	Number of sub-moves in the array
            /// </summary>
            public int SubMovesCnt;

            /// <summary>
            /// Rank of the corresponding piece after the move was taken
            /// (can differ from the original rank of the piece in case of a "transformation" move).
            /// </summary>
            public int FinalPieceRank;
        };

        /// <summary>
        /// "Make move" delegate
        /// </summary>
        public delegate int MakeMoveCallBack(
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)]
            int[] state, int stateSize,
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)]
            MoveDto[] moves, int movesSize);

        /// <summary>
        /// "Game over" delegate
        /// </summary>
        public delegate void GameOverCallBack(
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)]
            int[] state, int stateSize, int gameResult);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        public static extern IntPtr ConstructInteractiveAgent(
            [MarshalAs(UnmanagedType.FunctionPtr)]
            MakeMoveCallBack makeMoveCallBack,
            [MarshalAs(UnmanagedType.FunctionPtr)]
            GameOverCallBack gameOverCallBack,
            [MarshalAs(UnmanagedType.U1)]
            bool playForWhites);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool FreeInteractiveAgent(IntPtr agentPtr);
        #endregion

        #region Agent
        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool AgentSetTrainingMode(IntPtr agentPtr,
            [MarshalAs(UnmanagedType.U1)]
            bool trainingMode);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        public static extern byte AgentGetTrainingMode(IntPtr agentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        public static extern byte AgentGetCanTrainFlag(IntPtr agentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName, EntryPoint = "AgentGetId")]
        private static extern IntPtr AgentGetIdInternal(IntPtr agentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        [return: MarshalAs(UnmanagedType.I4)]
        public static extern StateTypeId AgentGetStateTypeId(IntPtr agentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName, EntryPoint = "AgentGetName")]
        private static extern IntPtr AgentGetNameInternal(IntPtr agentPtr);

        /// <summary>
        /// Converts given pointer to ANSI string
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
        [DllImport(dllName: DllName, CharSet = CharSet.Ansi)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool AgentSetName(IntPtr agentPtr, string id);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        public static extern int AgentGetRecordsCount(IntPtr agentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName, EntryPoint = "AgentGetRecordById")]
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
        [DllImport(dllName: DllName, CharSet = CharSet.Ansi)]
        public static extern int AgentAddRecord(IntPtr agentPtr, string record);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        [return:MarshalAs(UnmanagedType.U1)]
        public static extern bool CanPlay(IntPtr agent0Ptr, IntPtr agent1Ptr, [MarshalAs(UnmanagedType.I4)] out StateTypeId suggestedStateTypeId);

        #endregion

        #region AgentPack
        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName, CharSet = CharSet.Ansi)]
        public static extern IntPtr AgentPackLoadFromFile(string path);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName, CharSet = CharSet.Ansi)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool AgentPackSaveToFile(IntPtr agentPackPtr, string path);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool AgentPackFree(IntPtr agentPackPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        public static extern IntPtr AgentPackGetAgentPtr(IntPtr agentPackPtr);
        #endregion

        #region EnsembleAgent
        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        public static extern IntPtr ConstructTdlEnsembleAgent(int subAgentCount,
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 0)]
            IntPtr[] tdlAgents);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool FreeTdlEnsembleAgent(IntPtr ensembleAgentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName, CharSet = CharSet.Ansi)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool SaveTdlEnsembleAgent(IntPtr ensembleAgentPtr, string filePath);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName, CharSet = CharSet.Ansi)]
        public static extern IntPtr LoadTdlEnsembleAgent(string filePath);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        public static extern int TdlEnsembleAgentGetSize(IntPtr ensembleAgentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName, EntryPoint = "TdlEnsembleAgentGetSubAgentId")]
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
        [DllImport(dllName: DllName)]
        public static extern int TdlEnsembleAgentAdd(IntPtr ensembleAgentPtr, IntPtr tdlAgentPtr);


        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        [return: MarshalAs(UnmanagedType.U1)]
        public static extern bool TdlEnsembleAgentRemove(IntPtr ensembleAgentPtr, int subAgentId);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        public static extern int TdlEnsembleAgentSetSingleAgentMode(IntPtr ensembleAgentPtr, bool setSingleAgentMode);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        public static extern int TdlEnsembleAgentGetSingleAgentId(IntPtr ensembleAgentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        public static extern IntPtr PackTdlEnsembleAgent(IntPtr ensembleAgentPtr);

        /// <summary>
        /// Wrapper for the corresponding method
        /// </summary>
        [DllImport(dllName: DllName)]
        public static extern IntPtr TdlEnsembleAgentGetSubAgentPtr(IntPtr ensembleAgentPtr, int subAgentId);
        #endregion
    }
}
