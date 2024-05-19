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
using System.Windows.Media;
using Monitor.DataStructures;
using Monitor.Dll;
using Monitor.UI.Board;
using Monitor.Utils;

namespace Monitor.State
{
    /// <summary>
    /// Wrapper for the corresponding piece of functionality on C++ side
    /// </summary>
    internal class StateEditor : IDisposable, IStateSeed
    {
        private IntPtr _editorPtr;

        /// <summary>
        /// Pointer to the underlying unmanaged agent
        /// </summary>
        public IntPtr Ptr => _editorPtr;

        /// <summary>
        /// Releases unmanaged resources
        /// </summary>
        public void Dispose()
        {
            if (_editorPtr == IntPtr.Zero) return;
            
            DllWrapper.FreeStateEditor(_editorPtr);
            _editorPtr = IntPtr.Zero;
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Finalizer.
        /// </summary>
        ~StateEditor()
        {
            Dispose();
        }

        /// <summary>
        /// Constructor.
        /// </summary>
        public StateEditor(DllWrapper.StateTypeId type)
        {
            _editorPtr = DllWrapper.ConstructStateEditor(type);
        }

        /// <summary>
        /// Type id of the edited state.
        /// </summary>
        public DllWrapper.StateTypeId Type => DllWrapper.StateEditorGetTypeId(_editorPtr);

        /// <summary>
        /// Returns array representation of the edited state.
        /// </summary>
        /// <returns></returns>
        private IList<int> GetState()
        {
            var result = new int[Checkerboard.Fields];

            DllWrapper.StateEditorGetState(_editorPtr, (size, arr) =>
            {
                if (size != result.Length)
                    throw new Exception("Inconsistent state");
                
                Array.Copy(arr, result, size);
            });

            return result;
        }

        /// <summary>
        /// Returns array of edit options for a field of checkerboard at the given position.
        /// </summary>
        private IList<int> GetOptions(PiecePosition pos)
        {
            int[] result = null;

            DllWrapper.StateEditorGetOptions(_editorPtr, pos, (size, arr) =>
            {
                if (size < 0)
                    throw new Exception("Inconsistent size");

                result = new int[size];
                
                if (size > 0)
                    Array.Copy(arr, result, size);
            });

            return result;
        }

        /// <summary>
        /// Applies edit option with the given ID to the field at the given position.
        /// </summary>
        private void ApplyOption(PiecePosition pos, int optionId)
        {
            DllWrapper.StateEditorApplyOption(_editorPtr, pos, optionId);
        }

        /// <summary>
        /// Resets edited state to the "initial" configuration.
        /// </summary>
        public void Reset()
        {
            DllWrapper.StateEditorReset(_editorPtr);
            UpdateState();
        }

        /// <summary>
        /// Clears edited state.
        /// </summary>
        public void Clear()
        {
            DllWrapper.StateEditorClear(_editorPtr);
            UpdateState();
        }

        private ICheckerBoard _board;
        
        /// <summary>
        /// Connects to the given board.
        /// </summary>
        public void ConnectToBoard(ICheckerBoard board)
        {
            DisconnectFromBoard();
            _board = board;

            if (_board == null)
                return;
            
            _board.FieldHovered += BoardOnFieldHovered;
            _board.FieldSelected += BoardOnFieldSelected;
            _board.MouseWheelChanged += BoardOnMouseWheelChanged;
            
            UpdateState();
            if (_board.FieldUnderMousePointer.IsValid)
                BoardOnFieldHovered(_board.FieldUnderMousePointer);
        }

        /// <summary>
        /// Disconnects from the board and sets board pointer to null.
        /// </summary>
        private void DisconnectFromBoard()
        {
            if (_board == null)
                return;
            
            _board.FieldHovered -= BoardOnFieldHovered;
            _board.FieldSelected -= BoardOnFieldSelected;
            _board.MouseWheelChanged -= BoardOnMouseWheelChanged;
            _board = null;
        }

        private OptionSelector _optionSelector = null;

        /// <summary>
        /// Updates markers of the board.
        /// </summary>
        private void UpdateMarkers()
        {
            if (_board == null)
                return;

            if (_optionSelector == null || _optionSelector.SelectedOptionId < 0)
            {
                _board.UpdateMarkers(null);
                return;
            }
            
            _board.UpdateMarkers(new[]
            {
                new CheckerBoard.Marker(_board.FieldUnderMousePointer,
                    _optionSelector.SelectedOption == 0 ? Brushes.Red: Brushes.GreenYellow)
            });
        }

        /// <summary>
        /// Updates preview
        /// </summary>
        private void UpdatePreview()
        {
            if (_board == null)
                return;

            if (_optionSelector == null || _optionSelector.SelectedOptionId < 0)
                _board.UpdatePreviewPiece(0);
            else
                _board.UpdatePreviewPiece(_optionSelector.SelectedOption);
        }

        /// <summary>
        /// Updates state.
        /// </summary>
        private void UpdateState()
        {
            var state = GetState();
            _board.UpdateState(new State(state, Type));
        }
        
        /// <summary>
        /// Event handler.
        /// </summary>
        private void BoardOnMouseWheelChanged(int delta, PiecePosition pos)
        {
            if (_optionSelector == null || delta == 0)
                return;

            if (delta > 0)
                _optionSelector.SelectNextMove();
            else
                _optionSelector.SelectPreviousMove();

            UpdatePreview();
        }

        /// <summary>
        /// Event handler
        /// </summary>
        private void BoardOnFieldSelected(PiecePosition pos)
        {
            if (_optionSelector == null || _optionSelector.SelectedOptionId < 0)
                return;
            
            ApplyOption(pos, _optionSelector.SelectedOptionId);
            UpdateState();
            BoardOnFieldHovered(pos);
        }

        /// <summary>
        /// Event handler.
        /// </summary>
        private void BoardOnFieldHovered(PiecePosition pos)
        {
            _optionSelector = null;

            if (_board == null)
                return;

            _optionSelector = new OptionSelector(GetOptions(pos));
            UpdateMarkers();
            UpdatePreview();
        }
    }
}
