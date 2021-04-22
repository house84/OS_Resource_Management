/*
 * Author: Nick House
 * Project: Process Scheduling
 * Coures: CS-4760 Operating Systems, Spring 2021
 * File Name: oss.c
 */

#include "oss.h"
#include "sharedFunc.h"

int main(int argc, char * argv[]){
	

	//Set Bool
	sigFlag = false; 
	spawnFlag = false;
	verbose = false; 
	
	//Initialize Signal Handling
	signal(SIGINT, signalHandler); 

	//Set Initial Parameters
	memset(logfile, '\0', sizeof(logfile)); 
	strcpy(logfile, "logfile"); 
	totalProc = 0;  
	srand(time(NULL)); 

	int logfileLength = 50; 
	
	//Parse input Args
	int c = 0; 
	
	while(( c = getopt(argc, argv, "hv")) != -1){

	  switch(c){
				case 'h':	help(argv[0]); 
							exit(EXIT_SUCCESS); 

				case 'v': 	//Set Time
							verbose = true; 
							break; 

				default:	//Defalut Failure Exit
							fprintf(stderr, "Invalid Argument, see usage [-h]\n"); 
							exit(EXIT_FAILURE); 
				}

	}

	
	//Create Shared Memory
	createSharedMemory(); 
	
	//Open logfile
	openLogfile(); 

	//Set shmidSem
	setSemID(shmidSem); 

	//Initialize Queue
	GQue = initQueue(); 

	//Initilize BQ
	initBlockedQ();

	//Initialize Resource Array
	initResourceArr(sysTimePtr); 
	memset(active, 0, 18); 
	
	//Testing
	printArrHead(); 
	printArr(sysTimePtr->SysR.resources, "Sys-Resources"); 
	printArr(sysTimePtr->SysR.availableResources, "Sys-Available"); 
	printArr(sysTimePtr->SysR.sharedResources, "Sys-Shared"); 
	
	//Initialize STAT
	sysTimePtr->stats.totalProc = 0; 
	sysTimePtr->stats.cpu_Time = 0; 
	sysTimePtr->stats.waited_Time = 0; 
	sysTimePtr->stats.blocked_Time = 0; 
	sysTimePtr->stats.idle_Time = 0; 

	//Initialize CPU Node
	CPU_Node = (struct p_Node*)malloc(sizeof(struct p_Node)); 

	//=========== Add Program Logic =============

	int i = 0; 
	int index; 
	int deadLockTimer = 0; 
	int iterTime; 
	float newUser = getTime() + newUserTime(); 
	concProc = 0; 
	
	//Set 3 Second Timer
	stopProdTimer = false; 
 
	time_t now; 
	struct tm *tm; 
	now = time(0); 
	
	tm = localtime(&now);
	int startSeconds = tm->tm_sec; 

	while(true){

		//Check Timer
		if(stopProdTimer == false){
			
			now = time(0); 
			tm = localtime(&now); 
			int stopSeconds = tm->tm_sec; 

			if((stopSeconds - startSeconds) >= 5){
				stopProdTimer = true; 
			} 
		}

		
		//Increment System Time by 1-500 ms
		iterTime = rand()%500000000 + 1000001; 
		
		//critical 
		semWait(mutex); 
		incrementSysTime(iterTime); 
		semSignal(mutex); 

		//Check Blocked Processes
		checkBlockedQ(); 

		//Check for Deadlock
		if( sysTimePtr->seconds > deadLockTimer){ 
			
			//Send to Log
			fprintf(stderr, "Checking for Deadlock\n"); 
			checkDeadLock();
			deadLockTimer = sysTimePtr->seconds; 
		} 



		//Spawn Child Process //Set to 20 for testing
		if( concProc < procMax && totalProc < 40 && stopProdTimer == false && (newUser < getTime())){

			index = getBitVectorPos(); 
			if(index != -1) { 
				
				spawn(index);
				active[index] = 1; 
				++totalProc; 
				++concProc;

				//Allow User to initialize
				if(msgrcv(shmidMsg3, &buf3, sizeof(buf3.mtext), index+1, 0) == -1){

					perror("OSS: ERROR: Failed to RCV Message from user msgrcv() ");
					exit(EXIT_FAILURE); 
				}

				//Message new Process Created 
				fprintf(stderr, "OSS: Time: %s PID: %d\t|||| New User Process Created\n", getSysTime(), index);
				fprintf(logfilePtr, "OSS: Time: %s PID: %d\t|||| New User Process Created\n", getSysTime(), index);

				//Add to RunQ
				enqueue(index);

				newUser = getTime() + newUserTime(); 
			}
		}
	
		//Check runQueue
		allocateCPU();
		
		//check for Finished Processes
		int status; 
		
		pid_t user_id = waitpid(-1, &status, WNOHANG); 

		if(user_id > 0 ){
 			
			//fprintf(stderr,"waitPid user_id: %d\n", user_id); 
			--concProc;
		}

		//Break Loop clean up memory
		if(totalProc == 40 && concProc == 0){
		
			sysTimePtr->stats.end_Time = getTime(); 
			break; 
		}


		//Print for verbose
		verbose = true; 
		if(verbose == true){

			printArrHead(); 
			printArr(sysTimePtr->SysR.resources, "Resources");
			printArr(sysTimePtr->SysR.availableResources, "Available"); 
			printArr(sysTimePtr->SysR.sharedResources, "Shared"); 
			fprintf(stderr, "\n"); 
			
			int j; 
			for(j=0; j < 18; ++j){

				if(active[j] == 1 ){

					if(j == 0){

						fprintf(stderr, "Process Resources Allocated (A) and Maximum (M)\n"); 
						printArrHead(); 
					}
					
					fmt(sysTimePtr->pcbTable[j].allocated, "(A) P%d", j); 
					fmt(sysTimePtr->pcbTable[j].maximum, "(M) P%d", j); 	
			 	}
			}
			
			fprintf(stderr, "\n"); 
		}

	}


	//==========================================
	

	//Allow Processes to finish
	while(wait(NULL) > 0){} 

	
	//TESTING
	printArrHead();
	printArr(sysTimePtr->SysR.resources, "Sys-Resources"); 
	printArr(sysTimePtr->SysR.availableResources, "Sys-Available"); 
	printArr(sysTimePtr->SysR.sharedResources, "Shared"); 

	//Clean up Resources
	signalHandler(3126);

	return 0; 

}


