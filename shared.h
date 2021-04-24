/* 
 * Author: Nick House
 * Project: Resource Management
 * File Name: shared.h
 */
 
#ifndef SHARED_H
#define SHARED_H

#include "headers.h"
#define maxResources 20

//For usability: ready = 0, blocked = 1, running = 2, terminated = 3
enum state{release, request, terminated}; 
enum sems{mutex}; 

//Process Control Block 
struct PCB{
                             
  	//Simulated Time Values
	bool requestBool;          //Flag to check if resources requested
	bool releaseBool;          //Flag if Resources where released
	bool allocateBool;         //Flag if resources where allcoated
	float time_Started;        //Time User Created
  	float cpu_Time;            //Time spent on CPU 
  	float system_Time;         //Time spent in System
  	float waited_Time;         //Time spent waiting
  	float blocked_Time;        //Time spent Blocked        
	pid_t pid;                 //Process Id
	int msgID;                 //Message ID
  	int index;                 //Index
	int requestIDX;            //Requesting Index
	
	int maximum[maxResources];  
	int allocated[maxResources];
	int requested[maxResources];
	int release[maxResources]; 

  	int sprint_Time;           //Recent Run time in CPU
	float wake_Up;             //Time to wake up
	
	

}; 

//Hold Stats 
struct STAT{

	//Scheduling Stats
	float totalProc; 
	float cpu_Time; 
	float system_Time; 
	float waited_Time;
	float blocked_Time; 
	float idle_Time;
	float end_Time;

	//Resource Management Stats
	int numDL;                  //Number of Deadlock Detections Run
	int numReqI;                //Number of Requests Approved Immediately
	int numReqW;                //Number of Requests But on the Wait Q
	int terminatedDL;           //Number of Processes Terminated from Deadlock
	int terminatedN;            //Number of Normal terminated Processes 
	int deadlockCond;           //Number of Deadlock conditions requiring termination
	float avgPercTerm;          //Sum of average termina


}; 

struct msgBuf{

  	long mtype; 
  	char mtext[200];

};

struct msgBuf bufS;         //Send Msg from OSS -> User
struct msgBuf bufR;         //Send Msg from User -> OSS
struct msgBuf buf3;         //Message signal User initialized

int shmidMsg;               //Msg id for OSS->User
int shmidMsg2;              //Msg id for User->OSS
int shmidMsg3;              //Msg id for User initialize
int shmidSysTime;           //Shared Memory Id
int shmidSem;               //Shared Memory for Sem

struct SysResources{

	int resources[maxResources]; 
	int availableResources[maxResources]; 
	int sharedResources[maxResources]; 
}; 

//System Time
struct system_Time{

	int seconds;
	int nanoSeconds;
	int fileLength;
	int grantedReq; 
	bool verbose; 
	struct PCB pcbTable[18]; 
	struct STAT stats; 
	struct SysResources SysR; 
};
 
#endif
