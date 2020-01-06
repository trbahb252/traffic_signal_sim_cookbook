#include "StdSignalGuiLogic.h"
#include "StdSignalControlDialog.h"
#include <fstream>
#include <sstream>

/*==========================================================================*/

namespace SC_GUI
{
  /**
   * @brief The tokens are required for reading/writing configuration files.
   */
  #define SCGUI_TOKEN_EOF      "eof"
  #define SCGUI_TOKEN_COMPUTER "computer"
  #define SCGUI_TOKEN_PORT     "port"
  #define SCGUI_TOKEN_SG       "sg"

  /*--------------------------------------------------------------------------*/

  /** 
   * @brief Shows the edit dialog for the given signal control.
   *
   * The function changes the control to the edit mode and shows
   * the dialog. If editing is canceled, all made changes are 
   * discarded, otherwise the changes are written to the given 
   * properties file.
   *
   * @param scNo The signal control.
   */
  void SignalGuiLogic::ShowDialog(long scNo)
  {
    SignalControl* control = GetSignalControl(scNo);

    std::string fileName = control->FileName();

    if (fileName.empty()) {
      std::stringstream buffer;
      buffer << inputFile_ << control->GetNumber() << ".scprops";
      fileName = buffer.str();
    }

    control->EditModus(true);

    SC_GUI::SignalControlDialog^ dlg = gcnew SC_GUI::SignalControlDialog(control, gcnew String(fileName.c_str()), gcnew String(computerName_.c_str()), portNumber_);
    if (dlg->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
      computerName_ = static_cast<char*>(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(dlg->ComputerName).ToPointer());
      portNumber_ = dlg->PortNumber;
      control->FileName() = static_cast<char*>(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(dlg->FileName).ToPointer());
      control->ComputeUpdatedSignalGroups();
      WriteConfigurationFile(scNo, control->FileName());
    }
  }

  /*--------------------------------------------------------------------------*/

  /**
   * @brief Shows the given error in a message box.
   *
   * @param scNO The signal control, where the error is occured.
   * @param errorMessage The message to display.
   */
  void SignalGuiLogic::ShowError(long scNO, std::string & errorMessage) 
  {
    System::Windows::Forms::MessageBox::Show(gcnew String(errorMessage.c_str()), "Signal control " + scNO, 
                                             System::Windows::Forms::MessageBoxButtons::OK, 
                                             System::Windows::Forms::MessageBoxIcon::Error);
  }

  /*--------------------------------------------------------------------------*/

  /** 
   * @brief Adds a signal control with the given id to the editable signal controls.
   *
   * If a signal control with the id already exists, it will be erased and the 
   * new one added. This is required to discard user defined properties.
   *
   * @param scNO The signal control to add.
   *
   * @return Returns always <code>true</code>.
   */
  bool SignalGuiLogic::AddSignalControl(long scNO)
  {
    SignalControls::iterator scIter = signalControls_.find(scNO);

    if (scIter != signalControls_.end()) {
      delete scIter->second;
      signalControls_.erase(scIter);
    }
    
    signalControls_.insert(std::make_pair(scNO, new SignalControl(scNO)));

    return true;
  }

  /*--------------------------------------------------------------------------*/

  /**
   * @brief Gets, whether the signal control exists or not.
   *
   * @param scNO The signal control.
   *
   * @return <code>true</code>, if the signal control exists, otherwise
   *         <code>false</code>.
   */
  bool SignalGuiLogic::ContainsSignalControl(long scNO)
  {
    return (signalControls_.find(scNO) != signalControls_.end());
  }

  /*--------------------------------------------------------------------------*/

  /**
   * @brief Gets the signal control with the given id.
   *
   * @param scNO The signal control.
   *
   * @return The signal control, if exists, otherwise <code>NULL</code>.
   */
  SignalControl* SignalGuiLogic::GetSignalControl(long scNO)
  {
    SignalControls::iterator scIter = signalControls_.find(scNO);
    return (scIter == signalControls_.end()? NULL: (*scIter).second);
  }

