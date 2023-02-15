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

namespace Monitor.Checkers
{
    /// <summary>
    /// Wrapper class for the naive AgentPack structure
    /// </summary>
    class AgentPack : Agent
    {
        /// <summary>
        /// Pointer to an instance of native object
        /// </summary>
        private IntPtr _packPtr;

        /// <summary>
        /// Constructs by initializing native resources from file
        /// </summary>
        public AgentPack(string filePath)
        {
            _packPtr = DllWrapper.AgentPackLoadFromFile(filePath);

            if (_packPtr == IntPtr.Zero)
                throw new Exception("Failed to load from file");
        }

        /// <summary>
        /// Packs the given agent
        /// </summary>
        public AgentPack(TdLambdaAgent agent)
        {
            if (agent == null)
                throw new Exception("Invalid input");

            _packPtr = DllWrapper.PackTdLambdaAgent(agent.Ptr);
        }

        public AgentPack(EnsembleAgent agent)
        {
            if (agent == null)
                throw new Exception("Invalid input");

            _packPtr = DllWrapper.PackTdlEnsembleAgent(agent.Ptr);
        }

        /// <summary>
        /// Releases native resources
        /// </summary>
        public override void Dispose()
        {
            if (_packPtr == IntPtr.Zero)
                return;

            if (!DllWrapper.AgentPackFree(_packPtr))
                throw new Exception("Failed to release agent pointer");

            _packPtr = IntPtr.Zero;
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Saves native resource to the given file on disk
        /// </summary>
        public void SaveToFile(string filePath)
        {
            if (!DllWrapper.AgentPackSaveToFile(_packPtr, filePath))
                throw new Exception("Saving failed");
        }

        /// <summary>
        /// Read-only access to the packed agent
        /// </summary>
        public override IntPtr Ptr => DllWrapper.AgentPackGetAgentPtr(_packPtr);

        /// <summary>
        /// Training mode parameter of the packed agent
        /// </summary>
        public bool TrainingMode
        {
            get => DllWrapper.AgentGetTrainingMode(Ptr).ToBool();

            set
            {
                if (!DllWrapper.AgentSetTrainingMode(Ptr, value))
                    throw new Exception("Failed to set parameter");
            }
        }

        /// <summary>
        /// Can train flag of the packed agent
        /// </summary>
        public bool CanTrain => DllWrapper.AgentGetCanTrainFlag(Ptr).ToBool();
    }
}
