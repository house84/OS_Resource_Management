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
static float getTime();           //Get time for blocked
static void freeSHM();            //Free Shered Mem
static void initPCB();            //Set up Initial PCB Values
static void printStats();         //Display User Stats
static void initSysTime();        //Set Shared Mem
static void sendMessage();        //Send Message
static void blockedWait();        //Wait while Blocked
static float getRandTime();       //Return Time spent in CPU
static void updateGlobal();       //Update Global Stats
static int getMessageType();      //Return if Ready, blocked or Terminate
struct system_Time *sysTimePtr;   //Pointer to System Time

#endif