//Signal Handler
void signalHandler(int sig){

  	//set sigFlag to block the creation of an further child processes
 	sigFlag == true; 

  	//openLogfile(); 
  
  	time(&t); 
  
  	//Check for Signal Type
  	if( sig == SIGINT ) {

		fprintf(stderr, "\nProgram Terminated by User\n"); 
		fprintf(logfilePtr, "\nTime: %sProgram Terminated by User\n", ctime(&t)); 
	
  	}else if( sig == 3126 ){

	  	fprintf(stderr, "\nAll Processes have finished\n"); 
	  	fprintf(logfilePtr, "\nTime: %sAll Processes have finished\n", ctime(&t));
	
	}else{

	  	fprintf(stderr, "\nProgram Terminated due to Timer\n"); 
	}

	//Display Stats
	displayStats(); 
	
	//Close Logfile Ptr
	closeLogfile(); 
	
	//Allow Potential Creating Processes to add PID to Array
	while(spawnFlag == true){}
	
	//Free Memory Resources
	freeSharedMemory();
	
	//Exit Normally
	if( sig == 3126 ) { exit(EXIT_SUCCESS); }


	//Terminate Child Processes
	int i; 
	for( i = 0; i < totalProc; ++i){
	    
	    if(kill(pidArray[i], SIGKILL ) == -1 && errno != ESRCH ){
	    		
	        perror("oss: ERROR: Failed to Kill Processes "); 
	        exit(EXIT_FAILURE); 
	
	    }
	}
	
	//Destroy Potential Zombies
	while( wait(NULL) != -1 || errno == EINTR ); 

	exit(EXIT_SUCCESS); 
}


