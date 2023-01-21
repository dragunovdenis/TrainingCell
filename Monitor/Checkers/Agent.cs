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
}
