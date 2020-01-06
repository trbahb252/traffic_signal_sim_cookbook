#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <iostream>
#include <vector>
using namespace boost::interprocess;
class SignalGroupStates
{
	public:
		SignalGroupStates(unsigned long sc_num,unsigned long number)
		{
			sc_no=sc_num;
			SignalGroupNumber=number;
		};
		SignalGroupStates()
		{
			duration=0;
			min_red=0;
			min_green=0;
			amber=0;
			redamber=0;
		};
		unsigned long SignalGroupNumber;
		int state; //current state
		double duration; //current phase duration
		int sc_no; //signal control ID
		double min_red; //min red
		double min_green; //minimum green
		double amber; // yellow time
		double redamber; // red-amber time, it is zero in North America
};
class DetectorStates
{
	public:
		DetectorStates(unsigned long number)
		{
			det_no=number;
		};
	//DetectorStates(const &DetectorStates);
		DetectorStates(){};
		unsigned long det_no;
		int Detection;//The definition is the same as in sc_dll_functions.h
		int Presence;
		int FrontEnds;
		int RearEnds;
		double FrontEndTime;
		double RearEndTime;
		double OccupancyTime;
		double OccupancyRate;
		double OccupancyRateSmoothed;
		double GapTime;
		double VehSpeed;
		double VehLength;
		short Type;
};

class ControllerStates
{
public:
	int det_index;//the dimension of dets array, this is NOT associate det ID
	int sg_index;//the dimension of dets array and this is NOT det ID.
	SignalGroupStates sgs[50];
	DetectorStates dets[50];
	int sender_done;
	int receiver_done;
	unsigned long sc_no;
	ControllerStates() //Construct function
	{
		sc_no=0;
		det_index=0; //how many associated detectors 
		sg_index=0; //how many associated signal groups
		sender_done=0;
		receiver_done=0;
	};

	void add_sgs(SignalGroupStates const &d1)
	{
		sg_index=sg_index+1;
		sgs[sg_index]=d1;
	};
	void add_dets(DetectorStates const &d2)
	{
		det_index=det_index+1;
		dets[det_index]=d2;
	};
	//mutex is a lock for synchronization of data access in shared memory 
	boost::interprocess::interprocess_mutex mutex; 
};
/*------(obsolete, but have referencing meaning for future---template<typename T1, typename T2> class MyController
{
	public:
		std::vector<T1> data_sgs; //Vector defined for future
		std::vector<T2> data_dets; //vector defined for future
		int det_index;//the dimension of dets array, this is NOT associate det ID
		int sg_index;//the dimension of dets array and this is NOT det ID.
		SignalGroupStates sgs[50];//Up to 50 signal groups could be set up 
		                          //for each controller so far, this value 
								  //could be increased contingent on the networks
		DetectorStates dets[50];//up to 50 detectors could be set up so far. 
		int sender_done;//Token of finishing sending data from VISSIM to memory;
		int receiver_done;//Token of finish receiving by independent controller;
		unsigned long sc_no; //Controller ID. 
		MyController() //Construct function
		{
			sc_no=0;
			det_index=0; //how many associated detectors 
			sg_index=0; //how many associated signal groups
			sender_done=0;
			receiver_done=0;
		};
		// This is to add a sg, vector is reserved for future. 
		template<typename T1> void add_sgs(T1 const &d1)
		{
			data_sgs.push_back(d1);
			sg_index=sg_index+1;
			sgs[sg_index]=d1;	
		};
		// This is to remove a sg
		template <typename T1> void Remove_sgs(int number)//remove No. number vector member.
		{
			data_sgs.erase(data_sgs.begin()+number-1);
			sgs[number-1]=NULL;
		};
		// This is to associate a new detector to controller, vector is reserved for future. 
		template<typename T2> void add_dets(T2 const &d2)
		{
			data_dets.push_back(d2);
			det_index=det_index+1;
			dets[det_index]=d2;
		};
		// This is to remove a detector from controller
		template <typename T2> void Remove_dets(int number)//remove No. number vector member.
		{
			data_dets.erase(data_dets.begin()+number-1);
		};
		//mutex is a lock for synchronization of data access in shared memory 
		boost::interprocess::interprocess_mutex mutex; 
		
};

----------------*/
class det_config // for each detector
{
public:
	float pos;
	float ext;
};
class dets_config //for all three detectors
{
public:
	det_config dets[3];
	boost::interprocess::interprocess_mutex mutex; 		
};
