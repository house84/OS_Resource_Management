/*
 * Author: Nick House
 * Project: Resource Management
 * File Name: user.h
 */
 
#ifndef USER_H
#define USER_H

#include "headers.h"

bool run;                         //Continue Process
bool requestBool;                 //Track if resources have been requested
bool releaseBool;                 //Track if resources where released; 
//struct PCB pcb;                   //PCB struct 
float timeLocal;                  //System Time for Blocked
struct PCB *pcbPtr;               //Pointer for PCB
float getTime();                  //Get time for blocked
void freeSHM();                   //Free Shered Mem
void initPCB();                   //Set up Initial PCB Values
void printStats();                //Display User Stats
void initSysTime();               //Set Shared Mem
void sendMessage();               //Send Message
void releaseRes();                //release resource
void releaseAll();                //reslease all Resources
void requested();                 //Randomly select resources to request
float getRandTime();              //Return Time spent in CPU
void updateGlobal();              //Update Global Stats
int getMessageType();             //Return if Ready, blocked or Terminate
struct system_Time *sysTimePtr;   //Pointer to System Time
void initLocalPCB();              //Initialize Local PCB P5
//void allocate();                  //Allocate requested resources

#endif
