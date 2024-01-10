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
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Controls;
using Monitor.Agents;
using Monitor.Dll;

namespace Monitor.UI
{
    /// <summary>
    /// Interaction logic for EnsembleAgentControl.xaml
    /// </summary>
    public partial class EnsembleAgentControl : UserControl, INotifyPropertyChanged
    {
        /// <summary>
        /// Representation of an agent in the ListBox
        /// </summary>
        public class AgentItem : INotifyPropertyChanged
        {
            /// <summary>
            /// Read-only access to the underlying agent
            /// </summary>
            public ITdLambdaAgentReadOnly Agent { get; }

            /// <summary>
            /// Index of the agent 
            /// </summary>
            internal int Index { get; }

            /// <summary>
            /// Indicates whether the agent associated with the item can be added
            /// (or it is already in the ensemble and thus can be only removed)
            /// </summary>
            internal bool CanBeAdded { get; }

            /// <summary>
            /// Constructor
            /// </summary>
            internal AgentItem(ITdLambdaAgentReadOnly agent, int index, bool canBeAdded)
            {
                Agent = agent;
                Index = index;
                CanBeAdded = canBeAdded;
                Summary = Agent.Summary;
            }

            public bool _checked;

            /// <summary>
            /// Property representing if the agent is checked in the ListBox
            /// </summary>
            public bool Checked
            {
                get => _checked;
                set
                {
                    if (_checked != value)
                    {
                        _checked = value;
                        OnPropertyChanged();
                    }
                }
            }

            private bool _isEnabled;

            /// <summary>
            /// Returns "true" if the current item is available for selection.
            /// </summary>
            public bool IsEnabled
            {
                get => _isEnabled;

                set
                {
                    if (value != _isEnabled)
                    {
                        _isEnabled = value;
                        OnPropertyChanged();
                    }
                }
            }

            /// <summary>
            /// Summary of the agent item which is supposed to be shown as a tooltip.
            /// </summary>
            public string Summary { get; }

            // Event handler
            public event PropertyChangedEventHandler PropertyChanged;

