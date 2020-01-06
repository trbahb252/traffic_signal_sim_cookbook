/*==========================================================================*/
/*  LSA_FEHL.CPP                                     Teil von ????????/SIM  */
/*                                                                          */
/*  Dieses Modul nimmt von allen anderen Modulen Fehlermeldungen entgegen   */
/*  und sorgt fuer ihre Ausgabe.                                            */
/*                                                                          */
/*  Version von 2004-05-11                                  Lukas Kautzsch  */
/*==========================================================================*/

#include "lsapuffr.h"
#include "lsa_rahm.h"  /* Gemeinsamer Header fuer XYZ_RAHM.C */
#include "lsa_fehl.h"
#include "lsa_puff.h"
#include "lsa_rahm.rh"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>

#include <algorithm>
#include <vector>

#pragma unmanaged

/*--------------------------------------------------------------------------*/

FehlerStatusTyp  FehlerStatus = Okay;
char             res_string [255];
char             errorfile_name [255] = "";

static  bool  console_mode = FALSE;
static  int   process_id   = NO_PROCESS_ID;
static  bool is_canceled   = FALSE;

static  BOOLEAN  fehler_aufgetreten = FALSE;
static  FILE     *fehler_datei;

class sc_el {
  public:
  unsigned long  sc; /* SC No.        */
  unsigned long  el; /* SG or Det No. */
  bool operator== (const sc_el& a) {return el == a.el && sc == a.sc;}
};
typedef  std::vector<sc_el>  sc_el_vector;
sc_el_vector  sg_fehler;
sc_el_vector  det_fehler;

/*--------------------------------------------------------------------------*/

char  *HoleRessourcenString (int str_id, char *buffer, int buffer_size)
{
  /* Holt aus den Ressourcen des Programms den String mit der Nummer */
  /* <str_id> und schreibt ihn in den Puffer <*buffer> der Laenge    */
  /* <buffer_size>. Zurueckgegeben wird im Fehlerfall NULL, sonst    */
  /* <buffer>.                                                       */
  if (LoadString ((HINSTANCE)CONTROLLER_HMODULE, str_id, buffer, buffer_size)) {
    return buffer;
  }
  else {
    return NULL;
  }
} /* HoleRessourcenString */

/*--------------------------------------------------------------------------*/

static  char  *lsa_string (int nr)
{
  /* Gibt einen Zeiger auf den String "lsaxy" zurueck, wobei fuer "xy"   */
  /* <nr> eingesetzt wird, ggf. mit fuehrender Null. Ist <nr> groesser   */
  /* als <MAXINSTANZ> - 1 oder kleiner als 0, erfolgt ein Fehlerabbruch. */

  static  char  t_string[255], nr_string [5];

  HoleRessourcenString (IDS_ERR_FILE_PREFIX, t_string, sizeof (t_string) - 10);
  sprintf (nr_string, "%d", nr);
  strcat (t_string, nr_string);
  return t_string;
} /* lsa_string */

/*--------------------------------------------------------------------------*/

static  void  fehlerabbruch (char *string)
{
  /* Gibt die Meldung <string> aus und setzt den Fehlerstatus auf Abbruch. */

  FehlerStatus = Abbruch;

  if (!console_mode) {
    BildschirmMeldung (string);
  }
} /* fehlerabbruch */

/*--------------------------------------------------------------------------*/

void  DateiMeldung (char *string)
{
  /* Schreibt die Meldung <string> in die Fehlerdatei (LSAxy.ERR) */
  /* und oeffnet letztere dafuer gegebenenfalls.                  */

  char  message [1000], message0 [255];

  if (!fehler_aufgetreten) {
    fehler_aufgetreten = TRUE;
    if (strlen(errorfile_name) == 0) {
      strcpy (errorfile_name, CONTROLLER_DLL_NAME);
      strcat (errorfile_name, ".err");

      if (process_id != NO_PROCESS_ID) {
        sprintf(errorfile_name, "%s_%d", errorfile_name, process_id);
      }      
    } 
    fehler_datei = fopen (errorfile_name, "w");
    if (fehler_datei == NULL) {
      fehler_aufgetreten = FALSE;
      HoleRessourcenString (IDS_OPEN_ERR_FILE, res_string, sizeof (res_string));
      sprintf (message, res_string, errorfile_name, string);
      fehlerabbruch (message);
      return;
    }
    HoleRessourcenString (IDS_LSA, res_string, sizeof (res_string));
    sprintf (message0, "%s %lu\n", res_string, LSA_Puffer->Number);
    if (fprintf (fehler_datei, message0) <= 0) {
      HoleRessourcenString (IDS_WRITE_ERR_FILE, res_string, sizeof (res_string));
      sprintf (message, res_string, errorfile_name, message0);
      fehlerabbruch (message);
      return;
    }
  }
  if (fprintf (fehler_datei, "%s", string) <= 0) {
    HoleRessourcenString (IDS_WRITE_ERR_FILE, res_string, sizeof (res_string));
    sprintf (message, res_string, errorfile_name, string);
    fehlerabbruch (message);
    return;
  }
} /* DateiMeldung */

/*--------------------------------------------------------------------------*/

void  BildschirmMeldung (char *string)
{
  /* Gibt die Meldung <string> aus: Unter Windows in einer Messagebox, */
  /* unter UNIX nach stderr.                                           */

  if (console_mode) {
    DateiMeldung(string);
  }
  else {
    MessageBox (NULL, (LPSTR)string, (LPSTR)CONTROLLER_DLL_NAME, 
                MB_ICONSTOP | MB_OK | MB_TOPMOST);
  }
} /* BildschirmMeldung */

