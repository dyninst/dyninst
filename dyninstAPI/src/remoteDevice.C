/*
 * Copyright (c) 1998 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */


/* 
 * $Id: remoteDevice.C,v 1.1 2001/08/01 15:39:57 chadd Exp $
 */


#include "dyninstapi/h/remoteDevice.h"
#include "deviceStructs.h"
#include "common/h/Types.h"
#include "baseTrampTemplate.h"


#define PORTNUM               5000    // Port number  
#define MAX_PENDING_CONNECTS  2       // Maximum length of the queue 
                                      // of pending connections
//constructor
//start the socket listening for the device to 
//attach.

//ccw 2 aug 2000
//this also now launches the client on the CE device using
//the RAPI.H interface.
remoteDevice::remoteDevice(){
  int index = 0;                      // Integer index
  TCHAR szError[100];                 // Error message string

  SOCKET WinSocket = INVALID_SOCKET;  // Window socket
          // Socket for communicating 
                                      // between the server and client
  SOCKADDR_IN local_sin,              // Local socket address
              accept_sin;             // Receives the address of the 
                                      // connecting entity
  int accept_sin_len;                 // Length of accept_sin

  WSADATA WSAData;                    // Contains details of the Winsock
                                      // implementation


  // Initialize Winsock.
  if (WSAStartup (MAKEWORD(1,1), &WSAData) != 0) 
  {
    /*wsprintf (szError, TEXT("WSAStartup failed. Error: %d"), 
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
	*/
    success = FALSE;
  }

  // Create a TCP/IP socket, WinSocket.
  if ((WinSocket = socket (AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) 
  {
    /*wsprintf (szError, TEXT("Allocating socket failed. Error: %d"), 
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
	*/
    success = FALSE;
  }

 // Fill out the local socket's address information.
  local_sin.sin_family = AF_INET;
  local_sin.sin_port = htons (PORTNUM);  
  local_sin.sin_addr.s_addr = htonl (INADDR_ANY);

  // Associate the local address with WinSocket.
  if (bind (WinSocket, 
            (struct sockaddr *) &local_sin, 
            sizeof (local_sin)) == SOCKET_ERROR) 
  {
    /*wsprintf (szError, TEXT("Binding socket failed. Error: %d"), 
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);*/
    closesocket (WinSocket);
    success = FALSE;
  }

  // Establish a socket to listen for incoming connections.
  if (listen (WinSocket, MAX_PENDING_CONNECTS) == SOCKET_ERROR) 
  {
    /*wsprintf (szError, 
              TEXT("Listening to the client failed. Error: %d"),
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
	*/
    closesocket (WinSocket);
    success = FALSE;
  }

  accept_sin_len = sizeof (accept_sin);



	//// ccw 2 aug 2000
	//launch the CE client when we are sure the socket creation has
	//succeded.
	////
	HRESULT hr = CeRapiInit();   
	WCHAR *appName= L"testRemoteCE.exe";
	bool result;
	//PROCESS_INFORMATION  procInfo;
	if ( FAILED(hr) )    {
		cout<<" SPAWN FAILED"<<endl;
		exit(-1);
	}
	
	result = 1;
	if ( result ==0 )    {
		cout<<"ERR--"<< CeGetLastError() <<endl;
		exit(-11);  
	}
	hr = CeRapiGetError();
	if ( FAILED(hr) )    {     
		cout<<"ERR2--"<<endl;
		exit(-11);  
	}
	CeRapiUninit();

  // Accept an incoming connection attempt on WinSocket.
  deviceSocket = accept (WinSocket, 
                       (struct sockaddr *) &accept_sin, 
                       (int *) &accept_sin_len);

  // Stop listening for connections from clients.
  closesocket (WinSocket);

  if (deviceSocket == INVALID_SOCKET) 
  {
    /*wsprintf (szError, TEXT("Accepting connection with client failed.")
              TEXT(" Error: %d"), WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
	*/
    success = FALSE;
  }else{
	//good connection.
	//set up the baseTramp stuff....
	  if(!RemoteGetTrampTemplate()){
		cout <<" ERROR.. GETTING TRAMP ASM"<<endl;
	        exit(-1);
	 }
  }

}
 
bool remoteDevice::RemoteGetTrampTemplate(){
	//struct iGetTrampTemplate input;
	struct oGetTrampTemplate *output;
	int functionID = RGetTrampTemplate;
	int totalSize;
	int retVal;
	char *buffer;
	int received = 0;

	//construct input parameters
	//no input parameters

	WSASetLastError(0);

	retVal = send(deviceSocket, (char*) &functionID, sizeof(functionID), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	//retVal = send(deviceSocket, (char*) &totalSize, sizeof(totalSize),0);
	//do error checking here

	//retVal = send(deviceSocket, (char*) &input, totalSize, 0);

	// do error checking here

	retVal = recv(deviceSocket,(char*)  &totalSize, sizeof(totalSize), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	
	//allocate memory for the output parameters
	buffer = new char[totalSize];
	output = (struct oGetTrampTemplate*) buffer;


//	DebugBreak();
	WSASetLastError(0);
	received = recv(deviceSocket, (char*) output, totalSize, 0); //ccw 10 aug 2000
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	WSASetLastError(0);
	while(received < totalSize){
		retVal = received;
		received += recv(deviceSocket, (char*) &(buffer[retVal]), totalSize - retVal, 0);
		if(retVal == 0xffffffff){
			retVal = WSAGetLastError();
			DebugBreak();
		}
		WSASetLastError(0);
	}

	//do error checking here

	retVal = output->retVal;

	baseTrampMem = new char[output->bufferSize];
	memcpy(baseTrampMem, &output->buffer, output->bufferSize); 
	baseTramp = (Address) baseTrampMem;

	baseTemplate_savePreInsOffset = output->baseTramp_savePreInsOffset;
	baseTemplate_skipPreInsOffset = output->baseTramp_skipPreInsOffset;
	baseTemplate_globalPreOffset = output->baseTramp_globalPreOffset;
	baseTemplate_localPreOffset = output->baseTramp_localPreOffset;
	baseTemplate_localPreReturnOffset = output->baseTramp_localPreReturnOffset;
	baseTemplate_updateCostOffset = output->baseTramp_updateCostOffset;
	baseTemplate_restorePreInsOffset = output->baseTramp_restorePreInsOffset;
	baseTemplate_emulateInsOffset = output->baseTramp_emulateInsOffset;
	baseTemplate_skipPostInsOffset = output->baseTramp_skipPostInsOffset;
	baseTemplate_savePostInsOffset = output->baseTramp_savePostInsOffset;
	baseTemplate_globalPostOffset = output->baseTramp_globalPostOffset;
	baseTemplate_localPostOffset = output->baseTramp_localPostOffset;
	baseTemplate_localPostReturnOffset = output->baseTramp_localPostReturnOffset;
	baseTemplate_restorePostInsOffset = output->baseTramp_restorePostInsOffset;
	baseTemplate_returnInsOffset = output->baseTramp_returnInsOffset;

	baseTemplate_trampTemp = (unsigned long) baseTrampMem;
	baseTemplate_size = output->baseTrampSize;
	baseTemplate_cost = output->baseTrampCost;
	baseTemplate_prevBaseCost = output->baseTrampPrevBaseCost;
	baseTemplate_postBaseCost = output->baseTrampPostBaseCost;
	baseTemplate_prevInstru = output->baseTrampPrevInstru;
	baseTemplate_postInstru = output->baseTrampPostInstru;
	baseTramp_endTramp = output->baseTramp_endTrampOffset;
	/////non recursive
	baseNonRecursiveTrampMem= new char[output->bufferNRSize];
	memcpy(baseNonRecursiveTrampMem, &((&output->buffer)[output->bufferSize]), output->bufferNRSize); 
	baseNonRecursiveTramp = (Address) baseNonRecursiveTrampMem;

	nonRecursiveBaseTemplate_guardOffPost_beginOffset = output->baseNonRecursiveTramp_guardOffPost_beginOffset;

	nonRecursiveBaseTemplate_savePreInsOffset = output->baseNonRecursiveTramp_savePreInsOffset;
	nonRecursiveBaseTemplate_skipPreInsOffset = output->baseNonRecursiveTramp_skipPreInsOffset;
	nonRecursiveBaseTemplate_globalPreOffset  = output->baseNonRecursiveTramp_globalPreOffset;
	nonRecursiveBaseTemplate_localPreOffset = output->baseNonRecursiveTramp_localPreOffset;
	nonRecursiveBaseTemplate_localPreReturnOffset = output->baseNonRecursiveTramp_localPreReturnOffset;
	nonRecursiveBaseTemplate_updateCostOffset = output->baseNonRecursiveTramp_updateCostOffset;
	nonRecursiveBaseTemplate_restorePreInsOffset = output->baseNonRecursiveTramp_restorePreInsOffset;
	nonRecursiveBaseTemplate_emulateInsOffset = output->baseNonRecursiveTramp_emulateInsOffset;
	nonRecursiveBaseTemplate_skipPostInsOffset = output->baseNonRecursiveTramp_skipPostInsOffset;
	nonRecursiveBaseTemplate_savePostInsOffset = output->baseNonRecursiveTramp_savePostInsOffset;
	nonRecursiveBaseTemplate_globalPostOffset = output->baseNonRecursiveTramp_globalPostOffset;
	nonRecursiveBaseTemplate_localPostOffset = output->baseNonRecursiveTramp_localPostOffset;
	nonRecursiveBaseTemplate_localPostReturnOffset = output->baseNonRecursiveTramp_localPostReturnOffset;
	nonRecursiveBaseTemplate_restorePostInsOffset = output->baseNonRecursiveTramp_restorePostInsOffset;
	nonRecursiveBaseTemplate_returnInsOffset = output->baseNonRecursiveTramp_returnInsOffset;
	nonRecursiveBaseTemplate_guardOnPre_beginOffset = output->baseNonRecursiveTramp_guardOnPre_beginOffset;
	nonRecursiveBaseTemplate_guardOffPre_beginOffset = output->baseNonRecursiveTramp_guardOffPre_beginOffset;
	nonRecursiveBaseTemplate_guardOnPost_beginOffset = output->baseNonRecursiveTramp_guardOnPost_beginOffset;
	nonRecursiveBaseTemplate_guardOffPost_beginOffset = output->baseNonRecursiveTramp_guardOffPost_beginOffset;
	nonRecursiveBaseTemplate_guardOnPre_endOffset = output->baseNonRecursiveTramp_guardOnPre_endOffset;
	nonRecursiveBaseTemplate_guardOffPre_endOffset = output->baseNonRecursiveTramp_guardOffPre_endOffset;
	nonRecursiveBaseTemplate_guardOnPost_endOffset = output->baseNonRecursiveTramp_guardOnPost_endOffset;
	nonRecursiveBaseTemplate_guardOffPost_endOffset = output->baseNonRecursiveTramp_guardOffPost_endOffset;
	nonRecursiveBaseTemplate_trampTemp = (unsigned long)baseNonRecursiveTrampMem; //ccw 10 aug 2000
	nonRecursiveBaseTemplate_size = output->bufferNRSize;
	nonRecursiveBaseTemplate_cost = output->baseNonRecursiveTrampCost;
	nonRecursiveBaseTemplate_prevBaseCost = output->baseNonRecursiveTrampPrevBaseCost;
	nonRecursiveBaseTemplate_postBaseCost = output->baseNonRecursiveTrampPostBaseCost;
	nonRecursiveBaseTemplate_prevInstru = output->baseNonRecursiveTrampPrevInstru;
	nonRecursiveBaseTemplate_postInstru = output->baseNonRecursiveTrampPostInstru;

	free(buffer);

	return true;//retVal;
}

bool remoteDevice::test(LPCWSTR st1r,LPCWSTR s2tr, int y, LPCWSTR str){
	y=y*89;

	return true;
}

bool remoteDevice::RemoteCreateProcess(LPCWSTR lpApplicationName,LPCWSTR lpCommandLine,
LPSECURITY_ATTRIBUTES lpProcessAttributes,
LPSECURITY_ATTRIBUTES lpThreadAttributes, 
bool bInheritHandles, DWORD dwCreationFlags, LPVOID
lpEnvironment, 
LPCWSTR lpCurrentDirectory, LPSTARTUPINFO
lpStartupInfo, 
LPPROCESS_INFORMATION lpProcessInformation ){


	//LPCWSTR lpCommandLine = NULL;
	struct iCreateProcess *input;
	struct oCreateProcess output;
	int functionID = RCreateProcess;
	int totalSize;
	int retVal;
	int stringSize;
//	int outputSize;//ccw 4 oct 2000

	char *buffer;

	WSASetLastError(0);
	stringSize = 0;
	//allocate memory for the input parameters
	if(lpApplicationName == NULL){
		stringSize = 1;
	}else{
		stringSize = wcslen(lpApplicationName);
	}  
	stringSize *=2; // unicode is two bytes
	if(lpCommandLine == NULL){
		stringSize+=2;
	}else{
		stringSize += (wcslen(lpCommandLine) ) *2;
	}
	stringSize += 4 ; //for the two NULLs

	buffer = new char[sizeof(struct iCreateProcess)+stringSize];
	memset(buffer, 0, sizeof(struct iCreateProcess)+stringSize);

	input = (struct iCreateProcess*) buffer;

	input->lpApplicationNameSize = lpApplicationName ? 2*wcslen(lpApplicationName):0;
	input->lpCommandLineSize = lpCommandLine ? 2*wcslen(lpCommandLine):0;

	input->dwCreationFlagsSize = sizeof(DWORD);
	
	memcpy(&input->dwCreationFlags, &dwCreationFlags, input->dwCreationFlagsSize);
	input->lpApplicationNameOffset = sizeof(struct iCreateProcess);


	memcpy(&(((char*)input)[input->lpApplicationNameOffset]), lpApplicationName, input->lpApplicationNameSize);

	input->lpCommandLineOffset = input->lpApplicationNameOffset + input->lpApplicationNameSize + 2; //for the NULL
	if (lpCommandLine){
		memcpy(&(((char*)input)[input->lpCommandLineOffset]),lpCommandLine, 
		 input->lpCommandLineSize-2);
	}else{ 
		//memcpy(&(((char*)input)[input->lpCommandLineOffset]),"\0", 
		// input->lpCommandLineSize);
	}

	//send to device.
	totalSize = sizeof(struct iCreateProcess)+stringSize;



	//send function ID
	retVal = send(deviceSocket, (char*) &functionID, sizeof(functionID), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	//send size of incoming data
	retVal = send(deviceSocket, (char*) &totalSize, sizeof(totalSize),0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	//send the actual data
	retVal = send(deviceSocket, (char*) input, totalSize, 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here


	retVal = recv(deviceSocket, (char*) &output, sizeof(output), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}

	*lpProcessInformation = output.lpProcessInformation;

	retVal = output.retVal;

	free(buffer);
	
	return retVal;
}

HANDLE remoteDevice::RemoteOpenProcess( DWORD fdwAccess, bool fInherit, 
						 DWORD IDProcess){


	iOpenProcess input;
	oOpenProcess output;
	int functionID = ROpenProcess;
	int retVal;

	WSASetLastError(0);
	input.IDProcess = IDProcess;

	retVal = send(deviceSocket, (char*) &functionID, sizeof(functionID), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}

	retVal = send(deviceSocket, (char*) &input, sizeof(input), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	// do error checking here

	retVal = recv(deviceSocket, (char*) &output, sizeof(output), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}

	return output.retVal;

}



bool remoteDevice::RemoteWriteProcessMemory( HANDLE hProcess, LPVOID lpBaseAddress, 
											LPVOID lpBuffer, DWORD nSize, 
											LPDWORD lpNumberOfBytesWritten){

	struct iWriteProcessMemory *input;
	struct oWriteProcessMemory output;
	int	   functionID = RWriteProcessMemory;
	int	   totalSize;
	int    retVal;

	char *buffer;

	WSASetLastError(0);
	//allocate memory for the input parameters
	buffer = new char[sizeof(struct iWriteProcessMemory)+nSize];

	input = (struct iWriteProcessMemory *) buffer;
	
	//construct input parameters
	input->hProcessSize = sizeof(HANDLE);
	memcpy(&input->hProcess, &hProcess,input->hProcessSize);

	input->lpBaseAddressSize = sizeof(LPVOID);
	memcpy(&input->lpBaseAddress, &lpBaseAddress, input->lpBaseAddressSize);

	input->nSizeSize = sizeof(DWORD);
	memcpy(&input->nSize, &nSize, input->nSizeSize);

	input->lpBufferSize = nSize;
	memcpy(&input->lpBuffer, lpBuffer, input->lpBufferSize);

	
	//send to device.
	totalSize = sizeof(struct iWriteProcessMemory)+nSize;
	retVal = send(deviceSocket, (char*) &functionID, sizeof(functionID), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	retVal = send(deviceSocket, (char*) &totalSize, sizeof(totalSize),0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here
	
	int sent, totalLeft = totalSize;
	while(totalLeft > 0){ //ccw 10 aug 2000
		//send chunks of 1000!
		WSASetLastError(0);
 		sent = send(deviceSocket, (char*) &buffer[totalSize-totalLeft], (totalLeft > 999 ? 1000 : totalLeft), 0);
		if(retVal == 0xffffffff){
			retVal = WSAGetLastError();
			DebugBreak();
		}
		totalLeft -= sent;
	}


	//retVal = send(deviceSocket, (char*) input, totalSize, 0);

	// do error checking here
	retVal = recv(deviceSocket,(char*)  &totalSize, sizeof(totalSize), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}

	retVal = recv(deviceSocket, (char*) &output, totalSize, 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	*lpNumberOfBytesWritten = output.lpNumberOfBytesWritten;

	free(buffer);

	return output.retVal;
}


bool remoteDevice::RemoteReadProcessMemory( HANDLE hProcess, LPCVOID lpBaseAddress, 
											LPVOID lpBuffer, DWORD nSize, 
											LPDWORD	lpNumberOfBytesRead){

	struct iReadProcessMemory input;
	struct oReadProcessMemory *output;
	int	   functionID = RReadProcessMemory;
	int	   totalSize;
	int    retVal;
	bool	retValB;
	char *buffer;

	WSASetLastError(0);
	//construct input parameters
	input.hProcessSize = sizeof(HANDLE);
	memcpy(&input.hProcess, &hProcess,input.hProcessSize);

	input.lpBaseAddressSize = sizeof(LPVOID);
	memcpy(&input.lpBaseAddress, &lpBaseAddress, input.lpBaseAddressSize);

	input.nSizeSize = sizeof(DWORD);
	memcpy(&input.nSize, &nSize, input.nSizeSize);

	
	//send to device.
	totalSize = sizeof(struct iWriteProcessMemory);
	retVal = send(deviceSocket, (char*) &functionID, sizeof(functionID), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	retVal = send(deviceSocket, (char*) &totalSize, sizeof(totalSize),0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	retVal = send(deviceSocket, (char*) &input, totalSize, 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}

	// do error checking here

	//allocate memory for the output parameters
	buffer = new char[sizeof(struct oReadProcessMemory)+nSize];

	output = (struct oReadProcessMemory *) buffer;
	
	retVal = recv(deviceSocket,(char*)  &totalSize, sizeof(totalSize), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}


	int received = recv(deviceSocket, (char*) output, totalSize, 0); //ccw 10 aug 2000
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	WSASetLastError(0);
	while(received < totalSize){
		retVal = received;
		received += recv(deviceSocket, (char*) &(buffer[retVal]), totalSize - retVal, 0);
		if(retVal == 0xffffffff){
			retVal = WSAGetLastError();
			DebugBreak();
		}
		WSASetLastError(0);
	}

//	retVal = recv(deviceSocket, (char*) output, totalSize, 0);
	//do error checking here

	*lpNumberOfBytesRead = output->lpNumberOfBytesRead;
	memcpy(lpBuffer, (void*) &output->lpBuffer, *lpNumberOfBytesRead); 
	retValB = output->retVal;
	free(buffer);

	return retValB;
}


bool remoteDevice::RemoteDebugActiveProcess( DWORD dwProcessId ){

	struct iDebugActiveProcess input;
	struct oDebugActiveProcess output;
	int	   functionID = RDebugActiveProcess;
	int	   totalSize;
	int    retVal;

	WSASetLastError(0);
	//construct input parameters
	input.dwProcessIdSize = sizeof(DWORD);
	memcpy(&input.dwProcessId, &dwProcessId,input.dwProcessIdSize);

	//send to device.
	totalSize = sizeof(struct iDebugActiveProcess);
	retVal = send(deviceSocket, (char*) &functionID, sizeof(functionID), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	retVal = send(deviceSocket, (char*) &totalSize, sizeof(totalSize),0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	retVal = send(deviceSocket, (char*) &input, totalSize, 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}

	// do error checking here

	retVal = recv(deviceSocket,(char*)  &totalSize, sizeof(totalSize), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}

	retVal = recv(deviceSocket, (char*) &output, totalSize, 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	return output.retVal;
}


bool remoteDevice::RemoteGetThreadContext( HANDLE hThread, w32CONTEXT* lpContext){

	struct iGetThreadContext input;
	struct oGetThreadContext output;
	int	   functionID = RGetThreadContext;
	int	   totalSize;
	int    retVal;
	bool   retValB;


	WSASetLastError(0);
	//construct input parameters
	input.hThreadSize = sizeof(HANDLE);
	memcpy(&input.hThread, &hThread,input.hThreadSize);

	
	//send to device.
	totalSize = sizeof(struct iGetThreadContext);
	retVal = send(deviceSocket, (char*) &functionID, sizeof(functionID), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	retVal = send(deviceSocket, (char*) &totalSize, sizeof(totalSize),0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	retVal = send(deviceSocket, (char*) &input, totalSize, 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}

	// do error checking here

	retVal = recv(deviceSocket,(char*)  &totalSize, sizeof(totalSize), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}

	retVal = recv(deviceSocket, (char*) &output, totalSize, 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	memcpy(lpContext, &output.lpContext, output.lpContextSize);
	retValB = output.retVal;

	return retValB;
}

bool remoteDevice::RemoteSetThreadContext( HANDLE hThread, CONST w32CONTEXT * lpContext){
	struct iSetThreadContext input;
	struct oSetThreadContext output;
	int	   functionID = RSetThreadContext;
	int	   totalSize;
	int    retVal;
	
	WSASetLastError(0);
	//DebugBreak();
	//construct input parameters
	input.hThreadSize = sizeof(HANDLE);
	memcpy(&input.hThread, &hThread,input.hThreadSize);

	input.lpContextSize = sizeof(w32CONTEXT); 
	memcpy(&input.lpContext, lpContext, input.lpContextSize);


	//send to device.
	totalSize = sizeof(struct iSetThreadContext);
	retVal = send(deviceSocket, (char*) &functionID, sizeof(functionID), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	retVal = send(deviceSocket, (char*) &totalSize, sizeof(totalSize),0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	retVal = send(deviceSocket, (char*) &input, totalSize, 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}

	// do error checking here
	retVal = recv(deviceSocket,(char*)  &totalSize, sizeof(totalSize), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}

	retVal = recv(deviceSocket, (char*) &output, totalSize, 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	return output.retVal;
}

bool remoteDevice::RemoteWaitForDebugEvent(LPDEBUG_EVENT lpDebugEvent, 
											DWORD dwMilliseconds){

	struct iWaitForDebugEvent input;
	struct oWaitForDebugEvent output;
	int	   functionID = RWaitForDebugEvent;
	int	   totalSize;
	int    retVal;


	WSASetLastError(0);

	//construct input parameters
	input.dwMillisecondsSize= sizeof(DWORD);
	memcpy(&input.dwMilliseconds, &dwMilliseconds,input.dwMillisecondsSize);

	//send to device.
	totalSize = sizeof(struct iWaitForDebugEvent);
	retVal = send(deviceSocket, (char*) &functionID, sizeof(functionID), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	retVal = send(deviceSocket, (char*) &totalSize, sizeof(totalSize),0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	retVal = send(deviceSocket, (char*) &input, totalSize, 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	
	// do error checking here

	retVal = recv(deviceSocket,(char*)  &totalSize, sizeof(totalSize), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}

	retVal = recv(deviceSocket, (char*) &output, totalSize, 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	memcpy(lpDebugEvent, (void*) &output.lpDebugEvent, output.lpDebugEventSize);


	return output.retVal;
}

bool remoteDevice::RemoteContinueDebugEvent( DWORD dwProcessId, DWORD dwThreadId, 
											 DWORD dwContinueStatus ){

	struct iContinueDebugEvent input;
	struct oContinueDebugEvent output;
	int	   functionID = RContinueDebugEvent;
	int	   totalSize;
	int    retVal;

	WSASetLastError(0);
	//construct input parameters
	input.dwProcessIdSize = sizeof(DWORD);
	memcpy(&input.dwProcessId, &dwProcessId,input.dwProcessIdSize);

	input.dwContinueStatusSize = sizeof(DWORD);
	memcpy(&input.dwContinueStatus, &dwContinueStatus,input.dwContinueStatusSize);

	input.dwThreadIdSize = sizeof(DWORD);
	memcpy(&input.dwThreadId, &dwThreadId,input.dwThreadIdSize);

	
	//send to device.
	totalSize = sizeof(struct iContinueDebugEvent);
	retVal = send(deviceSocket, (char*) &functionID, sizeof(functionID), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	retVal = send(deviceSocket, (char*) &totalSize, sizeof(totalSize),0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	retVal = send(deviceSocket, (char*) &input, totalSize, 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}

	// do error checking here
	retVal = recv(deviceSocket,(char*)  &totalSize, sizeof(totalSize), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	
	retVal = recv(deviceSocket, (char*) &output, totalSize, 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	return output.retVal;
}


bool remoteDevice::RemoteResumeThread(HANDLE hThread){
	struct iResumeThread input;
	struct oResumeThread output;
	int	   functionID = RResumeThread;
	int	   totalSize;
	int    retVal;


	WSASetLastError(0);
	//construct input parameters
	input.hThreadSize = sizeof(HANDLE);
	memcpy(&input.hThread, &hThread,input.hThreadSize);

	
	//send to device.
	totalSize = sizeof(struct iResumeThread);
	retVal = send(deviceSocket, (char*) &functionID, sizeof(functionID), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	retVal = send(deviceSocket, (char*) &totalSize, sizeof(totalSize),0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	retVal = send(deviceSocket, (char*) &input, totalSize, 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}

	// do error checking here

	retVal = recv(deviceSocket,(char*)  &totalSize, sizeof(totalSize), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}

	retVal = recv(deviceSocket, (char*) &output, totalSize, 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	return output.retVal;
}


bool remoteDevice::RemoteSuspendThread(HANDLE hThread){
	struct iSuspendThread input;
	struct oSuspendThread output;
	int	   functionID = RSuspendThread;
	int	   totalSize;
	int    retVal;


	WSASetLastError(0);
	//construct input parameters
	input.hThreadSize = sizeof(HANDLE);
	memcpy(&input.hThread, &hThread,input.hThreadSize);

	
	//send to device.
	totalSize = sizeof(struct iSuspendThread);
	retVal = send(deviceSocket, (char*) &functionID, sizeof(functionID), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	retVal = send(deviceSocket, (char*) &totalSize, sizeof(totalSize),0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	retVal = send(deviceSocket, (char*) &input, totalSize, 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}

	// do error checking here

	retVal = recv(deviceSocket,(char*)  &totalSize, sizeof(totalSize), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}

	retVal = recv(deviceSocket, (char*) &output, totalSize, 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	return output.retVal;
}



bool remoteDevice::RemoteTerminateProcess(HANDLE hThread, UINT uExitCode){

	struct iTerminateProcess input;
	struct oTerminateProcess output;
	int	   functionID = RTerminateProcess;
	int	   totalSize;
	int    retVal;


	WSASetLastError(0);
	//construct input parameters
	input.hThreadSize = sizeof(HANDLE);
	memcpy(&input.hThread, &hThread,input.hThreadSize);

	input.uExitCodeSize = sizeof(uExitCode);
	memcpy(&input.uExitCode, &uExitCode, input.uExitCodeSize);
	
	//send to device.
	totalSize = sizeof(struct iTerminateProcess);
	retVal = send(deviceSocket, (char*) &functionID, sizeof(functionID), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	retVal = send(deviceSocket, (char*) &totalSize, sizeof(totalSize),0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	retVal = send(deviceSocket, (char*) &input, totalSize, 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}

	// do error checking here

	retVal = recv(deviceSocket,(char*)  &totalSize, sizeof(totalSize), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}

	retVal = recv(deviceSocket, (char*) &output, totalSize, 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	return retVal;
}
 
bool remoteDevice::RemoteCloseHandle(HANDLE hObject){

	struct iCloseHandle input;
	struct oCloseHandle output;
	int	   functionID = RCloseHandle;
	int	   totalSize;
	int    retVal;


	WSASetLastError(0);
	//construct input parameters
	input.hThreadSize = sizeof(HANDLE);
	memcpy(&input.hThread, &hObject,input.hThreadSize);

	//send to device.
	totalSize = sizeof(struct iCloseHandle);
	retVal = send(deviceSocket, (char*) &functionID, sizeof(functionID), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	retVal = send(deviceSocket, (char*) &totalSize, sizeof(totalSize),0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	retVal = send(deviceSocket, (char*) &input, totalSize, 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}

	// do error checking here

	retVal = recv(deviceSocket,(char*)  &totalSize, sizeof(totalSize), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}

	retVal = recv(deviceSocket, (char*) &output, totalSize, 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	return retVal;
}

//ccw 29 sep 2000
bool remoteDevice::RemoteFlushInstructionCache(HANDLE hProcess, LPCVOID lpBaseAddress,
											   DWORD dwSize){

	struct iFlushInstructionCache input;
	struct oFlushInstructionCache output;
	int	   functionID = RFlushInstructionCache;
	int	   totalSize;
	int    retVal;


	WSASetLastError(0);
	//construct input parameters
	input.hProcessSize = sizeof(HANDLE);
	memcpy(&input.hProcess, &hProcess,input.hProcessSize);


	//send to device.
	totalSize = sizeof(struct iFlushInstructionCache);
	retVal = send(deviceSocket, (char*) &functionID, sizeof(functionID), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	retVal = send(deviceSocket, (char*) &totalSize, sizeof(totalSize),0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	retVal = send(deviceSocket, (char*) &input, totalSize, 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}

	// do error checking here

	retVal = recv(deviceSocket,(char*)  &totalSize, sizeof(totalSize), 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}

	retVal = recv(deviceSocket, (char*) &output, totalSize, 0);
	if(retVal == 0xffffffff){
		retVal = WSAGetLastError();
		DebugBreak();
	}
	//do error checking here

	return retVal;

}