//Display Usage
static void help(char *program){

  	printf("\n//=== %s Usage Page ===//\n", program);  
	printf("\n%s [-h][-v]\n", program); 
	printf("%s -h      This Usage Page\n", program); 
	printf("%s -v      Turn Verbose Mode On\n", program); 

}


//Set Timer
static void setTimer(int t){

  	signal(SIGALRM, signalHandler); 
	
	timer.it_value.tv_sec = t; 
	timer.it_value.tv_usec = 0; 
	timer.it_interval.tv_sec = 0; 
	timer.it_interval.tv_usec = 0; 

	if(setitimer(ITIMER_REAL, &timer, NULL) == -1){

		perror("oss: ERROR: Failed to set timer setitimer() ");
		exit(EXIT_FAILURE);
	}
}


//Create Shared Memory
static void createSharedMemory(){  

	//=== System Time Shared Memory
	if((keySysTime = ftok("Makefile", 'a')) == -1){
		
		perror("oss: ERROR: Failed to generate keySysTime ftok() ");
		exit(EXIT_FAILURE); 
	}
	
	if((shmidSysTime = shmget(keySysTime, sizeof(struct system_Time), IPC_CREAT|S_IRUSR|S_IWUSR)) == -1){
		
		perror("oss: ERROR: Failed to get shmidSysTime, shmget() ");
		exit(EXIT_FAILURE); 
	}
	

	sysTimePtr = (struct system_Time *) shmat(shmidSysTime, NULL, 0); 


	//=== Messaging to Send to User
	if((keyMsg = ftok("oss.c", 'a')) == -1){
		
		perror("oss: ERROR: Failed to generate keyMsg, ftok() ");
		exit(EXIT_FAILURE);
	}
	
	if((shmidMsg = msgget(keyMsg, IPC_CREAT|S_IRUSR|S_IWUSR)) == -1){

		perror("oss: ERROR: Failed to generate shmidMsg, msgget() "); 
		exit(EXIT_FAILURE); 
	}
	
	//=== Message to Receive From User
	if((keyMsg2 = ftok("user.c", 'a')) == -1){

		perror("oss: ERROR: Failed to generate keyMsg2, ftok() ");
		exit(EXIT_FAILURE); 
	}

	if((shmidMsg2 = msgget(keyMsg2, IPC_CREAT|S_IRUSR|S_IWUSR)) == -1){

		perror("oss: ERROR: Failed to generate shmidMsgRcv, msgget() ");
		exit(EXIT_FAILURE); 
	}

	//=== Message to Receive Initialized from User
	if((keyMsg3 = ftok("user.h", 'a')) == -1){

		perror("oss: ERROR: Failed to generate keyMsg3, ftok() "); 
		exit(EXIT_FAILURE); 
	}

	if((shmidMsg3 = msgget(keyMsg3, IPC_CREAT|S_IRUSR|S_IWUSR)) == -1){

		perror("oss: ERROR: Failed to generate shmidMsg3, msgget() ");
		exit(EXIT_FAILURE); 
	}

	//=== Set Mutex Sem
	if((keySem = ftok("sharedFunc.c", 'a')) == -1){

		perror("OSS: ERROR: Failed to generate semKey, ftok() "); 
		exit(EXIT_FAILURE); 
	}

	if((shmidSem = semget(keySem, 1, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR )) == -1){
		perror("OSS: ERROR: Failed to generate shmidSem, semget() ");
		exit(EXIT_FAILURE); 
	}

	if((semctl(shmidSem, mutex, SETVAL, 1)) == -1){

		perror("OSS: ERROR: Failed to create Mutex Sem semctl() "); 
		exit(EXIT_FAILURE); 
	}
}


