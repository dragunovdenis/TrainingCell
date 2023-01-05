#pragma once
#include <string>

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
	private: System::Windows::Forms::Button^ randomMoveButton;
	private: System::Windows::Forms::Button^ resetButton;
	private: System::Windows::Forms::Button^ playButton;
	private: System::Windows::Forms::Label^ whiteScoreLabel;

	private: System::Windows::Forms::Label^ movesCountLabel;
	private: System::Windows::Forms::Label^ blackScoreLabel;

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
			this->blackScoreLabel = (gcnew System::Windows::Forms::Label());
			this->whiteScoreLabel = (gcnew System::Windows::Forms::Label());
			this->movesCountLabel = (gcnew System::Windows::Forms::Label());
			this->playButton = (gcnew System::Windows::Forms::Button());
			this->resetButton = (gcnew System::Windows::Forms::Button());
			this->randomMoveButton = (gcnew System::Windows::Forms::Button());
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
			this->infoPanel->Controls->Add(this->blackScoreLabel);
			this->infoPanel->Controls->Add(this->whiteScoreLabel);
			this->infoPanel->Controls->Add(this->movesCountLabel);
			this->infoPanel->Controls->Add(this->playButton);
			this->infoPanel->Controls->Add(this->resetButton);
			this->infoPanel->Controls->Add(this->randomMoveButton);
			this->infoPanel->Controls->Add(this->moveButton);
			this->infoPanel->Controls->Add(this->nextActionButton);
			this->infoPanel->Dock = System::Windows::Forms::DockStyle::Right;
			this->infoPanel->Location = System::Drawing::Point(666, 0);
			this->infoPanel->Name = L"infoPanel";
			this->infoPanel->Size = System::Drawing::Size(125, 614);
			this->infoPanel->TabIndex = 1;
			// 
			// blackScoreLabel
			// 
			this->blackScoreLabel->AutoSize = true;
			this->blackScoreLabel->Location = System::Drawing::Point(5, 588);
			this->blackScoreLabel->Name = L"blackScoreLabel";
			this->blackScoreLabel->Size = System::Drawing::Size(87, 17);
			this->blackScoreLabel->TabIndex = 7;
			this->blackScoreLabel->Text = L"Black Score:";
			// 
			// whiteScoreLabel
			// 
			this->whiteScoreLabel->AutoSize = true;
			this->whiteScoreLabel->Location = System::Drawing::Point(5, 554);
			this->whiteScoreLabel->Name = L"whiteScoreLabel";
			this->whiteScoreLabel->Size = System::Drawing::Size(89, 17);
			this->whiteScoreLabel->TabIndex = 6;
			this->whiteScoreLabel->Text = L"White Score:";
			// 
			// movesCountLabel
			// 
			this->movesCountLabel->AutoSize = true;
			this->movesCountLabel->Location = System::Drawing::Point(5, 520);
			this->movesCountLabel->Name = L"movesCountLabel";
			this->movesCountLabel->Size = System::Drawing::Size(93, 17);
			this->movesCountLabel->TabIndex = 5;
			this->movesCountLabel->Text = L"Total Moves: ";
			// 
			// playButton
			// 
			this->playButton->Location = System::Drawing::Point(3, 167);
			this->playButton->Name = L"playButton";
			this->playButton->Size = System::Drawing::Size(120, 35);
			this->playButton->TabIndex = 4;
			this->playButton->Text = L"Play";
			this->playButton->UseVisualStyleBackColor = true;
			this->playButton->Click += gcnew System::EventHandler(this, &TCMonitorForm::playButton_Click);
			// 
			// resetButton
			// 
			this->resetButton->Location = System::Drawing::Point(3, 126);
			this->resetButton->Name = L"resetButton";
			this->resetButton->Size = System::Drawing::Size(120, 35);
			this->resetButton->TabIndex = 3;
			this->resetButton->Text = L"Reset";
			this->resetButton->UseVisualStyleBackColor = true;
			this->resetButton->Click += gcnew System::EventHandler(this, &TCMonitorForm::resetButton_Click);
			// 
			// randomMoveButton
			// 
			this->randomMoveButton->Location = System::Drawing::Point(3, 85);
			this->randomMoveButton->Name = L"randomMoveButton";
			this->randomMoveButton->Size = System::Drawing::Size(120, 35);
			this->randomMoveButton->TabIndex = 2;
			this->randomMoveButton->Text = L"Random move";
			this->randomMoveButton->UseVisualStyleBackColor = true;
			this->randomMoveButton->Click += gcnew System::EventHandler(this, &TCMonitorForm::randomMoveButton_Click);
			// 
			// moveButton
			// 
			this->moveButton->Location = System::Drawing::Point(3, 44);
			this->moveButton->Name = L"moveButton";
			this->moveButton->Size = System::Drawing::Size(120, 35);
			this->moveButton->TabIndex = 1;
			this->moveButton->Text = L"Move";
			this->moveButton->UseVisualStyleBackColor = true;
			this->moveButton->Click += gcnew System::EventHandler(this, &TCMonitorForm::moveButton_Click);
			// 
			// nextActionButton
			// 
			this->nextActionButton->Location = System::Drawing::Point(3, 3);
			this->nextActionButton->Name = L"nextActionButton";
			this->nextActionButton->Size = System::Drawing::Size(120, 35);
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
			this->infoPanel->PerformLayout();
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void nextActionButton_Click(System::Object^ sender, System::EventArgs^ e) {
		_checkers->NextActionCallBack();
	}
	private: System::Void moveButton_Click(System::Object^ sender, System::EventArgs^ e) {
		_checkers->MakeMoveCallBack();
	}
	private: System::Void randomMoveButton_Click(System::Object^ sender, System::EventArgs^ e) {
		_checkers->MakeRandomMoveCallBack();
	}
	private: System::Void resetButton_Click(System::Object^ sender, System::EventArgs^ e) {
		_checkers->ResetCallBack();
	}

	private: System::Void UpdateInfo()
	{
		this->movesCountLabel->Text = gcnew String("Total Moves: ") + static_cast<int>(_checkers->GetWhiteWonCounter() + _checkers->GetBlackWonCounter());
		this->whiteScoreLabel->Text = gcnew String("White: ") + _checkers->GetWhiteWonCounter();
		this->blackScoreLabel->Text = gcnew String("Black: ") + _checkers->GetBlackWonCounter();
	}

	private: System::Void PlayMultiple()
	{
		for (auto i = 0; i < 4000; i++)
		{
			_checkers->ResetCallBack();
			while (_checkers->MakeRandomMoveCallBack()) { }
			Invoke(gcnew System::Action(this, &TCMonitorForm::UpdateInfo));
		}
	}

	private: System::Void playButton_Click(System::Object^ sender, System::EventArgs^ e) {

		Threading::ThreadStart^ playMethod = gcnew Threading::ThreadStart(this, &TCMonitorForm::PlayMultiple);
		Threading::Thread^ playThread = gcnew Threading::Thread(playMethod);
		playThread->Start();

		//_checkers->ResetCallBack();
		//int moves_count = 0;
		//while (_checkers->MakeRandomMoveCallBack()) { moves_count++; }
		//UpdateInfo()
	}
};
}
