/*==========================================================================*/
/*  SignalGUI.h                                      DLL Module for VISSIM  */
/*                                                                          */
/*  Interface module for GUI elements for signal controllers.               */
/*                                                                          */
/*  Version of 2004-05-26                                   Lukas Kautzsch  */
/*==========================================================================*/

#ifndef __SIGNALGUI_H
#define __SIGNALGUI_H


/* In the creation of SignalGUI.DLL all files must be compiled */
/* with the preprocessor definition SIGNALGUI_EXPORTS.         */
/* Programs that use SignalGUI.DLL must not be compiled        */
/* with that preprocessor definition.                          */

#ifdef SIGNALGUI_EXPORTS
#define SIGNALGUI_API extern "C" __declspec(dllexport)
#else
#define SIGNALGUI_API extern "C" __declspec(dllimport)
#endif

/*==========================================================================*/

/* general data: */
#define  SIGNALGUI_DATA_INP_FILE                101
           /* string: *.inp file name prefix */
#define  SIGNALGUI_DATA_INP_PATH                102
           /* string: absolute path of current working directory (where *.inp file is) */
#define  SIGNALGUI_DATA_NEMA_LEVEL              111
           /* long: NEMA level in VISSIM license */
#define  SIGNALGUI_DATA_DEMO_VERSION            112
           /* long: 1 = demo version, 0 = full version */
#define  SIGNALGUI_DATA_SIM_RUN_ACTIVE          113
           /* long: 1 = simulation is currently running, 0 = currently no simulation run */
#define  SIGNALGUI_DATA_EDITOR_TYPE             114
           /* long: 1 = use simple editor, 0 = use standard editor   */
           /* VISUM makes a SetValue call before nema editor is      */
           /* started and always sets this to 1=use simple editor.   */

/* global signal controller data: */
/* (index1 = data file number [1..2]) */
#define  SIGNALGUI_DATA_DATA_FILE               201
           /* string: data file name (including .extension) */

#define  SIGNALGUI_DATA_DATA_FILE_CHANGED       203
           /* long: 1 = file was changed, 0 = file was not changed  */
           /* (index1 = data file number [1..2])                    */

#define  SIGNALGUI_DATA_CYCLE_TIME              202
           /* double: cycle time [s] (0.0 = variable) */

#define  SIGNALGUI_DATA_OFFSET                  204
           /* double: offset [s] */
           /* Used in VISUM if user changes Nema controller to signalgroup-based controller. */

#define  SIGNALGUI_DATA_COORDINATED             205
         /* long: 1 = controller is coordinated, 0 = not coordinated */
         /* VISUM makes a GetValue call after nema editor returns.   */

#define  SIGNALGUI_CONTROLTYPE_NONE     0
#define  SIGNALGUI_CONTROLTYPE_FIXED    1
#define  SIGNALGUI_CONTROLTYPE_ACTUATED 2
#define  SIGNALGUI_CONTROLTYPE_SEMIACT  3
         /* return values for SIGNALGUI_DATA_CONTROLTYPE */

#define  SIGNALGUI_DATA_CONTROLTYPE             206
         /* long: 0..3, see above                                  */
         /* VISUM makes a GetValue call after nema editor returns. */

#define  SIGNALGUI_DATA_STARTUPLOSTTIME         207
         /* long: time lost at the beginning due to driver reaction time */
         /* VISUM makes a GetValue call after nema editor returns.       */

/*--------------------------------------------------------------------------*/

/* signal group data: */
/* (index1 = signal group number) */
#define  SIGNALGUI_DATA_SG_NUMBER               301
           /* long: signal group number (== index1) */
#define  SIGNALGUI_DATA_SG_NAME                 302
           /* string: signal group name */
#define  SIGNALGUI_DATA_SG_T_AMBER              311
           /* double: amber time [s] */
#define  SIGNALGUI_DATA_SG_T_RED_AMBER          312
           /* double: red/amber time [s] */
#define  SIGNALGUI_DATA_SG_T_MIN_GREEN          313
           /* double: minimum green time [s] */
#define  SIGNALGUI_DATA_SG_T_MIN_RED            314
           /* double: minimum red time [s] */
#define  SIGNALGUI_DATA_SG_T_REDCLEARANCE       315
           /* double: red clearance time [s] */
