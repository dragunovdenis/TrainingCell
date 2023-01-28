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
using System.Linq;

namespace Monitor.Checkers
{
    /// <summary>
    /// Wrapper for the corresponding native class
    /// </summary>
    class TdLambdaAgent : Agent
    {
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

            if (_ptr == null)
                throw new Exception("Failed to construct agent");
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
            if (_ptr == IntPtr.Zero)
                return;

            if (!DllWrapper.FreeTdLambdaAgent(_ptr))
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
            if (!DllWrapper.SaveTdLambdaAgent(_ptr, filePath))
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
            get => DllWrapper.TdLambdaAgentGetEpsilon(_ptr);

            set
            {
                if (!DllWrapper.TdLambdaAgentSetEpsilon(_ptr, value))
                    throw new Exception("Failed to set parameter");
            }
        }

        /// <summary>
        /// Lambda parameter
        /// </summary>
        public double Lambda
        {
            get => DllWrapper.TdLambdaAgentGetLambda(_ptr);

            set
            {
                if (!DllWrapper.TdLambdaAgentSetLambda(_ptr, value))
                    throw new Exception("Failed to set parameter");
            }
        }

        /// <summary>
        /// Discount (gamma) parameter
        /// </summary>
        public double Discount
        {
            get => DllWrapper.TdLambdaAgentGetGamma(_ptr);

            set
            {
                if (!DllWrapper.TdLambdaAgentSetGamma(_ptr, value))
                    throw new Exception("Failed to set parameter");
            }
        }

        /// <summary>
        /// Discount (gamma) parameter
        /// </summary>
        public double LearningRate
        {
            get => DllWrapper.TdLambdaAgentGetLearningRate(_ptr);

            set
            {
                if (!DllWrapper.TdLambdaAgentSetLearningRate(_ptr, value))
                    throw new Exception("Failed to set parameter");
            }
        }

        /// <summary>
        /// Training mode parameter
        /// </summary>
        public bool TrainingMode
        {
            get => DllWrapper.AgentGetTrainingMode(_ptr).ToBool();

            set
            {
                if (!DllWrapper.AgentSetTrainingMode(_ptr, value))
                    throw new Exception("Failed to set parameter");
            }
        }
    }
}
