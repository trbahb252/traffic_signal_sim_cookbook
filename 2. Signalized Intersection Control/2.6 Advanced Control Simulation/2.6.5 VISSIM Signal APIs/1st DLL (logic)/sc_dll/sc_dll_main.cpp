/*==========================================================================*/
/*  SC_DLL_MAIN.CPP                                                         */
/*  Main module of a signal controller DLL for VISSIM.                      */
/*                                                                          */
/*  Version of 2007-01-17                                   Lukas Kautzsch  */
/*  ======================================================================  */
/* This code was modified by Pengfei (Taylor) Li @UT Arlington, Dec-2019   */

/* STOP! PLEASE READ THE DISCLAIMER.                                       */
/* This program will be delivered "AS IS". GPL open-source license applies */
/* This program depends on the APIs provided by PTV. The author comsumes no */
/* responsiblity of PTV's API changes in the future.                        */
/* Users are encouraged to ask questions in the Linkedin Traffic Sig CookBook formum*/
/*==========================================================================*/
#pragma once
#define _WIN32_WINNT 0x0501
//----Controller Initialization---
#define MIN_GREEN 5
#define MAX_GREEN 50
#define YELLOW 4
#define ALL_RED 2
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "lsa_fehl.h"
#include "sc_dll_functions.h"
#include "sc_dll_main.h"
#include <winsock2.h>
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
#include <stdlib.h>
#include <direct.h>
#include <string.h>
#include <map>
#include <math.h>
#include <sstream>
#include "json.hpp"
using json = nlohmann::json;
//using namespace std;
#pragma comment(lib, "ws2_32.lib")  
const int BUF_SIZE = 64;
WSADATA			wsd;			//
SOCKET			sHost;			//
SOCKET			sockConn;
SOCKADDR_IN		servAddr;		//
SOCKADDR_IN		clientAddr;
int clientLength;
char recvBuf[64] = { 0 };
std::string bufstr;
int	nServAddlen;
/*--------------------------------------------------------------------------*/

void  SC_DLL_Init (unsigned long sc_no) //this function is called only once for the specified external controller while simulation is starting...
{
	u_long iMode = 0; //iMode=0: blocking; iMode!=0, non-blocking
	SC_SetFrequency (sc_no, 10); //ScreenMessage("set frequency is 10!");
	//Dec-2019 Taylor Li: TCP/IP server is launched here. It occurs only once for the whole simulation. I used winsock.
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
	{
		WSACleanup();
		ScreenMessage("WASStartup failed!");
		exit(1);
	}
	sHost = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == sHost)
	{
		WSACleanup();
		ScreenMessage("socket shost cannot  be initialized...");
		exit(1);
	}
	int iResult = ioctlsocket(sHost, FIONBIO, &iMode);
	if (iResult != NO_ERROR)
	{
		printf("ioctlsocket (blocking setting) failed with error: %ld\n", iResult);
	}
	short portno = 5999;//Taylor Li, Dec-2019: This port can ben modified according to user's own preference
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servAddr.sin_port = htons(portno);
	servAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	nServAddlen = sizeof(servAddr);
	int checkCall = bind(sHost, (LPSOCKADDR)&servAddr, sizeof(SOCKADDR_IN));
	if (checkCall == SOCKET_ERROR)
	{
		ScreenMessage("Screen Message...");
		exit(1);
	}
	//ScreenMessage("binding succeeds!");
	if (listen(sHost, 10) == SOCKET_ERROR){
		ScreenMessage("Listen failed...");
		exit(1);
	}
	clientLength = sizeof(clientAddr);
	sockConn = accept(sHost, (SOCKADDR *)&clientAddr, &clientLength);
	if (sockConn == SOCKET_ERROR)
	{
		ScreenMessage("Accept Fail...");
		exit(1);
	}
	//Taylor Li, (Dec-2019): At this point, the TCP server is set up. But you will notice that VISSIM is completely frozen unless the external controller's program kicks in (i.e., it is a TCP client and set up a synchronous conneciton with this server)
} /* SC_DLL_Init */

