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
    /// Wrapper for the corresponding native class
    /// </summary>
    sealed class RandomAgent : Agent
    {
        private IntPtr _ptr;

        /// <summary>
        /// Pointer to the native agent
        /// </summary>
        public override IntPtr Ptr => _ptr;

        /// <summary>
        /// Constructor
        /// </summary>
        public RandomAgent()
        {
            _ptr = DllWrapper.ConstructRandomAgent();

            if (Ptr == IntPtr.Zero)
                throw new Exception("Failed to construct agent");

            Name = $"Random ({Guid.NewGuid()})";
        }

        /// <summary>
        /// Method to dispose native resources
        /// </summary>
        public override void Dispose()
        {
            if (Ptr == IntPtr.Zero)
                return;

            if (!DllWrapper.FreeRandomAgent(Ptr))
                throw new Exception("Failed to release agent pointer");

            _ptr = IntPtr.Zero;
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Brief summary of the current agent instance
        /// </summary>
        public override string Summary => $"Random Agent: {Name} ({Id})";
    }
}
