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
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Threading;
using Monitor.Agents;
using Monitor.Dll;
using static Monitor.Dll.DllWrapper;

namespace Monitor.UI
{
    /// <summary>
    /// Interaction logic for TrainControl.xaml
    /// </summary>
    public partial class TrainControl : UserControl, INotifyPropertyChanged
    {
        /// <summary>
        /// Delegate used to gather extra agents
        /// </summary>
        /// <returns></returns>
        public delegate void EnquireExtraAgentsDelegate(TrainControl sender, ConcurrentBag<ITdLambdaAgentReadOnly> collectionToAddTo);

        /// <summary>
        /// An event that is used to gather extra agents (probably from other training sessions)
        /// </summary>
        public event EnquireExtraAgentsDelegate OnEnquireExtraAgents;

        /// <summary>
        /// Collection of agents
        /// </summary>
        public  ObservableCollection<IAgent> Agents { get; } = new ObservableCollection<IAgent>();

        /// <summary>
        /// Adds given agent to the pull of agents
        /// </summary>
        private void AddAgent(IAgent agent)
        {
            Agents.Add(agent);
        }

        private readonly string _id;

        /// <summary>
        /// Constructor
        /// </summary>
        public TrainControl(string id)
        {
            InitializeComponent();

            _id = id;
            //Put the "random" agent to the list by default
            AddAgent(new RandomAgent());

            AppDomain.CurrentDomain.ProcessExit += (o, e) =>
            {
                //Handle the case when the play is ongoing
                //but the user decided to close application anyway
                if (IsPlaying && _playTaskCancellation != null)
                {
                    _playTaskCancellation.Cancel();

                    while (IsPlaying)
                        Thread.Sleep(100);
                }
            };
        }

        private CancellationTokenSource _playTaskCancellation;

        /// <summary>
        /// Read-only interface of the result of training
        /// </summary>
        private interface ITrainingResult
        {
            /// <summary>
            /// Total number of training episodes (games)
            /// </summary>
            int TotalEpisodes { get; }

            /// <summary>
            /// Number of times "Black" player won
            /// </summary>
            int BlackWins { get; }

            /// <summary>
            /// Number of times "White" player won
            /// </summary>
            int WhiteWins { get; }

            /// <summary>
            /// Used to synchronize interactions with UI
            /// </summary>
            DispatcherOperation DispatcherOp { get; }
        }

        /// <summary>
        /// Data structure to contain result of a series of training episodes 
        /// </summary>
        private class TrainingResult : ITrainingResult
        {
            /// <summary>
            /// Total number of training episodes (games)
            /// </summary>
            public int TotalEpisodes { get; set; }

            /// <summary>
            /// Number of times "Black" player won
            /// </summary>
            public int BlackWins { get; set; }

            /// <summary>
            /// Number of times "White" player won
            /// </summary>
            public int WhiteWins { get; set; }

            /// <summary>
            /// Used to synchronize interactions with UI
            /// </summary>
            public DispatcherOperation DispatcherOp { get; set; }
        }

        public ObservableCollection<string> InfoCollection { get; } = new ObservableCollection<string>();

