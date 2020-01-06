/*==========================================================================*/
/*  LSA_FEHL.H                                       Teil von ????????/SIM  */
/*                                                                          */
/*  Dieses Modul nimmt von allen anderen Modulen Fehlermeldungen entgegen   */
/*  und sorgt fuer ihre Ausgabe.                                            */
/*                                                                          */
/*  Version von 2007-05-23                                  Lukas Kautzsch  */
/*==========================================================================*/

#ifndef __LSA_FEHL_H
#define __LSA_FEHL_H

/*--------------------------------------------------------------------------*/

#define NO_PROCESS_ID 0

typedef  enum { Okay, Abbruch } FehlerStatusTyp;

extern  FehlerStatusTyp  FehlerStatus;
extern  char             errorfile_name [255];
extern  char             res_string [255];

/*--------------------------------------------------------------------------*/

char  *HoleRessourcenString (int str_id, char *buffer, int buffer_size);

/* Holt aus den Ressourcen des Programms den String mit der Nummer */
/* <str_id> und schreibt ihn in den Puffer <*buffer> der Laenge    */
/* <buffer_size>. Zurueckgegeben wird im Fehlerfall NULL, sonst    */
/* <buffer>.                                                       */

/*--------------------------------------------------------------------------*/

void  DateiMeldung (char *string);

/* Schreibt die Meldung <string> in die Fehlerdatei (LSAxy.ERR) */
/* und oeffnet letztere dafuer gegebenenfalls.                  */

/*--------------------------------------------------------------------------*/

void  BildschirmMeldung (char *string);

/* Gibt die Meldung <string> aus: Unter Windows in einer Messagebox, */
/* unter UNIX nach stderr.                                           */

/*--------------------------------------------------------------------------*/

void  AllgemeinerFehler (char *absender, char *grund);

/* Bildschirmmeldung "Allgemeiner Fehler" mit "Funktion: <absender>" */
/* und "Grund: <grund>"; <FehlerStatus> wird auf Abbruch gesetzt.    */

/*--------------------------------------------------------------------------*/

void  InternerFehler (const char *absender, const char *grund);

/* Bildschirmmeldung "Interner Fehler" mit "Funktion: <absender>" */
/* und "Grund: <grund>"; <FehlerStatus> wird auf Abbruch gesetzt. */

/*--------------------------------------------------------------------------*/

#define  SGPNR_UNZULAESSIG       1
#define  SGPNR_UNBEKANNT         2
#define  DETNR_UNZULAESSIG       3
#define  DETNR_UNBEKANNT         4

void  Laufzeitfehler (int fehlernr, char *prozedurname, unsigned nr);

/* Dateimeldung "Laufzeitfehler" mit <prozedurname> und Klartext fuer die */
/* Fehlernummer <fehlernr> sowie ggf. einer Zusatzinformation (<nr>).     */

/*--------------------------------------------------------------------------*/

void  ExitLSAFehler (void);

/* Schliesst die Fehlerdatei des Steuerungsprogramms, falls sie geoeffnet */
/* war, und gibt in diesem Fall auch eine Meldungsbox aus.                */

/*--------------------------------------------------------------------------*/

void SetConsoleMode (bool consoleMode);

/* Wenn <consoleMode> auf TRUE gesetzt wird, dann dürfen keine Meldungen */
/* auf dem Bildschirm ausgegeben werden, sondern nur in Datei.           */

/*--------------------------------------------------------------------------*/

bool IstConsoleModus (void);

/* Gibt true zurück, wenn VISSIM den Console-Modus gesetzt hat, sonst false. */

/*--------------------------------------------------------------------------*/

void SetProcessId (int processId);

/* Setzt die Prozess-ID auf den von VISSIM übergebenen Wert <processId>.    */
/* Die Prozess-ID wird von der Parallelversion und bei Mehrfachsimulation   */
/* gesetzt, und zwar auf  100 * Prozess-ID + Simulationslaufindex ([1..n]). */
/* Ansonsten ist sie immer NO_PROCESS_ID.                                   */

/*--------------------------------------------------------------------------*/

int GetProcessId (void);

/* Gibt den von VISSIM gesetzten Wert der Prozess-ID zurück.                */
/* Die Prozess-ID wird von der Parallelversion und bei Mehrfachsimulation   */
/* gesetzt, und zwar auf  100 * Prozess-ID + Simulationslaufindex ([1..n]). */
/* Ansonsten ist sie immer NO_PROCESS_ID.                                   */

/*--------------------------------------------------------------------------*/

#endif  /* __LSA_FEHL_H */

/*==========================================================================*/
/*  Ende von LSA_FEHL.H                                                     */
/*==========================================================================*/

