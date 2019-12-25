#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <unistd.h>
                         
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <sstream>
#include <istream>
#include "math.h"
#include "time.h"
#include <time.h>
#include <vector>
#include <sys/time.h>

#include "LinkedList.h"
#include "NMAP.h"
#include "geoCoord.h"
#include "BasicVehicle.h"
#include "ListHandle.h"
#include "ConnectedVehicle.h"
#include "Intersection.h"


#ifndef byte
    #define byte  char  // needed if byte not defined
#endif

#ifndef DEG2ASNunits
    #define DEG2ASNunits  (1000000.0)  // used for ASN 1/10 MICRO deg to unit converts
#endif

#define PI 3.1415926
#define PORT_FROM_ACA 33333      //port number for adaptive control algorithm


using namespace std;

//Global Variable
char tmp_log[512];
char buf[256];
BasicVehicle vehIn;

char pack_buf[1000000];

char CENTRAL_SERVER_ADDR[64]="10.254.56.5";
char LOCAL_HOST_ADDR[64]="127.0.0.1";

float ffspeed=13.89;     //m/s, equal to 50km/s

int inter_id;  //intersection id

float entry_location=200.0;    //Vehicle entry location, defined as vehicle enters the delay calculation range

char logfilename[256] = "/home/yiheng/nojournal/bin/log/CV_Trajectory_Aware";

LinkedList <ConnectedVehicle> trackedveh;   //A list to store the active connected vehicle list

//map related variables
NMAP Newmap;
vector<N_LaneNodes> MAP_Nodes;
string nmap_name;
char MAP_File_Name[64]  = "/home/yiheng/nojournal/bin/nmap_name.txt";
char Inter_conf_file[64] = "/home/yiheng/nojournal/bin/Intersection_Configure.txt";
Intersection Inter;  //intersection includes the map nodes and other intersection attributes

int msleep(unsigned long milisec);
double GetSeconds();
int Pack_BSM_Msg();  //pack the received BSMs since last sent time to a string, return the number of BSM messages packed
void xTimeStamp( char * pc_TimeStamp_ );
void get_map_name(); //Yiheng add 07/18/2014
int  outputlog(char *output);
void Reload_MapNodes();
int Read_Intersection_Config(char *filename);
int Geofencing(double heading, double N_Offset, double E_Offset);  //Geofence the point to see whether it is in the range of the road   return 0: vehicle on map; 1: within intersection 2: not on map
void DistanceFromLine(double cx, double cy, double ax, double ay, double bx, double by, double &distanceSegment, double &distanceLine);
void LocateVehOnMap(double speed, double heading, double N_offset, double E_offset, double &Dis_cur, double &ETA, int &approach, int &lane, int &req_phase, int &state);
int Pack_Traj_Data(byte* tmp_traj_data);


