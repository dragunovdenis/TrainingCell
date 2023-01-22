
using System;
using System.Runtime.InteropServices;

namespace Monitor
{
    /// <summary>
    /// Wrapper for the methods of the corresponding dynamic link library
    /// </summary>
    class TrainingCellInterface
    {
        internal const string DllName = "TrainingCellDll.dll";

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
