#pragma once;

#include "SignalGroup.h"
#include <string>

namespace SC_GUI
{
  /**
   * @brief The class contains the logic for a examplified signal
   *        controller gui dll.
   *
   * The used signal controller communicates with an external signal 
   * controller using TCP/IP, so it contains additional the definition
   * of a computer name and a port number to locate the signal controller.
   *
   * Per default the external controller should be located on the local 
   * machine. In the example the logic of the signal control is fully
   * encapsulated in the external controller, so signal groups has
   * only default the default parameters, that could be edited. But it is
   * easy to add additional parameters. This requires updating the SignalGroup
   * class, the ReadConfigurationFile and WriteConfigurationFile functions and
   * the SignalControlDialog class.
   *
   * Further informations are available at the specfic methods and classes.
   *
   */
  class SignalGuiLogic
  {
  public:
    SignalGuiLogic(void)
    : computerName_("localhost")
    , portNumber_(1234) 
    {}
    
    virtual ~SignalGuiLogic(void)
    {}

    void ShowDialog(long scNo);
    void ShowError(long scNo, std::string & errorMessage);

    bool ReadConfigurationFile(long scNo, std::string & file);
    bool WriteConfigurationFile(long scNo, std::string & file);

    bool AddSignalControl(long scNO);
    bool ContainsSignalControl(long scNO);
    SignalControl* GetSignalControl(long scNO);

    void SetInputFile(std::string & inputFile) {
      inputFile_ = inputFile;
    }

  private:
    SignalControls signalControls_;

    // Global properties
    std::string inputFile_;
    std::string computerName_;
    int portNumber_;
  };
}