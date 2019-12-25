/*NOTICE:  Copyright 2014 Arizona Board of Regents on behalf of University of Arizona.
 * All information, intellectual, and technical concepts contained herein is and shall
 * remain the proprietary information of Arizona Board of Regents and may be covered
 * by U.S. and Foreign Patents, and patents in process.  Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written permission
 * is obtained from Arizona Board of Regents or University of Arizona.
 */   



/*  ConnectedVehicle.cpp  
*  Created by Yiheng Feng 
*  University of Arizona   
*  ATLAS Research Center 
*  College of Engineering
*
*  This code was develop under the supervision of Professor Larry Head
*  in the ATLAS Research Center.

*/

#include "PositionLocal3D.h"
#include <vector>

class ConnectedVehicle
{
public:
	int TempID; //ID, temp, since it is changing every 5 mins
	double Speed[500];  //Vehicle current Speed
	int desiredPhase ; //traffic signal phase associated with a lane (MAP + phase association)
	double stopBarDistance[500] ; //distance to the stop bar from current location (MAP)
	double estArrivalTime ; // estimated arrival time at the stop bar given current speed
	PositionLocal3D traj[500]; //store vehicle trajectory, totally store 500s trajectory
	int nFrame;  //store how many frames are recorded.
	double heading[500]; //vehicle heading
	double N_Offset[500];  //offset to the center of the intersection
	double E_Offset[500];  //in cm
	int Dsecond;
	int pre_requested_phase;
	int Phase_Request_Counter;
	int req_phase;  //current requested phase
	double ETA;        //estimated travel time
	double time[500];       //Absolute time that the trajectory information is received
	int active_flag;   //a flag to indicate whether this vehicle is still in the network:
	                   //every time the vehicle send BSM to RSE, the flag is reset to 5
					   //every time COP starts the flag is reduced by 1
					   //if the flag is <0, delete the vehicle from the list
	double receive_timer;  //this is the time to set the frequency of receiving to reduce computational effort
	int approach;  //the current approach no
	int previous_approach; //stores the previous approach no of the vehicle
	int temp_approach; //Shows the approach of previous time point
	int lane;      //the lane number
	int previous_lane; //stores the previous lane no of the vehicle
	int temp_lane; //Shows the lane of previous time point
	double acceleration;  
	double time_stop;    //time point the vehicle first stopped
	double distance_stop; //distance to the stop bar when the stopped for the first time
	int stop_flag;      //flag that indicates whether the vehicle has stopped or not;  0: not stop;  1: stopped
	int queue_flag;		//flag that indicates whether the vehicle is in the queue or not; 0:not in queue; 1:in queue -------> mainly for Performance Observer
	int processed;      //flag indicating that if the vehicle already go through FindVehInMAP function, only vehicle be processed can be sent to other components
	double entry_time;     //entry time of the vehicle first in control range 
	int first_onmap_time;   //nframe that the vheicle is first processed on map
	double leaving_time;   //the last time point before the vehicle leaves the DSRC range, or the current time if the vehicle is still in range
	int states;          //vehicle states: 0: within the intersection 1: approaching 2: leaving 3: in queue 4: not on the map
	int appr_states;     //vehicle approaching states  1:approaching, never stopped; 2:vehicle in queue; 3: vehicle leave queue
//	-----------------Methods-------------------------


};
