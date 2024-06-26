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
using Monitor.Dll;

namespace Monitor.Agents
{
    /// <summary>
    /// Read-only interface of TD(lambda) agent
    /// </summary>
    public interface ITdLambdaAgentReadOnly : IAgentReadOnly
    {
        /// <summary>
        /// Saves agent to disk
        /// </summary>
        void SaveToFile(string filePath);

        /// <summary>
        /// Exploration epsilon parameter
        /// </summary>
        double Epsilon { get; }

        /// <summary>
        /// Lambda parameter
        /// </summary>
        double Lambda { get; }

        /// <summary>
        /// Discount (gamma) parameter
        /// </summary>
        double Discount { get; }

        /// <summary>
        /// Discount (gamma) parameter
        /// </summary>
        double LearningRate { get; }

        /// <summary>
        /// Training mode parameter
        /// </summary>
        bool TrainingMode { get; }

        /// <summary>
        /// Neural net dimensions of the agent
        /// </summary>
        uint[] NetDimensions { get; }

        /// <summary>
        /// Adds current agent to the given ensemble. This approach
        /// allows us not to expose native pointer through the "read-only"
        /// interface yet still makes it possible to add the agent to an ensemble
        /// Returns index of the added agent in the ensemble collection
        /// </summary>
        int AddToEnsemble(EnsembleAgent ensemble);

        /// <summary>
        /// Returns "true" if the current agent is equal to the given one
        /// </summary>
        bool IsEqualTo(IAgentReadOnly agent);
        
        /// <summary>
        /// Returns script representation of the agent.
        /// </summary>
        string Script { get; }
    }

    /// <summary>
    /// Interface for option evaluation functionality.
    /// </summary>
    public interface IOptionEvaluator
    {
        /// <summary>
        /// Evaluates options offered by the given state.
        /// </summary>
        double[] EvaluateOptions(IntPtr statePtr);
    }

    /// <summary>
    /// Wrapper for the corresponding native class
    /// </summary>
    public sealed class TdLambdaAgent : Agent, ITdLambdaAgentReadOnly, IOptionEvaluator
    {
        private IntPtr _ptr;

        /// <summary>
        /// Pointer to the native agent
        /// </summary>
        public override IntPtr Ptr => _ptr;

        /// <summary>
        /// Determines if the pointer should be disposed by the current instance of the agent
        /// </summary>
        private readonly bool _ownPointer = true;

        /// <summary>
        /// Adds current agent to the given ensemble.
        /// </summary>
        public int AddToEnsemble(EnsembleAgent ensemble)
        {
            if (ensemble == null)
                return -1;

            return ensemble.AddSubAgent(this);
        }

        /// <summary>
        /// Constructor
        /// </summary>
        public TdLambdaAgent(uint[] layerDims,
            double explorationEpsilon, double lambda, double gamma, double alpha, DllWrapper.StateTypeId stateTypeId)
        {
            if (layerDims == null)
                throw new Exception("Invalid input");

            _ptr = DllWrapper.ConstructTdLambdaAgent(layerDims, layerDims.Length,
                explorationEpsilon, lambda, gamma, alpha, stateTypeId);

            if (Ptr == IntPtr.Zero)
                throw new Exception("Failed to construct agent");
        }

        /// <summary>
        /// Constructor
        /// </summary>
        public TdLambdaAgent(uint[] layerDims, ITdlParameters parameters) :
            this(layerDims, 0, 0, 0, 0, parameters.StateTypeId)
        {
            SetTrainingParameters(parameters);
        }

        /// <summary>
        /// Constructor
        /// </summary>
        public TdLambdaAgent(IntPtr ptr, bool ownPointer = true)
        {
            _ptr = ptr;
            _ownPointer = ownPointer;
        }

        /// <summary>
        /// Returns "true" if the current agent is equal to the given one
        /// </summary>
        public bool IsEqualTo(IAgentReadOnly agent)
        {
            if (agent is TdLambdaAgent tdlAgent)
            {
                return DllWrapper.TdLambdaAgentsAreEqual(_ptr, tdlAgent._ptr);
            }

            return false;
        }

