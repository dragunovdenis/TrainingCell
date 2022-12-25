using namespace System;

#include "TCMonitorForm.h"

using namespace System::Windows::Forms;

[STAThread]
int main() {
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);
	Application::Run(gcnew TCMonotor::TCMonitorForm());
	return 0;
}