/*
 * Author: Nick House
 * Program: user.c
 * Project: Process Scheduling
 */ 

#include "user.h"

int main(int argc, char * argv[]){

	srand(time(NULL) ^ (getpid()<<16)); 

	//Set Shmids
	shmidSysTime = atoi(argv[2]);
	shmidMsg = atoi(argv[3]);
	shmidMsg2 = atoi(argv[4]); 
	shmidMsg3 = atoi(argv[5]); 

	//Set index
	int idx = atoi(argv[1]); 
	int mID = idx+1;
	run = true; 

	//Initiate SHM
	initSysTime();

	//Initialize PCB Values
	initPCB(idx); 
	
	//Used to Calculate Wait Time
	float waitLocal1;
	float waitLocal2; 
	buf3.mtype = mID;
	strcpy(buf3.mtext, ""); 

	//Messaging needed to allow consistent behavior 
	//of OSS running on real Hoare Kernel
	msgsnd(shmidMsg3, &buf3, sizeof(buf3.mtext), 0); 

	while(run == true){

		//Recienve Message to Run from CPU
		msgrcv(shmidMsg, &bufS, sizeof(bufS.mtext), mID, 0);
		
		//Send Message Back to OSS
		sendMessage(shmidMsg2, mID); 
	}

	//Free Memory
	freeSHM(); 
	
	exit(EXIT_SUCCESS); 

}


//Decide to block run or Terminate
static int getMessageType(int idx){

	return ((rand()+idx) % 3) ; 

}

//Decide how long to spend in quantum 10ms
static float getRandTime(){

	return (rand()% 8800001) + 200000; 

}

//Message_t ready = 0 , blocked = 1, terminated = 3
static void sendMessage(int msgid, int idx){

	bufS.mtype = idx; 

	//Get Type of message
	int messageT = getMessageType(idx); 

	if(messageT != ready){
		
		float rand = getRandTime(); 
		sysTimePtr->pcbTable[idx].sprint_Time = rand; 
		sysTimePtr->pcbTable[idx].cpu_Time += rand/1000000000;  
	}
	else{

		sysTimePtr->pcbTable[idx].sprint_Time = 10000000; 
		sysTimePtr->pcbTable[idx].cpu_Time += .010000000; 
	}
	
	if(messageT == ready){

		strcpy(bufS.mtext, "ready");
	}
	else if( messageT == blocked ){

		strcpy(bufS.mtext, "blocked"); 
		blockedWait(idx); 
	}
	else {

		strcpy(bufS.mtext, "terminated");
		sysTimePtr->pcbTable[idx].system_Time = getTime()-sysTimePtr->pcbTable[idx].time_Started;
		run = false; 
		

		//Display Process Stats	
		updateGlobal(idx); 
	 	printStats(idx); 
	}

	if((msgsnd(msgid, &bufS, sizeof(bufS.mtext), SA_RESTART)) == -1){

			perror("user: ERROR: Failed to msgsnd() ");
			exit(EXIT_FAILURE);
	}
}


//Wait while user blocked while User Blocked
static void blockedWait(int idx){

	float nanoWait = rand()%100000001; 
	float secWait = rand()%6; 
	float bWait = secWait + nanoWait/1000000000;
	sysTimePtr->pcbTable[idx].blocked_Time += bWait; 
	timeLocal = getTime(); 
	
	float unblocked = timeLocal + bWait; 
	sysTimePtr->pcbTable[idx].wake_Up = unblocked; 
}


//Get time 
static float getTime(){
	
	float decimal = sysTimePtr->nanoSeconds;
	decimal = decimal/1000000000;
	float second = sysTimePtr->seconds; 
	
	float localT = second+decimal; 

	return localT; 

}


//Initialize Shared Memory for System Time
static void initSysTime(){

	sysTimePtr = (struct system_Time *) shmat(shmidSysTime, NULL, 0); 

}


//Free Shared Memory PTR
static void freeSHM(){

	if(shmdt(sysTimePtr) == -1){

		perror("user: ERROR: Failed to free ptr shmdt() ");
		exit(EXIT_FAILURE); 
	}
}


//Initialize PCB Values
static void initPCB(int idx){

	sysTimePtr->pcbTable[idx].time_Started = getTime();
	sysTimePtr->pcbTable[idx].proc_id = getpid(); 
	sysTimePtr->pcbTable[idx].proc_id_Sim = idx; 
	sysTimePtr->pcbTable[idx].cpu_Time = 0; 
	sysTimePtr->pcbTable[idx].system_Time = 0; 
	sysTimePtr->pcbTable[idx].waited_Time = 0; 
	sysTimePtr->pcbTable[idx].blocked_Time = 0; 
	sysTimePtr->pcbTable[idx].sprint_Time = 0; 
	sysTimePtr->pcbTable[idx].msgID = idx+1;

}


//Update Global Stats
static void updateGlobal(int idx){

	sysTimePtr->stats.cpu_Time += sysTimePtr->pcbTable[idx].cpu_Time; 
	sysTimePtr->stats.system_Time += sysTimePtr->pcbTable[idx].system_Time; 
	sysTimePtr->stats.waited_Time += sysTimePtr->pcbTable[idx].waited_Time; 
	sysTimePtr->stats.blocked_Time += sysTimePtr->pcbTable[idx].blocked_Time; 
}


//Display stats upon Termination
static void printStats(int idx){

	float currTime = getTime(); 
	float cpu = sysTimePtr->pcbTable[idx].cpu_Time; 
	float start = sysTimePtr->pcbTable[idx].time_Started; 
	float blocked = sysTimePtr->pcbTable[idx].blocked_Time; 
	float system = sysTimePtr->pcbTable[idx].system_Time; 
	float wait = (currTime - ( start + cpu + blocked )); 
	
	sysTimePtr->pcbTable[idx].waited_Time += wait; 

	fprintf(stderr, "\n//////////// USER PROCESS STATS ////////////\n");
	fprintf(stderr, "Time: %f\n", getTime()); 
	fprintf(stderr, "User ID: %d\n", idx-1); 
	fprintf(stderr, "Total Start Time (seconds): %f\n", sysTimePtr->pcbTable[idx].time_Started); 
	fprintf(stderr, "Total System Time (seconds): %f\n", sysTimePtr->pcbTable[idx].system_Time); 
	fprintf(stderr, "Total CPU Time (seconds): %f\n", sysTimePtr->pcbTable[idx].cpu_Time); 
	fprintf(stderr, "Total Waited Time (seconds): %f\n", sysTimePtr->pcbTable[idx].waited_Time); 
//	fprintf(stderr, "Total Waited Time (seconds): %f\n", wait); 
	fprintf(stderr, "Total blocked Time (seconds): %f\n", sysTimePtr->pcbTable[idx].blocked_Time); 
	fprintf(stderr, "//////////// |||||||||||||||||| ////////////\n\n");
}
