﻿//Copyright (c) 2023 Denys Dragunov, dragunovdenis@gmail.com
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
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace Monitor.Checkers
{
    /// <summary>
    /// General interface for checkers agents
    /// </summary>
    public interface IAgent : IDisposable
    {
        /// <summary>
        /// Pointer to the underlying unmanaged agent
        /// </summary>
        IntPtr Ptr { get; }

        /// <summary>
        /// Identifier of the agent
        /// </summary>
        string Id { get; }
    }

    /// <summary>
    /// Abstract agent
    /// </summary>
    public abstract class Agent : IAgent, INotifyPropertyChanged
    {
        /// <summary>
        /// Pointer to the underlying unmanaged agent
        /// </summary>
        public abstract IntPtr Ptr { get; }

        /// <summary>
        /// Releases unmanaged resources
        /// </summary>
        public abstract void Dispose();

        /// <summary>
        /// Identifier of the agent
        /// </summary>
        public virtual string Id
        {
            get => DllWrapper.AgentGetId(Ptr);
            set
            {
                if (Id == value)
                    return;

                if (!DllWrapper.AgentSetId(Ptr, value))
                    throw new Exception("Failed to set agent ID");

                OnPropertyChanged();
            }
        }

        /// <summary>
        /// Finalizer
        /// </summary>
        ~Agent()
        {
            Dispose();
        }

        public event PropertyChangedEventHandler PropertyChanged;

        /// <summary>
        /// Calls property-changed notification for the property with the given name
        /// </summary>
        protected virtual void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        /// <summary>
        /// Field setter with property-changed notification
        /// </summary>
        protected bool SetField<T>(ref T field, T value, [CallerMemberName] string propertyName = null)
        {
            if (EqualityComparer<T>.Default.Equals(field, value)) return false;
            field = value;
            OnPropertyChanged(propertyName);
            return true;
        }
    }
}