            /// <summary>
            /// Notifies about change of a property with the given name
            /// </summary>
            protected virtual void OnPropertyChanged([CallerMemberName] string propertyName = null)
            {
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        EnsembleAgent _ensemble = null;

        /// <summary>
        /// Dependent collection of checked agents (those that need to be added to the corresponding ensemble)
        /// </summary>
        public IList<ITdLambdaAgentReadOnly> GetCheckedAgents()
        {
            return Agents.Where(x => x.CanBeAdded && x.Checked).Select(x => x.Agent).ToList();
        }

        /// <summary>
        /// Returns indices of unchecked sub-agents (sub-agents that need to be removed from the corresponding ensemble)
        /// </summary>
        public IList<int> GetUnCheckedSubAgentIds()
        {
            return Agents.Where(x => !x.CanBeAdded && !x.Checked).Select(x => x.Index).ToList();
        }

        /// <summary>
        /// Creates ensemble based on the user input
        /// </summary>
        public EnsembleAgent CreateEnsemble()
        {
            var checkedAgents = GetCheckedAgents();

            if (checkedAgents.Count == 0)
                return null;

            var result = new EnsembleAgent(null)
            {
                Name = AgentName,
                SingleAgentMode = UseSingleAgent,
            };

            result.AddSubAgents(checkedAgents);
            return result;
        }

        /// <summary>
        /// Collection of agents
        /// </summary>
        public ObservableCollection<AgentItem> Agents { get; set; } = new ObservableCollection<AgentItem>();

        /// <summary>
        /// Constructor
        /// </summary>
        public EnsembleAgentControl()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Updates agent items to prevent incompatible agents from being selected into the same ensemble.
        /// </summary>
        private void UpdateAgentCompatibility(DllWrapper.StateTypeId ensembleSpecialization)
        {
            foreach (var agentItem in Agents)
                agentItem.IsEnabled = (int)(ensembleSpecialization & agentItem.Agent.StateTypeId) != 0;
        }

        /// <summary>
        /// Based on the agent items that are currently selected deduces
        /// "current" specialization of the ensemble "under construction".
        /// </summary>
        private DllWrapper.StateTypeId DeduceCurrentSpecialization()
        {
            var result = DllWrapper.StateTypeId.All;

            foreach (var agentItem in Agents)
            {
                if (agentItem.Checked)
                {
                    result &= agentItem.Agent.StateTypeId;

                    // sanity check
                    if (!agentItem.IsEnabled)
                        throw new Exception("Checked agent bust be enabled");
                }
            }

            // sanity check
            if (result == DllWrapper.StateTypeId.Invalid)
                throw new Exception("Checked agents are incompatible.");
            
            return result;
        }

        /// <summary>
        /// Method to assign collection of available agents
        /// </summary>
        public void Assign(IEnumerable<ITdLambdaAgentReadOnly> agents, EnsembleAgent ensemble = null)
        {
            Agents = new ObservableCollection<AgentItem>(agents.Select((x, i) => new AgentItem(x, i, canBeAdded:true)));

            if (!AssignFromEnsemble(ensemble))
                UpdateAgentCompatibility(DllWrapper.StateTypeId.All);

            OnPropertyChanged(nameof(Agents));
        }

        /// <summary>
        /// Assigns properties of the control from the given ensemble.
        /// Returns "true" if succeeded.
        /// </summary>
        bool AssignFromEnsemble(EnsembleAgent ensemble)
        {
            if (ensemble == null)
                return false;

            _ensemble = ensemble;

            var subAgents = _ensemble.GetSubAgents();

            for (var subAgentId = 0; subAgentId < subAgents.Count; subAgentId++)
                Agents.Add(new AgentItem(subAgents[subAgentId], subAgentId, canBeAdded: false)
                {
                    Checked = true,
                });

            AgentName = _ensemble.Name;
            AgentId = _ensemble.Id;
            UseSingleAgent = _ensemble.SingleAgentMode;
            SearchMode = _ensemble.SearchMode;
            SearchIterations = _ensemble.SearchIterations;
            SearchDepth = _ensemble.SearchDepth;
            RunMultiThreaded = _ensemble.RunMultiThreaded;

            UpdateAgentCompatibility(_ensemble.StateTypeId);

            return true;
        }

        /// <summary>
        /// Update all the properties of the ensemble. Throws exception if ensemble is not assigned.
        /// </summary>
        public void UpdateEnsemble()
        {
            if (_ensemble == null)
                throw new Exception("Ensemble is not assigned.");

            _ensemble.RemoveSubAgents(GetUnCheckedSubAgentIds());
            _ensemble.AddSubAgents(GetCheckedAgents());
            _ensemble.Name = AgentName;
            _ensemble.SingleAgentMode = UseSingleAgent;
            _ensemble.SearchMode = SearchMode;
            _ensemble.SearchIterations = SearchIterations;
            _ensemble.SearchDepth = SearchDepth;
            _ensemble.RunMultiThreaded = RunMultiThreaded;
        }

        private string _agentName = "Ensemble-" + Guid.NewGuid();

        /// <summary>
        /// Agent identifier
        /// </summary>
        public string AgentName
        {
            get => _agentName;
            set => SetField(ref _agentName, value);
        }

        private string _agentId = "To be assigned upon construction";

        /// <summary>
        /// Unique identifier of the agent
        /// </summary>
        public string AgentId
        {
            get => _agentId;
            private set => SetField(ref _agentId, value);
        }

        private bool _useSingleAgent;

        /// <summary>
        /// Property determining whether ensemble uses all the
        /// agents in its to take decision or a single randomly chosen agent
        /// </summary>
        public bool UseSingleAgent
        {
            get => _useSingleAgent;
            set => SetField(ref _useSingleAgent, value);
        }

        private bool _searchMode;
        
        /// <summary>
        /// "Search mode" property of the ensemble.
        /// </summary>
        public bool SearchMode
        {
            get => _searchMode;
            set => SetField(ref _searchMode, value);
        }

        private int _searchIterations;

        /// <summary>
        /// "Search iterations" property of the ensemble.
        /// </summary>
        public int SearchIterations
        {
            get => _searchIterations;
            set => SetField(ref _searchIterations, value);
        }

        private int _searchDepth;

        /// <summary>
        /// "Search depth" property of the ensemble.
        /// </summary>
        public int SearchDepth
        {
            get => _searchDepth;
            set => SetField(ref _searchDepth, value);
        }

        private bool _runMultiThreaded;

        
        /// <summary>
        /// "Run multi-threaded" property of the ensemble.
        /// </summary>
        public bool RunMultiThreaded
        {
            get => _runMultiThreaded;
            set => SetField(ref _runMultiThreaded, value);
        }
        
        public event PropertyChangedEventHandler PropertyChanged;

        /// <summary>
        /// Notifies about change of a property with the given name
        /// </summary>
        protected virtual void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        /// <summary>
        /// Setter for a property with notification
        /// </summary>
        protected bool SetField<T>(ref T field, T value, [CallerMemberName] string propertyName = null)
        {
            if (EqualityComparer<T>.Default.Equals(field, value)) return false;
            field = value;
            OnPropertyChanged(propertyName);
            return true;
        }

        /// <summary>
        /// Event handler.
        /// </summary>
        private void CheckBox_OnChecked(object sender, RoutedEventArgs e)
        {
            var checkBox = (CheckBox)sender;
            if (checkBox.IsChecked.HasValue)
            {
                var currentSpecialization = DeduceCurrentSpecialization();
                UpdateAgentCompatibility(currentSpecialization);
            }
        }
    }
}
