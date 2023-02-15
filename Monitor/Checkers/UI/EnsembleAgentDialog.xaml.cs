using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace Monitor.Checkers.UI
{
    /// <summary>
    /// Interaction logic for EnsembleAgentDialog.xaml
    /// </summary>
    public partial class EnsembleAgentDialog : Window
    {
        /// <summary>
        /// Created ensemble
        /// </summary>
        public EnsembleAgent Ensemble { get; private set; }

        /// <summary>
        /// Constructor
        /// </summary>
        public EnsembleAgentDialog(IList<TdLambdaAgent> agents)
        {
            InitializeComponent();
            AgentControl.AssignAvailableAgents(agents);
        }

        /// <summary>
        /// Button click handler
        /// </summary>
        private void OkButton_OnClick(object sender, RoutedEventArgs e)
        {
            Ensemble = AgentControl.CreateEnsemble();
            DialogResult = true;
        }

        /// <summary>
        /// Button click handler
        /// </summary>
        private void CancelButton_OnClick(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
        }
    }
}