//Free Shared Memory
static void freeSharedMemory(){

	//Detach System Pointer
	if(shmdt(sysTimePtr) == -1){
		
		perror("oss: ERROR: Failed to detach shmidSysTime, shmdt() "); 
		exit(EXIT_FAILURE); 
	}

	//Destroy System Time Memory
	if(shmctl(shmidSysTime, IPC_RMID, NULL) == -1){
		
		perror("oss: ERROR: Failed to Desttory sysTimePtr, shmctl() "); 
		exit(EXIT_FAILURE); 
	}

	
	//Destroy Message Q
	if(msgctl(shmidMsg, IPC_RMID, NULL) == -1){

		perror("oss: ERROR: Failed to Destroy shmidMsg, msgctl() "); 
		exit(EXIT_FAILURE); 
	}

	if(msgctl(shmidMsg2, IPC_RMID, NULL) == -1){

		perror("oss: ERROR: Failed to Destroy shmidMsgRcv, msgctl() ");
		exit(EXIT_FAILURE); 
	}

	if(msgctl(shmidMsg3, IPC_RMID, NULL) == -1){

		perror("oss: ERROR: Failed to Destroy shmidMsg3, msgctl() "); 
		exit(EXIT_FAILURE); 
	}

	if((semctl(shmidSem, 0, IPC_RMID)) == -1 ){

		perror("OSS: ERROR: Failed to release sem Memory semctl() ");
		exit(EXIT_FAILURE); 
	}
}


//Set System Time
static void setSysTime(){

	sysTimePtr->seconds = 0; 
	sysTimePtr->nanoSeconds = 0;
}

//Increment System Time
static void incrementSysTime(int x){

	sysTimePtr->nanoSeconds = sysTimePtr->nanoSeconds + x; 

	while(sysTimePtr->nanoSeconds >= 1000000000 ){

		sysTimePtr->nanoSeconds = sysTimePtr->nanoSeconds - 1000000000;  
		
		sysTimePtr->seconds += 1; 
	}	
}


//Show System Time
static void showSysTime(){
	
	float nano = sysTimePtr->nanoSeconds;

	printf("System Time (seconds)-> %03d:%09d\n",sysTimePtr->seconds, sysTimePtr->nanoSeconds); 

}


//Spawn Child Process
static void spawn(int idx){

	//Check if prograom is terminating
	if(sigFlag == true) { return; }

	pid_t process_id; 

	if((process_id = fork()) < 0){

		perror("oss: ERROR: Failed to fork process fork() ");
		exit(EXIT_FAILURE); 
	}

	if(process_id == 0){

		//Temp Block Handler from Terminating
		spawnFlag = true; 
		
		//Add Process to Process Array
		pidArray[idx]  = process_id; 

		//Release Block 
		spawnFlag = false; 

		//Index arg
		char buffer_idx[10];
		sprintf(buffer_idx, "%d", idx); 

		//shmidSysTime arg
		char buffer_sysTime[50]; 
		sprintf(buffer_sysTime, "%d", shmidSysTime);

		//shmidMsg arg
		char buffer_msgId[50];
		sprintf(buffer_msgId, "%d", shmidMsg);

		//bool run = true; 
		//shmidMsgRcv arg
		char buffer_msgId2[50];
		sprintf(buffer_msgId2, "%d", shmidMsg2); 
		
		//shmidMsg3 arg
		char buffer_msgId3[50];
		sprintf(buffer_msgId3, "%d", shmidMsg3); 

		//shmidSem
		char buffer_shmidSem[50];
		sprintf(buffer_shmidSem, "%d", shmidSem); 

		//Call user file with child process
		if(execl("./user_proc", "user_proc", buffer_idx, buffer_sysTime, buffer_msgId,buffer_msgId2, buffer_msgId3, buffer_shmidSem, (char*) NULL)){

			perror("oss: ERROR: Failed to execl() child process "); 
			exit(EXIT_FAILURE); 
		}

	exit(EXIT_SUCCESS); 

	}
}


