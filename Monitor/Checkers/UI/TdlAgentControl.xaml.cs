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
using System.Linq;
using System.Runtime.CompilerServices;
using System.Windows.Controls;

namespace Monitor.Checkers.UI
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

        /// <summary>
        /// "Connects" agent to be edited
        /// </summary>
        internal void SetEdit(TdLambdaAgent agent)
        {
            if (agent == null)
                throw new Exception("Invalid agent to edit");

            Params = agent.GetTrainingParameters();
            HiddenLayerDimensions = agent.NetDimensions;
            AgentId = agent.Id;
            Records = agent.Records;
            EditMode = true;
        }

        /// <summary>
        /// Creates agent according to the current settings
        /// </summary>
        internal TdLambdaAgent CreateAgent()
        {
            return new TdLambdaAgent(HiddenLayerDimensions.Prepend((uint)32).Append((uint)1).ToArray(), Params);
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
            SearchDepth = 1000,
            RewardFactor = 1.0,
            Name = "Agent-" + Guid.NewGuid(),
        };

        /// <summary>
        /// Parameters that govern learning process of the agent (everything but the underlying neural net)
        /// </summary>
        internal ITdlParameters Params
        {
            get => _params;
            private set
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
                    OnPropertyChanged(nameof(RewardFactor));
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
                }
            }
        }

        /// <summary>
        /// Number of first moves in each iteration that result in update of the search neural network
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
                }
            }
        }

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
