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

using Monitor.Checkers;
using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using Point = System.Windows.Point;

namespace Monitor.Visualization.PieceControllers
{
    /// <summary>
    /// Chess specific implementation of the corresponding interface.
    /// </summary>
    internal class ChessPieceController : IPieceController
    {
        /// <summary>
        /// See the summary of the interface method.
        /// </summary>
        public DllWrapper.GameKind GetGameKind => DllWrapper.GameKind.Chess;

        /// <summary>
        /// Representation of a single chess field with all the related data/
        /// </summary>
        class Field
        {
            /// <summary>
            /// IMPORTANT: the constant below should be kept in sync with "PieceController" on C++ side
            /// Number of bits that are reserved for a piece rank
            /// </summary>
            private const int RankBitsCount = 3;

            /// <summary>
            /// IMPORTANT: the constant below should be kept in sync with "PieceController" on C++ side
            /// Bitmask used to extract plain rank of a piece from the "full" piece token
            /// (that can possibly contain flags and sign)
            /// </summary>
            private const int MinBitMask = (1 << RankBitsCount) - 1;

            /// <summary>
            /// IMPORTANT: the constant below should be kept in sync with "PieceController" on C++ side
            /// Number of bits reserved for flags.
            /// </summary>
            private const int FlagBitsCount = 3;

            /// <summary>
            /// IMPORTANT: the constant below should be kept in sync with "PieceController" on C++ side
            /// A flag telling that the piece is in its initial position
            /// </summary>
            private const int InitialPositionFlag = 1 << RankBitsCount;

            /// <summary>
            /// IMPORTANT: the constant below should be kept in sync with "PieceController" on C++ side
            /// A flag that a Pawn piece should be marked with in case it can be captured under the "En Passant Rule".
            /// </summary>
            private const int EnPassantFlag = InitialPositionFlag << 1;

            /// <summary>
            /// IMPORTANT: the constant below should be kept in sync with "PieceController" on C++ side
            /// A flag to mark ani-pieces
            /// </summary>
            private const int AntiPieceFlag = InitialPositionFlag << 2;

            /// <summary>
            /// IMPORTANT: the constant below should be kept in sync with "PieceController" on C++ side
            /// Total number of bits (except the sign bit) set aside for the piece token.
            /// </summary>
            private const int TotalBitsCount = RankBitsCount + FlagBitsCount;

            /// <summary>
            /// IMPORTANT: the constant below should be kept in sync with "PieceController" on C++ side
            /// Mask to cover all the bits of the piece rank token including the sign bit.
            /// </summary>
            private const int BitMask = (1 << TotalBitsCount) - 1;

            /// <summary>
            /// IMPORTANT: the constant below should be kept in sync with "AttackController" on C++ side
            /// Number of bits used to encode "long range" attacks.
            /// </summary>
            private const int LongRangeAttackBitsCount = 8;

            /// <summary>
            /// IMPORTANT: the constant below should be kept in sync with "AttackController" on C++ side
            /// Number of bits used to encode single side attack (i.e., either rival or ally attack).
            /// </summary>
            private const int AttackPackageBitsCount = 10;

            /// <summary>
            /// IMPORTANT: the constant below should be kept in sync with "AttackController" on C++ side
            /// Bit mask to extract "long range" attacks
            /// </summary>
            private const int LongRangeAttackBitMask = (1 << LongRangeAttackBitsCount) - 1;

            private readonly int _data;

            /// <summary>
            /// Constructor.
            /// </summary>
            public Field(int fieldData)
            {
                _data = fieldData;
            }

            /// <summary>
            /// Returns signed ID of a piece on the current field (or 0 if the field is empty).
            /// </summary>
            public int SignedPieceId
            {
                get
                {
                    var unsignedId = _data & MinBitMask;

                    if ((_data & AntiPieceFlag) == 0)
                        return unsignedId;

                    return -unsignedId;
                }
            }

            /// <summary>
            /// Returns string representation of the attack data encoded in the lowest 10 bits of the given integer.
            /// </summary>
            static string AttackToString(int attackEncoded)
            {
                var longRangePart = attackEncoded & LongRangeAttackBitMask;
                var shortRangeAttackPresent = (attackEncoded & (1 << LongRangeAttackBitsCount)) != 0;
                var knightAttackPresent = (attackEncoded & (1 << (LongRangeAttackBitsCount + 1))) != 0;

                string result = "";

                if (longRangePart != 0)
                {
                    var longRangeStr = Convert.ToString(longRangePart, 2);
                    longRangeStr = longRangeStr.PadLeft(8, '0');
                    result += $"{longRangeStr}";
                }

                if (shortRangeAttackPresent)
                    result += " S";

                if (knightAttackPresent)
                    result += " K";

                return $"[{result}]";
            }

            /// <summary>
            /// Returns string representation of the rival attack data associated with the current field.
            /// </summary>
            public string RivalAttackStr
            {
                get
                {
                    var temp = _data >> TotalBitsCount;
                    return AttackToString(temp);
                }
            }

            /// <summary>
            /// Returns string representation of the ally attack data associated with the current field.
            /// </summary>
            public string AllyAttackStr
            {
                get
                {
                    var temp = _data >> (TotalBitsCount + AttackPackageBitsCount);
                    return AttackToString(temp);
                }
            }

        }

        /// <summary>
        /// Streams with piece images.
        /// </summary>
        private readonly IDictionary<int, Stream> _steams = new Dictionary<int, Stream>();

        /// <summary>
        /// Flag determining whether debug info should be shown.
        /// </summary>
        private readonly bool _showDebugInfo = false;

        /// <summary>
        /// Constructor.
        /// </summary>
        public ChessPieceController(bool showDebugInfo = false)
        {
            _showDebugInfo = showDebugInfo;

            for (int i = -6; i <= 6; i++)
            {
                if (i == 0)
                    continue;

                _steams[i] = Assembly.GetExecutingAssembly().GetManifestResourceStream($"Monitor.Images.Chess.{i}.png");
            }
        }

        /// <summary>
        /// See the summary of the interface method.
        /// </summary>
        public void DrawPiece(Canvas canvas, Point topLeft, double fieldSize, int rawPieceId)
        {
            var field = new Field(rawPieceId);

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
                    Height = fieldSizeShrunk
                };

                Canvas.SetLeft(image, topLeftAdjusted.X);
                Canvas.SetTop(image, topLeftAdjusted.Y);

                canvas.Children.Add(image);
            }

            if (!_showDebugInfo)
                return;

            var textBlock = new TextBlock
            { FontSize = 10 };

            textBlock.Inlines.Add(new Run(field.AllyAttackStr)
            { Foreground = new SolidColorBrush(Colors.White) });

            textBlock.Inlines.Add(new Run(field.RivalAttackStr)
            { Foreground = new SolidColorBrush(Colors.Black) });

            Canvas.SetLeft(textBlock, topLeft.X);
            Canvas.SetTop(textBlock, topLeft.Y);
            canvas.Children.Add(textBlock);
        }

        /// <summary>
        /// See the summary of the interface method.
        /// </summary>
        public int GetCapturedPieceId(bool white)
        {
            return 0;
        }

        /// <summary>
        /// See the summary of the interface method.
        /// </summary>
        public int GetPieceTraceId(bool white)
        {
            return 0;
        }
    }
}
