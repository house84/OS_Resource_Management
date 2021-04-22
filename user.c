/*
 * Author: Nick House
 * Project: Resource Management
 * File Name: user.c
 */ 

#include "user.h"
#include "sharedFunc.h"

int localMaximum[maxResources]; 

// ============ Main Section of Code ============ //

int main(int argc, char * argv[]){

	srand(time(NULL) ^ (getpid()<<16)); 

	//Set Shmids
	shmidSysTime = atoi(argv[2]);
	shmidMsg = atoi(argv[3]);
	shmidMsg2 = atoi(argv[4]); 
	shmidMsg3 = atoi(argv[5]); 
	shmidSem = atoi(argv[6]); 

	//Set index
	int idx = atoi(argv[1]); 
	int mID = idx+1;
	run = true; 

	//Initiate SHM
	initSysTime();
	setSemID(shmidSem); 

	//Initialize PCB Values
	initPCB(idx); 
	initLocalPCB(idx, getpid()); 


	//Used to Calculate Wait Time
	float waitLocal1;
	float waitLocal2; 
	buf3.mtype = mID;
	strcpy(buf3.mtext, ""); 

	//Messaging needed to allow consistent behavior 
	msgsnd(shmidMsg3, &buf3, sizeof(buf3.mtext), 0); 

	while(run == true){

		//Recienve Message to Run from CPU
		msgrcv(shmidMsg, &bufS, sizeof(bufS.mtext), mID, 0);

		int cmp = strcmp(bufS.mtext, "terminate"); 
		//fprintf(stderr, "User Process %d terminate: %d\n", idx, cmp); 

		if( cmp == 0 ){
		
			releaseAll(idx); 
			sysTimePtr->pcbTable[idx].system_Time = getTime()-sysTimePtr->pcbTable[idx].time_Started;
			run = false; 
		
			//Display Process Stats	
			updateGlobal(idx); 
			
			break; 
		}

		//Send Message Back to OSS
		sendMessage(shmidMsg2, mID); 
	}

	//Free Memory
	freeSHM(); 
	
	exit(EXIT_SUCCESS); 
}




// ============ User Functions Begin ============= //


//Initiate local PCB P5
void initLocalPCB(int idx, pid_t proc_id){

	int i; 
	int r; 
	
	//Allocate Initial Reources
	for(i = 0; i < maxResources; ++i){
		
		r = getRand(0, sysTimePtr->SysR.resources[i]); 
		
		//Get Random value for Resources to initialize
		sysTimePtr->pcbTable[idx].maximum[i] = r; 
		localMaximum[i] = r; 

		//Clear User Request Array
		sysTimePtr->pcbTable[idx].requested[i] = 0; 

		if( r > 0 ){

			//If Resource are to be allocated make sure System has available Res
			if(sysTimePtr->SysR.availableResources[i] > 0){

				//Get 1/2 value of a random int between Sys Avail Resources and 0
				r = (getRand(0,sysTimePtr->SysR.availableResources[i])/2); 
				
				//If r greater than user max set to the user max
				if( r > localMaximum[i] ) {	r = localMaximum[i]; }
				
				//Allocate resource to user 
				sysTimePtr->pcbTable[idx].allocated[i] = r; 
			
				//Check if system resource is shared
				if(sysTimePtr->SysR.sharedResources[i] == 0){

					sysTimePtr->SysR.availableResources[i] = sysTimePtr->SysR.availableResources[i] - r;
				}
			}
		}
	}
		
	printArrHead(); 
	fmt(sysTimePtr->SysR.resources, "Sys-Resources"); 
	fmt(sysTimePtr->SysR.availableResources, "Sys-Available"); 
	fmt(sysTimePtr->SysR.sharedResources, "Sys-Shared");
	fmt(sysTimePtr->pcbTable[idx].allocated, "P%d Allocated", idx);
	fmt(sysTimePtr->pcbTable[idx].maximum, "P%d Maximum", idx);

}


//Decide to block run or Terminate
int getMessageType(int idx){

	return ((rand()+idx) % 3) ; 

}


//Decide how long to spend in quantum 10ms
float getRandTime(){

	return (rand()% 8800001) + 200000; 

}


