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

using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Shapes;

namespace Monitor.Visualization
{
    /// <summary>
    /// Utility methods to draw on canvas
    /// </summary>
    internal class CanvasDrawingUtils
    {
        /// <summary>
        /// Draws a "shape" (ellipse, rectangle...) with the given position and radius on the given canvas
        /// </summary>
        public static void DrawShape<S>(double x, double y, double width, double height,
            Brush borderColor, Brush fillColor, Canvas cv)
            where S : Shape, new()
        {
            var shape = new S()
            {
                Width = width,
                Height = height,
                Stroke = borderColor,
                Fill = fillColor,
                StrokeThickness = fillColor == null ? 4 : 1,
            };

            cv.Children.Add(shape);

            shape.SetValue(Canvas.LeftProperty, x);
            shape.SetValue(Canvas.TopProperty, y);
        }
    }
}
