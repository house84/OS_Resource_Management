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
	strcpy(logfile, "logfile_Sch"); 
	memset(logfile2, '\0', sizeof(logfile2)); 
	strcpy(logfile2, "logfile_P5"); 
	
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
	setShmid(sysTimePtr); 
	sysTimePtr->verbose = verbose; 

	//Initialize Queue
	GQue = initQueue(); 

	//Initilize BQ
	initBlockedQ();

	//Initialize Resource Array
	initResourceArr(sysTimePtr); 
	memset(active, 0, 18); 
	
	//Initialize STAT
	sysTimePtr->stats.totalProc = 0; 
	sysTimePtr->stats.cpu_Time = 0; 
	sysTimePtr->stats.waited_Time = 0; 
	sysTimePtr->stats.blocked_Time = 0; 
	sysTimePtr->stats.idle_Time = 0; 
	sysTimePtr->stats.numDL = 0; 
	sysTimePtr->stats.numReqI = 0; 
	sysTimePtr->stats.numReqW = 0; 
	sysTimePtr->stats.terminatedDL = 0; 
	sysTimePtr->stats.terminatedN = 0; 
	sysTimePtr->stats.deadlockCond = 0; 

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
			
			checkDeadLock();
			//deadLockTimer = sysTimePtr->seconds; 
			deadLockTimer = sysTimePtr->seconds + 1; 
		} 


		//Spawn Child Process 
		if( concProc < procMax && totalProc < 40 && stopProdTimer == false ){

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

	}


	//Allow Processes to finish
	while(wait(NULL) > 0){} 

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

	  	fprintf(stderr, "\n|||==> All Processes Have Completed <===|||\n"); 
	  	fprintf(stderr, "|||==>   Freeing System Resources   <===|||\n"); 
	  	fprintf(logfilePtr2, "\n|||==> All Processes Have Completed <===|||\n"); 
	  	fprintf(logfilePtr2, "|||==>   Freeing System Resources   <===|||\n"); 
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


static void openfile(){

	logfilePtr2 = fopen(logfile, "a"); 

}

//Open New Logfile
static void openLogfile(){
	
	logfilePtr = fopen(logfile, "w"); 

	if( logfilePtr == NULL ){

		perror("oss: ERROR: Failed to Open logfile_sch "); 
		exit(EXIT_FAILURE); 
	}
	
	logfilePtr2 = fopen(logfile2, "w"); 

	if( logfilePtr2 == NULL ){

		perror("oss: ERROR: Failed to Open logfile_P5 "); 
		exit(EXIT_FAILURE); 
	}


	time(&t); 

	fprintf(logfilePtr, "\n//========================= Log Opened ========================//\n"); 
	fprintf(logfilePtr, "Time: %s", ctime(&t));
	fprintf(logfilePtr, "//=============================================================//\n"); 

	fprintf(logfilePtr2, "\n//========================= Log Opened ========================//\n"); 
	fprintf(logfilePtr2, "Time: %s", ctime(&t));
	fprintf(logfilePtr2, "//=============================================================//\n"); 

//	fclose(logfilePtr2); 
}


//Close Logfile
static void closeLogfile(){
	 
	
	fprintf(logfilePtr, "\n//========================= Log Closed ========================//\n"); 
	fprintf(logfilePtr, "Time: %s", ctime(&t)); 
	fprintf(logfilePtr, "//=============================================================//\n\n"); 
	fclose(logfilePtr); 

	fprintf(logfilePtr2, "\n//========================= Log Closed ========================//\n"); 
	fprintf(logfilePtr2, "Time: %s", ctime(&t)); 
	fprintf(logfilePtr2, "//=============================================================//\n\n"); 
	fclose(logfilePtr2); 

}