#define  SIGNALGUI_DATA_SG_T_GREEN              316
           /* double: green time [s] */
#define  SIGNALGUI_DATA_SG_DETECTOR_NUMBER      317
           /* long: detector number, 0=no detector */

#define  SIGNALGUI_DATA_SG_CHANGED_NUMBER       321
           /* long: new signal group number (0 = to be deleted) */
           /* (index1 = old signal group number)                */
/* (index1 = irrelevant) */
#define  SIGNALGUI_DATA_NO_OF_NEW_GROUPS        331
           /* long: number of signal groups to be created */
/* (index1 = new signal group index [0..SIGNALGUI_DATA_NO_OF_NEW_GROUPS-1]) */
#define  SIGNALGUI_DATA_NEW_SG_NUMBER           332
           /* long: signal group number */

/*--------------------------------------------------------------------------*/

/* directions */
#define  SIGNALGUI_DIRECTION_NONE       0
#define  SIGNALGUI_DIRECTION_NORTH      1
#define  SIGNALGUI_DIRECTION_NORTHEAST  2
#define  SIGNALGUI_DIRECTION_EAST       3
#define  SIGNALGUI_DIRECTION_SOUTHEAST  4
#define  SIGNALGUI_DIRECTION_SOUTH      5
#define  SIGNALGUI_DIRECTION_SOUTHWEST  6
#define  SIGNALGUI_DIRECTION_WEST       7
#define  SIGNALGUI_DIRECTION_NORTHWEST  8


#define SIGNALGUI_DATA_APPROACH_RIGHTTURNOVERLAP     401
         /* index1: approach direction as defined SIGNALGUI_DIRECTION_XXX */
         /* long:   1 = RightTurnOverlap on, 0 = RightTurnOverlap off     */
         /* VISUM makes a GetValue call for the enabled approaches after  */
         /* nema editor returns.                                          */

#define SIGNALGUI_SIGNALTYPE_NONE         0
#define	SIGNALGUI_SIGNALTYPE_PROTECTED    1
#define	SIGNALGUI_SIGNALTYPE_SPLIT_PHASE  2
#define	SIGNALGUI_SIGNALTYPE_PERMITTED    3
#define	SIGNALGUI_SIGNALTYPE_PROTPERM     4
#define	SIGNALGUI_SIGNALTYPE_PERMPROT     5


#define SIGNALGUI_DATA_APPROACH_SIGNALTYPE           402
         /* index1: approach direction as defined SIGNALGUI_DIRECTION_XXX */
         /* long:   control type as defined in SIGNALGUI_SIGNALTYPE_XXX   */
         /* VISUM makes a GetValue call for the enabled approaches after  */
         /* nema editor returns.                                          */


#define SIGNALGUI_DATA_MOVEMENT_SIGNALGROUP          403
         /* index1: from-direction as defined SIGNALGUI_DIRECTION_XXX    */
         /* index2: to-direction as defined SIGNALGUI_DIRECTION_XXX      */
         /* long:   signalgroup number, 0 = no signalgroup               */
         /* VISUM makes a GetValue call for all movements after          */
         /* nema editor returns.                                         */


#define SIGNALGUI_DATA_APPROACH_PED_SIGNALGROUP      404
         /* index1: approach direction as defined SIGNALGUI_DIRECTION_XXX */
         /* long:   signalgroup number of the pedestrian crosswalk.       */
         /*         0 = no pedestrian walk on approach                    */
         /* VISUM makes a GetValue call for the enabled approaches after  */
         /* nema editor returns.                                          */

/*--------------------------------------------------------------------------*/

SIGNALGUI_API  int  SignalGUISetValue (long   type,
                                       long   sc_no,
                                       long   index1,
                                       long   index2,
                                       long   long_value,
                                       double double_value,
                                       char   *string_value);

/* Called from VISSIM to set the value of a data object of type <type> */
/* (see definition of type numbers above) of controller no. <sc_no>,   */
/* selected by <index1> and possibly <index2>, to <long_value>,        */
/* <double_value> or <*string_value> (object and value selection       */
/* depending on <type>).                                               */
/* Return value is 1 on success, otherwise 0.                          */

/*--------------------------------------------------------------------------*/

SIGNALGUI_API  int  SignalGUIGetValue (long   type,
                                       long   sc_no,
                                       long   index1,
                                       long   index2,
                                       long   *long_value,
                                       double *double_value,
                                       char   *string_value);

