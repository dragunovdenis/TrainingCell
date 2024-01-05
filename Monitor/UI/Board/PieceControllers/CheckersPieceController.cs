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
using System.IO;
using System.Reflection;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media.Imaging;
using Monitor.Dll;

namespace Monitor.UI.Board.PieceControllers
{
    /// <summary>
    /// Checkers specific implementation of the corresponding interface.
    /// </summary>
    internal class CheckersPieceController : IPieceController
    {
        /// <summary>
        /// Representation of a single checkers field with all the related data.
        /// </summary>

        private class Field
        {
            private const int RankBitsCount = 2;
            private const int RankBitMask = (1 << RankBitsCount) - 1;
            private const int WhiteCapturedPieceId = 1 << RankBitsCount;
            private const int BlackCapturedPieceId = -(1 << RankBitsCount);

            /// <summary>
            /// Returns all the possible signed piece IDs.
            /// </summary>
            public static IEnumerable<int> GetSignedPieceIds()
            {
                for (var id = -2; id <= 2; id++)
                {
                    if (id == 0)
                        continue;

                    yield return id;
                }
            }
            
            /// <summary>
            /// Constructor.
            /// </summary>
            public Field(int fieldData)
            {
                var unsignedId = (Math.Abs(fieldData) & RankBitMask);
                if (unsignedId == 0)
                {
                    unsignedId = (Math.Abs(fieldData) >> RankBitsCount) & RankBitMask;
                    IsTrace = true;
                }
                
                SignedPieceId = fieldData > 0 ? unsignedId : -unsignedId;
            }

            /// <summary>
            /// Returns signed ID of a piece on the current field (or 0 if the field is empty).
            /// This value is supposed to have one-to-one correspondence with piece icons (images).
            /// </summary>
            public int SignedPieceId { get; }

            /// <summary>
            /// Returns "true" if the given field represents a "piece trace".
            /// </summary>
            public bool IsTrace { get; }

            /// <summary>
            /// Converts given piece ID to "piece trace" ID (i.e., identifier
            /// used to mark previous or possible next position of a piece).
            /// </summary>
            public static int GetPieceTraceId(int pieceId)
            {
                var unsignedId = (Math.Abs(pieceId) & RankBitMask) << RankBitsCount;
                return pieceId < 0 ? -unsignedId : unsignedId;
            }

            /// <summary>
            /// Returns ID of a "captured" piece by a piece of the given color.
            /// </summary>
            public static int GetPieceCapturedId(bool white)
            {
                return white ? BlackCapturedPieceId : WhiteCapturedPieceId;
            }
        }

        /// <summary>
        /// See the summary of the interface method.
        /// </summary>
        public DllWrapper.StateTypeId StateTypeId => DllWrapper.StateTypeId.Checkers;

        /// <summary>
        /// Streams with piece images.
        /// </summary>
        private readonly IDictionary<int, Stream> _steams = new Dictionary<int, Stream>();

        /// <summary>
        /// Constructor.
        /// </summary>
        public CheckersPieceController()
        {
            foreach (int id in Field.GetSignedPieceIds())
                _steams[id] = Assembly.GetExecutingAssembly().GetManifestResourceStream($"Monitor.Images.Checkers.{id}.png");
        }

        /// <summary>
        /// See the summary of the interface method.
        /// </summary>
        public void DrawPiece(Canvas canvas, Point topLeft, double fieldSize, int pieceId)
        {
            foreach (var element in CreatePieceElements(topLeft, fieldSize, pieceId))
                canvas.Children.Add(element);
        }

        /// <summary>
        /// See the summary of the interface method.
        /// </summary>
        public IEnumerable<UIElement> CreatePieceElements(Point topLeft, double fieldSize, int pieceId)
        {
            var field = new Field(pieceId);

            if (_steams.TryGetValue(field.SignedPieceId, out var steam))
            {
                var shrink = 0.1 * fieldSize;
                var fieldSizeShrunk = fieldSize - 2 * shrink;
                var topLeftAdjusted = new Point(topLeft.X + shrink, topLeft.Y + shrink);

                BitmapImage bitmap = new BitmapImage();
                bitmap.BeginInit();
                bitmap.StreamSource = steam;
                bitmap.EndInit();

                var image = new Image
                {
                    Source = bitmap,
                    Width = fieldSizeShrunk,
                    Height = fieldSizeShrunk,
                    Opacity = field.IsTrace ? 0.3 : 1.0,
                };

                Canvas.SetLeft(image, topLeftAdjusted.X);
                Canvas.SetTop(image, topLeftAdjusted.Y);

                yield return image;
            }
        }

        /// <summary>
        /// See the summary of the interface method.
        /// </summary>
        public int GetCapturedPieceId(bool white)
        {
            return Field.GetPieceCapturedId(white);
        }

        /// <summary>
        /// See the summary of the interface method.
        /// </summary>
        public int GetPieceTraceId(int pieceId)
        {
            return Field.GetPieceTraceId(pieceId);
        }
    }
}