        /// <summary>
        /// Releases native resources
        /// </summary>
        public override void Dispose()
        {
            if (!_ownPointer)
                return;

            if (Ptr == IntPtr.Zero)
                return;

            if (!DllWrapper.FreeTdLambdaAgent(Ptr))
                throw new Exception("Failed to release pointed agent");

            _ptr = IntPtr.Zero;
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Saves agent to disk
        /// </summary>
        public void SaveToFile(string filePath)
        {
            if (!DllWrapper.SaveTdLambdaAgent(Ptr, filePath))
                throw new Exception("Failed to save");
        }

        /// <summary>
        /// Loads agent from file
        /// </summary>
        public static TdLambdaAgent LoadFromFile(string filePath)
        {
            var ptr = DllWrapper.LoadTdLambdaAgent(filePath);

            if (ptr == IntPtr.Zero)
                return null;

            return new TdLambdaAgent(ptr);
        }

        /// <summary>
        /// Exploration epsilon parameter
        /// </summary>
        public double Epsilon
        {
            get => DllWrapper.TdLambdaAgentGetEpsilon(Ptr);

            set
            {
                if (!Epsilon.Equals(value))
                {
                    if (!DllWrapper.TdLambdaAgentSetEpsilon(Ptr, value))
                        throw new Exception("Failed to set parameter");
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Lambda parameter
        /// </summary>
        public double Lambda
        {
            get => DllWrapper.TdLambdaAgentGetLambda(Ptr);

            set
            {
                if (!Lambda.Equals(value))
                {
                    if (!DllWrapper.TdLambdaAgentSetLambda(Ptr, value))
                        throw new Exception("Failed to set parameter");
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Discount (gamma) parameter
        /// </summary>
        public double Discount
        {
            get => DllWrapper.TdLambdaAgentGetGamma(Ptr);

            set
            {
                if (!Discount.Equals(value))
                {
                    if (!DllWrapper.TdLambdaAgentSetGamma(Ptr, value))
                        throw new Exception("Failed to set parameter");
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Discount (gamma) parameter
        /// </summary>
        public double LearningRate
        {
            get => DllWrapper.TdLambdaAgentGetLearningRate(Ptr);

            set
            {
                if (!LearningRate.Equals(value))
                {
                    if (!DllWrapper.TdLambdaAgentSetLearningRate(Ptr, value))
                        throw new Exception("Failed to set parameter");
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Training mode parameter
        /// </summary>
        public bool TrainingMode
        {
            get => DllWrapper.AgentGetTrainingMode(Ptr).ToBool();

            set
            {
                if (!TrainingMode.Equals(value))
                {
                    if (!DllWrapper.AgentSetTrainingMode(Ptr, value))
                        throw new Exception("Failed to set parameter");
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Flag indicating whether tree search mode is on or off
        /// </summary>
        public bool SearchMode
        {
            get => DllWrapper.TdLambdaAgentGetSearchMode(Ptr).ToBool();

            set
            {
                if (SearchMode != value)
                {
                    if (!DllWrapper.TdLambdaAgentSetSearchMode(Ptr, value))
                        throw new Exception("Failed to set parameter");
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Number of iterations to do if the search mode is on
        /// </summary>
        public int SearchIterations
        {
            get => DllWrapper.TdLambdaAgentGetSearchModeIterations(Ptr);

            set
            {
                if (SearchIterations != value)
                {
                    if (!DllWrapper.TdLambdaAgentSetSearchModeIterations(Ptr, value))
                        throw new Exception("Failed to set parameter");
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Number of first moves in each search iteration that result
        /// in update of the search neural network (afterstate value function)
        /// </summary>
        public int SearchDepth
        {
            get => DllWrapper.TdLambdaAgentGetSearchDepth(Ptr);

            set
            {
                if (SearchDepth != value)
                {
                    if (!DllWrapper.TdLambdaAgentSetSearchDepth(Ptr, value))
                        throw new Exception("Failed to set parameter");
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Number of first moves in each search episode in scope of which the exploration action can take place.
        /// The other two parameters that, in total, determine whether the exploration action actually will be taken are:
        /// <see cref="SearchExplorationVolume"/> and <see cref="SearchExplorationProbability"/>
        /// </summary>
        public int SearchExplorationDepth
        {
            get => DllWrapper.TdLambdaAgentGetSearchExplorationDepth(Ptr);

            set
            {
                if (SearchExplorationDepth != value)
                {
                    if (!DllWrapper.TdLambdaAgentSetSearchExplorationDepth(Ptr, value))
                        throw new Exception("Failed to set parameter");
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Number of moves with the highest reward values that will be taken into account
        /// when picking an exploration move during the search training.
        /// </summary>
        public int SearchExplorationVolume
        {
            get => DllWrapper.TdLambdaAgentGetSearchExplorationVolume(Ptr);

            set
            {
                if (SearchExplorationVolume != value)
                {
                    if (!DllWrapper.TdLambdaAgentSetSearchExplorationVolume(Ptr, value))
                        throw new Exception("Failed to set parameter");
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Probability that exploration action is taken within the current
        /// <see cref="SearchExplorationDepth"/> moves of a search training episode.
        /// </summary>
        public double SearchExplorationProbability
        {
            get => DllWrapper.TdLambdaAgentGetSearchExplorationProbability(Ptr);

            set
            {
                if (!SearchExplorationProbability.Equals(value))
                {
                    if (!DllWrapper.TdLambdaAgentSetSearchExplorationProbability(Ptr, value))
                        throw new Exception("Failed to set parameter");
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Scale factor for the value of internal reward function of the agent
        /// </summary>
        public double RewardFactor
        {
            get => DllWrapper.TdLambdaAgentGetRewardFactor(Ptr);

            set
            {
                if (!RewardFactor.Equals(value))
                {
                    if (!DllWrapper.TdLambdaAgentSetRewardFactor(Ptr, value))
                        throw new Exception("Failed to set parameter");
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Neural net dimensions of the agent
        /// </summary>
        public uint[] NetDimensions
        {
            get
            {
                uint[] result = null;
                if (!DllWrapper.TdLambdaAgentGetNetDimensions(Ptr, (size, array) =>
                    {
                        result = array;
                    }))
                    throw new Exception("Failed to acquire network dimensions");

                return result;
            }
        }

        /// <summary>
        /// "Performance evaluation mode" flag.
        /// </summary>
        public bool PerformanceEvaluationMode
        {
            get => DllWrapper.TdLambdaAgentGetPerformanceEvaluationMode(Ptr).ToBool();

            set
            {
                if (PerformanceEvaluationMode != value)
                {
                    if (!DllWrapper.TdLambdaAgentSetPerformanceEvaluationMode(Ptr, value))
                        throw new Exception("Failed to set parameter");
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Evaluates options offered by the given state.
        /// </summary>
        public double[] EvaluateOptions(IntPtr statePtr)
        {
            return DllWrapper.TdLambdaAgentEvaluateOptions(Ptr, statePtr);
        }
        
        /// <summary>
        /// Returns script representation of the agent.
        /// </summary>
        public string Script => DllWrapper.TdLambdaAgentGetScriptString(_ptr);

        /// <summary>
        /// Returns snapshot of training parameters (all but those related to the underlying neural net)
        /// </summary>
        public ITdlParameters GetTrainingParameters()
        {
            return new TdlParameters()
            {
                Epsilon = Epsilon,
                Lambda = Lambda,
                Discount = Discount,
                LearningRate = LearningRate,
                TrainingMode = TrainingMode,
                Name = Name,
                SearchMode = SearchMode,
                SearchIterations = SearchIterations,
                SearchDepth = SearchDepth,
                SearchExplorationDepth = SearchExplorationDepth,
                SearchExplorationVolume = SearchExplorationVolume,
                SearchExplorationProbability = SearchExplorationProbability,
                RewardFactor = RewardFactor,
                StateTypeId = StateTypeId,
                PerformanceEvaluationMode = PerformanceEvaluationMode,
            };
        }

        /// <summary>
        /// Sets training parameters of the agent
        /// </summary>
        public bool SetTrainingParameters(ITdlParameters parameters)
        {
            if (parameters == null)
                return false;

            Epsilon = parameters.Epsilon;
            Lambda = parameters.Lambda;
            Discount = parameters.Discount;
            LearningRate = parameters.LearningRate;
            TrainingMode = parameters.TrainingMode;
            Name = parameters.Name;
            SearchMode = parameters.SearchMode;
            SearchIterations = parameters.SearchIterations;
            SearchDepth = parameters.SearchDepth;
            SearchExplorationDepth = parameters.SearchExplorationDepth;
            SearchExplorationVolume = parameters.SearchExplorationVolume;
            SearchExplorationProbability = parameters.SearchExplorationProbability;
            RewardFactor = parameters.RewardFactor;
            PerformanceEvaluationMode = parameters.PerformanceEvaluationMode;

            // Sanity check
            if (StateTypeId != parameters.StateTypeId)
                throw new Exception("State type ID can't be updated");

            return true;
        }

        /// <summary>
        /// Brief summary of the current agent instance
        /// </summary>
        public override string Summary => Script + (Records != null ?  "\n\n" +string.Join(";\n", Records) : "");
    }
}
