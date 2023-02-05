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

namespace Monitor.Checkers
{
    /// <summary>
    /// General interface to access parameters of TD(lambda) agent
    /// </summary>
    internal interface ITdlParameters
    {
        /// <summary>
        /// Exploration probability
        /// </summary>
        double Epsilon { get; }

        /// <summary>
        /// The "lambda" parameter
        /// </summary>
        double Lambda { get; }

        /// <summary>
        /// Reward discount
        /// </summary>
        double Discount { get; }

        /// <summary>
        /// Learning rate of the semi-gradient method
        /// </summary>
        double LearningRate { get; }

        /// <summary>
        /// A toggle between the "training" and "non-training" modes
        /// </summary>
        bool TrainingMode { get; }

        /// <summary>
        /// Identifier
        /// </summary>
        string Id { get; }
    }

    /// <summary>
    /// Implementation of the corresponding interface
    /// </summary>
    internal class TdlParameters : ITdlParameters
    {
        /// <summary>
        /// Exploration probability
        /// </summary>
        public double Epsilon { get; set; }

        /// <summary>
        /// The "lambda" parameter
        /// </summary>
        public double Lambda { get; set; }

        /// <summary>
        /// Reward discount
        /// </summary>
        public double Discount { get; set; }

        /// <summary>
        /// Learning rate of the semi-gradient method
        /// </summary>
        public double LearningRate { get; set; }

        /// <summary>
        /// A toggle between the "training" and "non-training" modes
        /// </summary>
        public bool TrainingMode { get; set; }

        /// <summary>
        /// Identifier
        /// </summary>
        public string Id { get; set; }

        /// <summary>
        /// Default constructor
        /// </summary>
        public TdlParameters() {}

        /// <summary>
        /// A "copy" constructor
        /// </summary>
        public TdlParameters(ITdlParameters source)
        {
            if (source == null)
                throw new Exception("Invalid input");

            Epsilon = source.Epsilon;
            Discount = source.Discount;
            LearningRate = source.LearningRate;
            TrainingMode = source.TrainingMode;
            Id = source.Id;
        }
    }
}