//Message_t release = 0 , request = 1, terminated = 2
void sendMessage(int msgid, int mID){

	int idx = mID -1; 

	fprintf(stderr, "INDEX %d\n", idx);
	
	bufS.mtype = mID; 
	
	//Error checking
	int checked = 0; 
	requestBool = false; 
	releaseBool = false; 

	//Get Type of message
	int messageT = getMessageType(idx);

	if( getTime() - sysTimePtr->pcbTable[idx].time_Started < 1 ){ 
		
		fprintf(stderr, "Under 1 Second\n"); 
		messageT = request; }

	if(messageT != release){
		
		float rand = getRandTime(); 
		sysTimePtr->pcbTable[idx].sprint_Time = rand; 
		sysTimePtr->pcbTable[idx].cpu_Time += rand/1000000000;  
	}
	else{

		sysTimePtr->pcbTable[idx].sprint_Time = 10000000; 
		sysTimePtr->pcbTable[idx].cpu_Time += .010000000; 
	}
	
	if(messageT == release){

		strcpy(bufS.mtext, "release");
		releaseRes(idx); 
		
		if(releaseBool == false){
			
				strcpy(bufS.mtext, "terminated"); 
				releaseAll(idx); 
				sysTimePtr->pcbTable[idx].system_Time = getTime()-sysTimePtr->pcbTable[idx].time_Started;
				run = false; 
				updateGlobal(idx); 
			}
	}
	else if( messageT == request ){

		fprintf(stderr, "P%d requesting\n", idx);  
		strcpy(bufS.mtext, "request"); 
		requested(idx); 

		if( requestBool == false ){

				strcpy(bufS.mtext, "terminated");
				releaseAll(idx); 
				sysTimePtr->pcbTable[idx].system_Time = getTime()-sysTimePtr->pcbTable[idx].time_Started;
				run = false; 
				updateGlobal(idx); 
			}

	}
	else {

		strcpy(bufS.mtext, "terminated");
		releaseAll(idx); 
		sysTimePtr->pcbTable[idx].system_Time = getTime()-sysTimePtr->pcbTable[idx].time_Started;
		run = false; 
		

		//Display Process Stats	
		updateGlobal(idx); 
	 //	printStats(idx); 
	}

	if((msgsnd(msgid, &bufS, sizeof(bufS.mtext), 0)) == -1){

			perror("user: ERROR: Failed to msgsnd() ");
			exit(EXIT_FAILURE);
	}
}


//Wait while user blocked while User Blocked
void requested(int idx){

	float nanoWait = rand()%100000001; 
	float secWait = rand()%6; 
	float bWait = secWait + nanoWait/1000000000;
	sysTimePtr->pcbTable[idx].blocked_Time += bWait; 
	timeLocal = getTime(); 
	
	float unblocked = timeLocal + bWait; 
	sysTimePtr->pcbTable[idx].wake_Up = unblocked; 

	int r = getRand(0, 19);
	int	locIDX = r; 
	int count = 0; 

	fmt(sysTimePtr->SysR.availableResources, "Sys-Available"); 
	fmt(localMaximum, "P%d Maximum", idx);
	fprintf(stderr,"\n"); 
	
	while(localMaximum[locIDX] == 0 || ( sysTimePtr->pcbTable[idx].allocated[locIDX] == sysTimePtr->SysR.availableResources[locIDX]) || (localMaximum[locIDX] == sysTimePtr->pcbTable[locIDX].allocated[locIDX])){

		++r; 
		locIDX = (r)%20; //getRand(0,19); 
		
		fprintf(stderr, "P%d locIDX = %d Allocated:%d\n", idx, locIDX, sysTimePtr->pcbTable[idx].allocated[locIDX]); 

		//Give adequate attempts to find index for request
		if(count > 19 ){

			requestBool = false; 
			return; 
		}
		
		++count; 
	}
	
	//add request to requested resource array
	sysTimePtr->pcbTable[idx].requested[locIDX] = 1; //sysTimePtr->pcbTable[idx].maximum[r] - sysTimePtr->pcbTable[idx].allocated[r];

	sysTimePtr->pcbTable[idx].requestIDX = locIDX; 
	
	//Print Request
	fprintf(stderr, "P%d requesting resource R%d\n", idx, r);
	fmt(sysTimePtr->pcbTable[idx].requested, "P%d Req", idx); 

	requestBool = true; 
}


