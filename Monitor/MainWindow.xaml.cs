using System.Collections.Generic;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using Monitor.Checkers;

namespace Monitor
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private Ui _checkersUi;

        /// <summary>
        /// Constructor
        /// </summary>
        public MainWindow()
        {
            InitializeComponent();
            _checkersUi = new Ui(MainCanvas, this.Dispatcher);

            _checkersUi.InfoEvent += UpdateInfoTextBox;

            BindIsPlayingToIsEnabled(CancelButton, false);
            BindIsPlayingToIsEnabled(LoadWhiteAgentButton, true);
            BindIsPlayingToIsEnabled(LoadBlackAgentButton, true);
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
        /// Method to bind IsEnabledProperty of a button to IsPlaying property of the Checkers UI
        /// </summary>
        /// <param name="button">The button to work with</param>
        /// <param name="invert">If inversion of the boolean property should be used</param>
        private void BindIsPlayingToIsEnabled(Button button, bool invert)
        {
            var isEnabledBinding = new Binding("IsPlaying")
            {
                Source = _checkersUi,
                Mode = BindingMode.OneWay,
                Converter = invert ? new InvertBoolConverter() : null,
            };

            button.SetBinding(Button.IsEnabledProperty, isEnabledBinding);
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
            _checkersUi.CancelPlaying();
        }

        /// <summary>
        /// Event handler
        /// </summary>
        private void LoadWhiteAgentButton_OnClick(object sender, RoutedEventArgs e)
        {
            _checkersUi.Play(Ui.AgentType.AgentPack, Ui.AgentType.Interactive, 100);
        }

        /// <summary>
        /// Event handler
        /// </summary>
        private void LoadBlackAgentButton_OnClick(object sender, RoutedEventArgs e)
        {
            _checkersUi.Play(Ui.AgentType.Interactive, Ui.AgentType.AgentPack, 100);
        }
    }
}
