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

namespace Monitor.Dll
{
    /// <summary>
    /// Wrapper for the methods of the corresponding dynamic link library
    /// </summary>
    class MemoryUtils
    {
        /// <summary>
        /// Generic method to read array of managed structs from a pointer to unmanaged memory
        /// </summary>
        internal static unsafe T[] ReadArrayOfStructs<T>(IntPtr source, int length)
        {
            var result = new T[length];
            var totalBytesToCopy = (long)length * Marshal.SizeOf(typeof(T));
            var destMemHandle = GCHandle.Alloc(result, GCHandleType.Pinned);
            Buffer.MemoryCopy(source.ToPointer(), destMemHandle.AddrOfPinnedObject().ToPointer(),
                totalBytesToCopy, totalBytesToCopy);
            destMemHandle.Free();

            return result;
        }
    }
}
