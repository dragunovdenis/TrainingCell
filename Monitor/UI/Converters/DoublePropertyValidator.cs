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

using System.Globalization;
using System.Windows.Controls;

namespace Monitor.UI.Converters
{
    /// <summary>
    /// Validator for double precision property
    /// </summary>
    class DoublePropertyValidator : ValidationRule
    {
        /// <summary>
        /// Minimal acceptable value for the property
        /// </summary>
        public double MinVal { get; set; }

        /// <summary>
        /// Maximal acceptable value for the property
        /// </summary>
        public double MaxVal { get; set; }

        /// <summary>
        /// Error message to show in case of failed validation
        /// </summary>
        public string ErrorMessage { get; set; }

        /// <summary>
        /// Validation method
        /// </summary>
        public override ValidationResult Validate(object value,
            CultureInfo cultureInfo)
        {
            var input = (double)(value ?? double.NaN);

            if (input <= MaxVal && input >= MinVal)
                return new ValidationResult(true, null);

            return new ValidationResult(false, ErrorMessage);
        }
    }
}
