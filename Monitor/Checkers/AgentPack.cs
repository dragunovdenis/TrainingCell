using System;

namespace Monitor.Checkers
{
    /// <summary>
    /// Wrapper class for the naive AgentPack structure
    /// </summary>
    class AgentPack : IAgent
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
        /// Releases native resources
        /// </summary>
        public void Dispose()
        {
            if (_packPtr == IntPtr.Zero)
                return;

            if (!DllWrapper.AgentPackFree(_packPtr))
                throw new Exception("Failed to release agent pointer");

            _packPtr = IntPtr.Zero;
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Finalizer, just in case we forgot to call dispose
        /// </summary>
        ~AgentPack()
        {
            Dispose();
        }

        /// <summary>
        /// Saves native resource to the given file on disk
        /// </summary>
        void SaveToFile(string filePath)
        {
            if (!DllWrapper.AgentPackSaveToFile(_packPtr, filePath))
                throw new Exception("Saving failed");
        }

        /// <summary>
        /// Read-only access to the packed agent
        /// </summary>
        public IntPtr Ptr => DllWrapper.AgentPackGetAgentPtr(_packPtr);

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