/*--------------------------------------------------------------------------*/
void  SC_DLL_Calculate (unsigned long sc_no)
{	
	json sg_vector, det_vector, sc_vector,recv_json;//Taylor: Dec-2019: we use Json to transmit the object;
	//Step 1: Read all the signal group's status (at each simulation step)
	for (sg_iterator sg_iter = SignalGroups(); sg_iter.IsValid(); ++sg_iter)
	{
		int i = (*sg_iter).first; //Get sg ID
		json sg_json;
		sg_json["sg_no"] = i;
		sg_json["CurrentDuration"] = SG_CurrentDuration(sc_no, i);
		sg_json["CurrentState"] = SG_CurrentState(sc_no, i);
		sg_vector.push_back(sg_json);
	}

	//Step 2: Collect detectors data at current step (at each simulation step)
	for (detector_iterator det_iter = Detectors(); det_iter.IsValid(); ++det_iter)
	{
		int j = (*det_iter).first;//Get detector ID
		json myjson;
		myjson["det_no"] = j;
		myjson["presence"] = Det_Presence(sc_no, j);
		myjson["frontends"] = Det_FrontEnds(sc_no, j);
		myjson["rearends"] = Det_RearEnds(sc_no, j);
		myjson["OccupancyTime"] = Det_OccupancyTime(sc_no, j);
		myjson["Occupancyrate"] = Det_OccupancyRate(sc_no, j);
		myjson["OccupancyRateSmoothed"] = Det_OccupancyRateSmoothed(sc_no, j);
		myjson["OccupancyGapTime"] = Det_GapTime(sc_no, j);
		myjson["VehSpeed"] = Det_VehSpeed(sc_no, j);
		myjson["Type"] = Det_Type(sc_no, j);
		det_vector.push_back(myjson);
	}
	sc_vector["sg_list"] = sg_vector;
	sc_vector["det_list"] = det_vector;
	sc_vector["Sim_time_Now"] = Sim_Time();
	//////////////////////////////////////////////////////////////////////////
	//Step 3: Send all sg and detector data to external control logic (your independent tcp client program);
	try
	{
		std::string serialized_string = sc_vector.dump();
		int buf_size = (int)serialized_string.length();
		int iSend= send(sockConn, (char*)&buf_size,sizeof(int) , 0);//tell how long the following msg is
		if (iSend == SOCKET_ERROR)
		{
			ScreenMessage("1st msg length sending failed");
		}
		iSend = send(sockConn, serialized_string.c_str(), (int)serialized_string.length(),0); //real msg
		if (iSend == SOCKET_ERROR)
		{
			ScreenMessage("1st msg failed");
		}
		
		//Now let's receive the new sg states from external control logic
		int recv_buf_length = 0;
		int iRecv = recv(sockConn, (char*)&recv_buf_length, sizeof(long), 0); //Get the incoming msg length	

		char* receiving_buf = new char[recv_buf_length];
		iRecv = recv(sockConn, receiving_buf, recv_buf_length, 0);//At this point, the program will stop and listen to the port until the client sends in the latest signal states;
		
		std::stringstream(receiving_buf) >> recv_json;
		for (auto it = recv_json["sg_list"].begin(); it != recv_json["sg_list"].end(); ++it)
		{
			int sg_no = it.value()["sg_no"];
			int sg_state = it.value()["CurrentState"];
			SG_SetState(sc_no, sg_no, sg_state, 0);
		}
		
		delete receiving_buf;//clean the memory
	}
	catch (std::exception& e)
	{
		const std::string str = e.what();
		char *caution = const_cast<char *>(str.c_str());
		ScreenMessage(caution);
	}
} /* SC_DLL_Calculate */

/*--------------------------------------------------------------------------*/

void  SC_DLL_Cleanup (unsigned long sc_no)
{

	closesocket(sockConn);
	closesocket(sHost);
	WSACleanup();
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
