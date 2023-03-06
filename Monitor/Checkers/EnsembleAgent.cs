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
using System.Collections.Generic;
using System.Linq;

namespace Monitor.Checkers
{
    /// <summary>
    /// Wrapper for the TdlEnsembleAgent on C++ side
    /// </summary>
    public class EnsembleAgent : Agent
    {
        private IntPtr _ptr;

        /// <summary>
        /// Read-only access to the pointer to the native C++ structure
        /// </summary>
        public override IntPtr Ptr => _ptr;

        /// <summary>
        /// Constructor
        /// </summary>
        public EnsembleAgent(IList<TdLambdaAgent> collection)
        {
            if (collection == null)
                _ptr = DllWrapper.ConstructTdlEnsembleAgent(0, Array.Empty<IntPtr>());
            else
                _ptr = DllWrapper.ConstructTdlEnsembleAgent(collection.Count, 
                    collection.Select(x => x.Ptr).ToArray());
        }

        /// <summary>
        /// Returns read-only instance of the sub-agent with the given index.
        /// The is supposed to be between 0 and "size of the ensemble" - 1
        /// </summary>
        public ITdLambdaAgentReadOnly GetSubAgent(int subAgentId)
        {
            return new TdLambdaAgent(DllWrapper.TdlEnsembleAgentGetSubAgentPtr(Ptr, subAgentId), ownPointer: false);
        }

        /// <summary>
        /// Returns collection of the sub-agents in the order that corresponds to that of the native C++ agent
        /// </summary>
        public IList<ITdLambdaAgentReadOnly> GetSubAgents()
        {
            return Enumerable.Range(0, Size).Select(GetSubAgent).ToArray();
        }

        /// <summary>
        /// Constructor
        /// </summary>
        private EnsembleAgent(IntPtr ptr)
        {
            _ptr = ptr;
        }

        /// <summary>
        /// Frees unmanaged resources
        /// </summary>
        public override void Dispose()
        {
            if (_ptr == IntPtr.Zero)
                return;

            if (!DllWrapper.FreeTdlEnsembleAgent(_ptr))
                throw new Exception("Failed to release pointed agent");

            _ptr = IntPtr.Zero;
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Number of sub-agents in the underlying ensemble
        /// </summary>
        public int Size => DllWrapper.TdlEnsembleAgentGetSize(_ptr);

        /// <summary>
        /// Returns string identifier of a sub-agent with the given index (taking values from zero to size minus one)
        /// </summary>
        public string GetSubAgentIdentifier(int subAgentIndex)
        {
            return DllWrapper.TdlEnsembleAgentGetSubAgentId(_ptr, subAgentIndex);
        }

        /// <summary>
        /// Ids of all the sub-agents
        /// </summary>
        public IList<string> SubAgentIds => Enumerable.Range(0, Size).Select(GetSubAgentIdentifier).ToArray();

        /// <summary>
        /// Adds copy of the given agent to the given ensemble
        /// </summary>
        public int AddSubAgent(TdLambdaAgent agentToAdd)
        {
            return DllWrapper.TdlEnsembleAgentAdd(_ptr, agentToAdd.Ptr);
        }


        /// <summary>
        /// Adds given collection of agents to the ensemble
        /// </summary>
        public void AddSubAgents(IList<ITdLambdaAgentReadOnly> subAgents)
        {
            foreach (var agent in subAgents)
                agent.AddToEnsemble(this);
        }

        /// <summary>
        /// Removes sub-agents with the given indices
        /// </summary>
        public void RemoveSubAgents(IList<int> subAgentIds)
        {
            //remove sub-agents starting from the end of collection
            //so that indices do not get invalid after each removal
            foreach (var id in subAgentIds.OrderByDescending(i => i))
                RemoveSubAgent(id);
        }

        /// <summary>
        /// Removes sub-agent with the given index
        /// </summary>
        public bool RemoveSubAgent(int subAgentIndex)
        {
            return DllWrapper.TdlEnsembleAgentRemove(_ptr, subAgentIndex);
        }

        /// <summary>
        /// Sets "single agent mode" for the pointed native agent
        /// </summary>
        public bool SingleAgentMode
        {
            get => GetSingleAgentId() >= 0;
            set => DllWrapper.TdlEnsembleAgentSetSingleAgentMode(_ptr, value);
        }

        /// <summary>
        /// Returns index of a "single agent" if the corresponding mode is on or "-1" otherwise
        /// </summary>
        public int GetSingleAgentId()
        {
            return DllWrapper.TdlEnsembleAgentGetSingleAgentId(_ptr);
        }

        /// <summary>
        /// Saves the agent to the given file on disk, returns "true" if succeeded
        /// </summary>
        public bool SaveToFile(string filePath)
        {
            return DllWrapper.SaveTdlEnsembleAgent(Ptr, filePath);
        }

        /// <summary>
        /// Loads and returns an instance of ensemble agent from the given file
        /// Returns null if failed
        /// </summary>
        public static EnsembleAgent LoadFromFile(string filePath)
        {
            var ptr = DllWrapper.LoadTdlEnsembleAgent(filePath);

            if (ptr == IntPtr.Zero)
                return null;

            return new EnsembleAgent(ptr);
        }

        /// <summary>
        /// Brief summary of the current agent instance
        /// </summary>
        public override string Summary =>
            $"{Name} ({Id})\n" +
            $"Ensemble with {Size} sub-agents\n" +
            $"Single Agent Mode = {SingleAgentMode}\n";
    }
}