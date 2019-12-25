#include "NMAP.h"
#include <iostream>
#include <string.h>
#include <fstream>
#include "stdlib.h"
#include "math.h"
#include "geoCoord.h"

#define DEFAULT_ARRAY_SIZE 80

using namespace std;


char *data;

char* Getcontent(char* str, char left, char right)
{

	int left_location = 0,
		right_location=0;
	for (int i = 0; i < 128; i++)
	{
		char temp = *(str + i);
		if (temp == left)
			left_location = i;
		if (left_location > 0)
		{
			if (temp == right)
			{
				right_location = i;
				break;
			}
		}
	}

	data = new char[right_location - left_location - 1];
	for (int j = left_location + 1; j < right_location; j++)
		data[j - left_location - 1] = *(str+j);
	

	return data;
}


//This function parse information from the XML map description file and save to the MAP data structure
int NMAP::ParseIntersection_XML(char* filename)
{
	int No_lanes=1;
	int i, j, k,l;
	//Start parsing the MAP data from file
	ifstream map_file;
	map_file.open(filename);

	if (!map_file)
	{
		cout << "Error: Open file " << filename << endl;
		return -1;
	}

	string lineread;
	char token_char[128];
	char * temp_char;

	getline(map_file, lineread); // <?xml version="1.0" encoding="UTF-8"?>
	getline(map_file, lineread); // <J2735.GID.blob>

	//map version
	getline(map_file, lineread);
	sscanf(lineread.c_str(), "%s", token_char);
	temp_char=Getcontent(token_char, '>', '<');
	sscanf(temp_char, "%d", &Version);
	free(data);

	//intersection name
	getline(map_file, lineread);
	//sscanf(lineread.c_str(), "%s", token_char);
	//temp_char = Getcontent(token_char, '>', '<');
	//sscanf(temp_char, "%s", intersection.Map_Name);
	//free(data);

	//intersection ID
	getline(map_file, lineread);
	sscanf(lineread.c_str(), "%s", token_char);
	temp_char = Getcontent(token_char, '>', '<');
	sscanf(temp_char, "%d", &intersection.ID);
	free(data);

	getline(map_file, lineread);  //<Resolution>decimeter</Resoluti
	getline(map_file, lineread);  //<Geometry>
	getline(map_file, lineread);  //<ReferencePoint>

	//reference point
	getline(map_file, lineread);
	sscanf(lineread.c_str(), "%s", token_char);
	temp_char = Getcontent(token_char, '>', '<');
	sscanf(temp_char, "%lf", &intersection.Ref_Lat);
	free(data);
	getline(map_file, lineread);
	sscanf(lineread.c_str(), "%s", token_char);
	temp_char = Getcontent(token_char, '>', '<');
	sscanf(temp_char, "%lf", &intersection.Ref_Long);
	free(data);
	getline(map_file, lineread);
	sscanf(lineread.c_str(), "%s", token_char);
	temp_char = Getcontent(token_char, '>', '<');
	sscanf(temp_char, "%lf", &intersection.Ref_Ele);
	free(data);

	getline(map_file, lineread);  //</ReferencePoint>

	//No. approaches
	getline(map_file, lineread);
	sscanf(lineread.c_str(), "%s", token_char);
	temp_char = Getcontent(token_char, '>', '<');
	sscanf(temp_char, "%d", &intersection.Appro_No);
	free(data);

	//initialize approaches
	Approach_N temp_appro;
	for (i = 0; i<intersection.Appro_No; i++)
	{
		intersection.Approaches.push_back(temp_appro);
	}


	//Start to parse the approaches, lanes and lane nodes
	for (i = 0; i < intersection.Appro_No; i++)
	{
		//approach id
		getline(map_file, lineread); // <Approach Number="1">
		intersection.Approaches[i].ID = i + 1;

		// Read No of ingress lanes
		getline(map_file, lineread);
		sscanf(lineread.c_str(), "%s", token_char);
		temp_char = Getcontent(token_char, '>', '<');
		sscanf(temp_char, "%d", &intersection.Approaches[i].Ingress_Lane_No);
		free(data);

		//initialize lanes first
		Lane temp_lane;
		for (j = 0; j < intersection.Approaches[i].Ingress_Lane_No; j++)
			intersection.Approaches[i].Ingress_Lanes.push_back(temp_lane);

		//Parse Ingress lanes
		for (j = 0; j < intersection.Approaches[i].Ingress_Lane_No; j++)
		{
			//Lane ID
			getline(map_file, lineread); //<Lane Number = "1">
			intersection.Approaches[i].Ingress_Lanes[j].ID = No_lanes;
			No_lanes++;

			//Lane type
			getline(map_file, lineread);
			sscanf(lineread.c_str(), "%s", token_char);
			temp_char = Getcontent(token_char, '>', '<');
			sscanf(temp_char, "%d", &intersection.Approaches[i].Ingress_Lanes[j].Type);
			free(data);

			//Lane attributes
			getline(map_file, lineread);
			sscanf(lineread.c_str(), "%s", token_char);
			temp_char = Getcontent(token_char, '>', '<');
			int temp_attributes;
			sscanf(temp_char, "%d", &temp_attributes);
			free(data);
			//convert integer lane attributes to each bit
			for (l = 7; l >= 0; l--)
			{
				if (temp_attributes - pow(2, l) >= 0)
				{
					intersection.Approaches[i].Ingress_Lanes[j].Attributes[l] = 1;
					temp_attributes = temp_attributes - pow(2, l);
				}
				else
					intersection.Approaches[i].Ingress_Lanes[j].Attributes[l] = 0;
			}

			//Lane width
			getline(map_file, lineread);
			sscanf(lineread.c_str(), "%s", token_char);
			temp_char = Getcontent(token_char, '>', '<');
			sscanf(temp_char, "%d", &intersection.Approaches[i].Ingress_Lanes[j].Width);
			free(data);

			if (intersection.Approaches[i].Ingress_Lanes[j].Type == 1 || intersection.Approaches[i].Ingress_Lanes[j].Type == 5)  //vehicle lane or cross walk
			{
				//Number of lane nodes
				getline(map_file, lineread);
				sscanf(lineread.c_str(), "%s", token_char);
				temp_char = Getcontent(token_char, '>', '<');
				sscanf(temp_char, "%d", &intersection.Approaches[i].Ingress_Lanes[j].Node_No);
				free(data);

				//Initialize Nodes First
				N_LaneNodes temp_node;
				for (k = 0; k < intersection.Approaches[i].Ingress_Lanes[j].Node_No; k++)
				{
					intersection.Approaches[i].Ingress_Lanes[j].Nodes.push_back(temp_node);
				}

				getline(map_file, lineread); // <Nodes>

				//Lane node list
				for (k = 0; k < intersection.Approaches[i].Ingress_Lanes[j].Node_No; k++)
				{

					getline(map_file, lineread); // <Node Number="1">
					intersection.Approaches[i].Ingress_Lanes[j].Nodes[k].ID = k + 1;

					//Latitude
					getline(map_file, lineread);
					sscanf(lineread.c_str(), "%s", token_char);
					temp_char = Getcontent(token_char, '>', '<');
					sscanf(temp_char, "%lf", &intersection.Approaches[i].Ingress_Lanes[j].Nodes[k].Latitude);
					free(data);

					//Longitude
					getline(map_file, lineread);
					sscanf(lineread.c_str(), "%s", token_char);
					temp_char = Getcontent(token_char, '>', '<');
					sscanf(temp_char, "%lf", &intersection.Approaches[i].Ingress_Lanes[j].Nodes[k].Longitude);
					free(data);

					getline(map_file, lineread); // </Node>

					//transform to local X and Y data
					if (k == 0)  //first node: the offset should be to the reference point
					{
						//reference point
						geoCoord geoPoint;
						double ecef_x, ecef_y, ecef_z;
						double local_x, local_y, local_z;
						geoPoint.init(intersection.Ref_Long, intersection.Ref_Lat, intersection.Ref_Ele);
						geoPoint.lla2ecef(intersection.Approaches[i].Ingress_Lanes[j].Nodes[k].Longitude, intersection.Approaches[i].Ingress_Lanes[j].Nodes[k].Latitude, intersection.Ref_Ele, &ecef_x, &ecef_y, &ecef_z);
						geoPoint.ecef2local(ecef_x, ecef_y, ecef_z, &local_x, &local_y, &local_z);
						intersection.Approaches[i].Ingress_Lanes[j].Nodes[k].E_Offset = local_y;
						intersection.Approaches[i].Ingress_Lanes[j].Nodes[k].N_Offset = local_x;
						//cout<<"first node:"<<intersection.Approaches[i].Ingress_Lanes[j].Nodes[k].E_Offset<<" "<<intersection.Approaches[i].Ingress_Lanes[j].Nodes[k].N_Offset<<endl;
					}
					else  //not the first node: the offset should be to the previous node
					{
						//reference point
						geoCoord geoPoint;
						double ecef_x, ecef_y, ecef_z;
						double local_x, local_y, local_z;
						geoPoint.init(intersection.Approaches[i].Ingress_Lanes[j].Nodes[k - 1].Longitude, intersection.Approaches[i].Ingress_Lanes[j].Nodes[k - 1].Latitude, intersection.Ref_Ele);
						geoPoint.lla2ecef(intersection.Approaches[i].Ingress_Lanes[j].Nodes[k].Longitude, intersection.Approaches[i].Ingress_Lanes[j].Nodes[k].Latitude, intersection.Ref_Ele, &ecef_x, &ecef_y, &ecef_z);
						geoPoint.ecef2local(ecef_x, ecef_y, ecef_z, &local_x, &local_y, &local_z);
						intersection.Approaches[i].Ingress_Lanes[j].Nodes[k].E_Offset = local_y;
						intersection.Approaches[i].Ingress_Lanes[j].Nodes[k].N_Offset = local_x;
					}
				}

				getline(map_file, lineread); // </Nodes>

			}

			if (intersection.Approaches[i].Ingress_Lanes[j].Type == 2)  //Computed lane
			{
				//Reference Lane
				getline(map_file, lineread);
				sscanf(lineread.c_str(), "%s", token_char);
				temp_char = Getcontent(token_char, '>', '<');
				sscanf(temp_char, "%d", &intersection.Approaches[i].Ingress_Lanes[j].RefLane.LaneID);
				free(data);

				//lateral offset
				getline(map_file, lineread);
				sscanf(lineread.c_str(), "%s", token_char);
				temp_char = Getcontent(token_char, '>', '<');
				sscanf(temp_char, "%d", &intersection.Approaches[i].Ingress_Lanes[j].RefLane.LateralOffset);
				free(data);
			}


			//Number of lane connections
			getline(map_file, lineread);
			sscanf(lineread.c_str(), "%s", token_char);
			temp_char = Getcontent(token_char, '>', '<');
			sscanf(temp_char, "%d", &intersection.Approaches[i].Ingress_Lanes[j].Connection_No);
			free(data);

			if (intersection.Approaches[i].Ingress_Lanes[j].Connection_No > 0)
			{
				//Initialize lane connections
				LaneConnection temp_connection;
				for (k = 0; k < intersection.Approaches[i].Ingress_Lanes[j].Connection_No; k++)
				{
					intersection.Approaches[i].Ingress_Lanes[j].Connections.push_back(temp_connection);
				}

				getline(map_file, lineread); //<Lane_Connections>
				//start to read connections
				for (k = 0; k < intersection.Approaches[i].Ingress_Lanes[j].Connection_No; k++)
				{
					getline(map_file, lineread);  //<Lane_Connection Number = "1">
					//connection lane ID
					getline(map_file, lineread);
					sscanf(lineread.c_str(), "%s", token_char);
					temp_char = Getcontent(token_char, '>', '<');
					sscanf(temp_char, "%d", &intersection.Approaches[i].Ingress_Lanes[j].Connections[k].lane_num);
					free(data);
					//connection maneuver
					getline(map_file, lineread);
					sscanf(lineread.c_str(), "%s", token_char);
					temp_char = Getcontent(token_char, '>', '<');
					sscanf(temp_char, "%d", &intersection.Approaches[i].Ingress_Lanes[j].Connections[k].Maneuver);
					free(data);

					getline(map_file, lineread);  //</Lane_Connection>
				}
				getline(map_file, lineread);  //</Lane_Connections>
			}
			//end_lane indicator
			getline(map_file, lineread); // </Lane>

		}
		//-----------------------------------------------------------------------------------------------------------
		// Read No of egress lanes
		getline(map_file, lineread);
		sscanf(lineread.c_str(), "%s", token_char);
		temp_char = Getcontent(token_char, '>', '<');
		sscanf(temp_char, "%d", &intersection.Approaches[i].Egress_Lane_No);
		free(data);

		//initialize lanes first
		//Lane temp_lane;
		for (j = 0; j < intersection.Approaches[i].Egress_Lane_No; j++)
			intersection.Approaches[i].Egress_Lanes.push_back(temp_lane);

		for (j = 0; j < intersection.Approaches[i].Egress_Lane_No; j++)
		{
			//Lane ID
			getline(map_file, lineread); //<Lane Number = "1">
			intersection.Approaches[i].Egress_Lanes[j].ID = No_lanes;
			No_lanes++;

			//Lane type
			getline(map_file, lineread);
			sscanf(lineread.c_str(), "%s", token_char);
			temp_char = Getcontent(token_char, '>', '<');
			sscanf(temp_char, "%d", &intersection.Approaches[i].Egress_Lanes[j].Type);
			free(data);

			//Lane attributes
			getline(map_file, lineread);
			sscanf(lineread.c_str(), "%s", token_char);
			temp_char = Getcontent(token_char, '>', '<');
			int temp_attributes;
			sscanf(temp_char, "%d", &temp_attributes);
			free(data);
			//convert integer lane attributes to each bit
			for (l = 7; l >= 0; l--)
			{
				if (temp_attributes - pow(2, l) >= 0)
				{
					intersection.Approaches[i].Egress_Lanes[j].Attributes[l] = 1;
					temp_attributes = temp_attributes - pow(2, l);
				}
				else
					intersection.Approaches[i].Egress_Lanes[j].Attributes[l] = 0;
			}

			//Lane width
			getline(map_file, lineread);
			sscanf(lineread.c_str(), "%s", token_char);
			temp_char = Getcontent(token_char, '>', '<');
			sscanf(temp_char, "%d", &intersection.Approaches[i].Egress_Lanes[j].Width);
			free(data);

			if (intersection.Approaches[i].Egress_Lanes[j].Type == 1 || intersection.Approaches[i].Egress_Lanes[j].Type == 5)  //vehicle lane or cross walk
			{
				//Number of lane nodes
				getline(map_file, lineread);
				sscanf(lineread.c_str(), "%s", token_char);
				temp_char = Getcontent(token_char, '>', '<');
				sscanf(temp_char, "%d", &intersection.Approaches[i].Egress_Lanes[j].Node_No);
				free(data);

				//Initialize Nodes First
				N_LaneNodes temp_node;
				for (k = 0; k < intersection.Approaches[i].Egress_Lanes[j].Node_No; k++)
				{
					intersection.Approaches[i].Egress_Lanes[j].Nodes.push_back(temp_node);
				}

				getline(map_file, lineread); // <Nodes>

				//Lane node list
				for (k = 0; k < intersection.Approaches[i].Egress_Lanes[j].Node_No; k++)
				{

					getline(map_file, lineread); // <Node Number="1">
					intersection.Approaches[i].Egress_Lanes[j].Nodes[k].ID = k + 1;

					//Latitude
					getline(map_file, lineread);
					sscanf(lineread.c_str(), "%s", token_char);
					temp_char = Getcontent(token_char, '>', '<');
					sscanf(temp_char, "%lf", &intersection.Approaches[i].Egress_Lanes[j].Nodes[k].Latitude);
					free(data);

					//Longitude
					getline(map_file, lineread);
					sscanf(lineread.c_str(), "%s", token_char);
					temp_char = Getcontent(token_char, '>', '<');
					sscanf(temp_char, "%lf", &intersection.Approaches[i].Egress_Lanes[j].Nodes[k].Longitude);
					free(data);

					getline(map_file, lineread); // </Node>

					//transform to local X and Y data
					if (k == 0)  //first node: the offset should be to the reference point
					{
						//reference point
						geoCoord geoPoint;
						double ecef_x, ecef_y, ecef_z;
						double local_x, local_y, local_z;
						geoPoint.init(intersection.Ref_Long, intersection.Ref_Lat, intersection.Ref_Ele);
						geoPoint.lla2ecef(intersection.Approaches[i].Egress_Lanes[j].Nodes[k].Longitude, intersection.Approaches[i].Egress_Lanes[j].Nodes[k].Latitude, intersection.Ref_Ele, &ecef_x, &ecef_y, &ecef_z);
						geoPoint.ecef2local(ecef_x, ecef_y, ecef_z, &local_x, &local_y, &local_z);
						intersection.Approaches[i].Egress_Lanes[j].Nodes[k].E_Offset = local_y;
						intersection.Approaches[i].Egress_Lanes[j].Nodes[k].N_Offset = local_x;
					}
					else  //not the first node: the offset should be to the previous node
					{
						//reference point
						geoCoord geoPoint;
						double ecef_x, ecef_y, ecef_z;
						double local_x, local_y, local_z;
						geoPoint.init(intersection.Approaches[i].Egress_Lanes[j].Nodes[k - 1].Longitude, intersection.Approaches[i].Egress_Lanes[j].Nodes[k - 1].Latitude, intersection.Ref_Ele);
						geoPoint.lla2ecef(intersection.Approaches[i].Egress_Lanes[j].Nodes[k].Longitude, intersection.Approaches[i].Egress_Lanes[j].Nodes[k].Latitude, intersection.Ref_Ele, &ecef_x, &ecef_y, &ecef_z);
						geoPoint.ecef2local(ecef_x, ecef_y, ecef_z, &local_x, &local_y, &local_z);
						intersection.Approaches[i].Egress_Lanes[j].Nodes[k].E_Offset = local_y;
						intersection.Approaches[i].Egress_Lanes[j].Nodes[k].N_Offset = local_x;
					}
				}

				getline(map_file, lineread); // </Nodes>

			}

			if (intersection.Approaches[i].Egress_Lanes[j].Type == 2)  //Computed lane
			{
				//Reference Lane
				getline(map_file, lineread);
				sscanf(lineread.c_str(), "%s", token_char);
				temp_char = Getcontent(token_char, '>', '<');
				sscanf(temp_char, "%d", &intersection.Approaches[i].Egress_Lanes[j].RefLane.LaneID);
				free(data);

				//lateral offset
				getline(map_file, lineread);
				sscanf(lineread.c_str(), "%s", token_char);
				temp_char = Getcontent(token_char, '>', '<');
				sscanf(temp_char, "%d", &intersection.Approaches[i].Egress_Lanes[j].RefLane.LateralOffset);
				free(data);
			}


			//Number of lane connections
			getline(map_file, lineread);
			sscanf(lineread.c_str(), "%s", token_char);
			temp_char = Getcontent(token_char, '>', '<');
			sscanf(temp_char, "%d", &intersection.Approaches[i].Egress_Lanes[j].Connection_No);
			free(data);

			if (intersection.Approaches[i].Egress_Lanes[j].Connection_No > 0)
			{
				//Initialize lane connections
				LaneConnection temp_connection;
				for (k = 0; k < intersection.Approaches[i].Egress_Lanes[j].Connection_No; k++)
				{
					intersection.Approaches[i].Egress_Lanes[j].Connections.push_back(temp_connection);
				}
				//start to read connections
				for (k = 0; k < intersection.Approaches[i].Egress_Lanes[j].Connection_No; k++)
				{
					getline(map_file, lineread); //<Lane_Connections>
					getline(map_file, lineread);  //<Lane_Connection Number = "1">
					//connection lane ID
					getline(map_file, lineread);
					sscanf(lineread.c_str(), "%s", token_char);
					temp_char = Getcontent(token_char, '>', '<');
					sscanf(temp_char, "%d", &intersection.Approaches[i].Egress_Lanes[j].Connections[k].lane_num);
					free(data);
					//connection maneuver
					getline(map_file, lineread);
					sscanf(lineread.c_str(), "%s", token_char);
					temp_char = Getcontent(token_char, '>', '<');
					sscanf(temp_char, "%d", &intersection.Approaches[i].Egress_Lanes[j].Connections[k].Maneuver);
					free(data);

					getline(map_file, lineread);  //</Lane_Connection>
				}
				getline(map_file, lineread);  //</Lane_Connections>
			}
			//end_lane indicator
			getline(map_file, lineread); // </Lane>

		}
		getline(map_file, lineread); // </Approach>
	}

	getline(map_file, lineread); // </Geometry>
	getline(map_file, lineread); // </J2735.GID.blob>
	
return 0;
}