        /// <summary>
        /// Play button click handler
        /// </summary>
        private void PlayButton_Click(object sender, System.Windows.RoutedEventArgs e)
        {
            if (!CanPlay)
                throw new Exception("Can't play: the button must be disabled!");

            IsPlaying = true;

            var timePrev = DateTime.Now;
            InfoCollection.Clear();
            _playTaskCancellation = new CancellationTokenSource();
            var trainingStart = DateTime.Now;
            var playTask = new Task<ITrainingResult>(() =>
            {
                var result = new TrainingResult();
                int whiteWinsPrev = 0;
                int blackWinsPrev = 0;
                int totalGamersPrev = 0;

                DllWrapper.RunTraining(
                    WhiteAgent.Ptr, BlackAgent.Ptr, EpisodesToPlay, GameKind.Checkers,
                    null,
                    (whiteWinsLocal, blackWinsLocal, totalGamesLocal) =>
                    {
                        result.TotalEpisodes = totalGamesLocal;
                        result.BlackWins += blackWinsLocal ? 1 : 0;
                        result.WhiteWins += whiteWinsLocal ? 1 : 0;

                        if (totalGamesLocal % 1000 == 0 || totalGamesLocal == EpisodesToPlay)
                        {
                            var gamesPlayedSinceLastReport = totalGamesLocal - totalGamersPrev;
                            var whiteWinsPercentsSinceLastReport = (result.WhiteWins - whiteWinsPrev) * 100.0 /
                                                                   gamesPlayedSinceLastReport;
                            var blackWinsPercentsSinceLastReport = (result.BlackWins - blackWinsPrev) * 100.0 /
                                                                   gamesPlayedSinceLastReport;
                            var drawsPercentsFromLastReport = 100.0 - (whiteWinsPercentsSinceLastReport +
                                                                       blackWinsPercentsSinceLastReport);

                            totalGamersPrev = totalGamesLocal;
                            whiteWinsPrev = result.WhiteWins;
                            blackWinsPrev = result.BlackWins;

                            var elapsedTimeSec = (DateTime.Now - timePrev).TotalMilliseconds * 1e-3;
                            timePrev = DateTime.Now;

                            var infoLine =
                                $"White Wins total/inst. %: {result.WhiteWins}/{whiteWinsPercentsSinceLastReport:F1}; " +
                                $"Black Wins total/inst. %: {result.BlackWins}/{blackWinsPercentsSinceLastReport:F1}; " +
                                $"Draws total/inst. %: {totalGamesLocal - result.WhiteWins - result.BlackWins}/{drawsPercentsFromLastReport:F1}; " +
                                $"Total Games: {totalGamesLocal}; Elapsed time: {elapsedTimeSec:F1} sec.";

                            result.DispatcherOp?.Wait();
                            result.DispatcherOp = Dispatcher.BeginInvoke(new Action(() =>
                            {
                                InfoCollection.Add(infoLine);

                                if (InfoCollection.Count > 100)
                                    InfoCollection.RemoveAt(0);

                                InfoScroll.ScrollToBottom();
                            }));
                        }
                    },
                    () => _playTaskCancellation.IsCancellationRequested,
                    (errorMessage) =>
                    {
                        result.DispatcherOp?.Wait();
                        result.DispatcherOp = Dispatcher.BeginInvoke(new Action(() =>
                        {
                            InfoCollection.Add("Error :" + errorMessage);
                        }));
                        WhiteAgent.AddTrainingFailRecord(BlackAgent, errorMessage);
                        BlackAgent.AddTrainingFailRecord(WhiteAgent, errorMessage);
                    });

                return result;
            });
            playTask.ContinueWith((task) =>
            {
                var res = task.Result;
                res.DispatcherOp?.Wait();

                _playTaskCancellation = null;
                IsPlaying = false;
                WhiteAgent.AddTrainingCompletionRecord(BlackAgent, res.TotalEpisodes, res.BlackWins);
                BlackAgent.AddTrainingCompletionRecord(WhiteAgent, res.TotalEpisodes, res.WhiteWins);
                InfoCollection.Add($"Total training time : {DateTime.Now - trainingStart}");
            }, TaskScheduler.FromCurrentSynchronizationContext());
            playTask.Start();
        }

        /// <summary>
        /// Termination handling
        /// </summary>
        private void TerminateButton_OnClick(object sender, RoutedEventArgs e)
        {
            if (_playTaskCancellation != null)
                _playTaskCancellation.Cancel();
        }

        /// <summary>
        /// Creation of an agent
        /// </summary>
        private void CreateButton_Click(object sender, RoutedEventArgs e)
        {
            var dialog = new TdlAgentDialog();
            if (dialog.ShowDialog() == true)
            {
                var agent = dialog.Agent;
                AddAgent(agent);
            }
        }

        private IAgent _whiteAgent;

        /// <summary>
        /// White agent
        /// </summary>
        public IAgent WhiteAgent
        {
            get => _whiteAgent;
            set
            {
                if (SetField(ref _whiteAgent, value))
                    OnPropertyChanged(nameof(CanPlay));
            }
        }

        private IAgent _blackAgent;

        /// <summary>
        /// White agent
        /// </summary>
        public IAgent BlackAgent
        {
            get => _blackAgent;
            set
            {
                if (SetField(ref _blackAgent, value))
                    OnPropertyChanged(nameof(CanPlay));
            }
        }

