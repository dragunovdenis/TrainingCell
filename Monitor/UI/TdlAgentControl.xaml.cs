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
using System.Windows.Controls;
using Monitor.Agents;
using Monitor.Dll;

namespace Monitor.UI
{
    /// <summary>
    /// Interaction logic for TdlAgentControl.xaml
    /// </summary>
    public partial class TdlAgentControl : UserControl, INotifyPropertyChanged
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public TdlAgentControl()
        {
            InitializeComponent();
        }

        private TdLambdaAgent _agent = null;

        /// <summary>
        /// "Connects" agent to be edited
        /// </summary>
        internal void SetEdit(TdLambdaAgent agent)
        {
            _agent = agent ?? throw new Exception("Invalid agent to edit");
            Params = agent.GetTrainingParameters();
            HiddenLayerDimensions = agent.NetDimensions;
            AgentId = agent.Id;
            Records = agent.Records;
            EditMode = true;
        }

        /// <summary>
        /// In the current implementation there are some hidden
        /// dependencies between parameters of TD-lambda agent.
        /// In order to update UI accordingly we need to do the synchronization operation below.
        /// </summary>
        private void TrySynchronizeParametersWithAgent()
        {
            if (_agent == null)
                return;

            _agent.SetTrainingParameters(Params);
            Params = _agent.GetTrainingParameters();
        }

        /// <summary>
        /// Creates agent according to the current settings
        /// </summary>
        internal TdLambdaAgent CreateAgent()
        {
            return new TdLambdaAgent(HiddenLayerDimensions.ToArray(), Params);
        }

        public event PropertyChangedEventHandler PropertyChanged;

        /// <summary>
        /// Property changed handler
        /// </summary>
        protected virtual void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        /// <summary>
        /// Property setter
        /// </summary>
        protected bool SetField<T>(ref T field, T value, [CallerMemberName] string propertyName = null)
        {
            if (EqualityComparer<T>.Default.Equals(field, value)) return false;
            field = value;
            OnPropertyChanged(propertyName);
            return true;
        }

        private bool _editMode;

        /// <summary>
        /// A toggle between edit and creation modes
        /// </summary>
        public bool EditMode
        {
            get => _editMode;
            set => SetField(ref _editMode, value);
        }

        private uint[] _hiddenLayerDimensions = {64, 32, 16, 8};

        /// <summary>
        /// Dimensions of the agent hidden neural layers
        /// </summary>
        public uint[] HiddenLayerDimensions
        {
            get => _hiddenLayerDimensions;
            set => SetField(ref _hiddenLayerDimensions, value);
        }

        private TdlParameters _params = new TdlParameters()
        {
            Epsilon = 0.05,
            Discount = 0.8,
            Lambda = 0.2,
            LearningRate = 0.01,
            TrainingMode = true,
            SearchMode = false,
            SearchIterations = 10,
            SearchDepth = 10000,
            SearchExplorationDepth = 10000,
            SearchExplorationVolume = 10000,
            SearchExplorationProbability = 0.05,
            RewardFactor = 1.0,
            Name = "Agent-" + Guid.NewGuid(),
            StateTypeId = DllWrapper.StateTypeId.Checkers,
        };

        /// <summary>
        /// Parameters that govern learning process of the agent (everything but the underlying neural net)
        /// </summary>
        private ITdlParameters Params
        {
            get => _params;
            set
            {
                if (!_params.Equals(value))
                {
                    _params = new TdlParameters(value);
                    OnPropertyChanged();
                    OnPropertyChanged(nameof(Lambda));
                    OnPropertyChanged(nameof(Discount));
                    OnPropertyChanged(nameof(Epsilon));
                    OnPropertyChanged(nameof(LearningRate));
                    OnPropertyChanged(nameof(TrainingMode));
                    OnPropertyChanged(nameof(AgentName));
                    OnPropertyChanged(nameof(SearchMode));
                    OnPropertyChanged(nameof(SearchIterations));
                    OnPropertyChanged(nameof(SearchDepth));
                    OnPropertyChanged(nameof(SearchExplorationDepth));
                    OnPropertyChanged(nameof(SearchExplorationVolume));
                    OnPropertyChanged(nameof(SearchExplorationProbability));
                    OnPropertyChanged(nameof(RewardFactor));
                    OnPropertyChanged(nameof(StateTypeId));
                    OnPropertyChanged(nameof(PerformanceEvaluationMode));
                }
            }
        }

        /// <summary>
        /// Exploration probability of the agent
        /// </summary>
        public double Epsilon
        {
            get => _params.Epsilon;
            set
            {
                if (!_params.Epsilon.Equals(value))
                {
                    _params.Epsilon = value;
                    OnPropertyChanged();
                    TrySynchronizeParametersWithAgent();
                }
            }
        }

        /// <summary>
        /// "Lambda" parameter of the agent
        /// </summary>
        public double Lambda
        {
            get => _params.Lambda;
            set
            {
                if (!_params.Lambda.Equals(value))
                {
                    _params.Lambda = value;
                    OnPropertyChanged();
                    TrySynchronizeParametersWithAgent();
                }
            }
        }

        /// <summary>
        /// Reward discount
        /// </summary>
        public double Discount
        {
            get => _params.Discount;
            set
            {
                if (!_params.Discount.Equals(value))
                {
                    _params.Discount = value;
                    OnPropertyChanged();
                    TrySynchronizeParametersWithAgent();
                }
            }
        }

