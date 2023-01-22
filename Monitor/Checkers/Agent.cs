using System;

namespace Monitor.Checkers
{
    /// <summary>
    /// General interface for checkers agents
    /// </summary>
    interface IAgent : IDisposable
    {
        /// <summary>
        /// Pointer to the underlying unmanaged agent
        /// </summary>
        IntPtr Ptr { get; }
    }

    /// <summary>
    /// Abstract agent
    /// </summary>
    abstract class Agent : IAgent
    {
        /// <summary>
        /// Pointer to the unmanaged resource
        /// </summary>
        protected IntPtr _ptr;

        /// <summary>
        /// Pointer to the underlying unmanaged agent
        /// </summary>
        public IntPtr Ptr => _ptr;

        /// <summary>
        /// Releases unmanaged resources
        /// </summary>
        public abstract void Dispose();
    }
}