//This function parse information from the map description file and save to the MAP data structure
int NMAP::ParseIntersection(char* filename)
{
	//Start parsing the MAP data from file
	ifstream map_file; 
    map_file.open(filename);

	    if (!map_file)
    {
        cout<<"Error: Open file "<<filename<<endl;
        return -1;
    }

	string lineread;
	char token_char [DEFAULT_ARRAY_SIZE];
	char temp_char [DEFAULT_ARRAY_SIZE];
	string token;
    // Read first 4 lines of Intersection information
	int i,j,k,l;
	for(i=0;i<4;i++)
	{
		getline(map_file, lineread); // Read line by line
		sscanf(lineread.c_str(), "%s",token_char);
		//token.assign(token_char);
	    if(strcmp(token_char,"MAP_Name")==0)
		{
			sscanf(lineread.c_str(), "%*s %s",temp_char);
			intersection.Map_Name.assign(temp_char);
		}
		else if(strcmp(token_char,"IntersectionID")==0)
		{
			sscanf(lineread.c_str(), "%*s %s",temp_char);
			intersection.ID=atoi(temp_char);
		}
		//else if(strcmp(token_char,"Intersection_attributes")==0)
		//{
		//	sscanf(lineread.c_str(), "%*s %s",temp_char);
		//	for (j=0;j<8;j++)
		//	{
		//		intersection.Attributes[7-j]=temp_char[j]-48;  //directly use ASCII codes,0 is 48,1 is 49, comply with bit position
		//	}
		//}
		else if(strcmp(token_char,"Reference_point")==0)
		{
			sscanf(lineread.c_str(), "%*s %lf %lf %lf",&intersection.Ref_Lat,&intersection.Ref_Long,&intersection.Ref_Ele);
			intersection.Ref_Ele = intersection.Ref_Ele / 10;   //change from decimeter to meters
		}
		else if(strcmp(token_char,"No_Approach")==0)
		{
			sscanf(lineread.c_str(), "%*s %d",&intersection.Appro_No);
		}
	}
//end of reading intersection information


//initialize approaches
Approach_N temp_appro;
for(i=0;i<intersection.Appro_No;i++)
{
	intersection.Approaches.push_back(temp_appro);
}


//Start to parse the approaches, lanes and lane nodes
	for(i=0;i<intersection.Appro_No;i++)
	{
		//Intersection ID
		getline(map_file, lineread);
		sscanf(lineread.c_str(), "%s", token_char);
		if (strcmp(token_char, "Approach") == 0)
		{
			sscanf(lineread.c_str(), "%*s %d", &intersection.Approaches[i].ID);
		}

		// Read No of ingress lanes
		getline(map_file, lineread); 
		sscanf(lineread.c_str(), "%s",token_char);
		if (strcmp(token_char, "No_Ingress_Lane") == 0)
		{
			sscanf(lineread.c_str(), "%*s %d", &intersection.Approaches[i].Ingress_Lane_No);
		}
		//initialize lanes first
		Lane temp_lane;
		for (j = 0; j < intersection.Approaches[i].Ingress_Lane_No; j++)
			intersection.Approaches[i].Ingress_Lanes.push_back(temp_lane);

		//Parse Ingress lanes
		for (j = 0; j < intersection.Approaches[i].Ingress_Lane_No; j++)
		{
			//Lane ID
			getline(map_file, lineread);
			sscanf(lineread.c_str(), "%s", token_char);
			if (strcmp(token_char, "Lane_ID") == 0)
				sscanf(lineread.c_str(), "%*s %d", &intersection.Approaches[i].Ingress_Lanes[j].ID);

			//Lane type
			getline(map_file, lineread);
			sscanf(lineread.c_str(), "%s", token_char);
			if (strcmp(token_char, "Lane_type") == 0)
				sscanf(lineread.c_str(), "%*s %d", &intersection.Approaches[i].Ingress_Lanes[j].Type);

			//Lane attributes
			getline(map_file, lineread);
			sscanf(lineread.c_str(), "%s", token_char);
			int temp_attributes;
			if (strcmp(token_char, "Lane_attributes") == 0)
				sscanf(lineread.c_str(), "%*s %d", &temp_attributes);
			//convert integer lane attributes to each bit
			for (l = 7; l >= 0; l--)
			{
				if (temp_attributes - pow(2, l) >= 0)
				{
					intersection.Approaches[i].Ingress_Lanes[j].Attributes[l] = 1;
					temp_attributes = temp_attributes - pow(2, l);
				}
				else
					intersection.Approaches[i].Ingress_Lanes[j].Attributes[l] = 0;
			}

			//Lane width
			getline(map_file, lineread);
			sscanf(lineread.c_str(), "%s", token_char);
			if (strcmp(token_char, "Lane_width") == 0)
				sscanf(lineread.c_str(), "%*s %d", &intersection.Approaches[i].Ingress_Lanes[j].Width);

			if (intersection.Approaches[i].Ingress_Lanes[j].Type == 1 || intersection.Approaches[i].Ingress_Lanes[j].Type == 5)  //vehicle lane or cross walk
			{
				//Number of lane nodes
				getline(map_file, lineread);
				sscanf(lineread.c_str(), "%s", token_char);
				if (strcmp(token_char, "No_nodes") == 0)
					sscanf(lineread.c_str(), "%*s %d", &intersection.Approaches[i].Ingress_Lanes[j].Node_No);

				//Initialize Nodes First
				N_LaneNodes temp_node;
				for (k = 0; k < intersection.Approaches[i].Ingress_Lanes[j].Node_No; k++)
				{
					intersection.Approaches[i].Ingress_Lanes[j].Nodes.push_back(temp_node);
				}

				//Lane node list
				for (k = 0; k < intersection.Approaches[i].Ingress_Lanes[j].Node_No; k++)
				{
					getline(map_file, lineread); // Read line by line
					sscanf(lineread.c_str(), "%s", token_char);
					//get lat, long data
					sscanf(lineread.c_str(), "%*s %lf %lf", &intersection.Approaches[i].Ingress_Lanes[j].Nodes[k].Latitude,
						&intersection.Approaches[i].Ingress_Lanes[j].Nodes[k].Longitude);
					//transform to local X and Y data
					if (k == 0)  //first node: the offset should be to the reference point
					{
						//reference point
						geoCoord geoPoint;
						double ecef_x, ecef_y, ecef_z;
						double local_x, local_y, local_z;
						geoPoint.init(intersection.Ref_Long, intersection.Ref_Lat, intersection.Ref_Ele);
						geoPoint.lla2ecef(intersection.Approaches[i].Ingress_Lanes[j].Nodes[k].Longitude, intersection.Approaches[i].Ingress_Lanes[j].Nodes[k].Latitude, intersection.Ref_Ele, &ecef_x, &ecef_y, &ecef_z);
						geoPoint.ecef2local(ecef_x, ecef_y, ecef_z, &local_x, &local_y, &local_z);
						intersection.Approaches[i].Ingress_Lanes[j].Nodes[k].E_Offset = local_y;
						intersection.Approaches[i].Ingress_Lanes[j].Nodes[k].N_Offset = local_x;
					}
					else  //not the first node: the offset should be to the previous node
					{
						//reference point
						geoCoord geoPoint;
						double ecef_x, ecef_y, ecef_z;
						double local_x, local_y, local_z;
						geoPoint.init(intersection.Approaches[i].Ingress_Lanes[j].Nodes[k - 1].Longitude, intersection.Approaches[i].Ingress_Lanes[j].Nodes[k - 1].Latitude, intersection.Ref_Ele);
						geoPoint.lla2ecef(intersection.Approaches[i].Ingress_Lanes[j].Nodes[k].Longitude, intersection.Approaches[i].Ingress_Lanes[j].Nodes[k].Latitude, intersection.Ref_Ele, &ecef_x, &ecef_y, &ecef_z);
						geoPoint.ecef2local(ecef_x, ecef_y, ecef_z, &local_x, &local_y, &local_z);
						intersection.Approaches[i].Ingress_Lanes[j].Nodes[k].E_Offset = local_y;
						intersection.Approaches[i].Ingress_Lanes[j].Nodes[k].N_Offset = local_x;
					}
				}
			}
			if (intersection.Approaches[i].Ingress_Lanes[j].Type == 2)  //Computed lane
			{
				//Reference Lane
				getline(map_file, lineread); // Read line by line
				sscanf(lineread.c_str(), "%s %d", token_char, &intersection.Approaches[i].Ingress_Lanes[j].RefLane.LaneID);

				//Lateral Offset
				getline(map_file, lineread); // Read line by line
				sscanf(lineread.c_str(), "%s %d", token_char, &intersection.Approaches[i].Ingress_Lanes[j].RefLane.LateralOffset);
			}
			//Lane connection
			getline(map_file, lineread); // Read line by line
			sscanf(lineread.c_str(), "%s %d", token_char, &intersection.Approaches[i].Ingress_Lanes[j].Connection_No);
			//Initialize lane connections
			LaneConnection temp_connection;
			for (k = 0; k<intersection.Approaches[i].Ingress_Lanes[j].Connection_No; k++)
			{
				intersection.Approaches[i].Ingress_Lanes[j].Connections.push_back(temp_connection);
			}
			//start to read connections
			for (k = 0; k<intersection.Approaches[i].Ingress_Lanes[j].Connection_No; k++)
			{
				getline(map_file, lineread); // Read line by line
				sscanf(lineread.c_str(), "%d %d",&intersection.Approaches[i].Ingress_Lanes[j].Connections[k].lane_num, &intersection.Approaches[i].Ingress_Lanes[j].Connections[k].Maneuver);			
			}
			//end_lane indicator
			getline(map_file, lineread); // Read line by line

		}

		// Read No of Egress lanes
		getline(map_file, lineread);
		sscanf(lineread.c_str(), "%s", token_char);
		if (strcmp(token_char, "No_Egress_Lane") == 0)
		{
			sscanf(lineread.c_str(), "%*s %d", &intersection.Approaches[i].Egress_Lane_No);
		}
		//initialize lanes first
		//Lane temp_lane;
		for (j = 0; j < intersection.Approaches[i].Egress_Lane_No; j++)
			intersection.Approaches[i].Egress_Lanes.push_back(temp_lane);

		//Parse Egress lanes
		for (j = 0; j < intersection.Approaches[i].Egress_Lane_No; j++)
		{
			//Lane ID
			getline(map_file, lineread);
			sscanf(lineread.c_str(), "%s", token_char);
			if (strcmp(token_char, "Lane_ID") == 0)
				sscanf(lineread.c_str(), "%*s %d", &intersection.Approaches[i].Egress_Lanes[j].ID);

			//Lane type
			getline(map_file, lineread);
			sscanf(lineread.c_str(), "%s", token_char);
			if (strcmp(token_char, "Lane_type") == 0)
				sscanf(lineread.c_str(), "%*s %d", &intersection.Approaches[i].Egress_Lanes[j].Type);

			//Lane attributes
			getline(map_file, lineread);
			sscanf(lineread.c_str(), "%s", token_char);
			int temp_attributes;
			if (strcmp(token_char, "Lane_attributes") == 0)
				sscanf(lineread.c_str(), "%*s %d", &temp_attributes);
			//convert integer lane attributes to each bit
			for (l = 7; l >= 0; l--)
			{
				if (temp_attributes - pow(2, l) >= 0)
				{
					intersection.Approaches[i].Egress_Lanes[j].Attributes[l] = 1;
					temp_attributes = temp_attributes - pow(2, l);
				}
				else
					intersection.Approaches[i].Egress_Lanes[j].Attributes[l] = 0;
			}

			//Lane width
			getline(map_file, lineread);
			sscanf(lineread.c_str(), "%s", token_char);
			if (strcmp(token_char, "Lane_width") == 0)
				sscanf(lineread.c_str(), "%*s %d", &intersection.Approaches[i].Egress_Lanes[j].Width);

			if (intersection.Approaches[i].Egress_Lanes[j].Type == 1 || intersection.Approaches[i].Egress_Lanes[j].Type == 5)  //vehicle lane or cross walk
			{
				//Number of lane nodes
				getline(map_file, lineread);
				sscanf(lineread.c_str(), "%s", token_char);
				if (strcmp(token_char, "No_nodes") == 0)
					sscanf(lineread.c_str(), "%*s %d", &intersection.Approaches[i].Egress_Lanes[j].Node_No);

				//Initialize Nodes First
				N_LaneNodes temp_node;
				for (k = 0; k < intersection.Approaches[i].Egress_Lanes[j].Node_No; k++)
				{
					intersection.Approaches[i].Egress_Lanes[j].Nodes.push_back(temp_node);
				}

				//Lane node list
				for (k = 0; k < intersection.Approaches[i].Egress_Lanes[j].Node_No; k++)
				{
					getline(map_file, lineread); // Read line by line
					sscanf(lineread.c_str(), "%s", token_char);
					//get lat, long data
					sscanf(lineread.c_str(), "%*s %lf %lf", &intersection.Approaches[i].Egress_Lanes[j].Nodes[k].Latitude,
						&intersection.Approaches[i].Egress_Lanes[j].Nodes[k].Longitude);
					//transform to local X and Y data
					if (k == 0)  //first node: the offset should be to the reference point
					{
						//reference point
						geoCoord geoPoint;
						double ecef_x, ecef_y, ecef_z;
						double local_x, local_y, local_z;
						geoPoint.init(intersection.Ref_Long, intersection.Ref_Lat, intersection.Ref_Ele);
						geoPoint.lla2ecef(intersection.Approaches[i].Egress_Lanes[j].Nodes[k].Longitude, intersection.Approaches[i].Egress_Lanes[j].Nodes[k].Latitude, intersection.Ref_Ele, &ecef_x, &ecef_y, &ecef_z);
						geoPoint.ecef2local(ecef_x, ecef_y, ecef_z, &local_x, &local_y, &local_z);
						intersection.Approaches[i].Egress_Lanes[j].Nodes[k].E_Offset = local_y;
						intersection.Approaches[i].Egress_Lanes[j].Nodes[k].N_Offset = local_x;
					}
					else  //not the first node: the offset should be to the previous node
					{
						//reference point
						geoCoord geoPoint;
						double ecef_x, ecef_y, ecef_z;
						double local_x, local_y, local_z;
						geoPoint.init(intersection.Approaches[i].Egress_Lanes[j].Nodes[k - 1].Longitude, intersection.Approaches[i].Egress_Lanes[j].Nodes[k - 1].Latitude, intersection.Ref_Ele);
						geoPoint.lla2ecef(intersection.Approaches[i].Egress_Lanes[j].Nodes[k].Longitude, intersection.Approaches[i].Egress_Lanes[j].Nodes[k].Latitude, intersection.Ref_Ele, &ecef_x, &ecef_y, &ecef_z);
						geoPoint.ecef2local(ecef_x, ecef_y, ecef_z, &local_x, &local_y, &local_z);
						intersection.Approaches[i].Egress_Lanes[j].Nodes[k].E_Offset = local_y;
						intersection.Approaches[i].Egress_Lanes[j].Nodes[k].N_Offset = local_x;
					}
				}
			}
			if (intersection.Approaches[i].Egress_Lanes[j].Type == 2)  //Computed lane
			{
				//Reference Lane
				getline(map_file, lineread); // Read line by line
				sscanf(lineread.c_str(), "%s %d", token_char, &intersection.Approaches[i].Egress_Lanes[j].RefLane.LaneID);

				//Lateral Offset
				getline(map_file, lineread); // Read line by line
				sscanf(lineread.c_str(), "%s %d", token_char, &intersection.Approaches[i].Egress_Lanes[j].RefLane.LateralOffset);
			}
			//Lane connection
			getline(map_file, lineread); // Read line by line
			sscanf(lineread.c_str(), "%s %d", token_char, &intersection.Approaches[i].Egress_Lanes[j].Connection_No);
			//Initialize lane connections
			LaneConnection temp_connection;
			for (k = 0; k<intersection.Approaches[i].Egress_Lanes[j].Connection_No; k++)
			{
				intersection.Approaches[i].Egress_Lanes[j].Connections.push_back(temp_connection);
			}
			//start to read connections
			for (k = 0; k<intersection.Approaches[i].Egress_Lanes[j].Connection_No; k++)
			{
				getline(map_file, lineread); // Read line by line
				sscanf(lineread.c_str(), "%d %d", &intersection.Approaches[i].Egress_Lanes[j].Connections[k].lane_num, &intersection.Approaches[i].Egress_Lanes[j].Connections[k].Maneuver);
			}
			//end_lane indicator
			getline(map_file, lineread); // Read line by line

		}

		//end_approach indicator
		getline(map_file, lineread); // Read line by line	
	}
//The end line
getline(map_file, lineread); // Read line by line
sscanf(lineread.c_str(), "%s",token_char);
if(strcmp(token_char,"end_map")==0)
{
	cout<<"Parse map file successfully!"<<endl;
}
}
