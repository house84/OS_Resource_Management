/*
 * Author: Nick House
 * Project: Process Scheduling
 * File Name: user.h
 */
 
#ifndef USER_H
#define USER_H

#include "headers.h"

bool run;                         //Continue Process
struct PCB pcb;                   //PCB struct 
float timeLocal;                  //System Time for Blocked
struct PCB *pcbPtr;               //Pointer for PCB
float getTime();                  //Get time for blocked
void freeSHM();                   //Free Shered Mem
void initPCB();                   //Set up Initial PCB Values
void printStats();                //Display User Stats
void initSysTime();               //Set Shared Mem
void sendMessage();               //Send Message
void blockedWait();               //Wait while Blocked
float getRandTime();              //Return Time spent in CPU
void updateGlobal();              //Update Global Stats
int getMessageType();             //Return if Ready, blocked or Terminate
struct system_Time *sysTimePtr;   //Pointer to System Time

#endif
