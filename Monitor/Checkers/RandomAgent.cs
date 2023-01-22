using System;

namespace Monitor.Checkers
{
    /// <summary>
    /// Wrapper for the corresponding native class
    /// </summary>
    class RandomAgent : Agent
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public RandomAgent()
        {
            _ptr = DllWrapper.ConstructRandomAgent();

            if (_ptr == null)
                throw new Exception("Failed to construct agent");
        }

        /// <summary>
        /// Method to dispose native resources
        /// </summary>
        public override void Dispose()
        {
            if (_ptr == IntPtr.Zero)
                return;

            if (!DllWrapper.FreeRandomAgent(_ptr))
                throw new Exception("Failed to release agent pointer");

            _ptr = IntPtr.Zero;
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Finalizer, just in case we forgot to call dispose
        /// </summary>
        ~RandomAgent()
        {
            Dispose();
        }
    }
}
