/*==========================================================================*/
/*  LSA_PUFF.CPP                                     Teil von ????????/SIM  */
/*                                                                          */
/*  Zugriffsfunktionen fuer den LSA-Puffer.                                 */
/*                                                                          */
/*  Version von 2007-05-23                                  Lukas Kautzsch  */
/*==========================================================================*/

#include "assert.h"
#include "lsa_fehl.h"
#include "lsa_rahm.h"   /* Gemeinsamer Header fuer XYZ_RAHM.C */
#include "lsapuffr.h"
#include "lsa_puff.h"
#include "lsa_rahm.rh"
#include "SC_Interface.h"

#ifdef _HPUX_SOURCE
#define  far           /* das kennt   HPUX nicht! */
#endif

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>

/*==========================================================================*/

P_LSA_Puffer    LSA_Puffer = NULL;
P_LSA_Puffer    Alle_LSA_Puffer = NULL;
double          SimulationTime;          /* [s] */
char            SimulationTimeOfDay [11] = "hh:mm:ss.s";
long            SimulationDate = 0;      /* YYYYMMDD */
char            SnapshotFilename [255];  /* VISSIM *.snp */

/*==========================================================================*/

static  char            kein_speicher [50];
static  TelegrammListe  AlleOEVTele = NULL;

/*==========================================================================*/

static  unsigned  long_to_unsigned (long wert)
{
  /* Gibt den Wert <wert> als unsigned int zurueck, wobei negative Werte */
  /* auf 0 abgebildet werden und Werte groesser als der groesste         */
  /* darstellbare Wert auf ebendiesen.                                   */

  if (wert < 0) {
    return 0;
  }
  else if (wert > UINT_MAX) {
    return UINT_MAX;
  }
  else {
    return (unsigned) wert;
  }
} /* long_to_unsigned */

/*--------------------------------------------------------------------------*/

static  unsigned short  long_to_unsigned_short (long wert)
{
  /* Gibt den Wert <wert> als unsigned short zurueck, wobei negative Werte */
  /* auf 0 abgebildet werden und Werte groesser als der groesste           */
  /* darstellbare Wert auf ebendiesen.                                     */

  if (wert < 0) {
    return 0;
  }
  else if (wert > USHRT_MAX) {
    return USHRT_MAX;
  }
  else {
    return (unsigned short) wert;
  }
} /* long_to_unsigned_short */

/*--------------------------------------------------------------------------*/

BOOLEAN  InitLSAPuffer (long sc_no)
{
  /* Belegt den Speicher fuer den LSA_Puffer der LSA Nr. <sc_no>. */
  /* Bei Erfolg wird TRUE zurueckgegeben, sonst FALSE und ein     */
  /* Laufzeitfehler erzeugt.                                      */

  int           i;
  P_LSA_Puffer  *pplp, plp;

  HoleRessourcenString (IDS_KEIN_SPEICHER, kein_speicher,
                        sizeof (kein_speicher));

  /* Grundgeruest des LSA_Puffers anlegen: */
//  plp = new T_LSA_Puffer ();
    plp = new T_LSA_Puffer();
    if (plp == NULL) {
      AllgemeinerFehler ("LSA_PUFF.InitLSAPuffer", kein_speicher);
      return FALSE;
    }
    plp->Number = sc_no;

  /* An Liste der LSA_Puffer anhängen: */
    pplp = &Alle_LSA_Puffer;
    while (*pplp != NULL) {
      pplp = &(*pplp)->next;
    }
    *pplp = plp;
    plp->next = NULL;

  /* Speicherplatz fuer Detektoren belegen: */
    for (i = 0; i <= MAXDET; i++) {
      plp->Det[i] = new T_Det();
      if (plp->Det[i] == NULL) {
        AllgemeinerFehler ("LSA_PUFF.InitLSAPuffer", kein_speicher);
        return FALSE;
      }
      plp->Det[i]->Type              = TrafficDetector;
      plp->Det[i]->VISSIM_Nr         = 0;
      plp->Det[i]->VorderkantenSumme = 0;
      plp->Det[i]->HinterkantenSumme = 0;
    }
    plp->num_Det = 0;
    plp->Det_No_to_Index = new NumberToIndexMap ();
    plp->Det_Name_to_Index = new NameToIndexMap ();

  /* Speicherplatz fuer Signalgruppen belegen: */
    for (i = 0; i <= MAXSIGGR; i++) {
      plp->SigGr[i] = new T_SigGr();
      if (plp->SigGr[i] == NULL) {
        AllgemeinerFehler ("LSA_PUFF.InitLSAPuffer", kein_speicher);
        return FALSE;
      }
      plp->SigGr[i]->Bild      = undefiniert;
      plp->SigGr[i]->Soll      = undefiniert;
      plp->SigGr[i]->AltSoll   = undefiniert;
      plp->SigGr[i]->VISSIM_Nr = 0;
    }
    plp->num_SG = 0;
    plp->SG_No_to_Index = new NumberToIndexMap ();

  /* Anfangswerte setzen (Liste der Protokoll-Tripel ist leer!): */
    plp->Schaltfrequenz   = 1;
    plp->ProtTripel       = NULL;
    plp->AnzProtTripel    = 0;
    plp->pNaechstesTripel = &plp->ProtTripel;
    plp->AlleEingaenge    = NULL;
    plp->AlleAusgaenge    = NULL;
    plp->ProgNr           = 1;     /* Default, falls von VISSIM nichts kommt */
    plp->DebugModus       = FALSE; /* Default, falls von VISSIM nichts kommt */

    AlleOEVTele = NULL;

  return TRUE;
} /* InitLSAPuffer */

/*--------------------------------------------------------------------------*/

bool  SetLSAPuffer (long sc_no)
{
  /* Sets the global variable <LSA_Puffer> to the list element of */
  /* SC No. <sc_no>. Returns true on success, else false (after a */
  /* runtime error message).                                      */

  static  long  last_no = 0;
          char  message[255];

  if (sc_no == last_no) {
    return true;
  }
  for (P_LSA_Puffer plp = Alle_LSA_Puffer; plp != NULL; plp = plp->next) {
    if (plp->Number == sc_no) {
      LSA_Puffer = plp;
      return true;
    }
  }
  sprintf (message, "SC No. %ld not found!", sc_no);
  AllgemeinerFehler ("LSA_PUFF.SetLSAPuffer", message);
  return false;
} /* SetLSAPuffer */

/*--------------------------------------------------------------------------*/

static  void  loesche_telegramm_liste (TelegrammListe *liste)
{
  /* Gibt den durch die uebergebene Telegrammliste <*liste> belegten */
  /* Speicher wieder frei und setzt <*liste> auf NULL.               */

  TelegrammListe  tl = *liste, tl2;

  while (tl != NULL) {
    tl2 = tl->next;
    free ((char *) tl->inhalt);
    free ((char *) tl);
    tl = tl2;
  }
  *liste = NULL;
} /* loesche_telegramm_liste */

/*--------------------------------------------------------------------------*/

static  void  loesche_eingang_liste (EingangListe *liste)
{
  /* Gibt den durch die uebergebene Eingangliste <*liste> belegten */
  /* Speicher wieder frei und setzt <*liste> auf NULL.             */

  EingangListe  tl = *liste, tl2;

  while (tl != NULL) {
    tl2 = tl->next;
    free ((char *) tl->inhalt);
    free ((char *) tl);
    tl = tl2;
  }
  *liste = NULL;
} /* loesche_eingang_liste */

/*--------------------------------------------------------------------------*/

void  InitDataFromSIM (void)
{
  /* Initializes dynamic data sent from VISSIM for the next */
  /* controller time step.                                  */

  unsigned  i, j;

  for (i = 0; i <= MAXDET; i++) {
    for (j = 0; j < MAX_KANTEN_PRO_SI; j++) {
      LSA_Puffer->Det[i]->tVorderkante[j] = -1;
      LSA_Puffer->Det[i]->tHinterkante[j] = -1;
    }
  }
} /* InitDataFromSIM */

/*--------------------------------------------------------------------------*/

void  InitDataToSIM (void)
{
  /* Initializes dynamic data sent to VISSIM for the next */
  /* controller time step.                                */

  unsigned  i;

  for (i = 0; i <= MAXDET; i++) {
    LSA_Puffer->Det[i]->ImpulsspeicherLoeschen = FALSE;
  }
  for (i = 0; i <= MAXSIGGR; i++) {
    if (LSA_Puffer->SigGr[i]->Soll != undefiniert) {
      LSA_Puffer->SigGr[i]->AltSoll = LSA_Puffer->SigGr[i]->Soll;
    }
    LSA_Puffer->SigGr[i]->Soll = undefiniert;
  }
  LSA_Puffer->AnzProtTripel = 0;
  LSA_Puffer->pNaechstesTripel = &LSA_Puffer->ProtTripel;
} /* InitDataToSIM */

/*--------------------------------------------------------------------------*/

void  ExitLSAPuffer (long sc_no)
{
  /* Gibt den Speicher fuer den LSA_Puffer der LSA Nr. <sc_no> wieder frei. */

  int           i;
  P_ProtTripel  p, p2;
  P_LSA_Puffer  *pplp, n_plp;

  for (pplp = &Alle_LSA_Puffer; *pplp != NULL; pplp = &(*pplp)->next) {
    if ((*pplp)->Number == sc_no) {
      for (i = 0; i <= MAXDET; i++) {
        assert ((*pplp)->Det[i] != NULL);
        delete (*pplp)->Det[i];
        (*pplp)->Det[i] = NULL;
      }
      for (i = 0; i <= MAXSIGGR; i++) {
        assert ((*pplp)->SigGr[i] != NULL);
        delete (*pplp)->SigGr[i];
        (*pplp)->SigGr[i] = NULL;
      }
      p = (*pplp)->ProtTripel;
      while (p != NULL) {
        p2 = p->next;
        free (p);
        p = p2;
      }
      (*pplp)->pNaechstesTripel = NULL;
      loesche_telegramm_liste (&AlleOEVTele);
      loesche_eingang_liste (&(*pplp)->AlleEingaenge);
      loesche_eingang_liste (&(*pplp)->AlleAusgaenge);
      (*pplp)->Det_No_to_Index->clear();
      delete (*pplp)->Det_No_to_Index;
      delete (*pplp)->Det_Name_to_Index;
      (*pplp)->SG_No_to_Index->clear();
      delete (*pplp)->SG_No_to_Index;
      n_plp = (*pplp)->next;
      delete *pplp;
      //free (*pplp);
      *pplp = n_plp;
      break;
    }
  }
} /* ExitLSAPuffer */