/*--------------------------------------------------------------------------*/

void  AllgemeinerFehler (char *absender, char *grund)
{
  /* Bildschirmmeldung "Allgemeiner Fehler" mit "Funktion: <absender>" */
  /* und "Grund: <grund>"; <FehlerStatus> wird auf Abbruch gesetzt.    */

  char  err_txt [200], fe_text [30], fn_text [20], gr_text [20];

  HoleRessourcenString (IDS_ALLGEMEINER_FEHLER, fe_text,    sizeof (fe_text));
  HoleRessourcenString (IDS_FUNKTION,           fn_text,    sizeof (fn_text));
  HoleRessourcenString (IDS_GRUND,              gr_text,    sizeof (gr_text));
  sprintf (err_txt, "%s!\n%s: %s\n%s:    %s\n", fe_text, fn_text, absender,
                                                gr_text, grund);
  fehlerabbruch (err_txt);
}  /* AllgemeinerFehler */

/*--------------------------------------------------------------------------*/

void  InternerFehler (const char *absender, const char *grund)
{
  /* Bildschirmmeldung "Interner Fehler" mit "Funktion: <absender>" */
  /* und "Grund: <grund>"; <FehlerStatus> wird auf Abbruch gesetzt. */

  char  err_txt [500], fe_text [30], fn_text [20], gr_text [20];

  HoleRessourcenString (IDS_INTERNER_FEHLER, fe_text,    sizeof (fe_text));
  HoleRessourcenString (IDS_FUNKTION,        fn_text,    sizeof (fn_text));
  HoleRessourcenString (IDS_GRUND,           gr_text,    sizeof (gr_text));
  sprintf (err_txt, "%s!\n%s: %s\n%s:    %s\n", fe_text, fn_text, absender,
                                                gr_text, grund);
  fehlerabbruch (err_txt);
}  /* InternerFehler */

/*--------------------------------------------------------------------------*/

void  Laufzeitfehler (int fehlernr, char *prozedurname, unsigned nr)
{
  /* Dateimeldung "Laufzeitfehler" mit <prozedurname> und Klartext fuer die */
  /* Fehlernummer <fehlernr> sowie ggf. einer Zusatzinformation (<nr>).     */

  char   err_txt1 [200], err_txt2 [100];
  sc_el  curr_sc_el;

  HoleRessourcenString (IDS_FUNKTION, res_string, sizeof (res_string));
  sprintf (err_txt1, "%s %s:\n", res_string, prozedurname);
  switch (fehlernr) {
    case SGPNR_UNBEKANNT   :
      curr_sc_el.sc = LSA_Puffer->Number;
      curr_sc_el.el = nr;
      if (find (sg_fehler.begin(), sg_fehler.end(), curr_sc_el) == sg_fehler.end()) {
        sg_fehler.push_back (curr_sc_el);
      }
      else {
        return;
      }
      HoleRessourcenString (IDS_SGPNR_UNBEKANNT, res_string, sizeof (res_string));
      sprintf (err_txt2, res_string, (unsigned long) nr);
      break;

    case DETNR_UNBEKANNT   :
      curr_sc_el.sc = LSA_Puffer->Number;
      curr_sc_el.el = nr;
      if (find (det_fehler.begin(), det_fehler.end(), curr_sc_el) == det_fehler.end()) {
        det_fehler.push_back (curr_sc_el);
      }
      else {
        return;
      }
      HoleRessourcenString (IDS_DETNR_UNBEKANNT, res_string, sizeof (res_string));
      sprintf (err_txt2, res_string, (unsigned long) nr);
      break;

    default                :
      HoleRessourcenString (IDS_UNBEKANNTE_FEHLERNR, res_string,
                            sizeof (res_string));
      InternerFehler ("LSA_FEHL.Laufzeitfehler", res_string);
      break;
  }
  strcat (err_txt1, err_txt2);
  strcat (err_txt1, "\n");
  DateiMeldung (err_txt1);
}  /* Laufzeitfehler */

/*--------------------------------------------------------------------------*/

void  ExitLSAFehler (void)
{
  /* Schliesst die Fehlerdatei des Steuerungsprogramms, falls sie geoeffnet */
  /* war, und gibt in diesem Fall auch eine Meldungsbox aus.                */

  if (fehler_aufgetreten) {
    fehler_aufgetreten = FALSE;
    sg_fehler.clear();
    det_fehler.clear();
    if (fehler_datei != NULL) {
      fclose (fehler_datei);
    }
  }
} /* ExitLSAFehler */

/*--------------------------------------------------------------------------*/

void SetConsoleMode (bool consoleMode)
{
  console_mode = consoleMode;
}

/*--------------------------------------------------------------------------*/

bool IstConsoleModus (void) 
{
  return console_mode;
}

/*--------------------------------------------------------------------------*/

bool IstSimulationAbgebrochen (void)
{
  return is_canceled;
}

/*--------------------------------------------------------------------------*/

void SetProcessId(int processId)
{
  process_id = processId;
}

/*--------------------------------------------------------------------------*/

int GetProcessId(void)
{
  return process_id;
}

/*--------------------------------------------------------------------------*/

void SetSimulationCanceled(bool isCanceled)
{
  is_canceled = isCanceled;
}

/*==========================================================================*/
/*  Ende von LSA_FEHL.CPP                                                   */
/*==========================================================================*/
