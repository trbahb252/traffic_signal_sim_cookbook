/*==========================================================================*/
/*  DriverModel.cpp                                  DLL Module for VISSIM  */
/*                                                                          */
/*  Interface module for external driver models.                            */
/*  Example version with a very simple driver model.                        */
/*                                                                          */
/*  Version of 2012-08-28                                   Lukas Kautzsch  */
/*==========================================================================*/

#include "DriverModel.h"

/*==========================================================================*/

double  current_speed        = 0.0;
double  desired_acceleration = 0.0;
double  desired_lane_angle   = 0.0;
long    active_lane_change   = 0;
long    rel_target_lane      = 0;
double  desired_velocity     = 0.0;
long    turning_indicator    = 0;
long    vehicle_color        = RGB(0,0,0);
double  lead_vehicle_distance         = 999.0;
double  lead_vehicle_speed_difference = -99.0;
double  lead_vehicle_length           =   0.0;

/*==========================================================================*/

BOOL APIENTRY DllMain (HANDLE  hModule,
                       DWORD   ul_reason_for_call,
                       LPVOID  lpReserved)
{
  switch (ul_reason_for_call) {
      case DLL_PROCESS_ATTACH:
      case DLL_THREAD_ATTACH:
      case DLL_THREAD_DETACH:
      case DLL_PROCESS_DETACH:
         break;
  }
  return TRUE;
}

/*==========================================================================*/

DRIVERMODEL_API  int  DriverModelSetValue (long   type,
                                           long   index1,
                                           long   index2,
                                           long   long_value,
                                           double double_value,
                                           char   *string_value)
{
  /* Sets the value of a data object of type <type>, selected by <index1> */
  /* and possibly <index2>, to <long_value>, <double_value> or            */
  /* <*string_value> (object and value selection depending on <type>).    */
  /* Return value is 1 on success, otherwise 0.                           */

  switch (type) {
    case DRIVER_DATA_PATH                   :
    case DRIVER_DATA_TIMESTEP               :
    case DRIVER_DATA_TIME                   :
      return 1;
    case DRIVER_DATA_VEH_ID                 :
      /* reset leading vehicle's data for this new vehicle */
      lead_vehicle_distance         = 999.0;
      lead_vehicle_speed_difference = -99.0;
      lead_vehicle_length           =   0.0;
      return 1;
    case DRIVER_DATA_VEH_LANE               :
    case DRIVER_DATA_VEH_ODOMETER           :
    case DRIVER_DATA_VEH_LANE_ANGLE         :
    case DRIVER_DATA_VEH_LATERAL_POSITION   :
      return 1;
    case DRIVER_DATA_VEH_VELOCITY           :
      current_speed = double_value;
      return 1;
    case DRIVER_DATA_VEH_ACCELERATION       :
    case DRIVER_DATA_VEH_LENGTH             :
    case DRIVER_DATA_VEH_WIDTH              :
    case DRIVER_DATA_VEH_WEIGHT             :
    case DRIVER_DATA_VEH_MAX_ACCELERATION   :
      return 1;
    case DRIVER_DATA_VEH_TURNING_INDICATOR  :
      turning_indicator = long_value;
      return 1;
    case DRIVER_DATA_VEH_CATEGORY           :
    case DRIVER_DATA_VEH_PREFERRED_REL_LANE :
    case DRIVER_DATA_VEH_USE_PREFERRED_LANE :
      return 1;
    case DRIVER_DATA_VEH_DESIRED_VELOCITY   :
      desired_velocity = double_value;
      return 1;
    case DRIVER_DATA_VEH_X_COORDINATE       :
    case DRIVER_DATA_VEH_Y_COORDINATE       :
    case DRIVER_DATA_VEH_TYPE               :
      return 1;
    case DRIVER_DATA_VEH_COLOR              :
      vehicle_color = long_value;
      return 1;
    case DRIVER_DATA_VEH_CURRENT_LINK       :
      return 0; /* (To avoid getting sent lots of DRIVER_DATA_VEH_NEXT_LINKS messages) */
                /* Must return 1 if these messages are to be sent from VISSIM!         */
    case DRIVER_DATA_VEH_NEXT_LINKS         :
    case DRIVER_DATA_VEH_ACTIVE_LANE_CHANGE :
    case DRIVER_DATA_VEH_REL_TARGET_LANE    :
    case DRIVER_DATA_NVEH_ID                :
    case DRIVER_DATA_NVEH_LANE_ANGLE        :
    case DRIVER_DATA_NVEH_LATERAL_POSITION  :
      return 1;
    case DRIVER_DATA_NVEH_DISTANCE          :
      if (index1 == 0 && index2 == 1) { /* leading vehicle on own lane */
        lead_vehicle_distance = double_value;
      }
      return 1;
    case DRIVER_DATA_NVEH_REL_VELOCITY      :
      if (index1 == 0 && index2 == 1) { /* leading vehicle on own lane */
        lead_vehicle_speed_difference = double_value;
      }
      return 1;
    case DRIVER_DATA_NVEH_ACCELERATION      :
      return 1;
    case DRIVER_DATA_NVEH_LENGTH            :
      if (index1 == 0 && index2 == 1) { /* leading vehicle on own lane */
        lead_vehicle_length = double_value;
      }
      return 1;
    case DRIVER_DATA_NVEH_WIDTH             :
    case DRIVER_DATA_NVEH_WEIGHT            :
    case DRIVER_DATA_NVEH_TURNING_INDICATOR :
    case DRIVER_DATA_NVEH_CATEGORY          :
    case DRIVER_DATA_NVEH_LANE_CHANGE       :
    case DRIVER_DATA_NO_OF_LANES            :
    case DRIVER_DATA_LANE_WIDTH             :
    case DRIVER_DATA_LANE_END_DISTANCE      :
    case DRIVER_DATA_RADIUS                 :
    case DRIVER_DATA_MIN_RADIUS             :
    case DRIVER_DATA_DIST_TO_MIN_RADIUS     :
    case DRIVER_DATA_SLOPE                  :
    case DRIVER_DATA_SLOPE_AHEAD            :
    case DRIVER_DATA_SIGNAL_DISTANCE        :
    case DRIVER_DATA_SIGNAL_STATE           :
    case DRIVER_DATA_SIGNAL_STATE_START     :
    case DRIVER_DATA_SPEED_LIMIT_DISTANCE   :
    case DRIVER_DATA_SPEED_LIMIT_VALUE      :
      return 1;
    case DRIVER_DATA_DESIRED_ACCELERATION :
      desired_acceleration = double_value;
      return 1;
    case DRIVER_DATA_DESIRED_LANE_ANGLE :
      desired_lane_angle = double_value;
      return 1;
    case DRIVER_DATA_ACTIVE_LANE_CHANGE :
      active_lane_change = long_value;
      return 1;
    case DRIVER_DATA_REL_TARGET_LANE :
      rel_target_lane = long_value;
      return 1;
    default :
      return 0;
  }
}