        /// <summary>
        /// Learning rate
        /// </summary>
        public double LearningRate
        {
            get => _params.LearningRate;
            set
            {
                if (!_params.LearningRate.Equals(value))
                {
                    _params.LearningRate = value;
                    OnPropertyChanged();
                    TrySynchronizeParametersWithAgent();
                }
            }
        }

        /// <summary>
        /// Training mode flag of the agent
        /// </summary>
        public bool TrainingMode
        {
            get => _params.TrainingMode;
            set
            {
                if (!_params.TrainingMode.Equals(value))
                {
                    _params.TrainingMode = value;
                    OnPropertyChanged();
                    TrySynchronizeParametersWithAgent();
                }
            }
        }

        /// <summary>
        /// Search mode flag of the agent
        /// </summary>
        public bool SearchMode
        {
            get => _params.SearchMode;

            set
            {
                if (!_params.SearchMode.Equals(value))
                {
                    _params.SearchMode = value;
                    OnPropertyChanged();
                    TrySynchronizeParametersWithAgent();
                }
            }
        }

        /// <summary>
        /// Number of search iterations to do if the TD-tree search mode is on
        /// </summary>
        public int SearchIterations
        {
            get => _params.SearchIterations;

            set
            {
                if (!_params.SearchIterations.Equals(value))
                {
                    _params.SearchIterations = value;
                    OnPropertyChanged();
                    TrySynchronizeParametersWithAgent();
                }
            }
        }

        /// <summary>
        /// Number of first moves in each search iteration (episode)
        /// that result in an update of the search neural network.
        /// </summary>
        public int SearchDepth
        {
            get => _params.SearchDepth;

            set
            {
                if (!_params.SearchDepth.Equals(value))
                {
                    _params.SearchDepth = value;
                    OnPropertyChanged();
                    TrySynchronizeParametersWithAgent();
                }
            }
        }

        /// <summary>
        /// Number of first moves in each search iteration (episode)
        /// in scope of which an exploration action can be taken.
        /// </summary>
        public int SearchExplorationDepth
        {
            get => _params.SearchExplorationDepth;

            set
            {
                if (!_params.SearchExplorationDepth.Equals(value))
                {
                    _params.SearchExplorationDepth = value;
                    OnPropertyChanged();
                    TrySynchronizeParametersWithAgent();
                }
            }
        }

        /// <summary>
        /// Number moves with the highest reward values that are taken into account when
        /// an exploration move is picked (within the current <see cref="SearchExplorationDepth"/>).
        /// </summary>
        public int SearchExplorationVolume
        {
            get => _params.SearchExplorationVolume;

            set
            {
                if (!_params.SearchExplorationVolume.Equals(value))
                {
                    _params.SearchExplorationVolume = value;
                    OnPropertyChanged();
                    TrySynchronizeParametersWithAgent();
                }
            }
        }

        /// <summary>
        /// Probability of an exploration move be taken within the current <see cref="SearchExplorationDepth"/>.
        /// </summary>
        public double SearchExplorationProbability
        {
            get => _params.SearchExplorationProbability;

            set
            {
                if (!_params.SearchExplorationProbability.Equals(value))
                {
                    _params.SearchExplorationProbability = value;
                    OnPropertyChanged();
                    TrySynchronizeParametersWithAgent();
                }
            }
        }

        /// <summary>
        /// Scale factor for the value of the internal reward function of the agent
        /// </summary>
        public double RewardFactor
        {
            get => _params.RewardFactor;

            set
            {
                if (!_params.RewardFactor.Equals(value))
                {
                    _params.RewardFactor = value;
                    OnPropertyChanged();
                    TrySynchronizeParametersWithAgent();
                }
            }
        }

        /// <summary>
        /// State type ID of the agent.
        /// </summary>
        public DllWrapper.StateTypeId StateTypeId
        {
            get => _params.StateTypeId;

            set
            {
                if (!_params.StateTypeId.Equals(value))
                {
                    _params.StateTypeId = value;
                    OnPropertyChanged();
                    TrySynchronizeParametersWithAgent();
                }
            }
        }

        /// <summary>
        /// "Performance evaluation mode" flag.
        /// </summary>
        public bool PerformanceEvaluationMode
        {
            get => _params.PerformanceEvaluationMode;

            set
            {
                if (_params.PerformanceEvaluationMode != value)
                {
                    _params.PerformanceEvaluationMode = value;
                    OnPropertyChanged();
                    TrySynchronizeParametersWithAgent();
                }
            }
        }

        /// <summary>
        /// Accessible options for the state type ID parameter.
        /// </summary>
        public ObservableCollection<DllWrapper.StateTypeId> StateIdOptions { get; set; } =
            new ObservableCollection<DllWrapper.StateTypeId>()
                { DllWrapper.StateTypeId.Checkers, DllWrapper.StateTypeId.Chess };

        /// <summary>
        /// Agent's identifier
        /// </summary>
        public string AgentName
        {
            get => _params.Name;
            set
            {
                if (!_params.Name.Equals(value))
                {
                    _params.Name = value;
                    OnPropertyChanged();
                    TrySynchronizeParametersWithAgent();
                }
            }
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

        private IList<string> _records;

        /// <summary>
        /// Agent's records
        /// </summary>
        public IList<string> Records
        {
            get => _records;
            set => SetField(ref _records, value);
        }
    }
}
