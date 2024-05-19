//Copyright (c) 2024 Denys Dragunov, dragunovdenis@gmail.com
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
using System.Linq;

namespace Monitor.Utils
{
    /// <summary>
    /// Functionality to facilitate selection of options (presumably on a checkerboard)
    /// </summary>
    internal class OptionSelector
    {
        private readonly IList<int> _options;
        private readonly IList<int> _optionsIds;
        private int _localOptionId;

        /// <summary>
        /// Constructor.
        /// </summary>
        public OptionSelector(IList<int> options, IList<int> optionIds = null)
        {
            if (options == null)
                throw new ArgumentException("Invalid collection of options.");

            if (optionIds == null)
                optionIds = Enumerable.Range(0, options.Count).ToArray();
            else if (options.Count != optionIds.Count)
                throw new ArgumentException("Inconsistent arguments.");

            _options = options.ToArray();
            _optionsIds = optionIds.ToArray();
        }

        /// <summary>
        /// Selects next move in the collection of moves.
        /// </summary>
        public void SelectNextMove()
        {
            if (_options.Count == 0)
            {
                _localOptionId = -1;
                return;
            }

            _localOptionId = (_localOptionId + 1) % _options.Count;
        }

        /// <summary>
        /// Selects previous move in the collection of moves.
        /// </summary>
        public void SelectPreviousMove()
        {
            if (_options.Count == 0)
            {
                _localOptionId = -1;
                return;
            }
            
            _localOptionId = (_localOptionId + _options.Count - 1) % _options.Count;
        }

        /// <summary>
        /// Index of the selected option.
        /// </summary>
        public int SelectedOptionId => _options.Count > 0 ? _optionsIds[_localOptionId] : -1;

        /// <summary>
        /// Returns the selected move.
        /// </summary>
        public int SelectedOption => _options.Count > 0 ? _options[_localOptionId] : -1;
    }
}