int main ( int argc, char* argv[] )
{
	
    //Struct for UDP socket timeout: 0.5s
	struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 500000;
	
	int flag_sim;  //indicate whehter the bsm is from simulation or cohda rse 1: cohda rse; 0: simulation
	
	if (argc>=2)	
	{		
		sscanf(argv[1],"%d",&flag_sim);
		sscanf(argv[2],"%s",CENTRAL_SERVER_ADDR);
	}
	
	float speed_coif;  //speed coifficient
	
	int port;
	if (flag_sim==1) //from Cohda
	{
		port=3333;  //port to receive BSM043
		speed_coif=1.0;
	}
	if (flag_sim==0) //from simulation
	{
		port=30000;  //port to receive BSM
		speed_coif=1.0;
	}
	if (flag_sim==2) //from Savari
	{
		port=3333;  //port to receive BSM
		speed_coif=3.6;
	}
	
	//create logs
	char timestamp[128];
    xTimeStamp(timestamp);
    strcat(logfilename,timestamp);    
    strcat(logfilename,".log");
    printf("%s\n",logfilename);
	fstream fs_log;
	fs_log.open(logfilename,fstream::out | fstream::trunc);
	
	//Read intersection map and save corresponding information
	Newmap.ID=1;
	Newmap.Version=1;
	// Parse the MAP file
	get_map_name();
	char mapname[128];
	sprintf(mapname,"%s",nmap_name.c_str());
	printf("%s",mapname);
	//NewMap.ParseIntersection(mapname);
	Newmap.ParseIntersection_XML(mapname);
	sprintf(tmp_log,"Read the map successfully At (%ld).\n",time(NULL));
	outputlog(tmp_log); cout<<tmp_log;
	//Read intersection configuration file for intersection parameters: id, lane_no, size, lane_phase_mapping, dsrc_range
	Read_Intersection_Config(Inter_conf_file);
	//load map nodes and calculate its heading and distance to first node
	Reload_MapNodes();
	//Initialize the reference point
	geoCoord geoPoint;
	geoPoint.init(Newmap.intersection.Ref_Long, Newmap.intersection.Ref_Lat, Newmap.intersection.Ref_Ele);
	double ecef_x,ecef_y,ecef_z;					
	//Important!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//Note the rectangular coordinate system: local_x is N(+) S(-) side, and local_y is E(+) W(-) side 
	//This is relative distance to the ref point!!!!
	//local z can be considered as 0 because no elevation change.
	double local_x,local_y,local_z;
	

	//Network setup	
	int sockfd;
	struct sockaddr_in sendaddr;
	struct sockaddr_in recvaddr;

	if((sockfd = socket(AF_INET,SOCK_DGRAM,0)) == -1)
	{
		perror("sockfd");
		exit(1);
	}
	
	//Setup time out
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
     perror("Error");
	}

     
	sendaddr.sin_family = AF_INET;
	sendaddr.sin_port = htons(port);  //*** IMPORTANT: the vissim,signal control should also have this port. ***//
	sendaddr.sin_addr.s_addr = INADDR_ANY;//inet_addr(LOCAL_HOST_ADDR);//inet_addr(OBU_ADDR);//INADDR_ANY;

	memset(sendaddr.sin_zero,'\0',sizeof sendaddr.sin_zero);

	if(bind(sockfd, (struct sockaddr*) &sendaddr, sizeof sendaddr) == -1)
	{
		perror("bind");        exit(1);
	}

	recvaddr.sin_family = AF_INET;
	recvaddr.sin_port = htons(PORT_FROM_ACA);
	recvaddr.sin_addr.s_addr = inet_addr(LOCAL_HOST_ADDR) ; //INADDR_BROADCAST;
	memset(recvaddr.sin_zero,'\0',sizeof recvaddr.sin_zero);

	int addr_length = sizeof ( sendaddr );
	int recv_data_len;
	//-----------------------End of Network Connection------------------//
	
	double begintime=GetSeconds();
	double currenttime=begintime;
	int traj_requested=0;   // whether the received package is a traj request or BSM
	
	cout<<"Enter the While Loop..."<<endl;
	while (true)
	{	
		recv_data_len = recvfrom(sockfd, buf, sizeof(buf), 0,
                        (struct sockaddr *)&sendaddr, (socklen_t *)&addr_length);
                                                
        //~ for (int i=0;i<50;i++)
		//~ {
			//~ printf("%.2X  ",buf[i]);
			//~ if (i%16==0 && i>0)
				//~ printf("\n");
		//~ }
		//~ printf("\n");
		
		
		if (recv_data_len<=30 && recv_data_len>0)     //must be a traj request from signal control
		{
			traj_requested=1;
		}
		else if (recv_data_len>30)
		{
			traj_requested=0;
		}
		else
		{
			traj_requested=2;  //didn't receive anything
	   }
			
   if (traj_requested==0)
   {
		//deal with the 7 or 8 bytes header problem from cohda
		int offset=7;
		if (flag_sim==1 || flag_sim==2)  //bsm from cohda or savari
		{
			if (buf[5]==129 && buf[6]==38)  //7 bytes header
				offset=7;
			if (buf[6]==129 && buf[7]==38)  //8 bytes header
				offset=8;
			if (buf[7]==129 && buf[8]==38)  //9 bytes header
				offset=9;
		}
		//cout<<"offset is :"<<offset<<endl;
   
	   //Copy BSM data blob
	   char BSM_Blob[38];
	   for(int i=0;i<38;i++)
	   {	   
			BSM_Blob[i]=buf[i+offset];
	   }
	   //Unpack BSM
	   vehIn.BSMToVehicle(BSM_Blob);  //change from BSM to Vehicle information
		
	   trackedveh.Reset();
	   
	   int found=0;
	   double t1=GetSeconds();
	   
	   while(!trackedveh.EndOfList())  //match vehicle according to vehicle ID
	   {
		   //if ID is a match, store trajectory information
		   if(trackedveh.Data().TempID==vehIn.TemporaryID)   
		   {
			   if(t1-trackedveh.Data().receive_timer>1)    //every 1s processes a trajectory
			   {
				    trackedveh.Data().active_flag=5;
					trackedveh.Data().receive_timer=t1;  //reset the timer
					trackedveh.Data().time[trackedveh.Data().nFrame]=t1;  
									
					trackedveh.Data().traj[trackedveh.Data().nFrame].latitude=vehIn.pos.latitude;
					trackedveh.Data().traj[trackedveh.Data().nFrame].longitude=vehIn.pos.longitude;
					trackedveh.Data().traj[trackedveh.Data().nFrame].elevation=vehIn.pos.elevation;
					trackedveh.Data().Speed[trackedveh.Data().nFrame]=vehIn.motion.speed/speed_coif;       //Speed from Savari BSM decoder is KM/H, need to change to m/s here, from cohda and simulation m/s 
					
					trackedveh.Data().heading[trackedveh.Data().nFrame]=vehIn.motion.heading;
					geoPoint.lla2ecef(vehIn.pos.longitude,vehIn.pos.latitude,vehIn.pos.elevation,&ecef_x,&ecef_y,&ecef_z);
					geoPoint.ecef2local(ecef_x,ecef_y,ecef_z,&local_x,&local_y,&local_z);
					trackedveh.Data().N_Offset[trackedveh.Data().nFrame]=local_x;
					trackedveh.Data().E_Offset[trackedveh.Data().nFrame]=local_y;
					
					//Geofencing
					int on_map_flag=Geofencing(vehIn.motion.heading,local_x,local_y);				
					//Locate vehicle on map
					if (on_map_flag==0)  //on map
					{
						if (trackedveh.Data().processed==0)  //first time vehicle is on map
						{
							trackedveh.Data().first_onmap_time=trackedveh.Data().nFrame;
						}
						
						//Locate vehicle on map
						double Dis_cur, ETA;
						int approach, lane, req_phase, state;
						LocateVehOnMap(vehIn.motion.speed/speed_coif, vehIn.motion.heading, local_x, local_y, Dis_cur, ETA, approach, lane, req_phase, state);
						trackedveh.Data().req_phase = req_phase;
						trackedveh.Data().approach = approach;
						trackedveh.Data().lane = lane;
						trackedveh.Data().states = state;
						trackedveh.Data().stopBarDistance[trackedveh.Data().nFrame] = Dis_cur;
						trackedveh.Data().processed=1;   //already go through the find vehicle in the mAP function
						
						if (vehIn.motion.speed/speed_coif<2)
						{
							if (trackedveh.Data().appr_states!=2)    //stop again, need to update stop time and location
							{
								trackedveh.Data().time_stop=t1;  //record stopped time
								trackedveh.Data().distance_stop=Dis_cur;  //record distance to stop bar 
								if (trackedveh.Data().stop_flag==1)
									printf ("Vehicle %d stopped again!!!!!!!!!!!!!!!!!!!",trackedveh.Data().TempID);
								
								trackedveh.Data().appr_states=2;  //in queue
							}
						}
						else
						{
							if (trackedveh.Data().stop_flag==1)  //already stopped
								trackedveh.Data().appr_states=3;  //vehicle leave queue
							else
								trackedveh.Data().appr_states=1;
						}
						
						//Determine whether a vehicle has stopped
						if (trackedveh.Data().stop_flag==0 && vehIn.motion.speed/speed_coif<2 ) //vehicle has not stopped before
						{
							trackedveh.Data().stop_flag=1;
							//trackedveh.Data().time_stop=t1;  //record first stopped time
							//trackedveh.Data().distance_stop=Dis_cur;  //record distance to stop bar at first stop
						}
						//Record the entry time
						if (trackedveh.Data().stopBarDistance[trackedveh.Data().first_onmap_time]>entry_location)  //vehicle arrived far away
						{
							if (trackedveh.Data().stopBarDistance[trackedveh.Data().nFrame-1]>entry_location && trackedveh.Data().stopBarDistance[trackedveh.Data().nFrame]<=entry_location)
							{
								trackedveh.Data().entry_time=t1;
								//~ if (vehIn.motion.speed/speed_coif>2)
									//~ trackedveh.Data().appr_states=1;
								//~ else
									//~ trackedveh.Data().appr_states=2;
							}
						}
						else  //vehicle first appear within the controlled location
						{
							//cout<<"@@@@@@@@@"<<trackedveh.Data().first_onmap_time<<" "<<trackedveh.Data().time[trackedveh.Data().first_onmap_time]<<" "<<
							//trackedveh.Data().stopBarDistance[trackedveh.Data().first_onmap_time]<<" "<<trackedveh.Data().Speed[trackedveh.Data().first_onmap_time]<<endl;
							trackedveh.Data().entry_time= trackedveh.Data().time[trackedveh.Data().first_onmap_time]-
							((entry_location-trackedveh.Data().stopBarDistance[trackedveh.Data().first_onmap_time])/ffspeed);
							//~ if (vehIn.motion.speed/speed_coif>2)
								//~ trackedveh.Data().appr_states=1;
							//~ else
								//~ trackedveh.Data().appr_states=2;
						}							
					}
					else if (on_map_flag==1)   //within intersection
					{
						trackedveh.Data().req_phase = 0;
						trackedveh.Data().approach = 0;
						trackedveh.Data().lane = 0;
						trackedveh.Data().states = 0;
						trackedveh.Data().stopBarDistance[trackedveh.Data().nFrame] = 0;
						trackedveh.Data().appr_states=0;
					}
					else   //not on map
					{
						trackedveh.Data().req_phase = 0;
						trackedveh.Data().approach = 0;
						trackedveh.Data().lane = 0;
						trackedveh.Data().states = 4;
						trackedveh.Data().stopBarDistance[trackedveh.Data().nFrame] = 0;
						trackedveh.Data().appr_states=0;
					}				
					
				trackedveh.Data().nFrame++;
				//if reached the end of the trajectory, start over
				if(trackedveh.Data().nFrame==500)
					trackedveh.Data().nFrame=0;
				//~ //print out the information			
				if (trackedveh.Data().processed==1)
				{			
					printf("ID: %d, Position: %.2f, Speed: %.2f, Appr_states: %d, fir_stop_time: %.2f, fir_stop_dis: %.2f, entry_time: %.2f, Req_phase: %d, App: %d, Lane: %d \n",
					vehIn.TemporaryID,trackedveh.Data().stopBarDistance[trackedveh.Data().nFrame-1],trackedveh.Data().Speed[trackedveh.Data().nFrame-1],
					trackedveh.Data().appr_states,trackedveh.Data().time_stop,trackedveh.Data().distance_stop,trackedveh.Data().entry_time,trackedveh.Data().req_phase,
					trackedveh.Data().approach,trackedveh.Data().lane);
		    	}
			   }
			   found=1; //find the vehicle
			   break;
		   }
		   trackedveh.Next();
	   }
	   if(found==0) //This is a new vehicle
	   {
		   ConnectedVehicle TempVeh;
		   TempVeh.TempID=vehIn.TemporaryID;
		   TempVeh.time[0]=GetSeconds();
		   TempVeh.traj[0].latitude=vehIn.pos.latitude;
		   TempVeh.traj[0].longitude=vehIn.pos.longitude;
		   TempVeh.traj[0].elevation=vehIn.pos.elevation;
		   TempVeh.Speed[0]=vehIn.motion.speed/speed_coif;
		   TempVeh.heading[0]=vehIn.motion.heading;
		   TempVeh.stop_flag=0;  //vehicle has not stopped yet
		   TempVeh.active_flag=5;   //initilize the active_flag
		   TempVeh.processed=0;
		   TempVeh.appr_states=1;  //new vehicle must be approaching without stop
		   TempVeh.time_stop=0;  //record first stopped time
		   TempVeh.distance_stop=0;  //record distance to stop bar at first stop
		   TempVeh.nFrame=1;
		   trackedveh.InsertRear(TempVeh);
		   
		   //sprintf(tmp_log,"Add Vehicle No. %d at %lf, the list size is %d\n",TempVeh.TempID, GetSeconds(),trackedveh.ListSize());
		   //cout<<tmp_log;
	   }
	}
	
	if(traj_requested==1)
	{
		cout<<"Receive Request from Control Algorithm!"<<endl;
		//Process the trajecotry request and generate a package
		byte tmp_traj_data[5000];  // 5k is the maximum can be sent
		int traj_data_size=Pack_Traj_Data(tmp_traj_data);
				
		//Send trajectory
		recvaddr.sin_family = AF_INET;
		recvaddr.sin_port = htons(PORT_FROM_ACA);  //to signal control, port is 33333!!!!!!!!!!!!!!!!!!!!!!!!!!
		recvaddr.sin_addr.s_addr = inet_addr("127.0.0.1") ; //Send to local host;
		memset(recvaddr.sin_zero,'\0',sizeof recvaddr.sin_zero);
		sendto(sockfd,tmp_traj_data,traj_data_size, 0,(struct sockaddr *)&recvaddr, addr_length);
		sprintf(tmp_log,"Send trajectory data to Control Algorithm! The size is %d\n",traj_data_size);
		outputlog(tmp_log); cout<<tmp_log;
	}
	  
	  currenttime=GetSeconds();
	  if(currenttime-begintime>1)  //every 1 second
	  {  
		  //Delete vehicle from the list if they left the DSRC range already
		  trackedveh.Reset();
			while(!trackedveh.EndOfList()) 
			{
				trackedveh.Data().active_flag--;
				
				if (trackedveh.Data().active_flag<0)   //if active flag is not updated over 5 seconds
				{
					//then delete the vehicle
					sprintf(tmp_log,"Delete Vehicle No. %d \n",trackedveh.Data().TempID);
					cout<<tmp_log;
					trackedveh.DeleteAt();
				}
				trackedveh.Next();
			}
			//reset begintime;
			begintime=currenttime;
	  }
	       
	}
}