/*==========================================================================*/

int  det_nr_to_index (long vissim_nr, char *function_name, bool create)
{
  /* Gibt den internen Detektorindex zur VISSIM-Detektornummer <vissim_nr> */
  /* in der aktuellen LSA (gesetzt durch SetLSAPuffer()) zurueck.          */
  /* Falls kein Detektor mit dieser VISSIM-Nummer definiert ist und        */
  /* <create> true ist, wird er neu angelegt und sein interner Index       */
  /* zurückgegeben. (Falls nicht genug Speicher vorhanden ist, wird eine   */
  /* Fehlermeldung angezeigt und -1 zurückgegeben.)                        */
  /* Falls kein Detektor mit dieser VISSIM-Nummer definiert ist und        */
  /* <create> false ist, wird -1 zurückgegeben. Außerdem wird, falls       */
  /* <function_name> nicht NULL ist, eine Fehlermeldung inclusive          */
  /* <function_name> auf dem Bildschirm angezeigt.                         */

  int   result;
  char  err_txt [200];
  NumberToIndexMap::iterator  pos = LSA_Puffer->Det_No_to_Index->find(vissim_nr);

  if (pos == LSA_Puffer->Det_No_to_Index->end()) {
    if (create) {
      if (LSA_Puffer->Det_No_to_Index->size() < MAXDET) {
        result = LSA_Puffer->Det_No_to_Index->insert (std::make_pair(vissim_nr, LSA_Puffer->num_Det++)).first->second;
        LSA_Puffer->Det[result]->VISSIM_Nr = vissim_nr;
      }
      else {
        HoleRessourcenString (IDS_TOO_MANY_DET, res_string, sizeof (res_string));
        sprintf (err_txt, res_string, MAXDET);
        AllgemeinerFehler (function_name, err_txt);
        return -1;
      }
    }
    else {
      if (function_name != NULL) {
        Laufzeitfehler (DETNR_UNBEKANNT, function_name, vissim_nr);
      }
      return -1;
    }
  }
  else {
    result = pos->second;
  }
  assert (result >= 0 && result <= MAXDET);
  return result;
} /* det_nr_to_index */

int det_name_to_index (const std::string & vissim_name, char *function_name)
{
  /* Gibt den internen Index des Detektors mit dem gegebenen Namen zurück */
  /* falls er existiert, sonst -1.                                        */
  int   result;
  NameToIndexMap::iterator iter = LSA_Puffer->Det_Name_to_Index->find(vissim_name);

  if (iter == LSA_Puffer->Det_Name_to_Index->end()) {
    if (function_name != NULL) {
      std::stringstream reason;
      reason << "No signal control with name \"" << vissim_name << "\" found.";
      InternerFehler (function_name, reason.str().c_str());
    }
    return -1;
  }
  else {
    result = iter->second;
  }
  assert (result >= 0 && result <= MAXDET);
  return result;
} /* det_nr_to_index */

/*--------------------------------------------------------------------------*/

int  sg_nr_to_index (long vissim_nr, char *function_name, bool create)
{
  /* Gibt den internen Signalgruppenindex zur VISSIM-Signalgruppennummer   */
  /* <vissim_nr> in der aktuellen LSA (gesetzt durch SetLSAPuffer())       */
  /* zurueck.                                                              */
  /* Falls keine Signalgruppe mit dieser VISSIM-Nummer definiert ist und   */
  /* <create> true ist, wird sie neu angelegt und ihr interner Index       */
  /* zurückgegeben. (Falls nicht genug Speicher vorhanden ist, wird eine   */
  /* Fehlermeldung angezeigt und -1 zurückgegeben.)                        */
  /* Falls keine Signalgruppe mit dieser VISSIM-Nummer definiert ist und   */
  /* <create> false ist, wird -1 zurückgegeben. Außerdem wird, falls       */
  /* <function_name> nicht NULL ist, eine Fehlermeldung inclusive          */
  /* <function_name> auf dem Bildschirm angezeigt.                         */

  int   result;
  char  err_txt [200];
  NumberToIndexMap::iterator  pos = LSA_Puffer->SG_No_to_Index->find(vissim_nr);

  if (pos == LSA_Puffer->SG_No_to_Index->end()) {
    if (create) {
      if (LSA_Puffer->SG_No_to_Index->size() < MAXSIGGR) {
        result = LSA_Puffer->SG_No_to_Index->insert (std::make_pair(vissim_nr, LSA_Puffer->num_SG++)).first->second;
        LSA_Puffer->SigGr[result]->VISSIM_Nr = vissim_nr;
      }
      else {
        HoleRessourcenString (IDS_TOO_MANY_SG, res_string, sizeof (res_string));
        sprintf (err_txt, res_string, MAXSIGGR);
        AllgemeinerFehler (function_name, err_txt);
        return -1;
      }
    }
    else {
      if (function_name != NULL) {
        Laufzeitfehler (SGPNR_UNBEKANNT, function_name, vissim_nr);
      }
      return -1;
    }
  }
  else {
    result = pos->second;
  }
  assert (result >= 0 && result <= MAXSIGGR);
  return result;
} /* sg_nr_to_index */

/*--------------------------------------------------------------------------*/

long  puffer_bild (SignalBild bild)
{
  /* Gibt den Signalbild-Code des LSA-Puffers fuer das Signalbild <bild> */
  /* zurueck.                                                            */

  switch (bild) {
    case rot :
      return BILD_ROT;
    case rotgelb :
      return BILD_ROTGELB;
    case gruen :
      return BILD_GRUEN;
    case gelb :
      return BILD_GELB;
    case aus :
      return BILD_AUS;
    case undefiniert :
      return BILD_UNDEFINIERT;
    case gelb_blink :
      return BILD_GELB_BLINK;
    case rot_blink :
      return BILD_ROT_BLINK;
    case gruen_blink :
      return BILD_GRUEN_BLINK;
    case rgwb :
      return BILD_RGWB;
    case gruengelb :
      return BILD_GRUENGELB;
    default :
      HoleRessourcenString (IDS_UNBEKANNTES_SIGNALBILD, res_string,
        sizeof (res_string));
      InternerFehler ("LSA_PUFF.puffer_bild", res_string);
      return -1L;
  }
} /* puffer_bild */

/*==========================================================================*/

static  SignalBild  lsa_bild (long bild)
{
  /* Gibt das Signalbild zurueck, das dem Signalbildcode <bild> */
  /* des LSAPuffers entspricht.                                 */

  switch (bild) {
    case BILD_AUS     :
      return aus;
    case BILD_AN      :
      return gruen;
    case BILD_ROT     :
      return rot;
    case BILD_ROTGELB :
      return rotgelb;
    case BILD_GELB    :
      return gelb;
    case BILD_GRUEN   :
      return gruen;
    case BILD_GELB_BLINK :
      return gelb_blink;
    case BILD_ROT_BLINK :
      return rot_blink;
    case BILD_GRUEN_BLINK :
      return gruen_blink;
    case BILD_RGWB :
      return rgwb;
    case BILD_GRUENGELB :
      return gruengelb;
    default      :
      HoleRessourcenString (IDS_UNBEKANNTES_SIGNALBILD, res_string,
                            sizeof (res_string));
      InternerFehler ("LSA_PUFF.lsa_bild", res_string);
      return aus;
  }
} /* lsa_bild */

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static  void  neues_oev_telegramm (long nummer)
{
  /* Legt im <LSA_Puffer> ein neues, leeres OEV-Telegramm mit der */
  /* Meldepunktsnummer <nr> an.                                   */

  char            prozedurname[] = "LSA_PUFF.neues_oev_telegramm";
  TelegrammListe  tl;

  tl = (TelegrammListe) malloc (sizeof (*tl));
  if (tl == NULL) {
    AllgemeinerFehler (prozedurname, kein_speicher);
    return;
  }
  tl->inhalt = (OEV_Telegramm) malloc (sizeof (*(tl->inhalt)));
  if (tl->inhalt == NULL) {
    AllgemeinerFehler (prozedurname, kein_speicher);
    return;
  }
  tl->next = AlleOEVTele;
  tl->inhalt->mp_nr             = nummer;
  tl->inhalt->linien_nr         = 0;
  tl->inhalt->kurs_nr           = 0;
  tl->inhalt->routen_nr         = 0;
  tl->inhalt->prioritaet        = 0;
  tl->inhalt->zuglaenge         = 0;
  tl->inhalt->richtung_von_hand = 0;
  tl->inhalt->fahrpl_sek        = 0;
  tl->inhalt->personen          = 0;
  AlleOEVTele = tl;
} /* neues_oev_telegramm */

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static  BOOLEAN  mp_nummer_stimmt (long nummer)
{
  /* Gibt TRUE zurueck, falls das letzte begonnene OEV-Telegramm */
  /* vom Meldepunkt Nummer <nummer> stammt, andernfalls FALSE.   */

  return (BOOLEAN) (   AlleOEVTele                != NULL
                    && AlleOEVTele->inhalt        != NULL
                    && AlleOEVTele->inhalt->mp_nr == nummer);
} /* mp_nummer_stimmt */

