using System;
using System.Globalization;
using System.Windows.Data;

namespace Monitor
{
    /// <summary>
    /// Boolean inverter for property binding
    /// </summary>
    class InvertBoolConverter : IValueConverter
    {
        /// <summary>
        /// Forward conversion
        /// </summary>
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
            => !(bool?)value ?? true;

        /// <summary>
        /// Backward conversion
        /// </summary>
        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
            => !(value as bool?);
    }
}
