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
	float time_Started;        //Time User Created
  	float cpu_Time;            //Time spent on CPU 
  	float system_Time;         //Time spent in System
  	float waited_Time;         //Time spent waiting
  	float blocked_Time;        //Time spent Blocked        
	pid_t pid;                 //Process Id
	int msgID;                 //Message ID
  	int index;                 //Index
	
	int maximum[maxResources];  
	int allocated[maxResources];
	int requested[maxResources]; 

  	int sprint_Time;           //Recent Run time in CPU
	float wake_Up;             //Time to wake up
	
	

}; 

//Hold Stats 
struct STAT{

	float totalProc; 
	float cpu_Time; 
	float system_Time; 
	float waited_Time;
	float blocked_Time; 
	float idle_Time;
	float end_Time; 
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

struct SysResources{

	int resources[maxResources]; 
	int availableResources[maxResources]; 
	int sharedResources[maxResources]; 
}; 

//System Time
struct system_Time{

	int seconds;
	int nanoSeconds;
	struct PCB pcbTable[18]; 
	struct STAT stats; 
	struct SysResources SysR; 
};
 
#endif