        /// <summary>
        /// Returns pointer to the agent that corresponds to the selected item in the list
        /// </summary>
        private IAgent GetSelectedAgent()
        {
            if (AgentPoolList.SelectedIndex < 0)
                return null;

            return Agents[AgentPoolList.SelectedIndex];
        }

        /// <summary>
        /// Returns all the available TdLambda-agents
        /// </summary>
        private IList<ITdLambdaAgentReadOnly> GetTdlAgents()
        {
            return Agents.OfType<ITdLambdaAgentReadOnly>().ToArray();
        }

        /// <summary>
        /// Collects all the TD(lambda) agents belonging to the current training control and adds
        /// all the extra agents collected through the corresponding "enquire" event
        /// </summary>
        private IList<ITdLambdaAgentReadOnly> GetTdlAgentsExtra()
        {
            var result = new List<ITdLambdaAgentReadOnly>();
            result.AddRange(GetTdlAgents());

            var extraAgentsBag = new ConcurrentBag<ITdLambdaAgentReadOnly>();
            OnEnquireExtraAgents?.Invoke(this, extraAgentsBag);
            result.AddRange(extraAgentsBag);

            return result;
        }

        /// <summary>
        /// Returns selected TD(l) agent or nul if there is no such agent selected
        /// </summary>
        /// <returns></returns>
        private TdLambdaAgent GetSelectedTdlAgent()
        {
            return GetSelectedAgent() as TdLambdaAgent;
        }

        /// <summary>
        /// Returns selected ensemble agent (or null if there is no such agent)
        /// </summary>
        private EnsembleAgent GetSelectedEnsembleAgent()
        {
            return GetSelectedAgent() as EnsembleAgent;
        }

        /// <summary>
        /// Assigns white or black players
        /// </summary>
        void AssignAgent(bool white)
        {
            if (AgentPoolList.SelectedIndex < 0)
                throw new Exception("No selected item found");

            if (white)
                WhiteAgent = GetSelectedAgent();
            else
                BlackAgent = GetSelectedAgent();

            OnPropertyChanged(nameof(CanRemove));
        }

        /// <summary>
        /// Flag determining if both agents are selected and we can start playing
        /// </summary>
        public bool CanPlay => BlackAgent != null && WhiteAgent != null && !IsPlaying;

        private bool _isPlaying;

        /// <summary>
        /// Indicates whether the playing ongoing
        /// </summary>
        public bool IsPlaying
        {
            get => _isPlaying;
            set
            {
                if (SetField(ref _isPlaying, value))
                {
                    OnPropertyChanged(nameof(CanPlay));
                    OnPropertyChanged(nameof(CanEdit));
                }
            }
        }

        private int _episodesToPlay = 1000;

        /// <summary>
        /// Number of episodes (games) to play
        /// </summary>
        public int EpisodesToPlay
        {
            get => _episodesToPlay;
            set => SetField(ref _episodesToPlay, value);
        }

        /// <summary>
        /// Assignment of "white" agent
        /// </summary>
        private void AssignWhiteAgentButton_OnClick(object sender, RoutedEventArgs e)
        {
            AssignAgent(white: true);
        }

        /// <summary>
        /// Assignment of "black" agent
        /// </summary>
        private void AssignBlackAgentButton_OnClick(object sender, RoutedEventArgs e)
        {
            AssignAgent(white: false);
        }

        public event PropertyChangedEventHandler PropertyChanged;

        /// <summary>
        /// Triggers property changed notification
        /// </summary>
        /// <param name="propertyName"></param>
        protected virtual void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        /// <summary>
        /// Setter with notification
        /// </summary>
        protected bool SetField<T>(ref T field, T value, [CallerMemberName] string propertyName = null)
        {
            if (EqualityComparer<T>.Default.Equals(field, value)) return false;
            field = value;
            OnPropertyChanged(propertyName);
            return true;
        }

        /// <summary>
        /// Indicates whether the selected agent is an "ensemble"
        /// </summary>
        public bool IsEnsembleSelected => GetSelectedEnsembleAgent() != null;

