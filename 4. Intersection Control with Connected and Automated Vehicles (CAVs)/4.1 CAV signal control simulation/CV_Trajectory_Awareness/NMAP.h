#pragma once

#include <vector>
#include <string.h>
#include "stdio.h"
#include <iostream>
#include <fstream>
#include "stdlib.h"
#include "math.h"

#ifndef M_PI
    #define M_PI           3.14159265358979323846
#endif

using namespace std;

class ElementID
{
 public:
   int Approach;
   int Lane;
   int Node;
};

class N_LaneNodes			// lane nodes
{
public:
    ElementID index;
	int ID;
	int direction; //0: ingress; 1: egress  same as the lane
	float Heading; //node heading is equal to the lane heading
	double dis_to_1st_node;
	int phase;        //related signal phase, only nodes in ingress lanes have none zero phase; nodes in egress lanes have zero phase
	double E_Offset;  //offset to the previous node; if first node of the lane, offset to the referencne point
	double N_Offset;
	double Latitude;
	double Longitude;
};

class LaneConnection
{
public:
	//ElementID ConnectedLaneName;
	int lane_num;
	int Maneuver;
};

class ReferenceLane
{
public:
	int LaneID;
	int LateralOffset;
};

class Lane
{
public:
	string Lane_Name;
	int ID;
	int Attributes[16];  //16 bits of attributes
	int Connection_No;  //Number of connected lanes
	vector<LaneConnection> Connections;
	int Node_No;   //Total number of lane nodes
	vector<N_LaneNodes> Nodes;  //Vector of lane nodes
	ReferenceLane RefLane;  //reference lane if this is a computed lane
	int Type;           //1: motorized vehicle lane; 2: computed lane; 5: cross walk
	int Width;   //in centermeters
};

class Approach_N
{
public:
	int ID;
	int Direction; //not use yet
	int Egress_Lane_No;  // Number of egress lane
	int Ingress_Lane_No; // Number of ingress lane
	vector<Lane> Egress_Lanes;  //Vector of egress lanes
	vector<Lane> Ingress_Lanes;  //Vector of ingress lanes
};

class Intersection_N
{
public:
	long ID;
	//int Attributes[8];  //8 bits of attributes;
	double Ref_Lat;
	double Ref_Long;
	double Ref_Ele;
	int Appro_No;
	vector<Approach_N> Approaches;
	string Map_Name;
	//string Rsu_ID;
	//method
};

class NMAP
{
public:
	Intersection_N intersection;
	int ID;
	int Version;
	//method
	int ParseIntersection(char* filename);
	int ParseIntersection_XML(char* filename);
};
