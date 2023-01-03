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
	private: System::Windows::Forms::Button^ moveButton;
	private: System::Windows::Forms::Button^ nextActionButton;
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
			this->moveButton = (gcnew System::Windows::Forms::Button());
			this->nextActionButton = (gcnew System::Windows::Forms::Button());
			this->infoPanel->SuspendLayout();
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
			this->infoPanel->Controls->Add(this->moveButton);
			this->infoPanel->Controls->Add(this->nextActionButton);
			this->infoPanel->Dock = System::Windows::Forms::DockStyle::Right;
			this->infoPanel->Location = System::Drawing::Point(666, 0);
			this->infoPanel->Name = L"infoPanel";
			this->infoPanel->Size = System::Drawing::Size(125, 614);
			this->infoPanel->TabIndex = 1;
			// 
			// moveButton
			// 
			this->moveButton->Location = System::Drawing::Point(3, 44);
			this->moveButton->Name = L"moveButton";
			this->moveButton->Size = System::Drawing::Size(119, 40);
			this->moveButton->TabIndex = 1;
			this->moveButton->Text = L"Move";
			this->moveButton->UseVisualStyleBackColor = true;
			this->moveButton->Click += gcnew System::EventHandler(this, &TCMonitorForm::moveButton_Click);
			// 
			// nextActionButton
			// 
			this->nextActionButton->Location = System::Drawing::Point(3, 3);
			this->nextActionButton->Name = L"nextActionButton";
			this->nextActionButton->Size = System::Drawing::Size(119, 35);
			this->nextActionButton->TabIndex = 0;
			this->nextActionButton->Text = L"Preview next";
			this->nextActionButton->UseVisualStyleBackColor = true;
			this->nextActionButton->Click += gcnew System::EventHandler(this, &TCMonitorForm::nextActionButton_Click);
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
			this->infoPanel->ResumeLayout(false);
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void nextActionButton_Click(System::Object^ sender, System::EventArgs^ e) {
		_checkers->NextActionCallBack();
	}
	private: System::Void moveButton_Click(System::Object^ sender, System::EventArgs^ e) {
		_checkers->MakeMoveCallBack();
	}
};
}