int msleep(unsigned long milisec)
{
    struct timespec req={0};
    time_t sec=(int)(milisec/1000);
    milisec=milisec-(sec*1000);
    req.tv_sec=sec;
    req.tv_nsec=milisec*1000000L;
    while(nanosleep(&req,&req)==-1)
        continue;
    return 1;
}
	

double GetSeconds()
{
	struct timeval tv_tt;
	gettimeofday(&tv_tt, NULL);
	return (tv_tt.tv_sec+tv_tt.tv_usec/1.0e6);    
}


void xTimeStamp( char * pc_TimeStamp_ )
{
    struct tm  * ps_Time;
    time_t       i_CurrentTime;
    char         ac_TmpStr[256];

    i_CurrentTime =  time(NULL);
    ps_Time = localtime( &i_CurrentTime );

    //year
    sprintf(ac_TmpStr, "%d", ps_Time->tm_year + 1900);
    strcpy(pc_TimeStamp_, ac_TmpStr);

    //month
    sprintf(ac_TmpStr, "_%d", ps_Time->tm_mon + 1 );
    strcat(pc_TimeStamp_, ac_TmpStr);

    //day
    sprintf(ac_TmpStr, "_%d", ps_Time->tm_mday );
    strcat(pc_TimeStamp_, ac_TmpStr);

    //hour
    sprintf(ac_TmpStr, "_%d", ps_Time->tm_hour  );
    strcat(pc_TimeStamp_, ac_TmpStr);

    //min
    sprintf(ac_TmpStr, "_%d", ps_Time->tm_min );
    strcat(pc_TimeStamp_, ac_TmpStr);

    //sec
    sprintf(ac_TmpStr, "_%d", ps_Time->tm_sec );
    strcat(pc_TimeStamp_, ac_TmpStr);
}

