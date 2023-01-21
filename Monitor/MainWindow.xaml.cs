using System;
using System.Threading;
using System.Threading.Tasks;
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
        public MainWindow()
        {
            InitializeComponent();
            _checkersUi = new Ui(MainCanvas);
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            new Task(() =>
            {
                using (var randomAgent = new RandomAgent())
                {
                    using (var tdlAgent = new TdLambdaAgent(
                               new uint[] { 32, 64, 32, 16, 8, 1 }, 0.05, 0.1, 0.9, 0.1))
                    {
                        DllWrapper.RunCheckersTraining(
                            randomAgent.Ptr, tdlAgent.Ptr, 10000,
                            (state, size, subMoves, subMovesCount) =>
                            {
                                Thread.Sleep(10);
                                this.Dispatcher.BeginInvoke(new Action(() =>
                                {
                                    _checkersUi.DrawBoard();
                                    _checkersUi.DrawState(state);
                                }));
                            },
                            (whiteWins, blackWins, totalGamers) =>
                            {
                                Thread.Sleep(10);
                                this.Dispatcher.BeginInvoke(new Action(() =>
                                {
                                    ScoreCombo.Items.Clear();
                                    ScoreCombo.Items.Add(new ComboBoxItem()
                                        { Content = "Whites Won: " + whiteWins, });
                                    ScoreCombo.Items.Add(new ComboBoxItem()
                                        { Content = "Blacks Won: " + blackWins, });
                                    ScoreCombo.Items.Add(
                                        new ComboBoxItem() { Content = "Total Games: " + totalGamers, });
                                }));
                            }, () => false);
                    }
                }
            }).Start();
        }

        private void MainPanel_OnSizeChanged(object sender, SizeChangedEventArgs e)
        {
            _checkersUi.DrawBoard();
        }
    }
}