//Find Index of empty Bit Arr
static int getBitVectorPos(){

	unsigned int i = 1; 
	int idx = 0; 

	//Search bitVector R->L until 0 is found
	while(( i & bitVector) && (idx < procMax)){

		i <<= 1; 
		++idx; 
	}

	if( idx < procMax ){

		//Set Bit Func
		setBitVectorVal(idx); 

		return idx; 
	}

	else{

		return -1; 
	}

}


//Set Bit Vector From Index 
void setBitVectorVal(int idx){
	
	bitVector |= ( 1 << idx ); 
}


//Unset Bit Vector from index
void unsetBitVectorVal(int idx){ 

	bitVector &= ~( 1 << idx ); 
}


//Open New Logfile
static void openLogfile(){
	
	logfilePtr = fopen(logfile, "w"); 

	if( logfilePtr == NULL ){

		perror("oss: ERROR: Failed to Open logfile "); 
		exit(EXIT_FAILURE); 
	}

	time(&t); 

	fprintf(logfilePtr, "\n//========================= Log Opened ========================//\n"); 
	fprintf(logfilePtr, "Time: %s", ctime(&t));
	fprintf(logfilePtr, "//=============================================================//\n"); 

}


//Close Logfile
static void closeLogfile(){

	
	fprintf(logfilePtr, "\n//========================= Log Closed ========================//\n"); 
	fprintf(logfilePtr, "Time: %s", ctime(&t)); 
	fprintf(logfilePtr, "//=============================================================//\n\n"); 
	fclose(logfilePtr); 


}


//Display Stats/Print Stats
static void displayStats(){
	
	int total  = totalProc; 

	//Print to Terminal
	fprintf(stderr, "\n\n//////////////// PROGRAM REPORT ////////////////\n"); 
	fprintf(stderr, "System Time: %f\n", getTime()); 
	fprintf(stderr, "Average Process CPU Time: %f\n", sysTimePtr->stats.cpu_Time/total); 
	fprintf(stderr, "Average Process System Time: %f\n", sysTimePtr->stats.system_Time/total); 
	fprintf(stderr, "Average Process Wait Time: %f\n", (sysTimePtr->stats.waited_Time/total)); 
	fprintf(stderr, "Average Process Blocked Time: %f\n", sysTimePtr->stats.blocked_Time/total); 
	fprintf(stderr, "CPU Idle Time: %f\n", (getTime() - sysTimePtr->stats.cpu_Time)); 
	fprintf(stderr, "//////////////// |||||||||||||| ////////////////\n"); 
	
	//Print to logs
	fprintf(logfilePtr, "\n\n//////////////// PROGRAM REPORT ////////////////\n"); 
	fprintf(logfilePtr, "System Time: %f\n", getTime()); 
	fprintf(logfilePtr, "Average Process CPU Time: %f\n", sysTimePtr->stats.cpu_Time/total); 
	fprintf(logfilePtr, "Average Process System Time: %f\n", sysTimePtr->stats.system_Time/total); 
	fprintf(logfilePtr, "Average Process Wait Time: %f\n", (sysTimePtr->stats.waited_Time/total)); 
	fprintf(logfilePtr, "Average Process Blocked Time: %f\n", sysTimePtr->stats.blocked_Time/total); 
	fprintf(logfilePtr, "CPU Idle Time: %f\n", (getTime() - sysTimePtr->stats.cpu_Time)); 
	fprintf(logfilePtr, "//////////////// |||||||||||||| ////////////////\n"); 
}	


//Initialize Queue
struct Queue * initQueue(){

	struct Queue *que = (struct Queue*)malloc(sizeof(struct Queue));

