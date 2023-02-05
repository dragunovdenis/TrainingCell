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
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;

namespace Monitor.Checkers.UI
{
    /// <summary>
    /// Interaction logic for TrainControl.xaml
    /// </summary>
    public partial class TrainControl : UserControl, INotifyPropertyChanged
    {
        /// <summary>
        /// Adds given agent to the white and black agents pool somultaneously
        /// </summary>
        private void AddAgent(IAgent agent)
        {
            var item = new ListViewItem()
            {
                Content = agent.Id,
                Tag = agent
            };

            AgentPoolList.Items.Add(item);

            //In case of TD(l) agent we bind its Id
            //to the content of the list-view item,
            //so that the latter gets automatically updated on edit
            if (agent is TdLambdaAgent tdlAgent)
            {
                var idBinding = new Binding("Id")
                {
                    Source = tdlAgent,
                    Mode = BindingMode.OneWay,
                };

                item.SetBinding(ContentProperty, idBinding);
            }
        }

        /// <summary>
        /// Constructor
        /// </summary>
        public TrainControl()
        {
            InitializeComponent();

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
        /// Play button click handler
        /// </summary>
        private void PlayButton_Click(object sender, System.Windows.RoutedEventArgs e)
        {
            if (!CanPlay)
                throw new Exception("Can't play: the button must be disabled!");

            IsPlaying = true;

            _playTaskCancellation = new CancellationTokenSource();
            var playTask = new Task(() =>
            {
                var episodeCounter = 0;
                DllWrapper.RunCheckersTraining(
                    WhiteAgent.Ptr, BlackAgent.Ptr, EpisodesToPlay,
                    null,
                    (whiteWins, blackWins, totalGamers) =>
                    {
                        if (episodeCounter % 100 == 0 || episodeCounter == (EpisodesToPlay - 1))
                            Dispatcher.BeginInvoke(new Action(() =>
                            {
                                InfoTExtBlock.Text +=
                                    $"White Wins:{whiteWins}; Black Wins:{blackWins}; Total Games {totalGamers}" + "\n";
                            }));
                        episodeCounter++;
                    },
                    () => _playTaskCancellation.IsCancellationRequested);
            });
            playTask.ContinueWith((task) =>
            {
                _playTaskCancellation = null;
                IsPlaying = false;
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

            return (AgentPoolList.SelectedItem as ListViewItem)?.Tag as IAgent;
        }

        /// <summary>
        /// Returns selected TD(l) agent or nul if there is no such agent selected
        /// </summary>
        /// <returns></returns>
        private TdLambdaAgent GetSelectedTdlAgent()
        {
            return GetSelectedAgent() as TdLambdaAgent;
        }

        void AssignAgent(bool white)
        {
            if (AgentPoolList.SelectedIndex < 0)
                throw new Exception("No selected item found");

            if (white)
                WhiteAgent = GetSelectedAgent();
            else
                BlackAgent = GetSelectedAgent();
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
                    OnPropertyChanged(nameof(CanPlay));
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

        private bool _canEdit;

        /// <summary>
        /// Indicates whether there is an item to edit
        /// </summary>
        public bool CanEdit
        {
            get => _canEdit;
            set => SetField(ref _canEdit, value);

        }

        /// <summary>
        /// Inspect or edit selected agent
        /// </summary>

        private void EditButton_OnClick(object sender, RoutedEventArgs e)
        {
            var dialog = new TdlAgentDialog(GetSelectedTdlAgent());
            dialog.ShowDialog();
        }

        /// <summary>
        /// ListView item selection event handler
        /// </summary>
        private void AgentPoolList_OnSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            CanEdit = GetSelectedTdlAgent() != null;
            OnPropertyChanged(nameof(CanSave));
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
    }
}
