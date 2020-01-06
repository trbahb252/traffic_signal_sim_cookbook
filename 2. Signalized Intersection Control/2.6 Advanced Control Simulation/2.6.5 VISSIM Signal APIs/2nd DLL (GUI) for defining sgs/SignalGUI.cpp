/*==========================================================================*/
/*  SignalGUI.cpp                                    DLL Module for VISSIM  */
/*                                                                          */
/*  Interface module for GUI elements for signal controllers.               */
/*                                                                          */
/*  Version of 2003-07-04                                   Lukas Kautzsch  */
/*==========================================================================*/
#include "SignalGUI.h"

#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "SignalGroup.h"
#include "StdSignalGuiLogic.h"
/*==========================================================================*/

static  wchar_t  AppName[] = L"StdSC_GUI.DLL";
static  SC_GUI::SignalGuiLogic scGUILogic;

/*==========================================================================*/

SIGNALGUI_API  int  SignalGUISetValue (long   type, 
                                       long   sc_no,
                                       long   index1,
                                       long   index2,
                                       long   long_value,
                                       double double_value,
                                       char   *string_value)
{
  /* Called from VISSIM to set the value of a data object of type <type> */
  /* (see definition of type numbers above) of controller no. <sc_no>,   */
  /* selected by <index1> and possibly <index2>, to <long_value>,        */
  /* <double_value> or <*string_value> (object and value selection       */
  /* depending on <type>).                                               */
  /* Return value is 1 on success, otherwise 0.                          */

  switch (type) {
    case SIGNALGUI_DATA_INP_FILE :
      scGUILogic.SetInputFile(std::string(string_value));
      return 1;
    case SIGNALGUI_DATA_INP_PATH :
      return 1;
    case SIGNALGUI_DATA_NEMA_LEVEL :
      return 1;
    case SIGNALGUI_DATA_DEMO_VERSION :
      return 1;
    case SIGNALGUI_DATA_SIM_RUN_ACTIVE :
      return 1;
    case SIGNALGUI_DATA_DATA_FILE :
    {
      if (index1 < 1 || index1 > 2) {
        return 0;
      }

      SignalControl* control = scGUILogic.GetSignalControl(sc_no);
      std::string & configFile = control->FileName();
      configFile = string_value;

      return 1;
    }
    case SIGNALGUI_DATA_SG_NUMBER :
    {
      SignalControl* control = scGUILogic.GetSignalControl(sc_no);
      control->AddSignalGroup(long_value);
      return 1;
    }
    case SIGNALGUI_DATA_SG_NAME :
    {
      SignalControl* control = scGUILogic.GetSignalControl(sc_no);
      SignalGroup* group = control->GetSignalGroup(index1);
      group->SetName(std::string(string_value));
      return 1;
    }
    default :
      return 0;
  }
} /* SignalGUISetValue */

/*--------------------------------------------------------------------------*/

SIGNALGUI_API  int  SignalGUIGetValue (long   type, 
                                       long   sc_no,
                                       long   index1,
                                       long   index2,
                                       long   *long_value,
                                       double *double_value,
                                       char   *string_value)
{
  /* Called from VISSIM to get the value of a data object of type <type>  */
  /* (see definition of type numbers above) of controller no. <sc_no>,    */
  /* selected by <index1> and possibly <index2>.                          */
  /* This function writes that value to <*double_value>, <*float_value>   */
  /* or <*string_value> (object and value selection depending on <type>). */
  /* Return value is 1 on success, otherwise 0.                           */

  switch (type) {
    case SIGNALGUI_DATA_DATA_FILE :
    {
      if (index1 < 1 || index1 > 2) {
        return 0;
      }
      SignalControl* control = scGUILogic.GetSignalControl(sc_no);
      strcpy (string_value, control->FileName().c_str());
      return 1;
    }
    case SIGNALGUI_DATA_CYCLE_TIME :
      *double_value = 0.0;
      return 1;
    case SIGNALGUI_DATA_SG_CHANGED_NUMBER :
    {
      SignalControl* control = scGUILogic.GetSignalControl(sc_no);
      SignalGroup* group = control->GetSignalGroup(index1, true);
      *long_value = group->GetNr();
      return 1;
    }
    case SIGNALGUI_DATA_NO_OF_NEW_GROUPS :
    {
      SignalControl* control = scGUILogic.GetSignalControl(sc_no);

      if (control != NULL) {
        *long_value = static_cast<long>(control->NewGroups().size());
      }
      else {
        *long_value = 0;
      }
      return 1;
    }
    case SIGNALGUI_DATA_NEW_SG_NUMBER :
    {
      SignalControl* control = scGUILogic.GetSignalControl(sc_no);
      
      std::vector<SignalGroup*> & newSGs = control->NewGroups();
      *long_value = newSGs[index1]->GetNr();

      return 1;
    }
    case SIGNALGUI_DATA_SG_NAME :
    {
      SignalControl* control = scGUILogic.GetSignalControl(sc_no);
      SignalGroup* group = control->GetSignalGroup(index1);
      strcpy(string_value, group->GetName().c_str());
      return 1;
    }
    case SIGNALGUI_DATA_SG_T_AMBER :
      *double_value = 3.0;
      return 1;
    case SIGNALGUI_DATA_SG_T_RED_AMBER :
      *double_value = 0.0;
      return 1;
    case SIGNALGUI_DATA_SG_T_MIN_GREEN :
      *double_value = 5.0;
      return 1;
    case SIGNALGUI_DATA_SG_T_MIN_RED :
      *double_value = 0.0;
      return 1;
    case SIGNALGUI_DATA_SG_T_REDCLEARANCE :
      *double_value = 2.0;
      return 1;
    default :
      return 0;
  }
} /* SignalGUIGetValue */

/*==========================================================================*/

SIGNALGUI_API  int  SignalGUIExecuteCommand (long number, long sc_no)
{
  /* Called from VISSIM to execute the command <number> if that is */
  /* available in the DLL (see definitions above) for controller   */
  /* no. <sc_no>.                                                  */
  /* Return value is 1 on success, otherwise 0.                    */

  switch (number) {
    case SIGNALGUI_COMMAND_INIT_CONTROLLER :
      scGUILogic.AddSignalControl(sc_no);
      return 1;
    case SIGNALGUI_COMMAND_READ_DATA_FILES :
    {
      SignalControl* control = scGUILogic.GetSignalControl(sc_no);
      if (!control->FileName().empty()) {
        scGUILogic.ReadConfigurationFile(sc_no, control->FileName());
      }
      return 1;
    }
    case SIGNALGUI_COMMAND_EDIT_SC_PARAMETERS : 
    {
      scGUILogic.ShowDialog(sc_no);
      return 1;
    }
    case SIGNALGUI_COMMAND_EDIT_SG_PARAMETERS :
    {
      return 1;
    }
    case SIGNALGUI_COMMAND_CHECK_SG_NUMBERS :
      return 1;
    default :
      return 0;
  }
} /* SignalGUIExecuteCommand */

/*==========================================================================*/
/*  End of SignalGUI.cpp                                                    */
/*==========================================================================*/
