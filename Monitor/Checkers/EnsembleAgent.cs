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
using System.Collections;
using System.Collections.Generic;
using System.Linq;

namespace Monitor.Checkers
{
    /// <summary>
    /// Wrapper for the TdlEnsembleAgent on C++ side
    /// </summary>
    internal class EnsembleAgent : Agent
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
        /// Returns number of sub-agents in the underlying ensemble
        /// </summary>
        public int Size()
        {
            return DllWrapper.TdlEnsembleAgentGetSize(_ptr);
        }

        /// <summary>
        /// Returns string identifier of a sub-agent with the given index (taking values from zero to size minus one)
        /// </summary>
        public string GetSubAgentIdentifier(int subAgentIndex)
        {
            return DllWrapper.TdlEnsembleAgentGetSubAgentId(_ptr, subAgentIndex);
        }

        /// <summary>
        /// Adds copy of the given agent to the given ensemble
        /// </summary>
        public int AddSubAgent(TdLambdaAgent agentToAdd)
        {
            return DllWrapper.TdlEnsembleAgentAdd(_ptr, agentToAdd.Ptr);
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
        public int SetSingleAgentMode(bool setSingleAgentMode)
        {
            return DllWrapper.TdlEnsembleAgentSetSingleAgentMode(_ptr, setSingleAgentMode);
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
    }
}