void get_map_name() //Yiheng add 07/18/2014
{
    fstream fs;

    fs.open(MAP_File_Name);

    char temp[128];

    getline(fs,nmap_name);

    if(nmap_name.size()!=0)
    {
        //std::cout<< "Current Vehicle ID:" << RSUID <<std::endl;
        sprintf(temp,"Current MAP is %s\n",nmap_name.c_str());
        cout<<temp<<endl;
        outputlog(temp);
    }
    else
    {
        sprintf(temp,"Reading MAP_File problem");
        cout<<temp<<endl;
        outputlog(temp);
        exit(0);
    }

    fs.close();
}

int outputlog(char *output)
{
	FILE * stream = fopen( logfilename, "r" );

	if (stream==NULL)
	{
		perror ("Error opening file");
	}

	fseek( stream, 0L, SEEK_END );
	long endPos = ftell( stream );
	fclose( stream );

	fstream fs;
	if (endPos <10000000)
		fs.open(logfilename, ios::out | ios::app);
	else
		fs.open(logfilename, ios::out | ios::trunc);

	//fstream fs;
	//fs.open("/nojournal/bin/OBU_logfile.txt", ios::out | ios::app);
	if (!fs || !fs.good())
	{
		cout << "could not open file!\n";
		return -1;
	}
	fs << output;

	if (fs.fail())
	{
		cout << "failed to append to file!\n";
		return -1;
	}
	fs.close();

	return 1;
}

