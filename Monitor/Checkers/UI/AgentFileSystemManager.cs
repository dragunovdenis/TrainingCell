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
using System.IO;
using System.Windows;
using Microsoft.Win32;

namespace Monitor.Checkers.UI
{
    /// <summary>
    /// Provides saving/loading functionality for checkers agents
    /// </summary>
    internal class AgentFileSystemManager
    {
        private const string TdlAgentExtension = ".tda";
        private const string AgentPackExtension = ".apack";

        private const string TdlAgentFilter = "TD-lambda Agent (*" + TdlAgentExtension + ")|*" + TdlAgentExtension;
        private const string AgentPackFilter = "Agent Pack (*" + AgentPackExtension + ")|*" + AgentPackExtension;

        /// <summary>
        /// Returns filter string for open dialogs
        /// </summary>
        private static string GetLoadFilter()
        {
            return $"{TdlAgentFilter}|{AgentPackFilter}";
        }

        /// <summary>
        /// Returns "true" if the given agent can be saved by the manager
        /// </summary>
        public static bool CanSave(IAgent agent)
        {
            return GetSaveFilter(agent) != null;
        }

        /// <summary>
        /// Returns filter string for save dialog suitable for the given agent
        /// </summary>
        private static string GetSaveFilter(IAgent agent)
        {
            if (agent is TdLambdaAgent)
                return $"{TdlAgentFilter}|{AgentPackFilter}";

            if (agent is AgentPack)
                return AgentPackFilter;

            return null;
        }

        /// <summary>
        /// Sets up open dialog and handles loading of checkers agents in different formats
        /// </summary>
        public static IAgent LoadAgent()
        {
            var openFileDialog = new OpenFileDialog
            {
                Filter = GetLoadFilter(),
                FilterIndex = 0
            };

            if (openFileDialog.ShowDialog() == true)
            {
                try
                {
                    var ext = Path.GetExtension(openFileDialog.FileName)?.ToLower();
                    if (ext == TdlAgentExtension)
                        return TdLambdaAgent.LoadFromFile(openFileDialog.FileName);

                    if (ext == AgentPackExtension)
                        return new AgentPack(openFileDialog.FileName);

                    throw new Exception($"Unsupported format: {ext}");
                }
                catch (Exception e)
                {
                    MessageBox.Show($"Failed to load agent from {openFileDialog.FileName} with message {e.Message}", "Error",
                        MessageBoxButton.OK, MessageBoxImage.Information);
                }
            }

            return null;
        }

        /// <summary>
        /// Sets up open dialog and handles saving of the given checkers agents in all appropriate formats
        /// </summary>
        public static void SaveAgent(IAgent agent)
        {
            if (agent == null)
                throw new Exception("Invalid agent");

            var saveFileDialog = new SaveFileDialog
            {
                Filter = GetSaveFilter(agent),
                FilterIndex = 0,
                FileName = agent.Id
            };

            if (saveFileDialog.ShowDialog() == true)
            {
                try
                {
                    var ext = Path.GetExtension(saveFileDialog.FileName)?.ToLower();
                    if (ext == TdlAgentExtension)
                    {
                        (agent as TdLambdaAgent)?.SaveToFile(saveFileDialog.FileName);
                        return;
                    }

                    if (ext == AgentPackExtension)
                        if (agent is TdLambdaAgent tldAgent)
                        {
                            using (var pack = new AgentPack(tldAgent))
                            {
                                pack.SaveToFile(saveFileDialog.FileName);
                            }

                            return;
                        }
                        else if (agent is AgentPack pack)
                        {
                            pack.SaveToFile(saveFileDialog.FileName);
                            return;
                        }

                    throw new Exception($"Unsupported format: {ext}");
                }
                catch (Exception e)
                {
                    MessageBox.Show($"Failed to save the agent {saveFileDialog.FileName} with message {e.Message}", "Error",
                        MessageBoxButton.OK, MessageBoxImage.Information);
                }
            }
        }
    }
}
