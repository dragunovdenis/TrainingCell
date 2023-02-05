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
using System.Linq;

namespace Monitor.Checkers
{
    /// <summary>
    /// Wrapper for the corresponding native class
    /// </summary>
    internal sealed class TdLambdaAgent : Agent
    {
        private IntPtr _ptr;

        /// <summary>
        /// Pointer to the native agent
        /// </summary>
        public override IntPtr Ptr => _ptr;

        /// <summary>
        /// Constructor
        /// </summary>
        public TdLambdaAgent(uint[] layerDims,
            double explorationEpsilon, double lambda, double gamma, double alpha)
        {
            if (layerDims == null || layerDims.Length < 2 ||
                layerDims[0] != 32 || layerDims.Last() != 1)
                throw new Exception("Invalid input");

            _ptr = DllWrapper.ConstructTdLambdaAgent(layerDims, layerDims.Length,
                explorationEpsilon, lambda, gamma, alpha);

            if (Ptr == null)
                throw new Exception("Failed to construct agent");
        }

        /// <summary>
        /// Constructor
        /// </summary>
        public TdLambdaAgent(uint[] layerDims, ITdlParameters parameters) : this(layerDims, 0, 0, 0, 0)
        {
            SetTrainingParameters(parameters);
        }

        private TdLambdaAgent(IntPtr ptr)
        {
            _ptr = ptr;
        }

        /// <summary>
        /// Releases native resources
        /// </summary>
        public override void Dispose()
        {
            if (Ptr == IntPtr.Zero)
                return;

            if (!DllWrapper.FreeTdLambdaAgent(Ptr))
                throw new Exception("Failed to release agent pointer");

            _ptr = IntPtr.Zero;
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Finalizer, just in case we forgot to call dispose
        /// </summary>
        ~TdLambdaAgent()
        {
            Dispose();
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
                Id = Id,
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
            Id = parameters.Id;

            return true;
        }
    }
}