        /// <summary>
        /// Indicates whether there is an item to edit
        /// </summary>
        public bool CanEdit =>
            (GetSelectedTdlAgent() != null || GetSelectedEnsembleAgent() != null) &&
            (!IsPlaying || GetSelectedAgent() != BlackAgent && GetSelectedAgent() != WhiteAgent);

        /// <summary>
        /// Indicates whether the selected agent can be removed
        /// </summary>
        public bool CanRemove => (GetSelectedTdlAgent() != null || GetSelectedEnsembleAgent() != null) &&
                                 GetSelectedAgent() != BlackAgent && GetSelectedAgent() != WhiteAgent;

        /// <summary>
        /// Inspect or edit selected agent
        /// </summary>
        private void EditButton_OnClick(object sender, RoutedEventArgs e)
        {
            if (GetSelectedTdlAgent() != null)
            {
                var dialog = new TdlAgentDialog(GetSelectedTdlAgent());
                dialog.ShowDialog();
            } else if (GetSelectedEnsembleAgent() != null)
            {
                var dialog = new EnsembleAgentDialog(GetTdlAgentsExtra(), GetSelectedEnsembleAgent());
                dialog.ShowDialog();
            }
            else throw new Exception("Can't edit");
        }

        /// <summary>
        /// ListView item selection event handler
        /// </summary>
        private void AgentPoolList_OnSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            OnPropertyChanged(nameof(CanSave));
            OnPropertyChanged(nameof(CanEdit));
            OnPropertyChanged(nameof(CanRemove));
            OnPropertyChanged(nameof(IsEnsembleSelected));
        }

        /// <summary>
        /// Property indicating whether the "selected" agent can be savesd
        /// </summary>
        public bool CanSave => AgentFileSystemManager.CanSave(GetSelectedAgent());

        /// <summary>
        /// Loading of an agent
        /// </summary>
        private void LoadButton_Click(object sender, RoutedEventArgs e)
        {
            var agent = AgentFileSystemManager.LoadAgent();

            if (agent != null)
                AddAgent(agent);
        }

        /// <summary>
        /// Save button handler
        /// </summary>
        private void SaveButton_Click(object sender, RoutedEventArgs e)
        {
            if (!AgentFileSystemManager.CanSave(GetSelectedAgent()))
                throw new Exception("Can't save the given agent");

            AgentFileSystemManager.SaveAgent(GetSelectedAgent());
        }

        /// <summary>
        /// Finish session event
        /// </summary>
        public event Action<TrainControl> OnFinishSession;

        /// <summary>
        /// Finish session button click handler
        /// </summary>
        private void FinishSession_OnClick(object sender, RoutedEventArgs e)
        {
            OnFinishSession?.Invoke(this);
        }

        /// <summary>
        /// Create ensemble button click handler
        /// </summary>
        private void CreateEnsemble_OnClick(object sender, RoutedEventArgs e)
        {
            var dialog = new EnsembleAgentDialog(GetTdlAgentsExtra(), null);
            if (dialog.ShowDialog() == true && dialog.Ensemble != null)
                AddAgent(dialog.Ensemble);
        }

        /// <summary>
        /// Remove button click handler
        /// </summary>
        private void RemoveButton_OnClick(object sender, RoutedEventArgs e)
        {
            if (!CanEdit || GetSelectedAgent() == null)
                throw new Exception("The button must be disabled");

            var agentToRemove = GetSelectedAgent();
            Agents.Remove(agentToRemove);
            agentToRemove.Dispose();
        }

        /// <summary>
        /// Extract agents from selected ensemble
        /// </summary>
        private void ExtractButton_OnClick(object sender, RoutedEventArgs e)
        {
            var selectedEnsemble = GetSelectedEnsembleAgent();

            if (selectedEnsemble == null)
                throw new Exception("The button must be disabled");

            for (var subAgentId = 0; subAgentId < selectedEnsemble.Size; ++subAgentId)
            {
                var subAgentToExtractReadOnly = selectedEnsemble.GetSubAgent(subAgentId);

                if (Agents.Any(x => subAgentToExtractReadOnly.IsEqualTo(x)))
                    continue; // such an agent is already present in the collection of agents so we continue

                Agents.Add(selectedEnsemble.CopySubAgent(subAgentId));
            }
        }
    }
}