	//Initialize Null Front and Rear Nodes
	que->head = NULL; 
	que->tail = NULL; 

	//que->maxSize = procMax; 
	que->maxSize = 18; 
	que->currSize = 0;  

	return que; 

}


//Enqueue Process to Queue
static void enqueue(int idx){

	struct p_Node *newNode = (struct p_Node*)malloc(sizeof(struct p_Node));  

	newNode->fakePID = idx; 
	newNode->next = NULL; 
	++GQue->currSize; 

	//Test Print
	fprintf(stderr, "OSS: Time: %s PID: %d\t|||| Added to position %d in Run Queue\n", getSysTime(), newNode->fakePID, GQue->currSize); 
	fprintf(logfilePtr, "OSS: Time: %s PID: %d\t|||| Added to position %d in Run Queue\n", getSysTime(), newNode->fakePID, GQue->currSize); 

	//Check if Empty
	if( GQue->head == NULL && GQue->tail == NULL ){

		GQue->head = newNode; 
		GQue->tail = newNode; 

		return; 
	}

	//Else add Node to End of Que
	GQue->tail->next = newNode; 
	GQue->tail = newNode; 
  
}


//Dequeue Process from Queue
struct p_Node * dequeue(){

	//Check if Que is empty
	if( GQue->head == NULL ){ return NULL; }

	struct p_Node *newNode = GQue->head; 

	--GQue->currSize; 

	//Make next node the head
	GQue->head = GQue->head->next; 

	//Check if Que is Now Empty
	if( GQue->head == NULL ){ GQue->tail = NULL; }

	//Test Print
	fprintf(stderr, "OSS: Time: %s PID: %d\t|||| Removed from Run Queue\n", getSysTime(), newNode->fakePID); 
	fprintf(logfilePtr, "OSS: Time: %s PID: %d\t|||| Removed from Run Queue\n", getSysTime(), newNode->fakePID); 

	return newNode; 

}


//Print The Queue for Testing
static void printQ(){

	//Test
	fprintf(stderr, "Print Q\n"); 
	
	struct p_Node *newNode = GQue->head; 

	fprintf(stderr, "QUEUE Sizez: %d -> ", GQue->currSize);

	while(newNode != NULL){

		fprintf(stderr, "%d ", newNode->fakePID); 
		newNode = newNode->next; 
	}

	fprintf(stderr,"\n"); 

}

//Display Stytem Time
const char * getSysTime(){
	
	char *sTime; 
	asprintf(&sTime, "%04d:%09d", sysTimePtr->seconds, sysTimePtr->nanoSeconds); 
	
	return sTime; 
}