void Reload_MapNodes()
{
	int i, j, k;
	Inter.MAP_Nodes.clear();
	for (i = 0; i < Newmap.intersection.Appro_No; i++)
	{
		for (j = 0; j < Newmap.intersection.Approaches[i].Ingress_Lane_No; j++)
		{
			for (k = 0; k < Newmap.intersection.Approaches[i].Ingress_Lanes[j].Node_No; k++)
			{
				geoCoord geoPoint;
				double ecef_x, ecef_y, ecef_z;
				double local_x, local_y, local_z;
				N_LaneNodes temp_node;
				temp_node.direction = 0;  //ingress lane
				temp_node.index.Approach = i + 1;
				temp_node.index.Lane = Newmap.intersection.Approaches[i].Ingress_Lanes[j].ID;
				temp_node.index.Node = k + 1;
				temp_node.Latitude = Newmap.intersection.Approaches[i].Ingress_Lanes[j].Nodes[k].Latitude;
				temp_node.Longitude = Newmap.intersection.Approaches[i].Ingress_Lanes[j].Nodes[k].Longitude;
				geoPoint.init(Newmap.intersection.Ref_Long, Newmap.intersection.Ref_Lat, Newmap.intersection.Ref_Ele);
				geoPoint.lla2ecef(temp_node.Longitude, temp_node.Latitude, Newmap.intersection.Ref_Ele, &ecef_x, &ecef_y, &ecef_z);
				geoPoint.ecef2local(ecef_x, ecef_y, ecef_z, &local_x, &local_y, &local_z);
				temp_node.N_Offset = local_x;
				temp_node.E_Offset = local_y;
				temp_node.phase = Inter.lane_phase_mapping[temp_node.index.Lane - 1];
				Inter.MAP_Nodes.push_back(temp_node);
			}
		}
		for (j = 0; j < Newmap.intersection.Approaches[i].Egress_Lane_No; j++)
		{
			for (k = 0; k < Newmap.intersection.Approaches[i].Egress_Lanes[j].Node_No; k++)
			{
				geoCoord geoPoint;
				double ecef_x, ecef_y, ecef_z;
				double local_x, local_y, local_z;
				N_LaneNodes temp_node;
				temp_node.direction = 1;  //Egress lane
				temp_node.index.Approach = i + 1;
				temp_node.index.Lane = Newmap.intersection.Approaches[i].Egress_Lanes[j].ID;
				temp_node.index.Node = k + 1;
				temp_node.Latitude = Newmap.intersection.Approaches[i].Egress_Lanes[j].Nodes[k].Latitude;
				temp_node.Longitude = Newmap.intersection.Approaches[i].Egress_Lanes[j].Nodes[k].Longitude;
				geoPoint.init(Newmap.intersection.Ref_Long, Newmap.intersection.Ref_Lat, Newmap.intersection.Ref_Ele);
				geoPoint.lla2ecef(temp_node.Longitude, temp_node.Latitude, Newmap.intersection.Ref_Ele, &ecef_x, &ecef_y, &ecef_z);
				geoPoint.ecef2local(ecef_x, ecef_y, ecef_z, &local_x, &local_y, &local_z);
				temp_node.N_Offset = local_x;
				temp_node.E_Offset = local_y;
				temp_node.phase = 0;
				Inter.MAP_Nodes.push_back(temp_node);
			}
		}
	}

//calculate Map nodes heading and distance to the first node of the lane
	int tmp_pos = 0;
	for (i = 0; i < Newmap.intersection.Appro_No; i++)
	{
		for (j = 0; j < Newmap.intersection.Approaches[i].Ingress_Lane_No; j++)
		{
			for (k = 0; k < Newmap.intersection.Approaches[i].Ingress_Lanes[j].Node_No; k++)
			{
				//ingress lanes
				if (k < Newmap.intersection.Approaches[i].Ingress_Lanes[j].Node_No - 1)
				{
					Inter.MAP_Nodes[tmp_pos].Heading = atan2(Inter.MAP_Nodes[tmp_pos].N_Offset - Inter.MAP_Nodes[tmp_pos + 1].N_Offset, Inter.MAP_Nodes[tmp_pos].E_Offset - Inter.MAP_Nodes[tmp_pos + 1].E_Offset)*180.0 / PI;
					Inter.MAP_Nodes[tmp_pos].Heading = 90.0 - Inter.MAP_Nodes[tmp_pos].Heading;
				}
				else
					Inter.MAP_Nodes[tmp_pos].Heading = Inter.MAP_Nodes[tmp_pos - 1].Heading;
				if (Inter.MAP_Nodes[tmp_pos].Heading<0)
					Inter.MAP_Nodes[tmp_pos].Heading += 360;
				if (Inter.MAP_Nodes[tmp_pos].index.Node == 1)
					Inter.MAP_Nodes[tmp_pos].dis_to_1st_node = 0;
				else
				{
					Inter.MAP_Nodes[tmp_pos].dis_to_1st_node = 0;
					for (int l = 0; l < Inter.MAP_Nodes[tmp_pos].index.Node - 1; l++)
					{
						Inter.MAP_Nodes[tmp_pos].dis_to_1st_node += sqrt((Inter.MAP_Nodes[tmp_pos - l].N_Offset - Inter.MAP_Nodes[tmp_pos - l - 1].N_Offset)*(Inter.MAP_Nodes[tmp_pos - l].N_Offset - Inter.MAP_Nodes[tmp_pos - l - 1].N_Offset) + (Inter.MAP_Nodes[tmp_pos - l].E_Offset - Inter.MAP_Nodes[tmp_pos - l - 1].E_Offset)*(Inter.MAP_Nodes[tmp_pos - l].E_Offset - Inter.MAP_Nodes[tmp_pos - l - 1].E_Offset));
					}
				}
				tmp_pos++;
			}
		}
		for (j = 0; j < Newmap.intersection.Approaches[i].Egress_Lane_No; j++)
		{
			for (k = 0; k < Newmap.intersection.Approaches[i].Egress_Lanes[j].Node_No; k++)
			{
				//egress lane
				if (k < Newmap.intersection.Approaches[i].Egress_Lanes[j].Node_No - 1)
				{
					Inter.MAP_Nodes[tmp_pos].Heading = atan2(Inter.MAP_Nodes[tmp_pos + 1].N_Offset - Inter.MAP_Nodes[tmp_pos].N_Offset, Inter.MAP_Nodes[tmp_pos + 1].E_Offset - Inter.MAP_Nodes[tmp_pos].E_Offset)*180.0 / PI;
					Inter.MAP_Nodes[tmp_pos].Heading = 90.0 - Inter.MAP_Nodes[tmp_pos].Heading;
				}
				else
					Inter.MAP_Nodes[tmp_pos].Heading = Inter.MAP_Nodes[tmp_pos - 1].Heading;
				if (Inter.MAP_Nodes[tmp_pos].Heading<0)
					Inter.MAP_Nodes[tmp_pos].Heading += 360;
				Inter.MAP_Nodes[tmp_pos].dis_to_1st_node = 999; //Egress lane doesn't have this attribute
				tmp_pos++;
			}
		}
	}
//print out map nodes information
printf("MAP Nodes Information:\n");
for (i=0;i<Inter.MAP_Nodes.size();i++)
	printf("%d %d %d %f %f %f %f\n",Inter.MAP_Nodes[i].index.Approach,Inter.MAP_Nodes[i].index.Lane,Inter.MAP_Nodes[i].index.Node,Inter.MAP_Nodes[i].Heading,Inter.MAP_Nodes[i].dis_to_1st_node, Inter.MAP_Nodes[i].N_Offset,Inter.MAP_Nodes[i].E_Offset);

}

