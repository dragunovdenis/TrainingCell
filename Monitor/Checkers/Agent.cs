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
using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace Monitor.Checkers
{
    /// <summary>
    /// Read-only agent interface
    /// </summary>
    public interface IAgentReadOnly
    {
        /// <summary>
        /// Name of the agent
        /// </summary>
        string Name { get; }

        /// <summary>
        /// Unique identifier of the agent
        /// </summary>
        string Id { get; }

        /// <summary>
        /// Collection of the agent's records
        /// </summary>
        IList<string> Records { get; }
    }

    /// <summary>
    /// General interface for checkers agents
    /// </summary>
    public interface IAgent : IDisposable, IAgentReadOnly
    {
        /// <summary>
        /// Pointer to the underlying unmanaged agent
        /// </summary>
        IntPtr Ptr { get; }

        /// <summary>
        /// Adds the given record to the record-book of the agent
        /// </summary>
        void AddRecord(string record);

        /// <summary>
        /// Adds record containing information about completed training
        /// </summary>
        void AddTrainingCompletionRecord(IAgentReadOnly opponent, int totalGames, int totalWis);

        /// <summary>
        /// Adds record about a training that was not completed for some reason
        /// </summary>
        void AddTrainingFailRecord(IAgentReadOnly opponent, string errorMessage);
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
        /// Name of the agent
        /// </summary>
        public string Name
        {
            get => DllWrapper.AgentGetName(Ptr);
            set
            {
                if (Name == value)
                    return;

                if (!DllWrapper.AgentSetName(Ptr, value))
                    throw new Exception("Failed to set agent ID");

                OnPropertyChanged();
            }
        }

        /// <summary>
        /// Unique identifier of the agent
        /// </summary>
        public string Id => DllWrapper.AgentGetId(Ptr);

        /// <summary>
        /// Collection of the agent's records
        /// </summary>
        public IList<string> Records
        {
            get
            {
                var recordsCount = DllWrapper.AgentGetRecordsCount(Ptr);
                if (recordsCount <= 0)
                    return null;

                var result = new List<string>();
                for (var recordId = 0; recordId < recordsCount; recordId++)
                    result.Add(DllWrapper.AgentGetRecordById(Ptr, recordId));

                return result;
            }
        }

        /// <summary>
        /// Adds the given record to the record-book of the agent
        /// </summary>
        public void AddRecord(string record)
        {
            DllWrapper.AgentAddRecord(Ptr, record);
        }

        /// <summary>
        /// Adds record containing information about completed training
        /// </summary>
        public void AddTrainingCompletionRecord(IAgentReadOnly opponent, int totalGames, int totalWis)
        {
            AddRecord($"{DateTime.Now}; opponent: {opponent.Name}/{opponent.Id}; {totalGames} games; {totalWis} wins");
        }

        /// <summary>
        /// Adds record about a training that was not completed for some reason
        /// </summary>
        public void AddTrainingFailRecord(IAgentReadOnly opponent, string errorMessage)
        {
            AddRecord($"{DateTime.Now}; opponent: {opponent.Name}/{opponent.Id}; failed : {errorMessage}");
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