//Display Stats/Print Stats
static void displayStats(){
	
	int total  = totalProc;
	float avgTermPDL, avgPerDL; 
	
	if(sysTimePtr->stats.terminatedDL > 0){

		avgTermPDL = ((float)sysTimePtr->stats.terminatedDL/sysTimePtr->stats.deadlockCond);
	}
	else { avgTermPDL = 0.00; }

	if(sysTimePtr->stats.deadlockCond > 0){

		avgPerDL = (sysTimePtr->stats.avgPercTerm/sysTimePtr->stats.deadlockCond); 
	}
	else { avgPerDL = 0.00; }

	int normalT = sysTimePtr->stats.terminatedN - sysTimePtr->stats.terminatedDL; 
	
//	openfile(); 

	//Print to Terminal
	fprintf(stderr, "\n\n///////////////////// PROGRAM REPORT /////////////////////\n"); 
	fprintf(stderr, "\n---------------------SCHEDULING STATS---------------------\n"); 
	fprintf(stderr, "System Time: %f\n", getTime()); 
	fprintf(stderr, "Average Process CPU Time: %f\n", sysTimePtr->stats.cpu_Time/total); 
	fprintf(stderr, "Average Process System Time: %f\n", sysTimePtr->stats.system_Time/total); 
	fprintf(stderr, "Average Process Blocked Time: %f\n", sysTimePtr->stats.blocked_Time/total);
	fprintf(stderr, "Average Process Wait Time: %f\n", sysTimePtr->stats.waited_Time/total); 
	fprintf(stderr, "CPU Idle Time: %f\n", (getTime() - sysTimePtr->stats.cpu_Time)); 
	fprintf(stderr, "\n----------------------RESOURCE STATS----------------------\n"); 
	fprintf(stderr, "Requests Granted Immediately: %d\n", sysTimePtr->stats.numReqI); 
	fprintf(stderr, "Requests Granted After Waiting: %d\n", sysTimePtr->stats.numReqW); 
	fprintf(stderr, "Number of Normally Terminated Processes: %d\n", sysTimePtr->stats.terminatedN); 
	fprintf(stderr, "Number of Deadlock Terminated Processes: %d\n", sysTimePtr->stats.terminatedDL); 
	fprintf(stderr, "Number of time Deadlock Detection Algorithm Ran: %d\n", sysTimePtr->stats.numDL);
	fprintf(stderr, "Number of Deadlock Conditions Detected: %d\n", sysTimePtr->stats.deadlockCond); 
	fprintf(stderr, "Average Number of Processes Terminated per Deadlock: %3.0f\n", avgTermPDL); 
	fprintf(stderr, "Percent of Processes Terminated per Deadlock on Avg: %3.2f%\n\n", avgPerDL); 
	fprintf(stderr, "///////////////////// |||||||||||||| /////////////////////\n"); 
	
	//Print to logs
	fprintf(logfilePtr, "\n\n///////////////////// PROGRAM REPORT /////////////////////\n"); 
	fprintf(logfilePtr, "\n---------------------SCHEDULING STATS---------------------\n"); 
	fprintf(logfilePtr, "System Time: %f\n", getTime()); 
	fprintf(logfilePtr, "Average Process CPU Time: %f\n", sysTimePtr->stats.cpu_Time/total); 
	fprintf(logfilePtr, "Average Process System Time: %f\n", sysTimePtr->stats.system_Time/total); 
	fprintf(logfilePtr, "Average Process Blocked Time: %f\n", sysTimePtr->stats.blocked_Time/total); 
	fprintf(logfilePtr, "Average Process Wait Time: %f\n", sysTimePtr->stats.waited_Time/total); 
	fprintf(logfilePtr, "CPU Idle Time: %f\n", (getTime() - sysTimePtr->stats.cpu_Time)); 
	fprintf(logfilePtr, "///////////////////// |||||||||||||| /////////////////////\n"); 
	fprintf(logfilePtr2, "\n\n///////////////////// PROGRAM REPORT /////////////////////\n"); 
	fprintf(logfilePtr2, "\n----------------------RESOURCE STATS----------------------\n"); 
	fprintf(logfilePtr2, "Requests Granted Immediately: %d\n", sysTimePtr->stats.numReqI); 
	fprintf(logfilePtr2, "Requests Granted After Waiting: %d\n", sysTimePtr->stats.numReqW); 
	fprintf(logfilePtr2, "Number of Normally Terminated Processes: %d\n", sysTimePtr->stats.terminatedN); 
	fprintf(logfilePtr2, "Number of Deadlock Terminated Processes: %d\n", sysTimePtr->stats.terminatedDL); 
	fprintf(logfilePtr2, "Number of time Deadlock Detection Algorithm Ran: %d\n", sysTimePtr->stats.numDL);
	fprintf(logfilePtr2, "Number of Deadlock Conditions Detected: %d\n", sysTimePtr->stats.deadlockCond); 
	fprintf(logfilePtr2, "Average Number of Processes Terminated per Deadlock: %3.0f\n", avgTermPDL); 
	fprintf(logfilePtr2, "Percent of Processes Terminated per Deadlock on Avg: %3.2f%\n\n", avgPerDL); 
	fprintf(logfilePtr2, "///////////////////// |||||||||||||| /////////////////////\n"); 

//	fclose(logfilePtr2); 
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
	
	if(verbose == true){

		if((sysTimePtr->grantedReq % 20 == 0) && sysTimePtr->fileLength < 99950 ){
		
			printArrHead();
			printArr(sysTimePtr->SysR.sharedResources, "Shared"); 
			printArr(sysTimePtr->SysR.availableResources, "Available"); 

			int j; 
			for(j=0; j < 18; ++j){

				if(j == 0){

					fprintf(stderr, "\nAllocated Resources"); 
					fprintf(logfilePtr2, "\nAllocated Resources"); 

					printArrHead(); 
					sysTimePtr->fileLength++; 
				}
					
				if(active[j] == 1 ){

					fmt(sysTimePtr->pcbTable[j].allocated, "P%d", j); 
			 	}
			}
			
			fprintf(stderr, "\n\n"); 
			fprintf(logfilePtr2, "\n\n"); 

			sysTimePtr->fileLength++; 
		}
	}
			
	//Check for runnable Processes
	if(GQue->currSize == 0){ 
		
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
	fprintf(logfilePtr, "OSS: Time: %s PID: %d\t|||| Sent to CPU\n", getSysTime(), idx); 


	//Wait for message from User to simulate end CPU 
	msgrcv(shmidMsg2, &bufR, sizeof(bufR.mtext), mID, 0);

	//Get dispatch Time and display
	dispatchTime(idx); 
	
	//Determine how much of the quantum was used
	int sprint  = sysTimePtr->pcbTable[idx].sprint_Time; 
	sprint = sprint; 
	
	//Show Time Run Scheduler
	fprintf(logfilePtr, "OSS: Time: %s PID: %d\t|||| Spent %d nanoseconds in CPU\n", getSysTime(), idx, sprint); 
	
	//Check return
	if( strcmp(bufR.mtext, "terminated") == 0){

		int i; 
		
		if(sysTimePtr->fileLength < 99950 && sysTimePtr->verbose == true ){
			
			logPrint("\nMaster: Detected P%d is Terminating", idx); 
			
			fprintf(stderr, "\n\tProcess P%d Releasing: ", idx); 
			fprintf(logfilePtr2, "\n\tProcess P%d Releasing: ", idx); 
			sysTimePtr->fileLength++; 

			for( i = 0; i < maxResources; ++i ){
	
				if( sysTimePtr->pcbTable[idx].release[i] > 0){

					fprintf(stderr, "R%d:%d  ", i, sysTimePtr->pcbTable[idx].release[i]); 
					fprintf(logfilePtr2, "R%d:%d  ", i, sysTimePtr->pcbTable[idx].release[i]); 
					sysTimePtr->pcbTable[idx].release[i] = 0; 
					sysTimePtr->fileLength++; 
				}
			}

			fprintf(stderr, "\n"); 
			fprintf(logfilePtr2, "\n"); 
			sysTimePtr->fileLength++; 
		}
		
		//Add Process Back to Queue
		incrementSysTime(sprint); 
		
		unsetBitVectorVal(idx); 

		active[idx] = 0; 

		++sysTimePtr->stats.terminatedN; 
		
		//Print Update for Ready Scheduler
		fprintf(logfilePtr, "OSS: Time: %s PID: %d\t|||| Terminated\n", getSysTime(), idx); 

		
		return; 
	}

	if( strcmp(bufR.mtext, "request") == 0){
	
	
		//request resources
		requesting(idx); 

		//Print Update
		fprintf(logfilePtr, "OSS: Time: %s PID: %d\t|||| Added to Blocked Queue\n", getSysTime(), idx);

		return; 
	}

	if(strcmp(bufR.mtext, "release") == 0){

		int i; 
		
		if(sysTimePtr->fileLength < 99950 && sysTimePtr->verbose == true ){
			
			logPrint("\nMaster: Detected P%d is releasing resource", idx, getTime()); 
			
			fprintf(stderr, "\n\tProcess P%d Releasing: ", idx); 
			fprintf(logfilePtr2, "\n\tProcess P%d Releasing: ", idx); 
			sysTimePtr->fileLength++; 

			for( i = 0; i < maxResources; ++i ){
	
				if( sysTimePtr->pcbTable[idx].release[i] > 0){

					fprintf(stderr, "R%d:%d  ", i, sysTimePtr->pcbTable[idx].release[i]); 
					fprintf(logfilePtr2, "R%d:%d  ", i, sysTimePtr->pcbTable[idx].release[i]); 
					sysTimePtr->pcbTable[idx].release[i] = 0; 
					sysTimePtr->fileLength++; 
				}
			}

			fprintf(stderr, "\n"); 
			fprintf(logfilePtr2, "\n"); 
			sysTimePtr->fileLength++; 
		}
		
		//Add Process Back to Queue
		enqueue(idx); 
		
		//Add Full Quantum to Systime
		incrementSysTime(1000000); 
	}
}


static void dispatchTime(int idx){

	int disTime = rand()%9901 + 100; 
	incrementSysTime(disTime); 
	
	fprintf(logfilePtr, "OSS: Time: %s PID: %d\t|||| Time in Dispatch %d nanoseconds\n", getSysTime(), idx, disTime);

}


static void requesting(int idx){


	int rIDX = sysTimePtr->pcbTable[idx].requestIDX; 

	if(sysTimePtr->SysR.availableResources[rIDX] > 0){
	
		if(sysTimePtr->fileLength < 99950){
			fprintf(logfilePtr2, "\nMaster: Allocating P%d Resource R%d:%d at Time: %f\n", idx, rIDX, sysTimePtr->SysR.availableResources[rIDX], getTime()); 
			fprintf(stderr, "\nMaster: Allocating P%d Resource R%d:%d at Time: %f\n", idx, rIDX, sysTimePtr->SysR.availableResources[rIDX], getTime()); 
			sysTimePtr->fileLength++; 
		}

		allocate(idx, sysTimePtr); 
		enqueue(idx); 
		++sysTimePtr->stats.numReqI; 
		
		return; 
	}
	
	if(sysTimePtr->verbose == true){

		if(sysTimePtr->fileLength < 99950){
			
			fprintf(logfilePtr2, "\nMaster: Process P%d Sent to Blocked Queue at Time: %f\n", idx, getTime()); 
			fprintf(stderr, "\nMaster: Process P%d Sent to Blocked Queue at Time: %f\n", idx, getTime()); 
			sysTimePtr->fileLength++; 
		}
	}

	blockedQ[idx] = 1;
	++sysTimePtr->stats.numReqW; 
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

	++sysTimePtr->stats.numDL; 

	if( deadlock(sysTimePtr, concProc) == true){

		//Add to Log
		if(sysTimePtr->fileLength < 99950){
			
			fprintf(logfilePtr2,"\nMaster: Deadlock Detected at Time %f\n", getTime()); 
			fprintf(stderr,"\nMaster: Deadlock Detected at Time: %f\n"); 
			sysTimePtr->fileLength++; 
		}
		++sysTimePtr->stats.deadlockCond; 
		
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
	float localCount = 0; 
	int tProc = concProc; 
	int status;
	bool freed = false; 
	

	for(i = 0; i < procMax; ++i){
		
		if( deadlock(sysTimePtr, concProc) == false){

			if(sysTimePtr->fileLength < 99950){
				
				fprintf(logfilePtr2, "\nMaster: Deadlock Cleared at Time: %f\n", getTime());
				fprintf(stderr, "\nMaster: Deadlock Cleared at Time: %f\n", getTime());
				sysTimePtr->fileLength++; 
			}

			sysTimePtr->stats.avgPercTerm = (localCount/concProc)*100; 
			
			return; 
		}
		
		if(blockedQ[i] == 1){
		
			if(sysTimePtr->verbose == true && sysTimePtr->fileLength < 99950){
				
				fprintf(logfilePtr2, "\nMaster: Killing Process P%d and Freeing Resources\n", i);
				fprintf(stderr, "\nMaster: Killing Process P%d and Freeing Resources\n"), i;
				sysTimePtr->fileLength++; 
			}	

			printArrHead(); 
			fmt(sysTimePtr->pcbTable[i].allocated, "Releasing P%d", i); 
			fprintf(stderr,"\n"); 
			fprintf(logfilePtr2,"\n");
			sysTimePtr->fileLength++; 
			
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
			++sysTimePtr->stats.terminatedDL;
			++localCount; 
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
			break;  
		}

		int idx = CPU_Node->fakePID; 
			
		if(sysTimePtr->verbose == true && sysTimePtr->fileLength < 99950){
				
			fprintf(logfilePtr2, "\nMaster: Killing Process P%d and Freeing Resources\n", i);
			fprintf(stderr, "\nMaster: Killing Process P%d and Freeing Resources\n", i);
			sysTimePtr->fileLength++; 
		}	
			
		printArrHead(); 
		fmt(sysTimePtr->pcbTable[idx].allocated, "Releasing P%d", idx); 
		fprintf(stderr,"\n"); 
		fprintf(logfilePtr2,"\n");
		sysTimePtr->fileLength++; 

		bufS.mtype = idx+1; 
		strcpy(bufS.mtext, "terminate"); 
	
		if((msgsnd(shmidMsg, &bufS, sizeof(bufS.mtext), IPC_NOWAIT)) == -1 ){

			perror("oss: ERROR: Failed to Send Msg to User msgsnd() "); 
			exit(EXIT_FAILURE); 
		}

		//Wait for message from User to simulate end CPU 
		msgrcv(shmidMsg2, &bufR, sizeof(bufR.mtext), idx+1, IPC_NOWAIT);
		
		unsetBitVectorVal(idx);
		active[idx] = 0; 
		wait(NULL);
		--concProc; 
		++sysTimePtr->stats.terminatedDL; 
		++localCount; 
	}
		
	if(sysTimePtr->fileLength < 99950){
			
		fprintf(logfilePtr2, "\nMaster: Deadlock Cleared at Time: %f\n", getTime());
		fprintf(stderr, "\nMaster: Deadlock Cleared at Time: %f\n", getTime());
		sysTimePtr->fileLength++; 
	}

	sysTimePtr->stats.avgPercTerm = (localCount/concProc)*100; 
			
	return; 
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


//Print ArrHead
static void printArrHead(){
	
	if(st->fileLength > 99950){ return; }

	fprintf(stdout, "\n                  R0  R1  R2  R3  R4  R5  R6  R7  R8  R9 R10 R11 R12 R13 R14 R15 R16 R17 R18 R19\n");//, blanks); 
	fprintf(logfilePtr2, "\n                  R0  R1  R2  R3  R4  R5  R6  R7  R8  R9 R10 R11 R12 R13 R14 R15 R16 R17 R18 R19\n");//, blanks); 

	st->fileLength++;
	
}


//Print Arr
static void printArr(int arr[], char name[]){
	
	if(st->fileLength > 99950){ return; }

	fprintf(stdout, "%15s:", name); 
	fprintf(logfilePtr2, "%15s:", name); 

	int i; 
	for(i = 0; i < maxResources; ++i){

		fprintf(stdout, "%3d ", arr[i]); 
		fprintf(logfilePtr2, "%3d ", arr[i]); 
	}
	
	fprintf(stdout, "\n"); 
	fprintf(logfilePtr2, "\n"); 

	st->fileLength += 3; 

}



//Format string for func calls
static void fmt(int arr[], char* string, ...){

	char buf[100];

	va_list vl; 
	va_start(vl, string); 

	vsnprintf(buf, sizeof(buf), string, vl); 
	va_end(vl); 

	printArr(arr, buf); 
}



//Format string for func calls
static void logPrint(char* string, ...){

	if( st->fileLength > 99950 ){ return; } 
	
	char buf[100];

	va_list vl; 
	va_start(vl, string); 

	vsnprintf(buf, sizeof(buf), string, vl); 
	va_end(vl); 

	fprintf(stderr, "%s at Time: %f", buf, getTime()); 
	fprintf(logfilePtr2, "%s at Time: %f", buf, getTime()); 
	st->fileLength++; 
}
