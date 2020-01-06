/*==========================================================================*/
/*  SC_Interface.cpp                                 DLL Module for VISSIM  */
/*                                                                          */
/*  Interface module for signal controller DLLs.                            */
/*                                                                          */
/*  Version of 2005-11-18                                   Lukas Kautzsch  */
/*==========================================================================*/

#include "lsa_fehl.h"
#include "lsa_puff.h"
#include "lsa_rahm.h"
#include "SC_Interface.h"
#include "lsa_fehl.h"
#include "lsa_rahm.rh"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#pragma unmanaged

/*==========================================================================*/

static  bool  demo_version = FALSE;

/*==========================================================================*/

BOOL APIENTRY DllMain (HANDLE  hModule, 
                       DWORD   ul_reason_for_call, 
                       LPVOID  lpReserved)
{
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
      break;
    case DLL_PROCESS_DETACH:
      ExitLSAFehler ();
      break;
  }
  return TRUE;
}

/*==========================================================================*/

SC_DLL_API  int  ControllerSetValue (long   type,
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

  if (sc_no == 0) {
    /* Values that are relevant for the library, not for a specific      */
    /* signal controller.                                                */
    switch (type) {
      case SC_DLL_DATA_LICENSE_CONTROLLERS :
        /* ### return 0 if the relevant bit is not set in long_value */
        return 1;

      case SC_DLL_DATA_CONSOLE_MODE:
        SetConsoleMode(long_value != 0);
        return 1;

      case SC_DLL_DATA_PROCESS_ID:
        SetProcessId(long_value);
        return 1;

      default:
        return 0;
    }
  }
  else {
    /* Values that are relevant for a specific signal controller         */

    if (!SetLSAPuffer (sc_no)) return 0;

    switch (type) {
      case SC_DLL_DATA_LICENSE_CONTROLLERS :
        /* ### return 0 if the relevant bit is not set in long_value */
        return 1;
      case SC_DLL_DATA_SNAPSHOT_FILE :
        strncpy (SnapshotFilename, string_value, sizeof (SnapshotFilename));
        return 1;

      case SC_DLL_DATA_DATA_FILE :
        if (index1 < 1 || index1 > 2) {
          return 0;
        }
        strncpy (LSA_Puffer->DataFileName[index1 - 1], string_value, 
                sizeof (LSA_Puffer->DataFileName[index1 - 1]) - 1);
        LSA_Puffer->DataFileName[index1 - 1][sizeof (LSA_Puffer->DataFileName[index1 - 1]) - 1] = '\0';
        return 1;

      case DEMOVERSION :
        demo_version = TRUE;
        return 1;

      case SC_DLL_DATA_SIM_TIME_OF_DAY :
        strncpy (SimulationTimeOfDay, string_value, sizeof (SimulationTimeOfDay));
        return 1;

      case SC_DLL_DATA_SIM_DATE :
        SimulationDate = long_value;
        return 1;

      case SC_DLL_DATA_SIM_TIME :
      case SC_DLL_DATA_CYCLE_TIME :
      case SC_DLL_DATA_CYCLE_SECOND :
      case SC_DLL_DATA_SG_T_GREEN_MIN :
      case SC_DLL_DATA_SG_T_RED_MIN :
      case SC_DLL_DATA_SG_T_AMBER :
      case SC_DLL_DATA_SG_T_RED_AMBER :
      case SC_DLL_DATA_SG_CURR_DURATION :
        return DoubleToLSAPuffer (type, index1, double_value);

      case SC_DLL_DATA_CONSOLE_MODE:
        SetConsoleMode(long_value != 0);
        return 1;

      case SC_DLL_DATA_PROCESS_ID:
        SetProcessId(long_value);
        return 1;
      case SC_DLL_DATA_DETECTOR_NAME:
        return StringToLSAPuffer(type, index1, std::string(string_value));

      default :
        return LongToLSAPuffer (type, index1, long_value);
    } /* switch (type) */
  }
} /* ControllerSetValue  */

/*--------------------------------------------------------------------------*/

P_ProtTripel  ppt_by_index (long sc_no, long index) 
{
  /* Returns a pointer to the <index>th record triple of controller */
  /* number <sc_no>. Is faster if it is called with increasing      */
  /* <index> values.                                                */

          P_ProtTripel  ppt, start_ppt;
          long          i, start_index;
  static  P_ProtTripel  last_ppt = NULL;
  static  long          last_sc_no = 0, last_index = 0;

  if (sc_no == last_sc_no) {
    if (index == last_index) {
      return last_ppt;
    }
    else if (index > last_index) {
      start_index = last_index;
      start_ppt   = last_ppt;
    }
    else {
      start_index = 1;
      start_ppt   = LSA_Puffer->ProtTripel;
    }
  }
  else {
    start_index = 1;
    start_ppt   = LSA_Puffer->ProtTripel;
  }
  for (ppt = start_ppt, i = start_index;
       ppt != NULL && i < index; 
       ppt = ppt->next, i++);
  return ppt;
} /* ppt_by_index */

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