//Put Ready Process into CPU
static void allocateCPU(){

	//TESTING
	printArrHead();
	printArr(sysTimePtr->SysR.sharedResources, "Shared"); 
	printArr(sysTimePtr->SysR.availableResources, "Available"); 

	//Check for runnable Processes
	if(GQue->currSize == 0){ 
		
		fprintf(stderr, "Run Queue Empty\n"); 

		return; 
		
	}
	
	//Dequeue Process
	CPU_Node = dequeue(); 
	
	//Check Node
	if(CPU_Node == NULL){

		fprintf(stderr,"Que Head Empty\n"); 
		return; 
	}

	int idx = CPU_Node->fakePID; 
	int mID = CPU_Node->fakePID+1; 

	bufS.mtype = mID; 
	strcpy(bufS.mtext, "Run"); 

	if((msgsnd(shmidMsg, &bufS, sizeof(bufS.mtext), 0)) == -1 ){

		fprintf(stderr, "OSS: FAILED::: mID: %d\n", mID);  
		perror("oss: ERROR: Failed to Send Msg to User msgsnd() "); 
		exit(EXIT_FAILURE); 
	}

	//Print Update from CPU 
	fprintf(stderr, "OSS: Time: %s PID: %d\t|||| Sent to CPU\n", getSysTime(), idx); 
	fprintf(logfilePtr, "OSS: Time: %s PID: %d\t|||| Sent to CPU\n", getSysTime(), idx); 


	//Wait for message from User to simulate end CPU 
	msgrcv(shmidMsg2, &bufR, sizeof(bufR.mtext), mID, 0);

	//Get dispatch Time and display
	dispatchTime(idx); 
	
	//Determine how much of the quantum was used
	int sprint  = sysTimePtr->pcbTable[idx].sprint_Time; 
	sprint = sprint; 
	
	//Show Time Run
	fprintf(stderr, "OSS: Time: %s PID: %d\t|||| Spent %d nanoseconds in CPU\n", getSysTime(), idx, sprint); 
	fprintf(logfilePtr, "OSS: Time: %s PID: %d\t|||| Spent %d nanoseconds in CPU\n", getSysTime(), idx, sprint); 
	
	//Check return
	if( strcmp(bufR.mtext, "terminated") == 0){

		incrementSysTime(sprint); 
		
		unsetBitVectorVal(idx); 

		active[idx] = 0; 
		
		//Print Update for Ready
	//	fprintf(stderr, "OSS: Time: %s PID: %d\t|||| Terminated\n", getSysTime(), idx); 
	//	fprintf(logfilePtr, "OSS: Time: %s PID: %d\t|||| Terminated\n", getSysTime(), idx); 

		
		return; 
	}

	if( strcmp(bufR.mtext, "request") == 0){
		
	//	blockedQ[idx] = 1; 
	
	//request resources
	requesting(idx); 

		//Print Update
	//	fprintf(stderr, "OSS: Time: %s PID: %d\t|||| Added to Blocked Queue\n", getSysTime(), idx);
	//	fprintf(logfilePtr, "OSS: Time: %s PID: %d\t|||| Added to Blocked Queue\n", getSysTime(), idx);

		return; 
	}

	if(strcmp(bufR.mtext, "release") == 0){

		//Add Process Back to Queue
		enqueue(idx); 
		//Add Full Quantum to Systime
		incrementSysTime(1000000); 
	}
}


static void dispatchTime(int idx){

	int disTime = rand()%9901 + 100; 
	incrementSysTime(disTime); 
	
	fprintf(stderr, "OSS: Time: %s PID: %d\t|||| Time in Dispatch %d nanoseconds\n", getSysTime(), idx, disTime);
	fprintf(logfilePtr, "OSS: Time: %s PID: %d\t|||| Time in Dispatch %d nanoseconds\n", getSysTime(), idx, disTime);

}


static void requesting(int idx){


	int rIDX = sysTimePtr->pcbTable[idx].requestIDX; 

	if(sysTimePtr->SysR.availableResources[rIDX] > 0){

		fprintf(stderr, "Allocating P%d Resource\n"); 
		allocate(idx, sysTimePtr); 
		enqueue(idx); 
		return; 
	}
	
	fprintf(stderr, "P%d Sent to Blocked Queue\n", idx); 
	blockedQ[idx] = 1; 
}



//Initialize BlockedQ
static void initBlockedQ(){

	int i = 0; 

	for(i = 0; i < procMax; ++i){

		blockedQ[i] = 0; 
	}

}


//Check for Dead Lock Condition
static void checkDeadLock(){

	//Add to Log
	//fprintf(stderr, "Checking for Deadlock Condition\n"); 
	
	if( deadlock(sysTimePtr, concProc) == true){

		//Add to Log
		fprintf(stderr, "Deadlock Detected\n"); 
		
		terminateProc(); 
	}
}




