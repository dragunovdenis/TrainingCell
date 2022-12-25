#pragma once
#include "Interfaces/CheckersUi.h"

namespace TCMonotor {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Main form
	/// </summary>
	public ref class TCMonitorForm : public System::Windows::Forms::Form
	{
		Interfaces::CheckersUi^ _checkers = nullptr;

	public:
		/// <summary>
		///	Constructor
		/// </summary>
		TCMonitorForm(void)
		{
			InitializeComponent();
			_checkers = gcnew Interfaces::CheckersUi(_mainPanel);
		}

	protected:
		/// <summary>
		///	Destructor
		/// </summary>
		~TCMonitorForm()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Panel^ _mainPanel;
	private: System::Windows::Forms::Panel^ infoPanel;
	protected:

	private:
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		///	Initialization
		/// </summary>
		void InitializeComponent(void)
		{
			this->_mainPanel = (gcnew System::Windows::Forms::Panel());
			this->infoPanel = (gcnew System::Windows::Forms::Panel());
			this->SuspendLayout();
			// 
			// _mainPanel
			// 
			this->_mainPanel->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->_mainPanel->Location = System::Drawing::Point(0, 0);
			this->_mainPanel->Name = L"_mainPanel";
			this->_mainPanel->Size = System::Drawing::Size(665, 614);
			this->_mainPanel->TabIndex = 0;
			// 
			// infoPanel
			// 
			this->infoPanel->BackColor = System::Drawing::Color::SlateGray;
			this->infoPanel->Dock = System::Windows::Forms::DockStyle::Right;
			this->infoPanel->Location = System::Drawing::Point(666, 0);
			this->infoPanel->Name = L"infoPanel";
			this->infoPanel->Size = System::Drawing::Size(125, 614);
			this->infoPanel->TabIndex = 1;
			// 
			// TCMonitorForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(791, 614);
			this->Controls->Add(this->infoPanel);
			this->Controls->Add(this->_mainPanel);
			this->Name = L"TCMonitorForm";
			this->Text = L"TCMonitor";
			this->ResumeLayout(false);

		}
#pragma endregion
	};
}
