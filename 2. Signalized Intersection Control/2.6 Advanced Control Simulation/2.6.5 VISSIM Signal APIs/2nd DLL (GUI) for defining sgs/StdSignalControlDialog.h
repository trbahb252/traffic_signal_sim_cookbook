#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

#include "SignalGroup.h"

namespace SC_GUI {

	/**
   * @brief The dialog for editing the signal control properties.
   *
   * The dialog offers functionalities to edit the signal control and
   * all signal groups that belongs to the signal group. If you have
   * additional properties for your signal control or the signal groups
   * you should update the dialog.
   *
   * It is required to update the lstSignalGroups_SelectedIndexChanged that is
   * called when the selected signal group changed and the OnLoad method that
   * is called for initial initialization.
   */
	public ref class SignalControlDialog : public System::Windows::Forms::Form
	{
	public:
    SignalControlDialog(void) 
    : control_(NULL)
    , configurationFile_(String::Empty)
    , selectedGroup_(NULL)
    , groupChanged_(false)
    , computerName_(String::Empty)
    , portNumber_(1234)
    , currentMaxSGNr_(0)
		{
			InitializeComponent();
		}

    SignalControlDialog(SignalControl* control, String^ fileName, String^ computerName, int portNumber);

    property String^ FileName
    {
      String^ get(void) {return configurationFile_;}
    }

    property String^ ComputerName
    {
      String^ get(void) {return computerName_;}
    }

    property int PortNumber
    {
      int get(void) {return portNumber_;}
    }

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~SignalControlDialog()
		{
			if (components)
			{
				delete components;
			}
		}


  protected: 

  protected: 





  private: System::Windows::Forms::Button^  btnCancel;




  private: System::Windows::Forms::Button^  btnOk;





  private: System::Windows::Forms::ErrorProvider^  errorProvider;
	private: System::Windows::Forms::TabControl^ tabulars;
	private: System::Windows::Forms::TabPage^ tabSignalControl;
	private: System::Windows::Forms::ListBox^ lstSignalGroups;
	private: System::Windows::Forms::Panel^ pnlProperties;
	private: System::Windows::Forms::TextBox^ numSG;
	private: System::Windows::Forms::TextBox^ txtName;
	private: System::Windows::Forms::Label^ lblSG;
	private: System::Windows::Forms::Label^ lblName;
	private: System::Windows::Forms::Button^ btnSaveAs;
	private: System::Windows::Forms::Button^ btnRemove;
	private: System::Windows::Forms::Label^ lblFile;
	private: System::Windows::Forms::Button^ btnAdd;
	private: System::Windows::Forms::TextBox^ txtFile;












  private: System::ComponentModel::IContainer^  components;


	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			this->btnCancel = (gcnew System::Windows::Forms::Button());
			this->btnOk = (gcnew System::Windows::Forms::Button());
			this->errorProvider = (gcnew System::Windows::Forms::ErrorProvider(this->components));
			this->tabSignalControl = (gcnew System::Windows::Forms::TabPage());
			this->txtFile = (gcnew System::Windows::Forms::TextBox());
			this->btnAdd = (gcnew System::Windows::Forms::Button());
			this->lblFile = (gcnew System::Windows::Forms::Label());
			this->btnRemove = (gcnew System::Windows::Forms::Button());
			this->btnSaveAs = (gcnew System::Windows::Forms::Button());
			this->pnlProperties = (gcnew System::Windows::Forms::Panel());
			this->lblName = (gcnew System::Windows::Forms::Label());
			this->lblSG = (gcnew System::Windows::Forms::Label());
			this->txtName = (gcnew System::Windows::Forms::TextBox());
			this->numSG = (gcnew System::Windows::Forms::TextBox());
			this->lstSignalGroups = (gcnew System::Windows::Forms::ListBox());
			this->tabulars = (gcnew System::Windows::Forms::TabControl());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->errorProvider))->BeginInit();
			this->tabSignalControl->SuspendLayout();
			this->pnlProperties->SuspendLayout();
			this->tabulars->SuspendLayout();
			this->SuspendLayout();
			// 
			// btnCancel
			// 
			this->btnCancel->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->btnCancel->Location = System::Drawing::Point(280, 266);
			this->btnCancel->Name = L"btnCancel";
			this->btnCancel->Size = System::Drawing::Size(75, 23);
			this->btnCancel->TabIndex = 9;
			this->btnCancel->Text = L"&Cancel";
			this->btnCancel->UseVisualStyleBackColor = true;
			// 
			// btnOk
			// 
			this->btnOk->DialogResult = System::Windows::Forms::DialogResult::OK;
			this->btnOk->Location = System::Drawing::Point(199, 266);
			this->btnOk->Name = L"btnOk";
			this->btnOk->Size = System::Drawing::Size(75, 23);
			this->btnOk->TabIndex = 8;
			this->btnOk->Text = L"&OK";
			this->btnOk->UseVisualStyleBackColor = true;
			// 
			// errorProvider
			// 
			this->errorProvider->ContainerControl = this;
			// 
			// tabSignalControl
			// 
			this->tabSignalControl->Controls->Add(this->lstSignalGroups);
			this->tabSignalControl->Controls->Add(this->pnlProperties);
			this->tabSignalControl->Controls->Add(this->btnSaveAs);
			this->tabSignalControl->Controls->Add(this->btnRemove);
			this->tabSignalControl->Controls->Add(this->lblFile);
			this->tabSignalControl->Controls->Add(this->btnAdd);
			this->tabSignalControl->Controls->Add(this->txtFile);
			this->tabSignalControl->Location = System::Drawing::Point(4, 22);
			this->tabSignalControl->Name = L"tabSignalControl";
			this->tabSignalControl->Padding = System::Windows::Forms::Padding(3);
			this->tabSignalControl->Size = System::Drawing::Size(344, 234);
			this->tabSignalControl->TabIndex = 1;
			this->tabSignalControl->Text = L"Signal control";
			this->tabSignalControl->UseVisualStyleBackColor = true;
			// 
			// txtFile
			// 
			this->txtFile->Location = System::Drawing::Point(6, 25);
			this->txtFile->Name = L"txtFile";
			this->txtFile->Size = System::Drawing::Size(303, 20);
			this->txtFile->TabIndex = 6;
			// 
			// btnAdd
			// 
			this->btnAdd->Location = System::Drawing::Point(6, 204);
			this->btnAdd->Name = L"btnAdd";
			this->btnAdd->Size = System::Drawing::Size(75, 23);
			this->btnAdd->TabIndex = 4;
			this->btnAdd->Text = L"&Add";
			this->btnAdd->UseVisualStyleBackColor = true;
			this->btnAdd->Click += gcnew System::EventHandler(this, &SignalControlDialog::btnAdd_Click);
			// 
			// lblFile
			// 
			this->lblFile->AutoSize = true;
			this->lblFile->Location = System::Drawing::Point(3, 4);
			this->lblFile->Name = L"lblFile";
			this->lblFile->Size = System::Drawing::Size(88, 13);
			this->lblFile->TabIndex = 9;
			this->lblFile->Text = L"Configuration file:";
			// 
			// btnRemove
			// 
			this->btnRemove->Location = System::Drawing::Point(87, 204);
			this->btnRemove->Name = L"btnRemove";
			this->btnRemove->Size = System::Drawing::Size(75, 23);
			this->btnRemove->TabIndex = 5;
			this->btnRemove->Text = L"&Remove";
			this->btnRemove->UseVisualStyleBackColor = true;
			this->btnRemove->Click += gcnew System::EventHandler(this, &SignalControlDialog::btnRemove_Click);
			// 
			// btnSaveAs
			// 
			this->btnSaveAs->Location = System::Drawing::Point(314, 22);
			this->btnSaveAs->Name = L"btnSaveAs";
			this->btnSaveAs->Size = System::Drawing::Size(22, 23);
			this->btnSaveAs->TabIndex = 7;
			this->btnSaveAs->Text = L"...";
			this->btnSaveAs->UseVisualStyleBackColor = true;
			this->btnSaveAs->Click += gcnew System::EventHandler(this, &SignalControlDialog::btnSaveAs_Click);
			// 
			// pnlProperties
			// 
			this->pnlProperties->Controls->Add(this->numSG);
			this->pnlProperties->Controls->Add(this->txtName);
			this->pnlProperties->Controls->Add(this->lblSG);
			this->pnlProperties->Controls->Add(this->lblName);
			this->pnlProperties->Location = System::Drawing::Point(168, 51);
			this->pnlProperties->Name = L"pnlProperties";
			this->pnlProperties->Size = System::Drawing::Size(169, 147);
			this->pnlProperties->TabIndex = 2;
			// 
			// lblName
			// 
			this->lblName->AutoSize = true;
			this->lblName->Location = System::Drawing::Point(3, 42);
			this->lblName->Name = L"lblName";
			this->lblName->Size = System::Drawing::Size(38, 13);
			this->lblName->TabIndex = 12;
			this->lblName->Text = L"Name:";
			// 
			// lblSG
			// 
			this->lblSG->AutoSize = true;
			this->lblSG->Location = System::Drawing::Point(3, 0);
			this->lblSG->Name = L"lblSG";
			this->lblSG->Size = System::Drawing::Size(107, 13);
			this->lblSG->TabIndex = 10;
			this->lblSG->Text = L"Signal group number:";
			// 
			// txtName
			// 
			this->txtName->Location = System::Drawing::Point(6, 58);
			this->txtName->Name = L"txtName";
			this->txtName->Size = System::Drawing::Size(162, 20);
			this->txtName->TabIndex = 3;
			// 
			// numSG
			// 
			this->numSG->Location = System::Drawing::Point(3, 19);
			this->numSG->Name = L"numSG";
			this->numSG->Size = System::Drawing::Size(88, 20);
			this->numSG->TabIndex = 2;
			this->numSG->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			this->numSG->Leave += gcnew System::EventHandler(this, &SignalControlDialog::numSG_ValueChanged);
			// 
			// lstSignalGroups
			// 
			this->lstSignalGroups->FormattingEnabled = true;
			this->lstSignalGroups->Location = System::Drawing::Point(6, 51);
			this->lstSignalGroups->Name = L"lstSignalGroups";
			this->lstSignalGroups->Size = System::Drawing::Size(156, 147);
			this->lstSignalGroups->Sorted = true;
			this->lstSignalGroups->TabIndex = 1;
			this->lstSignalGroups->SelectedIndexChanged += gcnew System::EventHandler(this, &SignalControlDialog::lstSignalGroups_SelectedIndexChanged);
			// 
			// tabulars
			// 
			this->tabulars->Controls->Add(this->tabSignalControl);
			this->tabulars->Location = System::Drawing::Point(3, 0);
			this->tabulars->Name = L"tabulars";
			this->tabulars->SelectedIndex = 0;
			this->tabulars->Size = System::Drawing::Size(352, 260);
			this->tabulars->TabIndex = 14;
			// 
			// SignalControlDialog
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(361, 296);
			this->Controls->Add(this->tabulars);
			this->Controls->Add(this->btnOk);
			this->Controls->Add(this->btnCancel);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"SignalControlDialog";
			this->Text = L"AHB25 Sim_Cookbook: Define SG";
			this->Load += gcnew System::EventHandler(this, &SignalControlDialog::SignalControlDialog_Load);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->errorProvider))->EndInit();
			this->tabSignalControl->ResumeLayout(false);
			this->tabSignalControl->PerformLayout();
			this->pnlProperties->ResumeLayout(false);
			this->pnlProperties->PerformLayout();
			this->tabulars->ResumeLayout(false);
			this->ResumeLayout(false);

		}
#pragma endregion

private: 
  System::Void txtSG_TextChanged(System::Object^  sender, System::EventArgs^  e);
  System::Void txtPort_TextChanged(System::Object^  sender, System::EventArgs^  e);
  System::Void txtComputerName_TextChanged(System::Object^  sender, System::EventArgs^  e);
  System::Void btnAdd_Click(System::Object^  sender, System::EventArgs^  e);
  System::Void btnRemove_Click(System::Object^  sender, System::EventArgs^  e);
  System::Void btnSaveAs_Click(System::Object^  sender, System::EventArgs^  e);
  System::Void lstSignalGroups_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e);
  System::Void numSG_ValueChanged(System::Object^  sender, System::EventArgs^  e);

  bool groupChanged_;
  SignalControl* control_;
  SignalGroup* selectedGroup_;
  String^ configurationFile_;
  String^ computerName_;
  int portNumber_;
  int currentMaxSGNr_;

protected:
  virtual void OnLoad(System::EventArgs^  e) override;
  virtual void OnClosing(System::ComponentModel::CancelEventArgs^ e) override;

private: System::Void SignalControlDialog_Load(System::Object^ sender, System::EventArgs^ e) {
}
};
}