  /*--------------------------------------------------------------------------*/

  /**
   * @brief Reads the configuration file for the given signal control.
   *
   * If required edit this method to add additional properties for the
   * signal controls to read.
   *
   * @param scNO The signal control.
   * @param file The configuration file.
   *
   * @return <code>true</code>, if reading succeeded, otherwise 
   *         <code>false</code>.
   */
  bool SignalGuiLogic::ReadConfigurationFile(long scNo, std::string & file)
  {
    SignalControl* control = GetSignalControl(scNo);

    if (control == NULL) {
      return false;
    }

    // Add additional data to existing groups
    std::ifstream fileStream(file.c_str());

    if (!fileStream) {
      return false;
    }

    std::string token(SCGUI_TOKEN_EOF);
    std::string name;
    int portNr = -1;

    fileStream >> token >> name;

    if (fileStream.eof() || fileStream.fail() || token != SCGUI_TOKEN_COMPUTER) {
      // Corrupted data read
      std::stringstream buffer;
      buffer << "The configuration file " << file << " is not valid.";
      ShowError(scNo, buffer.str());
      return false;
    }

    computerName_ = name;

    fileStream >> token >> std::dec >> portNr;

    if (fileStream.eof() || fileStream.fail() || token != SCGUI_TOKEN_PORT || portNr == -1) {
      // Corrupted data read
      std::stringstream buffer;
      buffer << "The configuration file " << file << " is not valid.";
      ShowError(scNo, buffer.str());
      return false;
    }

    portNumber_ = portNr;

    while (!fileStream.eof()) {
      std::string sgToken(SCGUI_TOKEN_EOF);
      long groupNr = -1;

      fileStream >> sgToken >> std::dec >> groupNr;

      if (fileStream.eof() && groupNr == -1) {
        // End of file reached
        break;
      }

      if (fileStream.fail() || sgToken != SCGUI_TOKEN_SG) {
        // Corrupted data read
        std::stringstream buffer;
        buffer << "The configuration file " << file << " is not valid.";
        ShowError(scNo, buffer.str());
        return false;
      }

      SignalGroup* group = control->GetSignalGroup(groupNr);

      if (group == NULL) {
        control->AddSignalGroup(groupNr);
      }
    }

    return true;
  }

  /*--------------------------------------------------------------------------*/

  /**
   * @brief Writes the configuration file for the signal control.
   *
   * Add here additional properties that should be written to the 
   * configuration file. Confirm, that the reader is also changed
   * equivalently. The configuration consists of pairs of a token
   * with a value in each line.
   *
   * @param scNo The signal control.
   * @param file The name of the configuration file.
   *
   * @return <code>true</code>, if writing succeeded, otherwise 
   *         <code>false</code>
   */
  bool SignalGuiLogic::WriteConfigurationFile(long scNo, std::string & file)
  {
    SignalControl* control = GetSignalControl(scNo);

    if (control == NULL) {
      return false;
    }

    std::ofstream fileStream(file.c_str());

    if (!fileStream) {
      std::stringstream buffer;
      buffer << "Cannot open inputfile " << file << ".";
      ShowError(scNo, buffer.str());
      return false;
    }

    fileStream << SCGUI_TOKEN_COMPUTER << " " << computerName_ << std::endl;
    fileStream << SCGUI_TOKEN_PORT << " " << portNumber_ << std::endl;

    SignalGroups & groups = control->SignalGroups();

    for (SignalGroups::iterator groupIter = groups.begin();
         groupIter != groups.end();
         ++groupIter) {
      SignalGroup* group = groupIter->second;

      if (group->GetNr() != 0) {
        fileStream << SCGUI_TOKEN_SG << " " << group->GetNr() << std::endl;
      }
    }

    fileStream.close();

    return true;
  }
}