int Read_Intersection_Config(char *filename)
{
	fstream fs;
	fs.open(filename);
	char temp[128];
	
	string temp_string;

	getline(fs, temp_string);  //First line comment
	getline(fs, temp_string);  //Second line, number of intersecions

	strcpy(temp, temp_string.c_str());
	int Intersection_No;
	sscanf(temp, "%d", &Intersection_No);
	for (int i = 0; i < Intersection_No; i++)
	{
		getline(fs, temp_string);  //Intersection id and lane number
		strcpy(temp, temp_string.c_str());
		sscanf(temp, "%d %d %f", &Inter.ID, &Inter.Laner_No, &Inter.inter_size);
		getline(fs, temp_string);  //get the mapping line
		strcpy(temp, temp_string.c_str());
		char *data = temp;
		int offset = 0;
		for (int j = 0; j < Inter.Laner_No;j++)
		{
			sscanf(data, "%d%n", &Inter.lane_phase_mapping[j], &offset);
			data += offset;
		}
		getline(fs, temp_string);  //get the DSRC_range line
		strcpy(temp, temp_string.c_str());
		char *data1 = temp;
		offset = 0;
		for (int j = 0; j < 4; j++)
		{
			sscanf(data1, "%d%n", &Inter.DSRC_range[j], &offset);
			data1 += offset;
		}

	}
	fs.close();
	return 1;
}

int Geofencing(double heading, double N_Offset, double E_Offset)  //Geofence the point to see whether it is in the range of the road
{
	int i;
	double dis_to_ref = sqrt(pow(N_Offset, 2) + pow(E_Offset, 2));
	if (dis_to_ref>=Inter.inter_size) //If a map is received and outside the boundary of the intersection
	{
		double tempdis = 1000000.0;
		int t_pos;   //position of the nearest MAP_Node
		int t_approach, t_lane, t_node;
		for (i = 0; i < Inter.MAP_Nodes.size(); i++)
		{
			double dis = sqrt((N_Offset - Inter.MAP_Nodes[i].N_Offset)*(N_Offset - Inter.MAP_Nodes[i].N_Offset) + (E_Offset - Inter.MAP_Nodes[i].E_Offset)*(E_Offset - Inter.MAP_Nodes[i].E_Offset));
			if (dis < tempdis && (abs(heading - Inter.MAP_Nodes[i].Heading) <= 50 || abs(heading - Inter.MAP_Nodes[i].Heading) >= 310))
			{
				tempdis = dis;
				t_approach = Inter.MAP_Nodes[i].index.Approach;
				t_lane = Inter.MAP_Nodes[i].index.Lane;
				t_node = Inter.MAP_Nodes[i].index.Node;
				t_pos = i;
			}
		}
		//Geofence the area
		//find the distance to the line connected by lane nodes
		double distanceLine,distanceSegment;
		if (t_node == 1)
			DistanceFromLine(N_Offset, E_Offset, Inter.MAP_Nodes[t_pos].N_Offset, Inter.MAP_Nodes[t_pos].E_Offset, Inter.MAP_Nodes[t_pos + 1].N_Offset, Inter.MAP_Nodes[t_pos + 1].E_Offset, distanceSegment, distanceLine);
		else
			DistanceFromLine(N_Offset, E_Offset, Inter.MAP_Nodes[t_pos].N_Offset, Inter.MAP_Nodes[t_pos].E_Offset, Inter.MAP_Nodes[t_pos - 1].N_Offset, Inter.MAP_Nodes[t_pos - 1].E_Offset, distanceSegment, distanceLine);
		if (dis_to_ref <= Inter.DSRC_range[t_approach - 1] && distanceLine <= 8)
			{
				return 0;  //vehicle is on map
			}
			else
			{
				return 2; //vehicle is outside of the intersection, but not on map
			}
	}
	else
	{
		return 1;   //inside the intersection
	}
}

void DistanceFromLine(double cx, double cy, double ax, double ay, double bx, double by, double &distanceSegment, double &distanceLine)
{

	//
	// find the distance from the point (cx,cy) to the line
	// determined by the points (ax,ay) and (bx,by)
	//
	// distanceSegment = distance from the point to the line segment
	// distanceLine = distance from the point to the line (assuming
	//					infinite extent in both directions
	//

	/*
	How do I find the distance from a point to a line?


	Let the point be C (Cx,Cy) and the line be AB (Ax,Ay) to (Bx,By).
	Let P be the point of perpendicular projection of C on AB.  The parameter
	r, which indicates P's position along AB, is computed by the dot product
	of AC and AB divided by the square of the length of AB:

	(1)     AC dot AB
	r = ---------
	||AB||^2

	r has the following meaning:

	r=0      P = A
	r=1      P = B
	r<0      P is on the backward extension of AB
	r>1      P is on the forward extension of AB
	0<r<1    P is interior to AB

	The length of a line segment in d dimensions, AB is computed by:

	L = sqrt( (Bx-Ax)^2 + (By-Ay)^2 + ... + (Bd-Ad)^2)

	so in 2D:

	L = sqrt( (Bx-Ax)^2 + (By-Ay)^2 )

	and the dot product of two vectors in d dimensions, U dot V is computed:

	D = (Ux * Vx) + (Uy * Vy) + ... + (Ud * Vd)

	so in 2D:

	D = (Ux * Vx) + (Uy * Vy)

	So (1) expands to:

	(Cx-Ax)(Bx-Ax) + (Cy-Ay)(By-Ay)
	r = -------------------------------
	L^2

	The point P can then be found:

	Px = Ax + r(Bx-Ax)
	Py = Ay + r(By-Ay)

	And the distance from A to P = r*L.

	Use another parameter s to indicate the location along PC, with the
	following meaning:
	s<0      C is left of AB
	s>0      C is right of AB
	s=0      C is on AB

	Compute s as follows:

	(Ay-Cy)(Bx-Ax)-(Ax-Cx)(By-Ay)
	s = -----------------------------
	L^2


	Then the distance from C to P = |s|*L.

	*/


	double r_numerator = (cx - ax)*(bx - ax) + (cy - ay)*(by - ay);
	double r_denomenator = (bx - ax)*(bx - ax) + (by - ay)*(by - ay);
	double r = r_numerator / r_denomenator;
	//
	double px = ax + r*(bx - ax);
	double py = ay + r*(by - ay);
	//     
	double s = ((ay - cy)*(bx - ax) - (ax - cx)*(by - ay)) / r_denomenator;

	distanceLine = fabs(s)*sqrt(r_denomenator);

	//
	// (xx,yy) is the point on the lineSegment closest to (cx,cy)
	//
	double xx = px;
	double yy = py;

	if ((r >= 0) && (r <= 1))
	{
		distanceSegment = distanceLine;
	}
	else
	{

		double dist1 = (cx - ax)*(cx - ax) + (cy - ay)*(cy - ay);
		double dist2 = (cx - bx)*(cx - bx) + (cy - by)*(cy - by);
		if (dist1 < dist2)
		{
			xx = ax;
			yy = ay;
			distanceSegment = sqrt(dist1);
		}
		else
		{
			xx = bx;
			yy = by;
			distanceSegment = sqrt(dist2);
		}


	}

	return;
}