//Check Blocked Que for Ready Proc
static void checkBlockedQ(){

	int i;
	float localT = getTime(); 
	int rIdx; 
	//int RVar; 
	int AvailR; 

	for(i = 0; i < procMax; ++i){
		
		rIdx = sysTimePtr->pcbTable[i].requestIDX;
		AvailR = sysTimePtr->SysR.availableResources[rIdx]; 
		
		
		if(blockedQ[i] == 1 && (sysTimePtr->pcbTable[i].requested[rIdx] <= sysTimePtr->SysR.availableResources[rIdx])){
		
			fprintf(stderr, "OSS: Time: %s PID: %d\t|||| Removed From Blocked Queue\n", getSysTime(), i);
			fprintf(logfilePtr, "OSS: Time: %s PID: %d\t|||| Removed From Blocked Queue\n", getSysTime(), i);			
			allocate(i, sysTimePtr); 
			enqueue(i);
			blockedQ[i] = 0; 
		}

	}
}


static void terminateProc(){


	int i;
	int j; 
	int status; 
	

	for(i = 0; i < procMax; ++i){
		
		if( deadlock(sysTimePtr, concProc) == false){

			fprintf(stderr, "Deadlock Cleared\n"); 

			return; 
		}
		
		if(blockedQ[i] == 1){
		
			fprintf(stderr,"Killing P%d and Freeing Resouces\n", i); 

			printArrHead(); 
			printArr(sysTimePtr->SysR.availableResources, "Sys-Available"); 
			fmt(sysTimePtr->pcbTable[i].allocated, "(A) P%d", i); 
			
			bufS.mtype = i+1; 
			strcpy(bufS.mtext, "terminate"); 
		

			if((msgsnd(shmidMsg, &bufS, sizeof(bufS.mtext), 0)) == -1 ){

				fprintf(stderr, "OSS: FAILED::: mID: %d\n", i+1);  
				perror("oss: ERROR: Failed to Send Msg to User msgsnd() "); 
					exit(EXIT_FAILURE); 
			}

			blockedQ[i] = 0; 
			unsetBitVectorVal(i); 
			active[i] = 0; 
			wait(NULL); 
			--concProc; 
		}
	}
	
	while( deadlock(sysTimePtr, concProc) == true){

		//Check for runnable Processes
		if(GQue->currSize == 0){ return; }
	
		//Dequeue Process
		CPU_Node = dequeue(); 
	
		//Check Node
		if(CPU_Node == NULL){

			fprintf(stderr,"Que Head Empty\n"); 
			return; 
		}

		int idx = CPU_Node->fakePID; 
			
		fprintf(stderr,"Killing P%d and freeing Resouces\n", idx); 

		printArrHead(); 
		printArr(sysTimePtr->SysR.availableResources, "Sys-Available"); 
		fmt(sysTimePtr->pcbTable[idx].allocated, "(A) P%d", idx); 
		
		bufS.mtype = idx+1; 
		strcpy(bufS.mtext, "terminate"); 
	
		if((msgsnd(shmidMsg, &bufS, sizeof(bufS.mtext), IPC_NOWAIT)) == -1 ){

			fprintf(stderr, "OSS: FAILED::: mID: %d\n", idx+1);  
			perror("oss: ERROR: Failed to Send Msg to User msgsnd() "); 
			exit(EXIT_FAILURE); 
		}

		//Wait for message from User to simulate end CPU 
		msgrcv(shmidMsg2, &bufR, sizeof(bufR.mtext), idx+1, IPC_NOWAIT);
		
		unsetBitVectorVal(idx);
		active[idx] = 0; 
		wait(NULL);
		--concProc; 
	}
}



//Get system Time for Calcs xx.xxx
static float getTime(){

	float decimal = sysTimePtr->nanoSeconds; 
	decimal = decimal/1000000000;
	float second = sysTimePtr->seconds; 
	float localT = second+decimal; 

	return localT; 
}


//Get Time for New user Process Offset
static float newUserTime(){

	float decimal = rand()%maxTimeBetweenNewProcNS;
	decimal = decimal/1000000000; 
	float second = rand()%maxTimeBetweenNewProcSecs; 
	float localT = second+decimal; 

	return localT; 
}
