/*==========================================================================*/
/*  SC_DLL_MAIN.CPP                                                         */
/*                                                                          */
/*  Main module of a signal controller DLL for VISSIM.                      */
/*                                                                          */
/*  Version of 2007-01-17                                   Lukas Kautzsch  */
/* This code is modified by Pengfei Li and Monty Abbas @VaTech, Aug-08      */
/*==========================================================================*/
//Boost is an open c++ library in source code form. For more info, please visit www. boost.org
#pragma once
#ifndef BOOST_DATE_TIME_NO_LIB 
#define BOOST_DATE_TIME_NO_LIB
#endif
#define _WIN32_WINNT 0x0501
//----Controller Initialization---
#define MIN_GREEN 5
#define MAX_GREEN 50
#define YELLOW 4
#define ALL_RED 2
#include "lsa_fehl.h"
#include "sc_dll_functions.h"
#include "sc_dll_main.h"
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <fstream>
#include <iomanip>/*This is for float array output to a file*/
#include <iostream>
#include <cstdio>
#include <vector>
#include <fstream>
#include <string>
#include <boost/asio.hpp>
#include <stdlib.h>
#include <direct.h>
#include <string.h>
#include <map>
#include <math.h>
#include "shm_obj.h"
using namespace std;
using boost::asio::ip::tcp;


int portnumber;
boost::asio::io_service io_service; //main asio object
tcp::socket mysocket(io_service);  //create a socket

/*--------------------------------------------------------------------------*/
//global variable
char smemory_name[100]; //shared memory name for real-time det/sg states
/*-----------------------------*/

void  SC_DLL_ReadDataFiles (unsigned long sc_no,
                            char *filename1, char *filename2)
{
	fstream iofile(filename1);
	std::string line;
	int counter=0;
	if(iofile.is_open())
	{
		while(!iofile.eof())
		{
			getline(iofile,line);
			if(counter==1)
			{
				break;
			}
			counter++;
		}
		portnumber=atoi(line.substr(5,6).c_str());
	}


} /* SC_DLL_ReadDataFiles */

/*--------------------------------------------------------------------------*/

void  SC_DLL_Init (unsigned long sc_no)
{
	SC_SetFrequency (sc_no, 1);
	//----TCP/IP servers are set up here
	tcp::endpoint endpoint(tcp::v4(), portnumber); //endpoint
	tcp::acceptor acceptor(io_service, endpoint);  //we accept connection there
	acceptor.accept(mysocket); //attach to the acceptor, now the server begins to listen to the port to see any connecting request. 

} /* SC_DLL_Init */

/*--------------------------------------------------------------------------*/

void  SC_DLL_Calculate (unsigned long sc_no)
{
	
	boost::array<ControllerStates,1>controller;//This is the real-time reader data sent via tcp/ip socket
	boost::array<ControllerStates,1>controller1;//This is the real-time reader data sent via tcp/ip socket


	//Collect all signal groups info at current step
	for (sg_iterator sg_iter=SignalGroups();sg_iter.IsValid(); ++sg_iter)
	{
		int i=(*sg_iter).first; //Get sg ID
		SignalGroupStates *sg=new SignalGroupStates(sc_no,i);
		sg->SignalGroupNumber=i;
		sg->duration=SG_CurrentDuration(sc_no, i);
		sg->state=SG_CurrentState(sc_no, i);
		controller[0].add_sgs(*sg);
	}
	//collect detectors data at current step
	for (detector_iterator det_iter=Detectors();det_iter.IsValid();++det_iter)
	{
		int j=(*det_iter).first;//Get detector ID
		DetectorStates *det=new DetectorStates(sc_no);
		det->det_no=j;
		det->Detection=Det_Detection(sc_no, j);
		det->Presence=Det_Presence(sc_no, j);
		det->FrontEnds=Det_FrontEnds(sc_no, j);
		det->RearEnds=Det_RearEnds(sc_no,j);
		det->OccupancyTime=Det_OccupancyTime (sc_no, j);
		det->OccupancyRate=Det_OccupancyRate (sc_no, j);
		det->OccupancyRateSmoothed=Det_OccupancyRateSmoothed (sc_no, j);
		det->GapTime=Det_GapTime (sc_no, j);
		det->VehSpeed=Det_VehSpeed (sc_no, j);
		det->VehLength=Det_VehLength (sc_no, j);
		det->Type=Det_Type (sc_no, j);
		controller[0].add_dets(*det);
	}
	  //////////////////////////////////////////////////////////////////////////
	  //Send all the readers' Data to socket
	  mysocket.send(boost::asio::buffer(controller));
	  mysocket.receive(boost::asio::buffer(controller1));
	  
	  //for(int iter1=1;iter1<=controller[0].sg_index;iter1++)
	  //{
		 // SG_SetState(sc_no, controller[0].sgs[iter1].SignalGroupNumber, controller[0].sgs[iter1].state, 0);
	  //}

} /* SC_DLL_Calculate */

/*--------------------------------------------------------------------------*/

void  SC_DLL_Cleanup (unsigned long sc_no)
{
	mysocket.close();
	io_service.stop();
} /* SC_DLL_Cleanup */

/*--------------------------------------------------------------------------*/

void  SC_DLL_WriteSnapshotFile (unsigned long sc_no, char *filename)
{
  /* Writes the current internal data of SC number <sc_no> to the file */
  /* <filename> in the current directory.                              */
  /* This function is called from VISSIM once per SC when the user     */
  /* selects Save Snapshot from the Simulation menu during a           */
  /* simulation run.
  */
	//shared_memory_object::remove(smemory_name);// remove the shared memory to avoid memory leaking
	
  /* ### */

} /* SC_DLL_WriteSnapshotFile */

/*--------------------------------------------------------------------------*/

void  SC_DLL_ReadSnapshotFile (unsigned long sc_no, char *filename)
{
  /* Reads the current internal data of SC number <sc_no> from the file */
  /* <filename> in the current directory.                               */
  /* This function is called from VISSIM once per SC when the user      */
  /* selects Load Snapshot from the Simulation menu.                    */

  /* ### */

} /* SC_DLL_ReadSnapshotFile */

/*==========================================================================*/
/*  End of SC_DLL_MAIN.CPP                                                  */
/*==========================================================================*/