void LocateVehOnMap(double speed, double heading, double N_offset, double E_offset, double &Dis_cur, double &ETA, int &approach, int &lane, int &req_phase, int &state)
{
	int i;
	//Find nearest lane nodes
	double tempdis = 1000000.0;
	int t_pos;   //position of the nearest MAP_Node
	for (i = 0; i <Inter.MAP_Nodes.size(); i++)
	{
		double dis = sqrt((N_offset - Inter.MAP_Nodes[i].N_Offset)*(N_offset - Inter.MAP_Nodes[i].N_Offset) + (E_offset - Inter.MAP_Nodes[i].E_Offset)*(E_offset - Inter.MAP_Nodes[i].E_Offset));
		if (dis < tempdis && (abs(heading - Inter.MAP_Nodes[i].Heading) <= 50 || abs(heading - Inter.MAP_Nodes[i].Heading) >= 310))
		{
			tempdis = dis;
			approach = Inter.MAP_Nodes[i].index.Approach;
			lane = Inter.MAP_Nodes[i].index.Lane;
			t_pos = i;
		}
	}
	int match = 0;   //whether the vehicle heading matches the lane heading 1: match; 0: not match
	//See if the vehicle heading matches the node (lane) heading
	if (abs(heading - Inter.MAP_Nodes[t_pos].Heading) < 20)  //threshold is set to 20 degree
		match = 1;    
	if (heading > 340 && Inter.MAP_Nodes[t_pos].Heading < 20)
	{
		if (abs(heading - 360 - Inter.MAP_Nodes[t_pos].Heading) < 20)
			match = 1;
	}
	if (heading < 20 && Inter.MAP_Nodes[t_pos].Heading > 340)
	{
		if (abs(Inter.MAP_Nodes[t_pos].Heading - 360 - heading) < 20)
			match = 1;
	}
	//Find out vehicle states: 1: approaching 2: leaving 3: in queue 4: not on the roadway (not yet coded!!!)
	state = 4;  //define vehicle as not not on the roadway first
	req_phase = 0;
	if (match == 1)
	{
		if (Inter.MAP_Nodes[t_pos].direction == 0)  //ingress lane: vehicle states should be 1 or 3
		{
			if (speed < 0.894)      //2mph
				state = 3;
			else
				state = 1;
		}
		if (Inter.MAP_Nodes[t_pos].direction == 1)  //egress lane: vehicle states should be 2
		{
			state = 2;
			req_phase = 0;
			Dis_cur = 999;
			ETA = 999;
		}
	}
	//Calculate distance to stop-bar
	if (Inter.MAP_Nodes[t_pos].direction == 0)  //only consider vehicle in ingress lane
	{
		if (Inter.MAP_Nodes[t_pos].index.Node == 1) //first node of the lane, dis_cur just the distance to that node
			Dis_cur = sqrt((N_offset - Inter.MAP_Nodes[t_pos].N_Offset)*(N_offset - Inter.MAP_Nodes[t_pos].N_Offset) + (E_offset - Inter.MAP_Nodes[t_pos].E_Offset)*(E_offset - Inter.MAP_Nodes[t_pos].E_Offset));
		else
		{
			int index1 = Inter.MAP_Nodes[t_pos].index.Node;
			double veh_dis_2_1st_node = sqrt((N_offset - Inter.MAP_Nodes[t_pos - index1 + 1].N_Offset)*(N_offset - Inter.MAP_Nodes[t_pos - index1 + 1].N_Offset) + (E_offset - Inter.MAP_Nodes[t_pos - index1 + 1].E_Offset)*(E_offset - Inter.MAP_Nodes[t_pos - index1 + 1].E_Offset));
			double nearest_node_dis_2_1st_node = sqrt((Inter.MAP_Nodes[t_pos].N_Offset - Inter.MAP_Nodes[t_pos - index1 + 1].N_Offset)*(Inter.MAP_Nodes[t_pos].N_Offset - Inter.MAP_Nodes[t_pos - index1 + 1].N_Offset) + (Inter.MAP_Nodes[t_pos].E_Offset - Inter.MAP_Nodes[t_pos - index1 + 1].E_Offset)*(Inter.MAP_Nodes[t_pos].E_Offset - Inter.MAP_Nodes[t_pos - index1 + 1].E_Offset));
			if (veh_dis_2_1st_node <= nearest_node_dis_2_1st_node)  //vehicle is ahead of the nearest node
			{
				//start from the distance to next node
				double total_dis = 0;
				total_dis += sqrt((N_offset - Inter.MAP_Nodes[t_pos - 1].N_Offset)*(N_offset - Inter.MAP_Nodes[t_pos - 1].N_Offset) + (E_offset - Inter.MAP_Nodes[t_pos - 1].E_Offset)*(E_offset - Inter.MAP_Nodes[t_pos - 1].E_Offset)); //the distance to next node
				total_dis += Inter.MAP_Nodes[t_pos - 1].dis_to_1st_node;
				Dis_cur = total_dis;
			}
			else //vehicle is behind the nearest node
			{
				double total_dis = 0;
				total_dis += sqrt((N_offset - Inter.MAP_Nodes[t_pos].N_Offset)*(N_offset - Inter.MAP_Nodes[t_pos].N_Offset) + (E_offset - Inter.MAP_Nodes[t_pos].E_Offset)*(E_offset - Inter.MAP_Nodes[t_pos].E_Offset)); //the distance to next node
				total_dis += Inter.MAP_Nodes[t_pos].dis_to_1st_node;
				Dis_cur = total_dis;
			}
		}
	}
	else
		Dis_cur = 999;
	if (state == 1 || state == 3)
	{
		req_phase = Inter.MAP_Nodes[t_pos].phase;   //find requested phase based on nearest node!
		if (state==1)
			ETA = Dis_cur / speed;
		if (state==3)
			ETA = 0;
	}
}