SC_DLL_API  int  ControllerGetValue (long   type,
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
  /* This function writes that value to <*long_value>, <*double_value>    */
  /* or <*string_value> (object and value selection depending on <type>). */
  /* Return value is 1 on success, otherwise 0.                           */

  int           sg_i, det_i;
  char          proc_name[] = "ControllerGetValue";
  P_ProtTripel  ppt;

  if (sc_no == 0) {
    /* Can be used in a later release
    switch (type) {
      default:
        return 0;
    }
    */
  }
  else {
    if (!SetLSAPuffer (sc_no)) return 0;

    switch (type) {      
      case SC_DLL_DATA_TERMINATION:
        if (FehlerStatus == Okay) {       
          if (strlen(errorfile_name) != 0) {
            *long_value = TERM_ERRORFILE_CREATED;
            strcpy(string_value, errorfile_name);
          }
          else {
            *long_value = TERM_SUCCESSFUL;
          }
        }
        else {
          *long_value = TERM_ABORT;
        }
        return 1;

      case SC_DLL_DATA_SG_TRANSITION :
        sg_i = sg_nr_to_index (index1, proc_name, false);
        if (sg_i < 0) {
          return 0;
        }
        else {
          *long_value = LSA_Puffer->SigGr[sg_i]->Uebergang;
          return 1;
        }
      case SC_DLL_DATA_NO_OF_RECORD_DATA :
        *long_value = LSA_Puffer->AnzProtTripel;
        return 1;
      case SC_DLL_DATA_RECORD_DATA_TYPE :
        ppt = ppt_by_index (sc_no, index1);
        if (ppt == NULL) {
          return 0;
        }
        *long_value = ppt->Code;
        return 1;
      case SC_DLL_DATA_RECORD_DATA_NUMBER :
        ppt = ppt_by_index (sc_no, index1);
        if (ppt == NULL) {
          return 0;
        }
        *long_value = ppt->Nummer;
        return 1;
      case SC_DLL_DATA_RECORD_DATA_VALUE :
        ppt = ppt_by_index (sc_no, index1);
        if (ppt == NULL) {
          return 0;
        }
        *long_value = ppt->Wert;
        return 1;
      case SCHALTFREQUENZ :
        *long_value = LSA_Puffer->Schaltfrequenz;
        return 1;
      case UMLAUFSEKUNDE :
        *long_value = (long) (LSA_Puffer->Sekunde * LSA_Puffer->Schaltfrequenz + 0.001);
        return 1;
      case PROGRAMMNUMMER :
        *long_value = LSA_Puffer->ProgNr;
        return 1;
      case SOLL_BILD :
        sg_i = sg_nr_to_index (index1, proc_name, false);
        if (sg_i < 0) {
          return 0;
        }
        else {
          *long_value = puffer_bild (LSA_Puffer->SigGr[sg_i]->Soll);
          return 1;
        }
      case IMPULSSPEICHER :
        det_i = det_nr_to_index (index1, proc_name, false);
        if (det_i < 0) {
          return 0;
        }
        else {
          *long_value = LSA_Puffer->Det[det_i]->ImpulsspeicherLoeschen;
          return 1;
        }
      case LSAKOPPLUNG :
      {
        for (EingangListe al = LSA_Puffer->AlleAusgaenge; al != NULL; al = al->next) {
          if (al->inhalt->nummer == index1) {
            *long_value = al->inhalt->wert;
            return 1;
          }
        }
        *long_value = 0;
        return 1;
      }
      default :
        return 0;
    }
  }

  return 0;
} /* ControllerGetValue  */

/*==========================================================================*/

SC_DLL_API  int  ControllerExecuteCommand (long number, long sc_no)
{
  /* Called from VISSIM to execute the command <number> if that is */
  /* available in the DLL (see definitions above) for controller   */
  /* no. <sc_no>.                                                  */
  /* Return value is 1 on success, otherwise 0.                    */
  
  char  filename [300];

  switch (number) {
    case SC_DLL_COMMAND_CREATE_CONTROLLER :
      InitLSAPuffer (sc_no);
      if (!SetLSAPuffer (sc_no)) return 0;
      InitDataFromSIM ();
      InitDataToSIM ();
      return 1;
    case SC_DLL_COMMAND_READ_DATA_FILES :
      if (!SetLSAPuffer (sc_no)) return 0;
      assert (LSA_Puffer->DataFileName[1][0] != '\0');
      SteuerungDateiLesen ();
      if (FehlerStatus != Okay) return 0;
      return 1;
    case SC_DLL_COMMAND_INIT_CONTROLLER :
      if (!SetLSAPuffer (sc_no)) return 0;
      SteuerungInitialisierung ();
      return 1;
    case SC_DLL_COMMAND_RUN_CONTROLLER :
#ifdef _DEMO_
      if (!demo_version) {
        HoleRessourcenString (IDS_LSA_DEMO, res_string, sizeof (res_string));
        AllgemeinerFehler ("ControllerExecuteCommand", res_string);
        return 0;
      }
#endif
      if (!SetLSAPuffer (sc_no)) return 0;
      InitDataToSIM ();
      SteuerungBerechnung ();
      if (FehlerStatus != Okay) return 0;
      InitDataFromSIM ();
      return 1;
    case SC_DLL_COMMAND_EXIT_CONTROLLER :
      if (!SetLSAPuffer (sc_no)) return 0;
      Aufraeumen ();
      ExitLSAPuffer (sc_no);
      if (FehlerStatus != Okay) return 0;
      return 1;
    case SC_DLL_COMMAND_WRITE_SNAPSHOT :
      if (!SetLSAPuffer (sc_no)) return 0;
      sprintf (filename, "%s%lu", SnapshotFilename, sc_no);
      ZustandSchreiben (filename);
      if (FehlerStatus != Okay) return 0;
      return 1;
    case SC_DLL_COMMAND_READ_SNAPSHOT :
      if (!SetLSAPuffer (sc_no)) return 0;
      sprintf (filename, "%s%lu", SnapshotFilename, sc_no);
      ZustandLesen (filename);
      if (FehlerStatus != Okay) return 0;
      return 1;
    default :
      return 0;
  }
} /* ControllerExecuteCommand  */

/*==========================================================================*/
/*  End of SC_Interface.cpp                                                 */
/*==========================================================================*/