/* Called from VISSIM to get the value of a data object of type <type>  */
/* (see definition of type numbers above) of controller no. <sc_no>,    */
/* selected by <index1> and possibly <index2>.                          */
/* This function writes that value to <*double_value>, <*float_value>   */
/* or <*string_value> (object and value selection depending on <type>). */
/* Return value is 1 on success, otherwise 0.                           */

/*==========================================================================*/

#define  SIGNALGUI_COMMAND_INIT_CONTROLLER   0
           /* Called from VISSIM before any parameters for a        */
           /* subsequent command call are sent.                     */
           /* Values set before: ---                                */
           /* Values got after:  ---                                */

#define  SIGNALGUI_COMMAND_READ_DATA_FILES      1
           /* Called from VISSIM before any signal control data     */
           /* are to be edited/passed.                              */
           /* Values set before: SIGNALGUI_DATA_INP_FILE            */
           /*                      (can be used for default naming  */
           /*                       if data file names are empty)   */
           /*                    SIGNALGUI_DATA_INP_PATH            */
           /*                    SIGNALGUI_DATA_DATA_FILE [1..2]    */
           /* Values got after:  ---                                */

#define  SIGNALGUI_COMMAND_EDIT_SC_PARAMETERS   2
           /* Called from VISSIM after click on "Edit Parameters"        */
           /* in the External Controller Attributes dialog box.          */
           /* Values set before: For each existing signal group:         */
           /*                      SIGNALGUI_DATA_SG_NUMBER              */
           /*                      SIGNALGUI_DATA_SG_NAME                */
           /*                    If SC type is NEMA:                     */
           /*                      SIGNALGUI_DATA_NEMA_LEVEL             */
           /*                    SIGNALGUI_DATA_DEMO_VERSION             */
           /*                    SIGNALGUI_DATA_SIM_RUN_ACTIVE           */
           /*                    SIGNALGUI_DATA_EDITOR_TYPE (only VISUM) */
           /* Values got after:  SIGNALGUI_DATA_CYCLE_TIME               */
           /*                    SIGNALGUI_DATA_OFFSET (only VISUM)      */
           /*                    SIGNALGUI_DATA_DATA_FILE [1..2]         */
           /*                    SIGNALGUI_DATA_DATA_FILE_CHANGED [1..2] (only VISUM)*/
           /*                    SIGNALGUI_DATA_COORDINATED (only VISUM) */
           /*                    SIGNALGUI_DATA_CONTROLTYPE  (only VISUM)*/
           /*                    SIGNALGUI_DATA_STARTUPLOSTTIME (only VISUM)*/
           /*                    For each (existing) signal group:       */
           /*                      SIGNALGUI_DATA_SG_CHANGED_NUMBER      */
           /*                    SIGNALGUI_DATA_NO_OF_NEW_GROUPS         */
           /*                    For each new signal group:              */
           /*                      SIGNALGUI_DATA_NEW_SG_NUMBER          */
           /*                    For each signal group (after changes):  */
           /*                      SIGNALGUI_DATA_SG_NAME                */
           /*       only VISUM: For each signal group (after changes):   */
           /*                   SIGNALGUI_DATA_SG_T_MIN_GREEN            */
           /*                   SIGNALGUI_DATA_SG_T_GREEN                */
           /*                   SIGNALGUI_DATA_SG_DETECTOR_NUMBER        */
           /*                   For each approach:                       */
           /*                   SIGNALGUI_DATA_APPROACH_RIGHTTURNOVERLAP */
           /*                   SIGNALGUI_DATA_APPROACH_SIGNALTYPE       */
           /*                   SIGNALGUI_DATA_APPROACH_PED_SIGNALGROUP  */
           /*                   For each movement:                       */
           /*                   SIGNALGUI_DATA_MOVEMENT_SIGNALGROUP      */

