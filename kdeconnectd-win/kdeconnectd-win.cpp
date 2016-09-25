// kdeconnectd-win.cpp: Hauptprojektdatei.

#ifndef UNICODE
#define UNICODE
#endif

//#define WIN32_LEAN_AND_MEAN

#include "stdafx.h"
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>
#include <ctype.h>

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#define MY_PORT 1716;

int main()
{

	int iResult = 0;

	WSADATA wsaData;

	SOCKET RecvSocket;
	sockaddr_in RecvAddr, SenderAddr;

	unsigned short Port = MY_PORT;

	char RecvBuf[1024];
	int BufLen = 1024;

	int SenderAddrSize = sizeof(SenderAddr);

	//-----------------------------------------------
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		wprintf(L"WSAStartup failed with error %d\n", iResult);
		return 1;
	}
	//-----------------------------------------------
	// Create a receiver socket to receive datagrams
	RecvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (RecvSocket == INVALID_SOCKET) {
		wprintf(L"socket failed with error %d\n", WSAGetLastError());
		return 1;
	}
	//-----------------------------------------------
	// Bind the socket to any address and the specified port.
	RecvAddr.sin_family = AF_INET;
	RecvAddr.sin_port = htons(Port);
	RecvAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	iResult = bind(RecvSocket, (SOCKADDR *)& RecvAddr, sizeof(RecvAddr));
	if (iResult != 0) {
		wprintf(L"bind failed with error %d\n", WSAGetLastError());
		return 1;
	}
	//-----------------------------------------------
	// Call the recvfrom function to receive datagrams
	// on the bound socket.
	wprintf(L"Receiving datagrams...\n");
	iResult = recvfrom(RecvSocket,
		RecvBuf, BufLen, 0, (SOCKADDR *)& SenderAddr, &SenderAddrSize);
	if (iResult == SOCKET_ERROR) {
		wprintf(L"recvfrom failed with error %d\n", WSAGetLastError());
	}
	else
		printf("%s\n", RecvBuf);

	//-----------------------------------------------
	// Close the socket when finished receiving datagrams
	wprintf(L"Finished receiving. Closing socket.\n");
	iResult = closesocket(RecvSocket);
	if (iResult == SOCKET_ERROR) {
		wprintf(L"closesocket failed with error %d\n", WSAGetLastError());
		return 1;
	}

	//-----------------------------------------------
	// Clean up and exit.
	wprintf(L"Exiting.\n");
	WSACleanup();
	
	return 0;
}
