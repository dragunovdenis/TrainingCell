using System.Windows;
using System.Windows.Controls;
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

            _checkersUi.InfoEvent += (info) =>
            {
                ScoreCombo.Items.Clear();
                foreach (var infoString in info)
                {
                    ScoreCombo.Items.Add(new ComboBoxItem()
                        { Content = infoString, });
                }
            };
        }

        /// <summary>
        /// Event handler
        /// </summary>
        private void Button_Click(object sender, RoutedEventArgs e)
        {
            _checkersUi.Play( Ui.AgentType.TdLambda, Ui.AgentType.Interactive, 1000);
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
    }
}
