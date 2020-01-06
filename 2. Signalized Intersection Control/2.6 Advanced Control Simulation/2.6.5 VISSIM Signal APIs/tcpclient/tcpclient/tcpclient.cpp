#include <WinSock2.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include "json.hpp"
#include <sstream>
#pragma comment(lib, "ws2_32.lib")
using namespace std;
using json = nlohmann::json;
void main()
{
	u_long iMode = 0; //iMode=0: blocking; iMode!=0, non-blocking
	WSADATA wsaData;
	std::string buff;
	json myjson;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("Failed to load Winsock");
		return;
	}
	SOCKADDR_IN addrSrv;
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(5999);
	addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);
	if (SOCKET_ERROR == sockClient){
		printf("Socket() error:%d", WSAGetLastError());
		return;
	}
	int iResult = ioctlsocket(sockClient, FIONBIO, &iMode);
	if (iResult != NO_ERROR)
	{
		printf("ioctlsocket (blocking setting) failed with error: %ld\n", iResult);
	}
	if (connect(sockClient, (struct  sockaddr*)&addrSrv, sizeof(addrSrv)) == INVALID_SOCKET){
		printf("Connect failed:%d", WSAGetLastError());
		return;
	}
	while (1)
	{
		try
		{
			int recv_buf_length = 0;
			int iRecv = recv(sockClient, (char*)&recv_buf_length, sizeof(long), 0);
			char* recv_buf = new(nothrow) char[recv_buf_length];
			iRecv = recv(sockClient, recv_buf, recv_buf_length, MSG_WAITALL);
			//cout << recv_buf << endl;
			std::stringstream(recv_buf) >> myjson;
			/////////////////////////////////////////
			// Place your control logic and then update the received obj's sg status
			for (auto it = myjson["det_list"].begin(); it != myjson["det_list"].end(); ++it)
			{
				float det_gaptime = it.value()["OccupancyGapTime"]; //feel free to collect any other information of this detector
			}
			for (auto it = myjson["sg_list"].begin(); it != myjson["sg_list"].end(); ++it)
			{
				if (it.value()["CurrentDuration"] >= 5.0)
				{
					int curstate = it.value()["CurrentState"];
					switch (curstate)
					{
					case 1:
						it.value()["CurrentState"] = 8;
						break;
					case 8:
						it.value()["CurrentState"] = 2;
						break;
					case 2:
						it.value()["CurrentState"] = 1;
						break;
					default:
						break;
					}
				}
			}
			////////////////////////////////////////////

			//Now, it's time to send out the new length of json
			std::string send_string = myjson.dump();
			int send_buf_size = (int)send_string.length();
			int iSend = send(sockClient, (char*)&send_buf_size, sizeof(int), 0);//first, tell VISSIM how long this msg is.
			char* sending_buf = new(nothrow) char[send_buf_size];
			iSend = send(sockClient, send_string.c_str(), send_buf_size, 0); //This is the real sending message
			//At this point, totally, two receivings and two sendings for each time stemp;
			delete sending_buf;
			delete recv_buf;
			/////////////////////////////
		}
		catch (std::exception & e)
		{
			const std::string str = e.what();
			cout << str << endl;
		}
	}
	//Just to complete, the program actually should never exit normally. 
	closesocket(sockClient);
	WSACleanup();
}