int StringToLSAPuffer (long typ, long nummer, std::string& wert)
{
  unsigned index;

  switch (typ) {
    case SC_DLL_DATA_DETECTOR_NAME:
      index = det_nr_to_index (nummer, NULL, true);
      if (index >= 0) {
        LSA_Puffer->Det[index]->Name = wert;

        if (!wert.empty() && 
          LSA_Puffer->Det_Name_to_Index->find(wert) != LSA_Puffer->Det_Name_to_Index->end()) {
          LSA_Puffer->Det_Name_to_Index->insert(std::make_pair(wert, index));
        }
      }
      break;
    default:
      return 0;
  }

  return 1;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int  DoubleToLSAPuffer (long typ, long nummer, double wert)
{
  /* Writes the value <wert> of type <typ>, index <nummer> to <LSA_Puffer>. */

  unsigned  index;

  switch (typ) {
    case SC_DLL_DATA_CYCLE_TIME :
      LSA_Puffer->TU = wert;
      break;

    case SC_DLL_DATA_CYCLE_SECOND :
      LSA_Puffer->Sekunde = wert;
      break;

    case SC_DLL_DATA_SIM_TIME :
      if (SimulationTime != wert) {
        loesche_telegramm_liste (&AlleOEVTele);
      }
      SimulationTime = wert;
      break;

    case SC_DLL_DATA_SG_T_GREEN_MIN :
      index = sg_nr_to_index (nummer, NULL, true);
      if (index >= 0) {
        LSA_Puffer->SigGr[index]->TGruenMin = wert;
      }
      break;

    case SC_DLL_DATA_SG_T_RED_MIN :
      index = sg_nr_to_index (nummer, NULL, true);
      if (index >= 0) {
        LSA_Puffer->SigGr[index]->TRotMin   = wert;
      }
      break;

    case SC_DLL_DATA_SG_T_RED_AMBER :
      index = sg_nr_to_index (nummer, NULL, true);
      if (index >= 0) {
        LSA_Puffer->SigGr[index]->TRotgelb  = wert;
      }
      break;

    case SC_DLL_DATA_SG_T_AMBER :
      index = sg_nr_to_index (nummer, NULL, true);
      if (index >= 0) {
        LSA_Puffer->SigGr[index]->TGelb     = wert;
      }
      break;

    case SC_DLL_DATA_SG_CURR_DURATION :
      index = sg_nr_to_index (nummer, NULL, true);
      if (index >= 0) {
        LSA_Puffer->SigGr[index]->Laufzeit  = wert;
      }
      break;

    default                       :
      /* unerwarteter oder unbekannter Typ => ignorieren! */
      return 0;
  } /* switch (typ) */

  return 1;
} /* DoubleToLSAPuffer */

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int  LongToLSAPuffer (long typ, long nummer, long wert)
{
  /* Writes the value <wert> of type <typ>, index <nummer> to <LSA_Puffer>. */

  static  char  prozedurname [] = "LSA_PUFF.LongToLSAPuffer";
  unsigned      i, index;
#ifdef _DEMO_
  BOOLEAN       demo_version = FALSE;
#endif
  EingangListe  el;

  switch (typ) {

    case DATEN_ENDE               :
      /* ende = TRUE; */
      break;

    case UMLAUFZEIT               :
      LSA_Puffer->TU = wert;
      break;

    case UMLAUFSEKUNDE            :
      LSA_Puffer->Sekunde = wert / LSA_Puffer->Schaltfrequenz;
      break;

    case PROGRAMMNUMMER           :
      LSA_Puffer->ProgNr  = (unsigned) wert;
      break;

    case DEBUGMODUS               :
      LSA_Puffer->DebugModus = (BOOLEAN) (wert != 0);
      break;

    case IST_BILD                 :
      index = sg_nr_to_index (nummer, NULL, true);
      if (index >= 0) {
        LSA_Puffer->SigGr[index]->Bild      = lsa_bild (wert);
      }
      break;

    case T_GRUEN_MIN              :
      index = sg_nr_to_index (nummer, NULL, true);
      if (index >= 0) {
        LSA_Puffer->SigGr[index]->TGruenMin = wert;
      }
      break;

    case T_ROT_MIN                :
      index = sg_nr_to_index (nummer, NULL, true);
      if (index >= 0) {
        LSA_Puffer->SigGr[index]->TRotMin   = wert;
      }
      break;

    case T_ROTGELB                :
      index = sg_nr_to_index (nummer, NULL, true);
      if (index >= 0) {
        LSA_Puffer->SigGr[index]->TRotgelb  = wert;
      }
      break;

    case T_GELB                   :
      index = sg_nr_to_index (nummer, NULL, true);
      if (index >= 0) {
        LSA_Puffer->SigGr[index]->TGelb     = wert;
      }
      break;

    case LAUFZEIT                 :
      index = sg_nr_to_index (nummer, NULL, true);
      if (index >= 0) {
        LSA_Puffer->SigGr[index]->Laufzeit  = wert;
      }
      break;

    case ANWESENHEIT              :
      index = det_nr_to_index (nummer, NULL, true);
      if (index >= 0) {
        LSA_Puffer->Det[index]->Anwesenheit = (BOOLEAN) (wert != 0);
      }
      break;

    case IMPULSSPEICHER           :
      index = det_nr_to_index (nummer, NULL, true);
      if (index >= 0) {
        LSA_Puffer->Det[index]->Impulsspeicher = (BOOLEAN) (wert != 0);
      }
      break;

    case DET_TYPE                 :
      index = det_nr_to_index (nummer, NULL, true);
      if (index >= 0) {
        LSA_Puffer->Det[index]->Type = static_cast<DetectorType>(wert);
      }
      break;
    case VORDERKANTEN             :
      index = det_nr_to_index (nummer, NULL, true);
      if (index >= 0) {
        LSA_Puffer->Det[index]->Vorderkanten =
          (unsigned short) wert;
        LSA_Puffer->Det[index]->VorderkantenSumme +=
          (unsigned short) wert;
      }
      break;

    case HINTERKANTEN             :
      index = det_nr_to_index (nummer, NULL, true);
      if (index >= 0) {
        LSA_Puffer->Det[index]->Hinterkanten =
          (unsigned short) wert;
        LSA_Puffer->Det[index]->HinterkantenSumme +=
          (unsigned short) wert;
      }
      break;

    case T_VORDERKANTE            :
      index = det_nr_to_index (nummer, NULL, true);
      if (index >= 0) {
        i = 0;
        while (   i < MAX_KANTEN_PRO_SI
                && LSA_Puffer->Det[index]->tVorderkante[i] != -1) {
          i++;
        }
        if (i < MAX_KANTEN_PRO_SI) {
          LSA_Puffer->Det[index]->tVorderkante[i] = (short) wert;
        }
      }
      break;

    case T_HINTERKANTE            :
      index = det_nr_to_index (nummer, NULL, true);
      if (index >= 0) {
        i = 0;
        while (   i < MAX_KANTEN_PRO_SI
                && LSA_Puffer->Det[index]->tHinterkante[i] != -1) {
          i++;
        }
        if (i < MAX_KANTEN_PRO_SI) {
          LSA_Puffer->Det[index]->tHinterkante[i] = (short) wert;
        }
      }
      break;

    case ANG_ZEITLUECKE           :
      index = det_nr_to_index (nummer, NULL, true);
      if (index >= 0) {
        LSA_Puffer->Det[index]->angefangeneZeitluecke = wert;
      }
      break;

    case KOMPL_ZEITLUECKE         :
      index = det_nr_to_index (nummer, NULL, true);
      if (index >= 0) {
        LSA_Puffer->Det[index]->abgeschlosseneZeitluecke =
          (unsigned short) wert;
      }
      break;

    case ANG_BELEGUNG             :
      index = det_nr_to_index (nummer, NULL, true);
      if (index >= 0) {
        LSA_Puffer->Det[index]->Belegungsdauer = wert;
      }
      break;

    case AKT_BELEGUNG             :
      index = det_nr_to_index (nummer, NULL, true);
      if (index >= 0) {
        LSA_Puffer->Det[index]->aktuelleBelegung =
          (unsigned short) wert;
      }
      break;

    case GEGL_BELEGUNG            :
      index = det_nr_to_index (nummer, NULL, true);
      if (index >= 0) {
        LSA_Puffer->Det[index]->geglaetteteBelegung =
          (unsigned short) wert;
      }
      break;

    case GESCHWINDIGKEIT          :
      index = det_nr_to_index (nummer, NULL, true);
      if (index >= 0) {
        LSA_Puffer->Det[index]->v         = wert;
      }
      break;

    case FZLAENGE                 :
      index = det_nr_to_index (nummer, NULL, true);
      if (index >= 0) {
        LSA_Puffer->Det[index]->l         = wert;
      }
      break;

    case OVT_MP                   :
      neues_oev_telegramm (nummer);
      break;

    case OVT_LINIE                :
      if (mp_nummer_stimmt (nummer)) {
        AlleOEVTele->inhalt->linien_nr = (unsigned) wert;
      }
      break;

    case OVT_KURS                 :
      if (mp_nummer_stimmt (nummer)) {
        AlleOEVTele->inhalt->kurs_nr = (unsigned) wert;
      }
      break;

    case OVT_ROUTE                :
      if (mp_nummer_stimmt (nummer)) {
        AlleOEVTele->inhalt->routen_nr = (unsigned) wert;
      }
      break;

    case OVT_PRIORITAET           :
      if (mp_nummer_stimmt (nummer)) {
        AlleOEVTele->inhalt->prioritaet = (unsigned) wert;
      }
      break;

    case OVT_ZUGLAENGE            :
      if (mp_nummer_stimmt (nummer)) {
        AlleOEVTele->inhalt->zuglaenge = (unsigned) wert;
      }
      break;

    case OVT_R_V_H                :
      if (mp_nummer_stimmt (nummer)) {
        AlleOEVTele->inhalt->richtung_von_hand = (unsigned) wert;
      }
      break;

    case OVT_FAHRPLAN             :
      if (mp_nummer_stimmt (nummer)) {
        AlleOEVTele->inhalt->fahrpl_sek = (int) wert;
      }
      break;

    case OVT_PERSONEN             :
      if (mp_nummer_stimmt (nummer)) {
        AlleOEVTele->inhalt->personen = (unsigned) wert;
      }
      break;

    case LSAKOPPLUNG              :
      el = LSA_Puffer->AlleEingaenge;
      while (el != NULL) {
        if (el->inhalt->nummer == nummer) {
          el->inhalt->wert = wert;
          break;
        }
        el = el->next;
      }
      if (el == NULL) {
        el = (EingangListe) malloc (sizeof (*el));
        if (el == NULL) {
          AllgemeinerFehler (prozedurname, kein_speicher);
          return 0;
        }
        el->next = LSA_Puffer->AlleEingaenge;
        LSA_Puffer->AlleEingaenge = el;
        el->inhalt = (Eingang) malloc (sizeof (*el->inhalt));
        if (el->inhalt == NULL) {
          AllgemeinerFehler (prozedurname, kein_speicher);
          return 0;
        }
        el->inhalt->nummer = nummer;
        el->inhalt->wert   = wert;
      }
      break;

    case SIMZEIT                  : {
      double sentSimTime = wert / LSA_Puffer->Schaltfrequenz;

      if (sentSimTime - SimulationTime > 0.001) {
        loesche_telegramm_liste (&AlleOEVTele);
      }
      SimulationTime = sentSimTime;
      break;
    }
    default                       :
      /* unerwarteter oder unbekannter Typ => ignorieren! */
      return 0;
  } /* switch (typ) */

  return 1;
} /* LongToLSAPuffer */

/*==========================================================================*/

void  VonLSA (int kommando, int nr, BOOLEAN uebergang)
{
  /* Gibt je nach <kommando> einen Schaltbefehl an die Signalgruppe <nr>: */
  /*  1: Signalgruppe gruen                                               */
  /*  2: Signalgruppe rot                                                 */
  /*  3: Solidstate ein                                                   */
  /*  4: Solidstate oder Signalgruppe aus                                 */
  /*  5: Schutzblinker ein                                                */
  /*  6: Schutzblinker aus                                                */
  /*  7: Signalgruppe rotgelb                                             */
  /*  8: Signalgruppe gelb                                                */
  /*  9: Signalgruppe gelb blinkend                                       */
  /* 10: Signalgruppe rot blinkend                                        */
  /* 11: Signalgruppe gruen blinkend                                      */
  /* 12: Signalgruppe rot-gruen wechselblinkend                           */
  /* 13: Signalgruppe gruengelb                                           */
  /* Falls <uebergang> gesetzt ist, soll beim Umschalten ggf. ein         */
  /* Uebergangssignal geschaltet werden.                                  */
  /* Falls die Signalgruppe <nr> im VISSIM-Netz nicht definiert ist oder  */
  /* <kommando> nicht im zulaessigen Bereich liegt, erfolgt eine          */
  /* Laufzeitfehlermeldung.                                               */

  char  prozedurname[] = "LSA_PUFF.VonLSA";
  int   index = sg_nr_to_index (nr, prozedurname, false);

  if (index < 0) {
    return;
  }
  assert (LSA_Puffer->SigGr[index] != NULL);

  LSA_Puffer->SigGr[index]->Uebergang = uebergang;
  switch (kommando) {
    case  1 :
    case  3 :
    case  5 :
      LSA_Puffer->SigGr[index]->Soll = gruen;
      break;
    case  2 :
      LSA_Puffer->SigGr[index]->Soll = rot;
      break;
    case  4 :
    case  6 :
      LSA_Puffer->SigGr[index]->Soll = aus;
      break;
    case  7 :
      LSA_Puffer->SigGr[index]->Soll = rotgelb;
      break;
    case  8 :
      LSA_Puffer->SigGr[index]->Soll = gelb;
      break;
    case  9 :
      LSA_Puffer->SigGr[index]->Soll = gelb_blink;
      break;
    case 10 :
      LSA_Puffer->SigGr[index]->Soll = rot_blink;
      break;
    case 11 :
      LSA_Puffer->SigGr[index]->Soll = gruen_blink;
      break;
    case 12 :
      LSA_Puffer->SigGr[index]->Soll = rgwb;
      break;
    case 13 :
      LSA_Puffer->SigGr[index]->Soll = gruengelb;
      break;
    default :
      HoleRessourcenString (IDS_UNZUL_KOMMANDO, res_string, sizeof (res_string));
      InternerFehler (prozedurname, res_string);
      return;
  }
} /* VonLSA */

/*--------------------------------------------------------------------------*/

unsigned short  NachLSA (int kommando, int nr)
{
  /* Gibt je nach <kommando> einen Wert der Signalgruppe <nr> bzw. LSA */
  /* an das Steuerungsprogramm zurueck:                                */
  /*  1: Mindestrotzeit                                                */
  /*  2: Vorbereitungszeit (Rotgelbzeit) (wie in VISSIM versorgt)      */
  /*  3: Mindestgruenzeit                                              */
  /*  4: Gelbzeit                        (wie in VISSIM versorgt)      */
  /*  5: Bild == Rot oder Gelb?        (falls ja: 1, sonst 0)          */
  /*  6: Bild == Rotgelb?              (falls ja: 1, sonst 0)          */
  /*  7: Bild == Gruen oder Rotgelb?   (falls ja: 1, sonst 0)          */
  /*  8: Bild == Gelb?                 (falls ja: 1, sonst 0)          */
  /*  9: Mindestrotzeit abgelaufen?    (falls ja: 0, sonst Laufzeit)   */
  /* 10: Mindestgruenzeit abgelaufen?  (falls ja: 0, sonst Laufzeit)   */
  /* 11: aktuelle Rotdauer (incl. Gelb!)                               */
  /* 12: aktuelle Vorbereitungsdauer (Rotgelbdauer)                    */
  /* 13: aktuelle Gruendauer (incl. Rotgelb!)                          */
  /* 14: aktuelle Gelbdauer                                            */
  /* 15: aktuelle Sekunde im Umlauf                                    */
  /* 16: Umlaufzeit                                                    */
  /* 17: Soll == Rot?                  (falls ja: 1, sonst 0)          */
  /* 18: Soll == Gruen?                (falls ja: 1, sonst 0)          */
  /* 19: bisherige Dauer des aktuellen Signalbilds                     */
  /* Falls die Signalgruppe im VISSIM-Netz nicht definiert ist,        */
  /* <nr> < 0, (<nr> == 0 und (<kommando> < 15 oder <kommando> > 16)), */
  /* <nr> > MAXSIGGR ist oder <kommando> ausserhalb des zulaessigen    */
  /* Bereichs liegt, erfolgt eine Laufzeitfehlermeldung (Rueckgabewert */
  /* 0). Alle Zeiten werden in ganzen Sekunden angegeben.              */

  char        prozedurname[] = "LSA_PUFF.NachLSA";
  SignalBild  Bild, Soll;
  long        TGruenMin;
  long        TRotMin;
  long        TRotgelb;
  long        TGelb;
  long        Laufzeit;
  int         zeitfaktor = 1;
  int         index;

  if (kommando <= 14 || kommando >= 17) {
    index = sg_nr_to_index (nr, prozedurname, false);
    if (index < 0) {
      return 0;
    }
    assert (LSA_Puffer->SigGr[index] != NULL);
    Bild      = LSA_Puffer->SigGr[index]->Bild;
    Soll      = LSA_Puffer->SigGr[index]->Soll;
    TGruenMin = (long) LSA_Puffer->SigGr[index]->TGruenMin;
    TRotMin   = (long) LSA_Puffer->SigGr[index]->TRotMin;
    TRotgelb  = (long) LSA_Puffer->SigGr[index]->TRotgelb;
    TGelb     = (long) LSA_Puffer->SigGr[index]->TGelb;
    Laufzeit  = (long) LSA_Puffer->SigGr[index]->Laufzeit;
  }

  switch (kommando) {
    case  1 : /* Mindestrotzeit */
      return long_to_unsigned_short (TRotMin * zeitfaktor);
    case  2 : /* Vorbereitungszeit (Rotgelbzeit) */
      return long_to_unsigned_short (TRotgelb * zeitfaktor);
    case  3 : /* Mindestgruenzeit */
      return long_to_unsigned_short (TGruenMin * zeitfaktor);
    case  4 : /* Gelbzeit */
      return long_to_unsigned_short (TGelb * zeitfaktor);
    case  5 : /* Bild == Rot oder Gelb? (falls ja: 1, sonst 0) */
      return (unsigned short) (Bild == rot || Bild == gelb);
    case  6 : /* Bild == Rotgelb? (falls ja: 1, sonst 0) */
      return (unsigned short) (Bild == rotgelb);
    case  7 : /* Bild == Gruen oder Rotgelb? (falls ja: 1, sonst 0) */
      return (unsigned short) (Bild == gruen || Bild == rotgelb);
    case  8 : /* Bild == Gelb? (falls ja: 1, sonst 0) */
      return (unsigned short) (Bild == gelb);
    case  9 : /* Mindestrotzeit abgelaufen? (falls ja: 0, sonst Laufzeit) */
      return Bild == rot && Laufzeit <= TRotMin
               ? long_to_unsigned_short (Laufzeit * zeitfaktor)
               : (unsigned short) (0);
    case 10 : /* Mindestgruenzeit abgelaufen? (falls ja: 0, sonst Laufzeit) */
      return Bild == gruen && Laufzeit <= TGruenMin
               ? long_to_unsigned_short (Laufzeit * zeitfaktor)
               : (unsigned short) (0);
    case 11 : /* aktuelle Rotdauer (incl. Gelb) */
      return Bild == rot
               ? long_to_unsigned_short ((Laufzeit + TGelb) * zeitfaktor)
               : Bild == gelb
                   ? long_to_unsigned_short (Laufzeit * zeitfaktor)
                   : (unsigned short) (0);
    case 12 : /* aktuelle Vorbereitungsdauer (Rotgelbdauer) */
      return Bild == rotgelb
               ? long_to_unsigned_short (Laufzeit * zeitfaktor)
               : (unsigned short) (0);
    case 13 : /* aktuelle Gruendauer (incl. Rotgelb) */
      return Bild == gruen
               ? long_to_unsigned_short ((Laufzeit + TRotgelb) * zeitfaktor)
               : Bild == rotgelb
                 ? long_to_unsigned_short (Laufzeit * zeitfaktor)
                 : (unsigned short) (0);
    case 14 : /* aktuelle Gelbdauer */
      return Bild == gelb
               ? long_to_unsigned_short (Laufzeit * zeitfaktor)
               : (unsigned short) (0);
    case 15 : /* aktuelle Sekunde im Umlauf */
      return long_to_unsigned_short ((long) LSA_Puffer->Sekunde * zeitfaktor);
    case 16 : /* Umlaufzeit */
      return long_to_unsigned_short ((long) LSA_Puffer->TU * zeitfaktor);
    case 17 : /* Soll == Rot? (falls ja: 1, sonst 0) */
      return (unsigned short) (Soll == rot);
    case 18 : /* Soll == Gruen? (falls ja: 1, sonst 0) */
      return (unsigned short) (Soll == gruen);
    case 19 : /* bisherige Dauer des aktuellen Signalbilds */
      return long_to_unsigned_short (Laufzeit * zeitfaktor);
    default :
      HoleRessourcenString (IDS_UNZUL_KOMMANDO, res_string, sizeof (res_string));
      InternerFehler (prozedurname, res_string);
      return 0;
  }
} /* NachLSA */

/*==========================================================================*/

float  Simulationssekunde (void)
{
  /* Gibt die aktuelle Simulationssekunde zurueck. */

  return (float) SimulationTime;
} /* Simulationssekunde */

/*==========================================================================*/

float  Umlaufsekunde (void)
{
  /* Gibt die aktuelle Umlaufsekunde zurueck. */

  return (float) LSA_Puffer->Sekunde;
} /* Umlaufsekunde */

/*==========================================================================*/

int  ProgrammNummer (void)
{
  /* Gibt die Nummer des laufenden Programms zurueck (Default = 1). */

  return (int) LSA_Puffer->ProgNr;
} /* ProgrammNummer */

/*==========================================================================*/

int  sg_definiert (int nr)
{
  /* Falls die Signalgruppe Nr. <nr> in VISSIM definiert ist, wird 1 */
  /* zurückgegeben, andernfalls 0.                                   */

  int  index = sg_nr_to_index (nr, NULL, false);

  if (index < 0) {
    return 0;
  }
  assert (LSA_Puffer->SigGr[index] != NULL);
  return 1;
} /* sg_definiert */

/*--------------------------------------------------------------------------*/

int  det_definiert (int nr)
{
  /* Falls der Detektor Nr. <nr> in VISSIM definiert ist, wird 1 */
  /* zurückgegeben, andernfalls 0.                               */

  int  index = det_nr_to_index (nr, NULL, false);

  if (index < 0) {
    return 0;
  }
  assert (LSA_Puffer->Det[index] != NULL);
  return 1;
} /* det_definiert */

/*==========================================================================*/

int  d_imp (int nr)
{
  /* Falls der Detektor Nr. <nr> in VISSIM nicht definiert ist, erfolgt eine  */
  /* Laufzeitfehlermeldung (Rueckgabewert 0).                                 */
  /* Ansonsten wird der Wert des Impulsspeichers des Detektors Nr. <nr>       */
  /* zurueckgegeben: 1, wenn sich der Zustand des Detektors von unbelegt auf  */
  /* belegt geändert hat (wenn also eine Fahrzeugvorderkante erfasst wurde,   */
  /* während nicht schon ein anderes Fahrzeug auf dem Detektor war), sonst 0. */
  /* Der Impulsspeicher wird von VISSIM nicht automatisch geloescht - siehe   */
  /* Funktion d_limp() weiter unten!                                          */

  char  prozedurname[] = "LSA_PUFF.d_imp";
  int   index = det_nr_to_index (nr, prozedurname, false);

  if (index < 0) {
    return 0;
  }
  assert (LSA_Puffer->Det[index] != NULL);
  return LSA_Puffer->Det[index]->Impulsspeicher;
} /* d_imp */

/*--------------------------------------------------------------------------*/

int  d_limp (int nr)
{
  /* Falls der Detektor Nr. <nr> in VISSIM nicht definiert ist, erfolgt */
  /* eine Laufzeitfehlermeldung (Rueckgabewert -1).                     */
  /* Ansonsten wird beim Detektor Nr. <nr> vermerkt, dass dessen        */
  /* Impulsspeicher am Ende des Zeitschritts auf 0 gesetzt werden soll, */
  /* und 0 zurueckgegeben.                                              */

  char  prozedurname[] = "LSA_PUFF.d_limp";
  int   index = det_nr_to_index (nr, prozedurname, false);

  if (index < 0) {
    return -1;
  }
  assert (LSA_Puffer->Det[index] != NULL);
  LSA_Puffer->Det[index]->ImpulsspeicherLoeschen = TRUE;
  return 0;
} /* d_limp */

/*--------------------------------------------------------------------------*/

int  d_blgzt (int nr)
{
  /* Falls der Detektor Nr. <nr> in VISSIM nicht definiert ist, erfolgt eine  */
  /* Laufzeitfehlermeldung (Rueckgabewert 0).                                 */
  /* Ansonsten wird 0 zurueckgegeben, falls sich zum Ende des Zeitschritts    */
  /* kein Fahrzeug auf dem Detektor Nr. <nr> befindet, andernfalls die Zeit   */
  /* seit seinem Eintreffen in 1/10 s.                                        */

  char  prozedurname[] = "LSA_PUFF.d_blgzt";
  int   index = det_nr_to_index (nr, prozedurname, false);

  if (index < 0) {
    return 0;
  }
  assert (LSA_Puffer->Det[index] != NULL);
  if (LSA_Puffer->Det[index]->Anwesenheit) {
    return (int) MIN (INT_MAX, LSA_Puffer->Det[index]->Belegungsdauer / 10);
  }
  else {
    return 0;
  }
} /* d_blgzt */

/*--------------------------------------------------------------------------*/

int  d_belgg (int nr)
{
  /* Falls der Detektor Nr. <nr> in VISSIM nicht definiert ist, erfolgt eine  */
  /* Laufzeitfehlermeldung (Rueckgabewert 0).                                 */
  /* Ansonsten wird der Wert des exponentiell geglaetteten Belegungsgrads     */
  /* in % des Detektors Nr. <nr> zurueckgegeben (Glaettungsfaktoren gemaess   */
  /* Versorgung dieses Detektors in VISSIM).                                  */

  char  prozedurname[] = "LSA_PUFF.d_belgg";
  int   index = det_nr_to_index (nr, prozedurname, false);

  if (index < 0) {
    return 0;
  }
  assert (LSA_Puffer->Det[index] != NULL);
  return (int) LSA_Puffer->Det[index]->geglaetteteBelegung;
} /* d_belgg */

/*--------------------------------------------------------------------------*/

long  d_ztlkn (int nr)
{
  /* Falls der Detektor Nr. <nr> in VISSIM nicht definiert ist, erfolgt    */
  /* eine Fehlermeldung (Rueckgabewert LONG_MAX = 2147483647L).            */
  /* Ansonsten wird die angefangene Nettozeitluecke beim Detektor Nr. <nr> */
  /* in 1/100 s zurueckgegeben (aber maximal LONG_MAX).                    */

  char  prozedurname[] = "LSA_PUFF.d_ztlkn";
  int   index = det_nr_to_index (nr, prozedurname, false);

  if (index < 0) {
    return LONG_MAX;
  }
  assert (LSA_Puffer->Det[index] != NULL);
  return MIN (LONG_MAX, LSA_Puffer->Det[index]->angefangeneZeitluecke);
} /* d_ztlkn */

/*--------------------------------------------------------------------------*/

long  d_v (int nr)
{
  /* Falls der Detektor Nr. <nr> in VISSIM nicht definiert ist, erfolgt eine  */
  /* Laufzeitfehlermeldung (Rueckgabewert 0).                                 */
  /* Ansonsten wird 0 zurueckgegeben, falls seit dem letzten Durchlauf durch  */
  /* die Steuerungslogik kein Fahrzeug vom Detektor Nr. <nr> erfasst wurde,   */
  /* andernfalls die aktuelle Geschwindigkeit des letzten erfassten Fahrzeugs */
  /* in 1/100 m/s.                                                            */

  char  prozedurname[] = "LSA_PUFF.d_v";
  int   index = det_nr_to_index (nr, prozedurname, false);

  if (index < 0) {
    return 0;
  }
  assert (LSA_Puffer->Det[index] != NULL);
  return MIN (LONG_MAX, LSA_Puffer->Det[index]->v);
} /* d_v */

/*--------------------------------------------------------------------------*/

long  d_l (int nr)
{
  /* Falls der Detektor Nr. <nr> in VISSIM nicht definiert ist, erfolgt eine  */
  /* Laufzeitfehlermeldung (Rueckgabewert 0).                                 */
  /* Ansonsten wird 0 zurueckgegeben, falls seit dem letzten Durchlauf durch  */
  /* die Steuerungslogik kein Fahrzeug vom Detektor Nr. <nr> erfasst wurde,   */
  /* andernfalls die Laenge des letzten erfassten Fahrzeugs in mm.            */

  char  prozedurname[] = "LSA_PUFF.d_v";
  int   index = det_nr_to_index (nr, prozedurname, false);

  if (index < 0) {
    return 0;
  }
  assert (LSA_Puffer->Det[index] != NULL);
  return MIN (LONG_MAX, LSA_Puffer->Det[index]->l);
} /* d_l */

/*--------------------------------------------------------------------------*/

DetectorType d_typ(int nr)
{
  char  prozedurname[] = "LSA_PUFF.d_typ";
  int   index = det_nr_to_index (nr, prozedurname, false);

  if (index < 0) {
    return UndefinedDetector;
  }
  assert (LSA_Puffer->Det[index] != NULL);
  return LSA_Puffer->Det[index]->Type;
}

/*--------------------------------------------------------------------------*/

std::string d_name(int nr)
{
  char  prozedurname[] = "LSA_PUFF.d_name";
  int   index = det_nr_to_index (nr, prozedurname, false);

  if (index < 0) {
    return 0;
  }
  assert (LSA_Puffer->Det[index] != NULL);
  return LSA_Puffer->Det[index]->Name;
}
/*--------------------------------------------------------------------------*/
unsigned char  anwesenheit (int nr)
{
  /* Falls der Detektor Nr. <nr> in VISSIM nicht definiert ist, erfolgt    */
  /* eine Laufzeitfehlermeldung (Rueckgabewert 0).                         */
  /* Ansonsten wird 1 zurueckgegeben, falls sich zum Ende des aktuellen    */
  /* Zeitschritts ein Fahrzeug im Erfassungsbereich des Detektors Nr. <nr> */
  /* befindet, andernfalls 0.                                              */

  char  prozedurname[] = "LSA_PUFF.anwesenheit";
  int   index = det_nr_to_index (nr, prozedurname, false);

  if (index < 0) {
    return 0;
  }
  assert (LSA_Puffer->Det[index] != NULL);
  if (LSA_Puffer->Det[index]->Anwesenheit) {
    return 1;
  }
  else {
    return 0;
  }
} /* anwesenheit */

/*--------------------------------------------------------------------------*/

unsigned char  zaehlwert (int nr)
{
  /* Falls der Detektor Nr. <nr> in VISSIM nicht definiert ist, erfolgt */
  /* eine Laufzeitfehlermeldung (Rueckgabewert 0).                      */
  /* Ansonsten wird die Anzahl der seit dem letzten Durchlauf durch die */
  /* Steuerungslogik erfassten Fahrzeugvorderkanten zurueckgegeben      */
  /* (wobei einander ueberlappende Impulse zu einem zusammengefasst     */
  /* werden).                                                           */

  char  prozedurname[] = "LSA_PUFF.zaehlwert";
  int   index = det_nr_to_index (nr, prozedurname, false);

  if (index < 0) {
    return 0;
  }
  assert (LSA_Puffer->Det[index] != NULL);
  return (unsigned char) LSA_Puffer->Det[index]->Vorderkanten;
} /* zaehlwert */

/*--------------------------------------------------------------------------*/

unsigned char  hinterkanten (int nr)
{
  /* Falls der Detektor Nr. <nr> in VISSIM nicht definiert ist, erfolgt */
  /* eine Laufzeitfehlermeldung (Rueckgabewert 0).                      */
  /* Ansonsten wird die Anzahl der seit dem letzten Durchlauf durch     */
  /* die Steuerungslogik erfassten Fahrzeughinterkanten zurueckgegeben  */
  /* (wobei einander ueberlappende Impulse zu einem zusammengefasst     */
  /* werden).                                                           */

  char  prozedurname[] = "LSA_PUFF.hinterkanten";
  int   index = det_nr_to_index (nr, prozedurname, false);

  if (index < 0) {
    return 0;
  }
  assert (LSA_Puffer->Det[index] != NULL);
  return (unsigned char) LSA_Puffer->Det[index]->Hinterkanten;
} /* hinterkanten */

/*--------------------------------------------------------------------------*/

unsigned long  vorderkantensumme (int nr)
{
  /* Falls der Detektor Nr. <nr> in VISSIM nicht definiert ist, erfolgt */
  /* eine Laufzeitfehlermeldung (Rueckgabewert 0).                      */
  /* Ansonsten wird die Anzahl der seit der letzten Loeschung erfassten */
  /* Fahrzeugvorderkanten zurueckgegeben (wobei einander ueberlappende  */
  /* Impulse zu einem zusammengefasst werden).                          */

  char  prozedurname[] = "LSA_PUFF.vorderkantensumme";
  int   index = det_nr_to_index (nr, prozedurname, false);

  if (index < 0) {
    return 0;
  }
  assert (LSA_Puffer->Det[index] != NULL);
  return LSA_Puffer->Det[index]->VorderkantenSumme;
} /* vorderkantensumme */

/*--------------------------------------------------------------------------*/

void  loesche_vorderkantensumme (int nr)
{
  /* Falls der Detektor Nr. <nr> in VISSIM nicht definiert ist, erfolgt */
  /* eine Laufzeitfehlermeldung (Rueckgabewert 0).                      */
  /* Ansonsten wird die Anzahl der erfassten Fahrzeugvorderkanten       */
  /* geloescht.                                                         */

  char  prozedurname[] = "LSA_PUFF.loesche_vorderkantensumme";
  int   index = det_nr_to_index (nr, prozedurname, false);

  if (index < 0) {
    return;
  }
  assert (LSA_Puffer->Det[index] != NULL);
  LSA_Puffer->Det[index]->VorderkantenSumme = 0;
} /* loesche_vorderkantensumme */

/*--------------------------------------------------------------------------*/

unsigned long  hinterkantensumme (int nr)
{
  /* Falls der Detektor Nr. <nr> in VISSIM nicht definiert ist, erfolgt */
  /* eine Laufzeitfehlermeldung (Rueckgabewert 0).                      */
  /* Ansonsten wird die Anzahl der seit der letzten Loeschung erfassten */
  /* Fahrzeughinterkanten zurueckgegeben (wobei einander ueberlappende  */
  /* Impulse zu einem zusammengefasst werden).                          */

  char  prozedurname[] = "LSA_PUFF.hinterkantensumme";
  int   index = det_nr_to_index (nr, prozedurname, false);

  if (index < 0) {
    return 0;
  }
  assert (LSA_Puffer->Det[index] != NULL);
  return LSA_Puffer->Det[index]->HinterkantenSumme;
} /* hinterkantensumme */

/*--------------------------------------------------------------------------*/

void  loesche_hinterkantensumme (int nr)
{
  /* Falls der Detektor Nr. <nr> in VISSIM nicht definiert ist, erfolgt */
  /* eine Laufzeitfehlermeldung (Rueckgabewert 0).                      */
  /* Ansonsten wird die Anzahl der erfassten Fahrzeughinterkanten       */
  /* geloescht.                                                         */

  char  prozedurname[] = "LSA_PUFF.loesche_hinterkantensumme";
  int   index = det_nr_to_index (nr, prozedurname, false);

  if (index < 0) {
    return;
  }
  assert (LSA_Puffer->Det[index] != NULL);
  LSA_Puffer->Det[index]->HinterkantenSumme = 0;
} /* loesche_hinterkantensumme */

/*--------------------------------------------------------------------------*/

unsigned char  belegung (int nr)
{
  /* Falls der Detektor Nr. <nr> in VISSIM nicht definiert ist, erfolgt */
  /* eine Laufzeitfehlermeldung (Rueckgabewert 0).                      */
  /* Ansonsten wird der (nicht geglaettete) Belegungsgrad des           */
  /* Detektors Nr. <nr> seit dem letzten Durchlauf durch die            */
  /* Steuerungslogik zurueckgegeben (in %).                             */

  char  prozedurname[] = "LSA_PUFF.belegung";
  int   index = det_nr_to_index (nr, prozedurname, false);

  if (index < 0) {
    return 0;
  }
  assert (LSA_Puffer->Det[index] != NULL);
  return (unsigned char) LSA_Puffer->Det[index]->aktuelleBelegung;
} /* belegung */

/*--------------------------------------------------------------------------*/

unsigned char  abgeschl_luecke (int nr)
{
  /* Falls der Detektor Nr. <nr> in VISSIM nicht definiert ist, erfolgt  */
  /* eine Laufzeitfehlermeldung (Rueckgabewert 0).                       */
  /* Ansonsten wird der 0 zurueckgegeben, falls seit dem letzten         */
  /* Durchlauf durch die Steuerungslogik keine Zeitluecke begonnen UND   */
  /* beendet wurde, andernfalls die Dauer der letzten solchen Zeitluecke */
  /* in 1/100 s.                                                         */

  char  prozedurname[] = "LSA_PUFF.abgeschl_luecke";
  int   index = det_nr_to_index (nr, prozedurname, false);

  if (index < 0) {
    return 0;
  }
  assert (LSA_Puffer->Det[index] != NULL);
  return (unsigned char) LSA_Puffer->Det[index]->abgeschlosseneZeitluecke;
} /* abgeschl_luecke */

/*--------------------------------------------------------------------------*/

int  d_on (int nr, int intervall_nr, int intervalle_pro_schaltint)
{
  /* Falls der Detektor Nr. <nr> in VISSIM nicht definiert ist,         */
  /* erfolgt eine Laufzeitfehlermeldung (Rueckgabewert -1).             */
  /* Ansonsten wird 1 zurueckgegeben, falls im Intervall Nummer         */
  /* <intervall_nr> (gezaehlt von 0 an) von <intervalle_pro_schaltint>  */
  /* ([1..100]) gleichlangen Intervallen seit dem letzten Durchlauf     */
  /* durch die Steuerungslogik eine Fahrzeugvorderkante erfasst wurde,  */
  /* sonst 0.                                                           */

  char  prozedurname[] = "LSA_PUFF.d_on";
  int   i;
  long  t_von, t_bis;
  int   index = det_nr_to_index (nr, prozedurname, false);

  if (index < 0) {
    return -1;
  }
  assert (LSA_Puffer->Det[index] != NULL);
  t_von = 100L * intervall_nr       / intervalle_pro_schaltint;
  t_bis = 100L * (intervall_nr + 1) / intervalle_pro_schaltint;
  i = 0;
  while (i < MAX_KANTEN_PRO_SI && LSA_Puffer->Det[index]->tVorderkante[i] != -1) {
    if (   LSA_Puffer->Det[index]->tVorderkante[i] >= t_von
        && LSA_Puffer->Det[index]->tVorderkante[i] <  t_bis) {
      return 1;
    }
    i++;
  }
  return 0;
} /* d_on */

/*--------------------------------------------------------------------------*/

int  d_off (int nr, int intervall_nr, int intervalle_pro_schaltint)
{
  /* Falls der Detektor Nr. <nr> in VISSIM nicht definiert ist,        */
  /* erfolgt eine Laufzeitfehlermeldung (Rueckgabewert -1).            */
  /* Ansonsten wird 1 zurueckgegeben, falls im Intervall Nummer        */
  /* <intervall_nr> (gezaehlt von 0 an) von <intervalle_pro_schaltint> */
  /* ([1..100]) gleichlangen Intervallen seit dem letzten Durchlauf    */
  /* durch die Steuerungslogik eine Fahrzeughinterkante erfasst wurde, */
  /* sonst 0.                                                          */

  char  prozedurname[] = "LSA_PUFF.d_off";
  int   i;
  long  t_von, t_bis;
  int   index = det_nr_to_index (nr, prozedurname, false);

  if (index < 0) {
    return -1;
  }
  assert (LSA_Puffer->Det[index] != NULL);
  t_von = 100L * intervall_nr       / intervalle_pro_schaltint;
  t_bis = 100L * (intervall_nr + 1) / intervalle_pro_schaltint;
  i = 0;
  while (i < MAX_KANTEN_PRO_SI && LSA_Puffer->Det[index]->tHinterkante[i] != -1) {
    if (   LSA_Puffer->Det[index]->tHinterkante[i] >= t_von
        && LSA_Puffer->Det[index]->tHinterkante[i] <  t_bis) {
      return 1;
    }
    i++;
  }
  return 0;
} /* d_off */

/*--------------------------------------------------------------------------*/

int  d_presence (int nr, int intervall_nr, int intervalle_pro_schaltint)
{
  /* Falls der Detektor Nr. <nr> in VISSIM nicht definiert ist,           */
  /* erfolgt eine Laufzeitfehlermeldung (Rueckgabewert -1).               */
  /* Ansonsten wird 1 zurueckgegeben, falls am Ende des Intervalls Nummer */
  /* <intervall_nr> (gezaehlt von 0 an) von <intervalle_pro_schaltint>    */
  /* ([1..100]) gleichlangen Intervallen seit dem letzten Durchlauf       */
  /* durch die Steuerungslogik ein Fahrzeug über dem Detektor war,        */
  /* sonst 0.                                                             */

  char  prozedurname[] = "LSA_PUFF.d_presence";
  int   i, t, before;
  int   index = det_nr_to_index (nr, prozedurname, false);

  if (index < 0) {
    return -1;
  }
  assert (LSA_Puffer->Det[index] != NULL);
  if (   LSA_Puffer->Det[index]->tVorderkante[0] == -1
      && LSA_Puffer->Det[index]->tHinterkante[0] == -1) {
    if (LSA_Puffer->Det[index]->Anwesenheit) {
      return 1;
    }
    else {
      return 0;
    }
  }
  else {
    t = 100 * (intervall_nr + 1) / intervalle_pro_schaltint;
    for (i = 0; i < MAX_KANTEN_PRO_SI; i++) {
      if (LSA_Puffer->Det[index]->tVorderkante[i] == -1) {
        if (LSA_Puffer->Det[index]->tHinterkante[i] == -1) {
          break;
        }
        else {
          if (LSA_Puffer->Det[index]->tHinterkante[i] <= t) {
            return 0;
          }
          else {
            return 1;
          }
        }
      } /* if (LSA_Puffer->Det[index]->tVorderkante[i] == -1) */
      else {
        if (LSA_Puffer->Det[index]->tHinterkante[i] == -1) {
          if (LSA_Puffer->Det[index]->tVorderkante[i] <= t) {
            return 1;
          }
          else {
            return 0;
          }
        }
        else {
          if (   LSA_Puffer->Det[index]->tVorderkante[i] 
              <= LSA_Puffer->Det[index]->tHinterkante[i]) {
            if (LSA_Puffer->Det[index]->tVorderkante[i] <= t) {
              before = 1;
            }
            else {
              return 0;
            }
            if (LSA_Puffer->Det[index]->tHinterkante[i] <= t) {
              before = 0;
            }
            else {
              return 1;
            }
          } /* if (tVorderkante[i] <= tHinterkante[i]) */
          else {
            if (LSA_Puffer->Det[index]->tHinterkante[i] <= t) {
              before = 0;
            }
            else {
              return 1;
            }
            if (LSA_Puffer->Det[index]->tVorderkante[i] <= t) {
              before = 1;
            }
            else {
              return 0;
            }
          } /* tVorderkante[i] > tHinterkante[i] */
        } /* LSA_Puffer->Det[index]->tHinterkante[i] != -1 */
      } /* LSA_Puffer->Det[index]->tVorderkante[i] != -1 */
    } /* for (i = 0; i < MAX_KANTEN_PRO_SI; i++) */
    return before;
  }
} /* d_presence */

/*--------------------------------------------------------------------------*/

void  SetzeUmlaufsekunde (float uml_sek)
{
  /* Setzt die aktuelle Umlaufsekunde auf <uml_sek>.                  */
  /* (Dieser Wert wird am Ende des Zeitschritts an VISSIM uebergeben, */
  /* auf ein volles Schaltintervall (Kehrwert der Schaltfrequenz)     */
  /* genau.)                                                          */

  LSA_Puffer->Sekunde = uml_sek;
} /* SetzeUmlaufsekunde */

/*--------------------------------------------------------------------------*/

void  SetzeProgrammnummer (unsigned prg_nr)
{
  /* Setzt die aktuelle Programmnummer auf <prg_nr>.                   */
  /* (Dieser Wert wird am Ende des Zeitschritts an VISSIM uebergeben.) */

  LSA_Puffer->ProgNr = prg_nr;
} /* SetzeProgrammnummer */

/*--------------------------------------------------------------------------*/

BOOLEAN  SchreibeProtTripel (long code, long nummer, long wert)
{
  /* Schreibt ein neues Protokoll-Tripel in den LSA-Puffer.    */
  /* <code>, <nummer> und <wert> sollten in den Wertebereichen */
  /* liegen, die in der WTT-Datei des Steuerungsverfahrens     */
  /* definiert sind. Insbesondere sollte <code> > 9999 sein.   */
  /* Bei Erfolg wird TRUE zurueckgegeben, sonst FALSE          */
  /* und ein Laufzeitfehler erzeugt.                           */

  if (*LSA_Puffer->pNaechstesTripel == NULL) {
    *LSA_Puffer->pNaechstesTripel = (P_ProtTripel) malloc (sizeof (**LSA_Puffer->pNaechstesTripel));
    if (*LSA_Puffer->pNaechstesTripel == NULL) {
      AllgemeinerFehler ("LSA_PUFF.SchreibeProtTripel", kein_speicher);
      return FALSE;
    }
    (*LSA_Puffer->pNaechstesTripel)->next = NULL;
  }
  (*LSA_Puffer->pNaechstesTripel)->Code   = code;
  (*LSA_Puffer->pNaechstesTripel)->Nummer = nummer;
  (*LSA_Puffer->pNaechstesTripel)->Wert   = wert;
  LSA_Puffer->pNaechstesTripel = &(*LSA_Puffer->pNaechstesTripel)->next;
  LSA_Puffer->AnzProtTripel++;
  return TRUE;
} /* SchreibeProtTripel */

/*--------------------------------------------------------------------------*/

int  Anzahl_OEV_Tele (void)
{
  /* Gibt die Anzahl der im aktuellen Zeitschritt empfangenen */
  /* OEV-Telegramme zurueck.                                  */

  TelegrammListe  tl = AlleOEVTele;
  int             result = 0;

  while (tl != NULL) {
    result++;
    tl = tl->next;
  }
  return result;
} /* Anzahl_OEV_Tele */

/*--------------------------------------------------------------------------*/

static  TelegrammListe  oev_tele_nr (unsigned index)
{
  /* Gibt einen Zeiger auf das Telegramm Nummer <index> (erstes: 1) */
  /* aus dem aktuellen Zeitschritt zurueck, bzw. NULL, wenn nicht   */
  /* soviele empfangen wurden.                                      */

  TelegrammListe  result = AlleOEVTele;
  unsigned        zaehler = 1;

  while (result != NULL && zaehler != index) {
    zaehler++;
    result = result->next;
  }
  return result;
} /* oev_tele_nr */

/*--------------------------------------------------------------------------*/

long  OEV_Tele_MP (unsigned index)
{
  /* Falls <index> groesser ist als die Anzahl der im aktuellen Zeitschritt */
  /* empfangenen OEV-Telegramme, wird -1 zurueckgegeben, andernfalls die    */
  /* Meldepunkt-Nummer aus dem Telegramm Nummer <index> (erstes: 1).        */

  TelegrammListe  tl = oev_tele_nr (index);

  if (tl != NULL) {
    return tl->inhalt->mp_nr;
  }
  else {
    return -1L;
  }
} /* OEV_Tele_MP */

/*--------------------------------------------------------------------------*/

int  OEV_Tele_Linie (unsigned index)
{
  /* Falls <index> groesser ist als die Anzahl der im aktuellen Zeitschritt */
  /* empfangenen OEV-Telegramme, wird -1 zurueckgegeben, andernfalls die    */
  /* Liniennummer aus dem Telegramm Nummer <index> (erstes: 1).             */

  TelegrammListe  tl = oev_tele_nr (index);

  if (tl != NULL) {
    return tl->inhalt->linien_nr;
  }
  else {
    return -1;
  }
} /* OEV_Tele_Linie */

/*--------------------------------------------------------------------------*/

int  OEV_Tele_Kurs (unsigned index)
{
  /* Falls <index> groesser ist als die Anzahl der im aktuellen Zeitschritt */
  /* empfangenen OEV-Telegramme, wird -1 zurueckgegeben, andernfalls die    */
  /* Kursnummer aus dem Telegramm Nummer <index> (erstes: 1).               */

  TelegrammListe  tl = oev_tele_nr (index);

  if (tl != NULL) {
    return tl->inhalt->kurs_nr;
  }
  else {
    return -1;
  }
} /* OEV_Tele_Kurs */

/*--------------------------------------------------------------------------*/

int  OEV_Tele_Route (unsigned index)
{
  /* Falls <index> groesser ist als die Anzahl der im aktuellen Zeitschritt */
  /* empfangenen OEV-Telegramme, wird -1 zurueckgegeben, andernfalls die    */
  /* Routennummer aus dem Telegramm Nummer <index> (erstes: 1).             */

  TelegrammListe  tl = oev_tele_nr (index);

  if (tl != NULL) {
    return tl->inhalt->routen_nr;
  }
  else {
    return -1;
  }
} /* OEV_Tele_Route */

/*--------------------------------------------------------------------------*/

int  OEV_Tele_Prio (unsigned index)
{
  /* Falls <index> groesser ist als die Anzahl der im aktuellen Zeitschritt */
  /* empfangenen OEV-Telegramme, wird -1 zurueckgegeben, andernfalls der    */
  /* Prioritaetswert aus dem Telegramm Nummer <index> (erstes: 1).          */

  TelegrammListe  tl = oev_tele_nr (index);

  if (tl != NULL) {
    return tl->inhalt->prioritaet;
  }
  else {
    return -1;
  }
} /* OEV_Tele_Prio */

/*--------------------------------------------------------------------------*/

int  OEV_Tele_Zuglaenge (unsigned index)
{
  /* Falls <index> groesser ist als die Anzahl der im aktuellen Zeitschritt */
  /* empfangenen OEV-Telegramme, wird -1 zurueckgegeben, andernfalls die    */
  /* Zuglaenge aus dem Telegramm Nummer <index> (erstes: 1).                */

  TelegrammListe  tl = oev_tele_nr (index);

  if (tl != NULL) {
    return tl->inhalt->zuglaenge;
  }
  else {
    return -1;
  }
} /* OEV_Tele_Zuglaenge */

/*--------------------------------------------------------------------------*/

int  OEV_Tele_RVH (unsigned index)
{
  /* Falls <index> groesser ist als die Anzahl der im aktuellen Zeitschritt */
  /* empfangenen OEV-Telegramme, wird -1 zurueckgegeben, andernfalls der    */
  /* Code "Richtung von Hand" aus dem Telegramm Nummer <index> (erstes: 1). */

  TelegrammListe  tl = oev_tele_nr (index);

  if (tl != NULL) {
    return tl->inhalt->richtung_von_hand;
  }
  else {
    return -1;
  }
} /* OEV_Tele_RVH */

/*--------------------------------------------------------------------------*/

int  OEV_Tele_Fahrplanlage (unsigned index)
{
  /* Falls <index> groesser ist als die Anzahl der im aktuellen Zeitschritt */
  /* empfangenen OEV-Telegramme, wird 0 zurueckgegeben, andernfalls die     */
  /* Fahrplanlage [s] aus dem Telegramm Nummer <index> (erstes: 1).         */

  TelegrammListe  tl = oev_tele_nr (index);

  if (tl != NULL) {
    return tl->inhalt->fahrpl_sek;
  }
  else {
    return 0;
  }
} /* OEV_Tele_Fahrplanlage */

/*--------------------------------------------------------------------------*/

int  OEV_Tele_Personen (unsigned index)
{
  /* Falls <index> groesser ist als die Anzahl der im aktuellen Zeitschritt */
  /* empfangenen OEV-Telegramme, wird 0 zurueckgegeben, andernfalls die     */
  /* Fahrgastanzahl aus dem Telegramm Nummer <index> (erstes: 1).           */

  TelegrammListe  tl = oev_tele_nr (index);

  if (tl != NULL) {
    return tl->inhalt->personen;
  }
  else {
    return 0;
  }
} /* OEV_Tele_Personen */

/*--------------------------------------------------------------------------*/

long  LiesEingang (long nummer)
{
  /* Gibt den am LSA-Kopplungs-Eingang Nummer <nummer> zuletzt eingegangen */
  /* Wert zurueck, bzw. -1, falls dieser Eingang nicht verkabelt ist.      */

  EingangListe  el = LSA_Puffer->AlleEingaenge;

  while (el != NULL) {
    if (el->inhalt->nummer == nummer) {
      return el->inhalt->wert;
    }
    el = el->next;
  }
  return -1;
} /* LiesEingang */

/*--------------------------------------------------------------------------*/

BOOLEAN  SetzeAusgang (long nummer, long wert)
{
  /* Setzt dem LSA-Kopplungs-Ausgang Nummer <nummer> auf <wert>. */
  /* Bei Erfolg wird TRUE zurueckgegeben, sonst FALSE            */
  /* und ein Laufzeitfehler erzeugt.                             */

  EingangListe  al = LSA_Puffer->AlleAusgaenge;
  char          prozedurname [] = "SetzeAusgang";

  while (al != NULL) {
    if (al->inhalt->nummer == nummer) {
      al->inhalt->wert = wert;
      return TRUE;
    }
    al = al->next;
  }
  al = (EingangListe) malloc (sizeof (*al));
  if (al == NULL) {
    AllgemeinerFehler (prozedurname, kein_speicher);
    return FALSE;
  }
  al->next = LSA_Puffer->AlleAusgaenge;
  LSA_Puffer->AlleAusgaenge = al;
  al->inhalt = (Eingang) malloc (sizeof (*al->inhalt));
  if (al->inhalt == NULL) {
    AllgemeinerFehler (prozedurname, kein_speicher);
    return FALSE;
  }
  al->inhalt->nummer = nummer;
  al->inhalt->wert   = wert;
  return TRUE;
} /* SetzeAusgang */

/*--------------------------------------------------------------------------*/

BOOLEAN  SetzeRoute (long rout_ent, long route, long rel_bel)
{
  /* Setzt die relative Belastung der Route <route> der */
  /* Routenentscheidung <rout_ent> auf <rel_bel>.       */
  /* Bei Erfolg wird TRUE zurueckgegeben, sonst FALSE   */
  /* und ein Laufzeitfehler erzeugt.                    */

  if (!SchreibeProtTripel (SETZE_ROUTE, rout_ent, 0)) {
    return FALSE;
  }
  if (!SchreibeProtTripel (SETZE_ROUT_BEL, route, rel_bel)) {
    return FALSE;
  }
  return TRUE;
} /* SetzeRoute */

/*--------------------------------------------------------------------------*/

BOOLEAN  SetzeVWunsch (long vwunsch_ent, long fz_kl, long vwunsch_vert)
{
  /* Ordnet in der Wunschgeschwindigkeitsentscheidung    */
  /* <vwunsch_ent>  der Fahrzeugklasse <fz_kl> die       */
  /* Wunschgeschwindigkeitsverteilung <vwunsch_vert> zu. */
  /* Bei Erfolg wird TRUE zurueckgegeben, sonst FALSE    */
  /* und ein Laufzeitfehler erzeugt.                     */

  if (!SchreibeProtTripel (SETZE_VWUNSCH, vwunsch_ent, 0)) {
    return FALSE;
  }
  if (!SchreibeProtTripel (SETZE_VWU_VERT, fz_kl, vwunsch_vert)) {
    return FALSE;
  }
  return TRUE;
} /* SetzeVWunsch */

/*--------------------------------------------------------------------------*/

BOOLEAN  LiesDebugModus (void)
{
  /* Gibt den von VISSIM uebergebenen Debug-Modus zurueck - */
  /* wenn nichts uebergeben wurde, FALSE.                   */

  return LSA_Puffer->DebugModus;
} /* LiesDebugModus */

/*--------------------------------------------------------------------------*/

SignalBild  SigGrBild (int nr)
{
  /* Falls die Signalgruppe Nr. <nr> in VISSIM nicht definiert ist,    */
  /* erfolgt eine Laufzeitfehlermeldung (Rueckgabewert <undefiniert>). */
  /* Ansonsten wird das aktuelle Signalbild der Signalgruppe Nummer    */
  /* <nr> zurueckgegeben.                                              */

  char  prozedurname[] = "LSA_PUFF.SigGrBild";
  int   index = sg_nr_to_index (nr, prozedurname, false);

  if (index < 0) {
    return undefiniert;
  }
  assert (LSA_Puffer->SigGr[index] != NULL);
  return LSA_Puffer->SigGr[index]->Bild;
} /* SigGrBild */

/*--------------------------------------------------------------------------*/

SignalBild  SigGrSoll (int nr)
{
  /* Falls die Signalgruppe Nr. <nr> in VISSIM nicht definiert ist,    */
  /* erfolgt eine Laufzeitfehlermeldung (Rueckgabewert <undefiniert>). */
  /* Ansonsten wird das letzte von der Steuerung gesetzte Sollbild     */
  /* der Signalgruppe Nummer <nr> zurueckgegeben.                      */

  char  prozedurname[] = "LSA_PUFF.SigGrSoll";
  int   index = sg_nr_to_index (nr, prozedurname, false);

  if (index < 0) {
    return undefiniert;
  }
  assert (LSA_Puffer->SigGr[index] != NULL);
  if (LSA_Puffer->SigGr[index]->Soll == undefiniert) {
    return LSA_Puffer->SigGr[index]->AltSoll;
  }
  else {
    return LSA_Puffer->SigGr[nr]->Soll;
  }
} /* SigGrSoll */

/*--------------------------------------------------------------------------*/

void  SetzeSchaltfrequenz (unsigned freq)
{
  /* Setzt die Schaltfrequenz der LSA auf <freq> [1..10] Durchlaeufe pro   */
  /* Sekunde.                                                              */
  /* Dieser Wert wird nach der Initialisierung der Steuerung an VISSIM     */
  /* uebergeben. Die Funktion muss also schon waehrend der Initialisierung */
  /* aufgerufen werden, wenn die Schaltfrequenz hoeher als der             */
  /* Standardwert 1 ist.                                                   */

  if (freq >= 1 && freq <= 10) {
    LSA_Puffer->Schaltfrequenz = freq;
  }
} /* SetzeSchaltfrequenz */

/*--------------------------------------------------------------------------*/

BOOLEAN  SetStopTimeDist (long stop, long fz_kl, long vwunsch_vert)
{
  /* Ordnet im Stopschild <stop> der Fahrzeugklasse <fz_kl>  */
  /* die Wunschgeschwindigkeitsverteilung <vwunsch_vert> zu. */
  /* Bei Erfolg wird TRUE zurueckgegeben, sonst FALSE        */
  /* und ein Laufzeitfehler erzeugt.                         */
  /*                                                         */
  /* Assigns the desired speed distribution <vwunsch_vert>   */
  /* to the vehicle class <fz_kl> in the stop sign <stop>.   */
  /* If that doesn't work, a runtime error occurs (return    */
  /* value FALSE). Otherwise the return value is TRUE.       */

  if (!SchreibeProtTripel (SETZE_STOP, stop, 0)) {
    return FALSE;
  }
  if (!SchreibeProtTripel (SETZE_STOP_ZVERT, fz_kl, vwunsch_vert)) {
    return FALSE;
  }
  return TRUE;
} /* SetStopTimeDist */

/*==========================================================================*/
/*  Ende von LSA_PUFF.CPP                                                   */
/*==========================================================================*/
