﻿//Copyright (c) 2023 Denys Dragunov, dragunovdenis@gmail.com
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

using System.Windows;
using Monitor.Agents;

namespace Monitor.UI
{
    /// <summary>
    /// Interaction logic for TdlAgentDialog.xaml
    /// </summary>
    public partial class TdlAgentDialog : Window
    {
        /// <summary>
        /// Constructor
        /// </summary>
        internal TdlAgentDialog(TdLambdaAgent agent = null)
        {
            InitializeComponent();
            Owner = Application.Current.MainWindow;

            if (agent != null)
            {
                Agent = agent;
                AgentControl.SetEdit(agent);
            }
        }

        /// <summary>
        /// Read access to the agent (constructed or edited)
        /// </summary>
        internal TdLambdaAgent Agent { get; private set; }

        /// <summary>
        /// "OK" button click handler
        /// </summary>
        private void OkButton_OnClick(object sender, RoutedEventArgs e)
        {
            if (Agent == null)
                Agent = AgentControl.CreateAgent();
                
            DialogResult = true;
        }

        /// <summary>
        /// "Cancel" button click handler
        /// </summary>
        private void CancelButton_OnClick(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
        }
    }
}
