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
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Input;
using Monitor.Agents;
using Monitor.UI.Board;
using Monitor.UI.Converters;
using Brushes = System.Windows.Media.Brushes;

namespace Monitor.UI
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window, INotifyPropertyChanged
    {
        private ITwoPlayerGameUi _checkersUi;

        /// <summary>
        /// Read-only access to the checkers UI
        /// </summary>
        public ITwoPlayerGameUi CheckersUi
        {
            get => _checkersUi;
            set => SetField(ref _checkersUi, value);
        }

        /// <summary>
        /// Constructor
        /// </summary>
        public MainWindow()
        {
            InitializeComponent();
            CheckersUi = new BoardUi(MainCanvas, this.Dispatcher);
            CheckersUi.InfoEvent += UpdateInfoTextBox;
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

        private int _trainTabCountTotal = 0;

        /// <summary>
        /// Flag defining if it is possible to add yet another training session tab
        /// </summary>
        public bool CanAddNewTraining
        {
            get
            {
                return MainTabControl.Items.Cast<TabItem>().Count(x => x.Content is TrainControl) <
                       Environment.ProcessorCount;
            }
        }

        /// <summary>
        /// Handling click on the "add training tab" button
        /// </summary>
        private void AddTab_OnPreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            if (!CanAddNewTraining)
                throw new Exception("The corresponding button must be disabled");

            var tabItem = new TabItem()
            {
                Header = "Train " + (_trainTabCountTotal == 0 ? "" : _trainTabCountTotal.ToString()),
            };
            _trainTabCountTotal++;

            var newItemId = MainTabControl.Items.Count - 1;
            var trainControl = new TrainControl(newItemId.ToString())
            {
                Tag = tabItem,
            };

            tabItem.SetBinding(ForegroundProperty, new Binding(nameof(trainControl.IsPlaying))
            {
                Source = trainControl,
                Converter = new BoolToColorConverter(Brushes.Red, Brushes.Black),
            });

            trainControl.OnFinishSession += TrainControlOnOnFinishSession;
            trainControl.OnEnquireExtraAgents += TrainControlOnOnEnquireExtraAgents;
            tabItem.Content = trainControl;

            //Put the new item in a pre-last position (since the last position is occupied by "add button")
            MainTabControl.Items.Insert(newItemId, tabItem);
            MainTabControl.SelectedIndex = newItemId;
            OnPropertyChanged(nameof(CanAddNewTraining));

            e.Handled = true;
        }

        /// <summary>
        /// Handler for the "enquire extra agent" event
        /// </summary>
        private void TrainControlOnOnEnquireExtraAgents(TrainControl sender,
            ConcurrentBag<ITdLambdaAgentReadOnly> collectionToAddTo)
        {
            if (collectionToAddTo == null || sender == null)
                throw new Exception("Invalid input");

            var trainControls = MainTabControl.Items.Cast<TabItem>().Select(x => x.Content).OfType<TrainControl>()
                .ToArray();

            foreach (var trainControl in trainControls)
            {
                if (trainControl == sender)
                    continue;

                foreach (var agent in trainControl.Agents.OfType<ITdLambdaAgentReadOnly>())
                    collectionToAddTo.Add(agent);
            }
        }

        /// <summary>
        /// Finish of a training session handler
        /// </summary>
        private void TrainControlOnOnFinishSession(TrainControl obj)
        {
            var tabId = MainTabControl.Items.IndexOf(obj.Tag);

            if (tabId < 0)
                throw new Exception("Invalid tab control to remove");

            MainTabControl.SelectedIndex = tabId == MainTabControl.Items.Count - 2 ? tabId - 1 : tabId + 1;

            MainTabControl.Items.Remove(obj.Tag);
            OnPropertyChanged(nameof(CanAddNewTraining));
        }

        public event PropertyChangedEventHandler PropertyChanged;

        /// <summary>
        /// Property changed invocation
        /// </summary>
        protected virtual void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        /// <summary>
        /// Property setter with notification
        /// </summary>
        protected bool SetField<T>(ref T field, T value, [CallerMemberName] string propertyName = null)
        {
            if (EqualityComparer<T>.Default.Equals(field, value)) return false;
            field = value;
            OnPropertyChanged(propertyName);
            return true;
        }

        /// <summary>
        /// Handles "Edit" button click
        /// </summary>
        private void EditAgentButton_OnClick(object sender, RoutedEventArgs e)
        {
            _checkersUi.EditInactiveAgent();
        }

        private void MenuAbout_OnClick(object sender, RoutedEventArgs e)
        {
            var aboutWindow = new AboutWindow();
            aboutWindow.Owner = this; // this refers to the main window
            aboutWindow.WindowStartupLocation = WindowStartupLocation.CenterOwner;
            aboutWindow.ShowDialog();       
        }
    }
}
