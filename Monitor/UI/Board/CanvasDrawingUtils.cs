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

using System.IO;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using Path = System.IO.Path;

namespace Monitor.UI.Board
{
    /// <summary>
    /// Utility methods to draw on canvas
    /// </summary>
    internal class CanvasDrawingUtils
    {
        /// <summary>
        /// Draws a "shape" (ellipse, rectangle...) with the given position and radius on the given canvas
        /// </summary>
        public static void DrawShape<S>(double x, double y, double width, double height, double strokeThickness,
            Brush borderColor, Brush fillColor, Canvas cv)
            where S : Shape, new()
        {
            cv.Children.Add(CreateShape<S>(x, y, width, height, strokeThickness, borderColor, fillColor));
        }

        /// <summary>
        /// Returns an instance of a "shape" (ellipse, rectangle...) with the given position on a canvas.
        /// </summary>
        public static S CreateShape<S>(double x, double y, double width, double height, double strokeThickness,
            Brush borderColor, Brush fillColor)
            where S : Shape, new()
        {
            var shape = new S()
            {
                Width = width,
                Height = height,
                Stroke = borderColor,
                Fill = fillColor,
                StrokeThickness = strokeThickness,
            };
            shape.SetValue(Canvas.LeftProperty, x);
            shape.SetValue(Canvas.TopProperty, y);

            return shape;
        }

        /// <summary>
        /// Saves content of the given canvas to the file with the given name in png format.
        /// </summary>
        public static void SaveCanvasToPng(Canvas canvas, string filename)
        {
            RenderTargetBitmap bitmap = new RenderTargetBitmap((int)canvas.ActualWidth, (int)canvas.ActualHeight,
                96d, 96d, PixelFormats.Pbgra32);
            bitmap.Render(canvas);
            PngBitmapEncoder encoder = new PngBitmapEncoder();
            encoder.Frames.Add(BitmapFrame.Create(bitmap));
            using (FileStream stream = new FileStream(filename, FileMode.Create))
            {
                encoder.Save(stream);
            }
        }

        /// <summary>
        /// Dumps current state of the canvas to the specified folder on disk.
        /// </summary>
        private void DumpCanvas(string folderPath, Canvas canvas, bool whiteWon, bool blackWon, int totalGamers)
        {
            if (whiteWon && blackWon)
            {
                SaveCanvasToPng(canvas, Path.Combine(folderPath, $"stale_mate_screen{totalGamers}.png"));
            }
            else if (whiteWon)
            {
                SaveCanvasToPng(canvas, Path.Combine(folderPath, $"white_won_screen{totalGamers}.png"));
            }
            else if (blackWon)
            {
                SaveCanvasToPng(canvas, Path.Combine(folderPath, $"black_won_screen{totalGamers}.png"));
            }
            else
            {
                SaveCanvasToPng(canvas, Path.Combine(folderPath, $"draw_screen{totalGamers}.png"));
            }
        }

    }
}