/*--------------------------------------------------------------------------*/

DRIVERMODEL_API  int  DriverModelGetValue (long   type,
                                           long   index1,
                                           long   index2,
                                           long   *long_value,
                                           double *double_value,
                                           char   **string_value)
{
  /* Gets the value of a data object of type <type>, selected by <index1> */
  /* and possibly <index2>, and writes that value to <*double_value>,     */
  /* <*float_value> or <**string_value> (object and value selection       */
  /* depending on <type>).                                                */
  /* Return value is 1 on success, otherwise 0.                           */

  switch (type) {
    case DRIVER_DATA_STATUS :
      *long_value = 0;
      return 1;
    case DRIVER_DATA_VEH_TURNING_INDICATOR :
      *long_value = turning_indicator;
      return 1;
    case DRIVER_DATA_VEH_DESIRED_VELOCITY   :
      *double_value = desired_velocity;
      return 1;
    case DRIVER_DATA_VEH_COLOR :
      *long_value = vehicle_color;
      return 1;
    case DRIVER_DATA_WANTS_SUGGESTION :
      *long_value = 1;
      return 1;
    case DRIVER_DATA_DESIRED_ACCELERATION : {
      /* ### start commenting out from here for "do nothing" */
      double net_distance       = lead_vehicle_distance - lead_vehicle_length;
      double lead_vehicle_speed = current_speed - lead_vehicle_speed_difference;
      double desired_distance   = lead_vehicle_speed; /* times 1 s */
      if (lead_vehicle_speed_difference > 0) {
        /* we are faster than the leading vehicle */
        if (lead_vehicle_speed > 0) {
          if (net_distance > desired_distance) {
            /* slow down to leading vehicle's speed with 1 s time gap */
            desired_acceleration = - lead_vehicle_speed_difference
                                   * lead_vehicle_speed_difference
                                   / (net_distance - desired_distance)
                                   / 2.0;
          }
          else {
            /* try to increase distance */
            desired_acceleration = - lead_vehicle_speed_difference - 1.0;
          }
        } /* if (lead_vehicle_speed > 0) */
        else {
          /* leading vehicle is standing still */
          if (net_distance < 2.1) {
            desired_acceleration = -9.9; /* emergency braking */
          }
          else {
            /* brake to standstill in 2.0 m distance */
            desired_acceleration = - lead_vehicle_speed_difference
                                   * lead_vehicle_speed_difference
                                   / (net_distance - 2.0)
                                   / 2.0;
          }
        }
      } /* if (lead_vehicle_speed_difference > 0) */
      else {
        /* accelerate to min of leading vehicle's speed and own desired speed */
        double new_speed = desired_velocity;
        if (lead_vehicle_speed < desired_velocity) {
          new_speed = lead_vehicle_speed;
        }
        desired_acceleration = new_speed - current_speed;
      }
      /* ### comment out until here for "do nothing" */
      *double_value = desired_acceleration;
      return 1;
    }
    case DRIVER_DATA_DESIRED_LANE_ANGLE :
      *double_value = desired_lane_angle;
      return 1;
    case DRIVER_DATA_ACTIVE_LANE_CHANGE :
      *long_value = active_lane_change;
      return 1;
    case DRIVER_DATA_REL_TARGET_LANE :
      *long_value = rel_target_lane;
      return 1;
    case DRIVER_DATA_SIMPLE_LANECHANGE :
      *long_value = 1;
      return 1;
    default :
      return 0;
  }
}

/*==========================================================================*/

DRIVERMODEL_API  int  DriverModelExecuteCommand (long number)
{
  /* Executes the command <number> if that is available in the driver */
  /* module. Return value is 1 on success, otherwise 0.               */

  switch (number) {
    case DRIVER_COMMAND_INIT :
      return 1;
    case DRIVER_COMMAND_CREATE_DRIVER :
      return 1;
    case DRIVER_COMMAND_KILL_DRIVER :
      return 1;
    case DRIVER_COMMAND_MOVE_DRIVER :
      return 1;
    default :
      return 0;
  }
}

/*==========================================================================*/
/*  Ende of DriverModel.cpp                                                 */
/*==========================================================================*/
