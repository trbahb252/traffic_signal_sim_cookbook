#include "StdSignalControlDialog.h"

/*==========================================================================*/

namespace SC_GUI
{
  /** 
   * @brief Creates a new instance of the SignalControlDialog class.
   *
   * The dialog is created to edit the given control.
   *
   * @param control The signal control.
   * @param fileName The name of the configuration file.
   * @param computerName The name of the computer of an external controller.
   * @param port The port number.
   */
  SignalControlDialog::SignalControlDialog(SignalControl* control, String^ fileName, String^ computerName, int portNumber)
  : configurationFile_(fileName)
  , control_(control)
  , selectedGroup_(NULL)
  , groupChanged_(false)
  , computerName_(computerName)
  , portNumber_(portNumber)
  , currentMaxSGNr_(0)
  {
    InitializeComponent();
  }

  /*--------------------------------------------------------------------------*/

  /**
   * @brief Is called for initial initialization of the dialog.
   */
  System::Void SignalControlDialog::OnLoad(System::EventArgs^ e)
  {
    Form::OnLoad(e);
    if (control_ != NULL) {
      lstSignalGroups->BeginUpdate();

      SignalGroups& groups = control_->SignalGroups();

      for (SignalGroups::iterator iterGroup = groups.begin();
           iterGroup != groups.end();
           ++iterGroup) {
        int index = iterGroup->second->GetNr();
        lstSignalGroups->Items->Add(index);
        if (currentMaxSGNr_ < index) {
          currentMaxSGNr_ = index;
        }
      }

      lstSignalGroups->EndUpdate();

      // Select the first group and update the controls
      if (!groups.empty()) {
        groupChanged_ = true;
        selectedGroup_ = groups.begin()->second;
        lstSignalGroups->SelectedItem = selectedGroup_->GetNr();
        numSG->Text = (gcnew System::Int32(selectedGroup_->GetNr()))->ToString();
        txtName->Text = gcnew String(selectedGroup_->GetName().c_str());
        groupChanged_ = false;
      }
      else {
        pnlProperties->Enabled = false;
      }
    }

    txtFile->Text = configurationFile_;
  }

  /*--------------------------------------------------------------------------*/

  /** 
   * @brief Is called, when the dialog is closing.
   */
  System::Void SignalControlDialog::OnClosing(System::ComponentModel::CancelEventArgs^ e)
  {
    Form::OnClosing(e);
    if (this->DialogResult != System::Windows::Forms::DialogResult::OK) {
      control_->RevertSignalGroups();
    }
    else {
      std::string & fileName = control_->FileName();
      fileName = static_cast<char*>(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(txtFile->Text).ToPointer());
      configurationFile_ = txtFile->Text;
    }
  }

  /*--------------------------------------------------------------------------*/

  // Some event handling routines
  System::Void SignalControlDialog::txtSG_TextChanged(System::Object^  sender, System::EventArgs^  e)
  {

  }

  /*--------------------------------------------------------------------------*/

  System::Void SignalControlDialog::btnAdd_Click(System::Object^  sender, System::EventArgs^  e)
  {
    ++currentMaxSGNr_;
    control_->AddSignalGroup(currentMaxSGNr_);
    lstSignalGroups->Items->Add(currentMaxSGNr_);
    lstSignalGroups->SelectedItem = currentMaxSGNr_;
    pnlProperties->Enabled = true;
  }

  /*--------------------------------------------------------------------------*/

  System::Void SignalControlDialog::btnRemove_Click(System::Object^  sender, System::EventArgs^  e)
  {
    if (lstSignalGroups->SelectedItem != nullptr) {
      groupChanged_ = true;
      int index = safe_cast<int>(lstSignalGroups->SelectedItem);
      lstSignalGroups->Items->Remove(lstSignalGroups->SelectedItem);
      control_->RemoveSignalGroup(index);
      selectedGroup_ = NULL;
      groupChanged_ = false;

      if (lstSignalGroups->Items->Count != 0) {
        lstSignalGroups->SelectedItem = lstSignalGroups->Items[0];
      }
      else {
        pnlProperties->Enabled = false;
      }
    }
  }

  /*--------------------------------------------------------------------------*/

  System::Void SignalControlDialog::btnSaveAs_Click(System::Object^  sender, System::EventArgs^  e)
  {
    System::Windows::Forms::SaveFileDialog^ fileDlg = gcnew System::Windows::Forms::SaveFileDialog();

    fileDlg->Filter = "Signal control configuration|*.scprops";
    
    if (fileDlg->ShowDialog(this) != System::Windows::Forms::DialogResult::OK) {
      return;
    }

    txtFile->Text = fileDlg->FileName;
  }

  /*--------------------------------------------------------------------------*/

  System::Void SignalControlDialog::lstSignalGroups_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
  {
    if (groupChanged_) {
      return;
    }

    // Save data from controls
    if (selectedGroup_ != NULL) {
      selectedGroup_->SetNr(System::Int32::Parse(numSG->Text));
      selectedGroup_->SetName(std::string(static_cast<char*>(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(txtName->Text).ToPointer())));
    }

    if (lstSignalGroups->SelectedItem == nullptr) {
      return;
    }

    int index = safe_cast<int>(lstSignalGroups->SelectedItem);
    selectedGroup_ = control_->GetSignalGroup(index);

    if (selectedGroup_ != NULL) {
      groupChanged_ = true;

      // Update controls
      numSG->Text = (gcnew Int32(selectedGroup_->GetNr()))->ToString();
      txtName->Text = gcnew String(selectedGroup_->GetName().c_str());
      groupChanged_ = false;
    }
  }

  /*--------------------------------------------------------------------------*/

  System::Void SignalControlDialog::txtPort_TextChanged(System::Object^  sender, System::EventArgs^  e)
  {
  }

  /*--------------------------------------------------------------------------*/

  System::Void SignalControlDialog::txtComputerName_TextChanged(System::Object^  sender, System::EventArgs^  e) {
    System::Text::RegularExpressions::Regex^ regex = gcnew System::Text::RegularExpressions::Regex("[0-9a-fA-F]+");

  }

  /*--------------------------------------------------------------------------*/

  System::Void SignalControlDialog::numSG_ValueChanged(System::Object^  sender, System::EventArgs^  e)
  {
    if (groupChanged_) {
      return;
    }

    // Update list box
    groupChanged_ = true;
    int oldIndex = safe_cast<int>(lstSignalGroups->SelectedItem);
    int newIndex;
    bool validNumber = true;
    
    try {
      newIndex = Int32::Parse(numSG->Text);
    }
    catch (...) {
      validNumber = false;
    }

    validNumber = (newIndex > 0);

    if (!validNumber) {
      errorProvider->SetError(numSG, "The signal group number was invalid.");
      numSG->Text = (gcnew Int32(oldIndex))->ToString();
      groupChanged_ = false;
      return;
    }

    if (!control_->RenameSignalGroup(oldIndex, newIndex)) {
      errorProvider->SetError(numSG, "The signal group number should be unique.");
      numSG->Text = (gcnew Int32(oldIndex))->ToString();
      groupChanged_ = false;
      return;
    }

    errorProvider->SetError(numSG, String::Empty);

    lstSignalGroups->Items->Remove(oldIndex);
    lstSignalGroups->Items->Add(newIndex);
    lstSignalGroups->SelectedItem = newIndex;

    if (currentMaxSGNr_ < newIndex) {
      currentMaxSGNr_ = newIndex;
    }

    groupChanged_ = false;
  }
}