//Release random resource
void releaseRes(int idx){

	//Add loop to release a random resource
	
	int r = getRand(0,19); 
	int locIDX = r; 
	int count = 0; 
	while(sysTimePtr->pcbTable[idx].allocated[locIDX] <= 0){

		locIDX = (++r)%20; 
		
		if(count > 20){
		
			releaseBool = false; 
			return;
		}

		++count; 
	}
	
	int temp; 
	temp  = sysTimePtr->pcbTable[idx].allocated[locIDX]; 
	sysTimePtr->pcbTable[idx].allocated[locIDX] = 0; 
	
	if(sysTimePtr->SysR.sharedResources[locIDX] == 0){

		sysTimePtr->SysR.availableResources[locIDX] += temp; 
	}

	releaseBool = true; 
	
	//Print
	fprintf(stderr, "Releasing Resources P%d -> R%d\n", idx, locIDX); 
	
	printArrHead(); 
	fmt(sysTimePtr->SysR.availableResources, "Available");
	fmt(sysTimePtr->SysR.sharedResources, "Shared");
	
	printArrHead(); 
	fmt(sysTimePtr->pcbTable[idx].allocated, "P%d Allo", idx);


}


//Release All resources
void releaseAll(int idx){

	//Release all resources
	int i;
	int temp = 0; 

	for(i = 0; i < maxResources; ++i){

		temp = sysTimePtr->pcbTable[idx].allocated[i]; 
		sysTimePtr->pcbTable[idx].allocated[i] = 0; 
		
		if(sysTimePtr->SysR.sharedResources[i] == 0){

			sysTimePtr->SysR.availableResources[i] += temp; 
		}
	}
	
	//Print
	fprintf(stderr, "All Resources for P%d have been released\n", idx); 
	
	printArrHead(); 
	fmt(sysTimePtr->SysR.resources, "Sys-Resources");
	fmt(sysTimePtr->SysR.availableResources, "Sys-Available");
	fmt(sysTimePtr->SysR.sharedResources, "Sys-Shared");
	
	printArrHead(); 
	fmt(sysTimePtr->pcbTable[idx].allocated, "P%d Allocated", idx); 
	fprintf(stderr, "\n"); 
}


//Get time 
float getTime(){
	
	float decimal = sysTimePtr->nanoSeconds;
	decimal = decimal/1000000000;
	float second = sysTimePtr->seconds; 
	
	float localT = second+decimal; 

	return localT; 

}


//Initialize Shared Memory for System Time
void initSysTime(){

	sysTimePtr = (struct system_Time *) shmat(shmidSysTime, NULL, 0); 

}


//Free Shared Memory PTR
void freeSHM(){

	if(shmdt(sysTimePtr) == -1){

		perror("user: ERROR: Failed to free ptr shmdt() ");
		exit(EXIT_FAILURE); 
	}
}


//Initialize PCB Values
void initPCB(int idx){

	sysTimePtr->pcbTable[idx].time_Started = getTime();
	sysTimePtr->pcbTable[idx].pid = getpid(); 
	sysTimePtr->pcbTable[idx].index = idx; 
	sysTimePtr->pcbTable[idx].cpu_Time = 0; 
	sysTimePtr->pcbTable[idx].system_Time = 0; 
	sysTimePtr->pcbTable[idx].waited_Time = 0; 
	sysTimePtr->pcbTable[idx].blocked_Time = 0; 
	sysTimePtr->pcbTable[idx].sprint_Time = 0; 
	sysTimePtr->pcbTable[idx].msgID = idx+1;

}


//Update Global Stats
void updateGlobal(int idx){

	sysTimePtr->stats.cpu_Time += sysTimePtr->pcbTable[idx].cpu_Time; 
	sysTimePtr->stats.system_Time += sysTimePtr->pcbTable[idx].system_Time; 
	sysTimePtr->stats.waited_Time += sysTimePtr->pcbTable[idx].waited_Time; 
	sysTimePtr->stats.blocked_Time += sysTimePtr->pcbTable[idx].blocked_Time; 
}


//Display stats upon Termination
void printStats(int idx){

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
	fprintf(stderr, "Total blocked Time (seconds): %f\n", sysTimePtr->pcbTable[idx].blocked_Time); 
	fprintf(stderr, "//////////// |||||||||||||||||| ////////////\n\n");
}