#define  SIGNALGUI_COMMAND_EDIT_SG_PARAMETERS   3
           /* Called from VISSIM after click on "Signal Groups"          */
           /* in the Signal Controller main dialog box.                  */
           /* Values set before: For each existing signal group:         */
           /*                      SIGNALGUI_DATA_SG_NUMBER              */
           /*                      SIGNALGUI_DATA_SG_NAME                */
           /*                    If SC type is NEMA:                     */
           /*                      SIGNALGUI_DATA_NEMA_LEVEL             */
           /*                    SIGNALGUI_DATA_DEMO_VERSION             */
           /*                    SIGNALGUI_DATA_SIM_RUN_ACTIVE           */
           /*                    SIGNALGUI_DATA_EDITOR_TYPE (only VISUM) */
           /* Values got after:  SIGNALGUI_DATA_CYCLE_TIME               */
           /*                    SIGNALGUI_DATA_OFFSET (only VISUM)      */
           /*                    SIGNALGUI_DATA_DATA_FILE [1..2]         */
           /*                    SIGNALGUI_DATA_DATA_FILE_CHANGED [1..2] (only VISUM)*/
           /*                    SIGNALGUI_DATA_COORDINATED (only VISUM) */
           /*                    SIGNALGUI_DATA_CONTROLTYPE  (only VISUM)*/
           /*                    SIGNALGUI_DATA_STARTUPLOSTTIME (only VISUM)*/
           /*                    For each (existing) signal group:       */
           /*                      SIGNALGUI_DATA_SG_CHANGED_NUMBER      */
           /*                    SIGNALGUI_DATA_NO_OF_NEW_GROUPS         */
           /*                    For each new signal group:              */
           /*                      SIGNALGUI_DATA_NEW_SG_NUMBER          */
           /*                    For each signal group (after changes):  */
           /*                      SIGNALGUI_DATA_SG_NAME                */
           /*       only VISUM: For each signal group (after changes):   */
           /*                   SIGNALGUI_DATA_SG_T_MIN_GREEN            */
           /*                   SIGNALGUI_DATA_SG_T_GREEN                */
           /*                   SIGNALGUI_DATA_SG_DETECTOR_NUMBER        */
           /*                   For each approach:                       */
           /*                   SIGNALGUI_DATA_APPROACH_RIGHTTURNOVERLAP */
           /*                   SIGNALGUI_DATA_APPROACH_SIGNALTYPE       */
           /*                   SIGNALGUI_DATA_APPROACH_PED_SIGNALGROUP  */
           /*                   For each movement:                       */
           /*                   SIGNALGUI_DATA_MOVEMENT_SIGNALGROUP      */

#define  SIGNALGUI_COMMAND_CHECK_SG_NUMBERS     4
           /* Called from VISSIM before a simulation run            */
           /* Values set before: For each existing signal group:    */
           /*                      SIGNALGUI_DATA_SG_NUMBER         */
           /*                      SIGNALGUI_DATA_SG_NAME           */
           /*                    If SC type is NEMA:                */
           /*                      SIGNALGUI_DATA_NEMA_LEVEL        */
           /*                    SIGNALGUI_DATA_DEMO_VERSION        */
           /*                    SIGNALGUI_DATA_SIM_RUN_ACTIVE      */
           /* Values got after:  SIGNALGUI_DATA_CYCLE_TIME          */
           /*                    For each signal group:             */
           /*                      SIGNALGUI_DATA_SG_NAME           */
           /*                      SIGNALGUI_DATA_SG_T_AMBER        */
           /*                      SIGNALGUI_DATA_SG_T_RED_AMBER    */
           /*                      SIGNALGUI_DATA_SG_T_MIN_GREEN    */
           /*                      SIGNALGUI_DATA_SG_T_MIN_RED      */
           /*                      SIGNALGUI_DATA_SG_T_REDCLEARANCE */

#define  SIGNALGUI_COMMAND_SAVE_CHANGED_VALUES   10
           /* Called from VISUM after Traffix optimize.             */
           /* Values set before: SIGNALGUI_DATA_CYCLE_TIME          */
           /*                    For each existing signal group:    */
           /*                      SIGNALGUI_DATA_SG_T_GREEN        */

/*--------------------------------------------------------------------------*/

SIGNALGUI_API  int  SignalGUIExecuteCommand (long number, long sc_no);

/* Called from VISSIM to execute the command <number> if that is */
/* available in the DLL (see definitions above) for controller   */
/* no. <sc_no>.                                                  */
/* Return value is 1 on success, otherwise 0.                    */


/*==========================================================================*/

#endif /* __SIGNALGUI_H */

/*==========================================================================*/
/*  Ende of SignalGUI.h                                                     */
/*==========================================================================*/
