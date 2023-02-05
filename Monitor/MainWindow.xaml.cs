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

using System.Collections.Generic;
using System.Windows;
using Monitor.Checkers;
using Monitor.Checkers.UI;

namespace Monitor
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private readonly ITwoPlayerGameUi _checkersUi;

        /// <summary>
        /// Constructor
        /// </summary>
        public MainWindow()
        {
            InitializeComponent();
            _checkersUi = new BoardUi(MainCanvas, this.Dispatcher);

            DataContext = _checkersUi;
            _checkersUi.InfoEvent += UpdateInfoTextBox;
        }

        /// <summary>
        /// Updates info text box with the given info strings
        /// </summary>
        private void UpdateInfoTextBox(IList<string> info)
        {
            InfoTextBox.Text = "";

            if (info == null)
                return;

            foreach (var infoString in info)
                InfoTextBox.Text += infoString + "\n";
        }

        /// <summary>
        /// Event handler
        /// </summary>
        private void MainPanel_OnSizeChanged(object sender, SizeChangedEventArgs e)
        {
            _checkersUi.Draw();
        }

        /// <summary>
        /// Event handler
        /// </summary>
        private void CancelButton_OnClick(object sender, RoutedEventArgs e)
        {
            _checkersUi.TerminateGame();
        }

        /// <summary>
        /// Event handler
        /// </summary>
        private void LoadWhiteAgentButton_OnClick(object sender, RoutedEventArgs e)
        {
            _checkersUi.LoadWhiteAgent();
        }

        /// <summary>
        /// Event handler
        /// </summary>
        private void LoadBlackAgentButton_OnClick(object sender, RoutedEventArgs e)
        {
            _checkersUi.LoadBlackAgent();
        }
    }
}