int Pack_Traj_Data(byte* tmp_traj_data)
{
	int offset=0;
	byte*   pByte;      // pointer used (by cast)to get at each byte 
                            // of the shorts, longs, and blobs
    byte    tempByte;   // values to hold data once converted to final format
	unsigned short   tempUShort;
    long    tempLong;
	//header 2 bytes
	tmp_traj_data[offset]=0xFF;
	offset+=1;
	tmp_traj_data[offset]=0xFF;
	offset+=1;
	//MSG ID: 0x01 for trajectory data send to control algorithm
	tmp_traj_data[offset]=0x01;
	offset+=1;
	//No. of Vehicles in the list
	int temp_veh_No=0;
	trackedveh.Reset();
	while(!trackedveh.EndOfList())
	{
		if(trackedveh.Data().processed==1)
			temp_veh_No++;
		trackedveh.Next();
	}
	
	tempUShort = (unsigned short)temp_veh_No;
	
	cout<<"send list number is: "<<tempUShort<<endl;
	cout<<"total list number is: "<<trackedveh.ListSize()<<endl;
	
	pByte = (byte* ) &tempUShort;
    tmp_traj_data[offset+0] = (byte) *(pByte + 1); 
    tmp_traj_data[offset+1] = (byte) *(pByte + 0); 
	offset = offset + 2;
	//for each vehicle
	trackedveh.Reset();
	while(!trackedveh.EndOfList())
	{
		if (trackedveh.Data().processed==1)
		{
			//vehicle temporary id
			tempUShort = (unsigned short)trackedveh.Data().TempID;
			pByte = (byte* ) &tempUShort;
			tmp_traj_data[offset+0] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 0); 
			offset = offset + 2;
			
			//the number of trajectory points: nFrame
			tempUShort = (unsigned short)trackedveh.Data().nFrame;
			pByte = (byte* ) &tempUShort;
			tmp_traj_data[offset+0] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 0); 
			offset = offset + 2;
			
			//Speed of the vehicle
			tempLong = (long)(trackedveh.Data().Speed[trackedveh.Data().nFrame-1]* DEG2ASNunits); 
			pByte = (byte* ) &tempLong;
			tmp_traj_data[offset+0] = (byte) *(pByte + 3); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 2); 
			tmp_traj_data[offset+2] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+3] = (byte) *(pByte + 0); 
			offset = offset + 4;
			
			//entry time
			tempLong = (long)(trackedveh.Data().entry_time); 
			pByte = (byte* ) &tempLong;
			tmp_traj_data[offset+0] = (byte) *(pByte + 3); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 2); 
			tmp_traj_data[offset+2] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+3] = (byte) *(pByte + 0); 
			offset = offset + 4;
							
			//vehicle approach
			tempUShort = (unsigned short)trackedveh.Data().approach;
			pByte = (byte* ) &tempUShort;
			tmp_traj_data[offset+0] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 0); 
			offset = offset + 2;
			
			//vehicle lane
			tempUShort = (unsigned short)trackedveh.Data().lane;
			pByte = (byte* ) &tempUShort;
			tmp_traj_data[offset+0] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 0); 
			offset = offset + 2;
			
			//Distance To Stop Bar of the current state
			tempLong = (long)(trackedveh.Data().stopBarDistance[trackedveh.Data().nFrame-1]* DEG2ASNunits); // to 1/10th micro degees units
			pByte = (byte* ) &tempLong;
			tmp_traj_data[offset+0] = (byte) *(pByte + 3); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 2); 
			tmp_traj_data[offset+2] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+3] = (byte) *(pByte + 0); 
			offset = offset + 4;
							
			//Time when vehicle first stops
			tempLong = (long)(trackedveh.Data().time_stop); 
			pByte = (byte* ) &tempLong;
			tmp_traj_data[offset+0] = (byte) *(pByte + 3); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 2); 
			tmp_traj_data[offset+2] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+3] = (byte) *(pByte + 0); 
			offset = offset + 4;
			
			//Location (distance to stop bar) when vehicle first stops
			tempLong = (long)(trackedveh.Data().distance_stop* DEG2ASNunits); 
			pByte = (byte* ) &tempLong;
			tmp_traj_data[offset+0] = (byte) *(pByte + 3); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 2); 
			tmp_traj_data[offset+2] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+3] = (byte) *(pByte + 0); 
			offset = offset + 4;
			
			//Request phase
			tempByte = (byte)trackedveh.Data().req_phase; 
			tmp_traj_data[offset] = (byte) tempByte;
			offset = offset + 1; // move past to next item
			
			//vehicle approaching states
			tempUShort = (unsigned short)trackedveh.Data().appr_states;
			pByte = (byte* ) &tempUShort;
			tmp_traj_data[offset+0] = (byte) *(pByte + 1); 
			tmp_traj_data[offset+1] = (byte) *(pByte + 0); 
			offset = offset + 2;
			
		}
	trackedveh.Next();	
	}
	return offset;
}
