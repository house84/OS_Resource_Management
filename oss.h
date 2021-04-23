/*
 * Author: Nick House
 * Project: Process Scheduling
 * File Name: oss.h
 */
 
#ifndef OSS_H
#define OSS_H

#define procMax 18
#define maxLine 10000
#define maxTimeBetweenNewProcNS 1000000000 
#define maxTimeBetweenNewProcSecs 2
#include "headers.h"

struct Queue{                          //Queue for Ready 

	struct p_Node *head; 
	struct p_Node *tail; 
	int	currSize;
	int maxSize;  

};

struct p_Node{                         //Struct for items in Que

  int fakePID;                        
  struct p_Node *next; 

}; 

//Function Prototypes
static void help();                    //Help Page
static void spawn();                   //Spawn Child Process
static void printQ();                  //Print Queue
static void enqueue();                 //Add Child to Queue
static void setTimer();                //Set initial Timer
void setBitVectorVal();                //Set BitVector used from idx
static float getTime();                //Get time with decimal xx.xxx
static void setTimer2();               //Set 3 Second Timer
static void initStats();               //Initiate Stats
void unsetBitVectorVal();              //Clear Bitvector from idx
static void setSysTime();              //Set the System Time
const char *getSysTime();              //Return formatted System Time
static void requesting();              //Requesting Resource Handler
struct p_Node * dequeue();             //Remove Head
static void showSysTime();             //Display System Time
static void allocateCPU();             //Try to Put Process in CPU
static void openLogfile();             //Open Logfile
static void closeLogfile();            //Close Logfile
static void displayStats();            //Display Stats
static void initBlockedQ();            //Initialize Blocked Q -> 0
static void dispatchTime();            //Increment Sys Time for Dispatching
static void checkDeadLock();           //Check for Dead Lock `
static void terminateProc();           //Terminate Process if deadlock
static void signalHandler();           //Handle Signal timer/ctrl+c
static void checkBlockedQ();           //Search Blocked Que for Freed Proc
static float  newUserTime();           //Get time to offset new User spawn
static void stopTimeHandler();         //Stop Producing Timer Handler
static int  getBitVectorPos();         //Search bit vector for open idx
static void freeSharedMemory();        //Release Shared Memory Resources
static void incrementSysTime();        //Increment System time
static void createSharedMemory();      //Allocate Shared Memory

time_t t;                              //Hold Time
int myTimer;                           //Timer Value
bool verbose;                          //Indicate verbose logfile
bool sigFlag;                          //Variable to pause termination
int concProc;                          //Number of Concurrent Processes
int logLines;                          //Holds Number of lines in logfile
int totalProc; 					       //Number of total procedures
int active[18];                        //track active user
key_t keySem;                          //Shm Key for Sem
key_t keyMsg;                          //Shm Key for Message 1
key_t keyMsg2;                         //Shm key for Message 2
key_t keyMsg3;                         //Shm key for Message 3
size_t memSize;                        //memSize for getshm()
struct PCB cpu;                        //PCB 
bool spawnFlag;                        //Varialbe to signal forking process
FILE *logfilePtr;                      //Logfile Pointer
FILE *logfilePtr2;                     //Logfile for Resource Managment
key_t keySysTime;                      //Shm Key
bool stopProdTimer;                    //Produce or not Bool
char logfile[50];                      //Logfile Name
char logfile2[50];                     //Scheduling Logfile
struct Queue *GQue;                    //Variable for Queue
pid_t pidArray[100];                   //Variable for Process PID's
int blockedQ[procMax];                 //Hold Index of Blocked Users
struct itimerval timer;                //Set Timer
struct p_Node *CPU_Node;               //Node to Hold CPU Process
struct Queue *initQueue();             //Create Queue
struct system_Time *sysTimePtr;        //System Time Pointer

typedef unsigned int bv_t;             //Bit Vector
bv_t bitVector;                        //BV Variable
#endif
