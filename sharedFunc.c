/*
 * Author: Nick House
 * Project: Resource Management
 * File Name: sharedFunc.c
 */

#include "headers.h"
#include "shared.h"
#include "sharedFunc.h"

//=== Sem Vars ===//
struct sembuf sops;  

void semWait(int sem){

	sops.sem_num = sem; 
	sops.sem_op = -1; 
	sops.sem_flg = 0; 

	if( semop(shmidSem, &sops, 1) == -1 ){

		perror("sharedLib: ERROR: semWait() "); 
		exit(EXIT_FAILURE); 
	}
}


void semSignal(int sem){

	sops.sem_num = sem; 
	sops.sem_op = 1; 
	sops.sem_flg = 0; 

	if( semop(shmidSem, &sops, 1) == -1){

		perror("sharedLib: ERROR: semSignal() ");
		exit(EXIT_FAILURE); 
	}
}

void initResourceArr( struct system_Time * st){

	int i; 
	int r; 
	
	//initialize System Resources
	for( i = 0; i < maxResources; ++i ){

		r = getRand(1,10); 
		st->SysR.resources[i] = r; 
		st->SysR.availableResources[i] = r; 

	}
	
	//Set Shared Resources
	
	for( i = 0; i < maxResources; ++i){
		
		st->SysR.sharedResources[i] = 0; 
	}

	int numShared = 2 + getRand(1,3); 
	int numArr[numShared]; 
	int j; 

	for( i = 0; i < numShared; ++i){
		
		r = getRand(0, 19);

		for(j = 0; j < i; ++j){

			while(numArr[j] == r || st->SysR.resources[j] <= 0){

				r = getRand(0,19); 
			}	
		}
	
		numArr[i] = r; 

		st->SysR.sharedResources[r] = 1; 
	}

}


void printArrHead(){

	fprintf(stdout, "\n                  R0  R1  R2  R3  R4  R5  R6  R7  R8  R9 R10 R11 R12 R13 R14 R15 R16 R17 R18 R19\n");//, blanks); 

}

void printArr(int arr[], char name[]){
	

	fprintf(stdout, "%15s:", name); 

	int i; 
	for(i = 0; i < maxResources; ++i){

		fprintf(stdout, "%3d ", arr[i]); 
	}
	
	fprintf(stdout, "\n"); 

}


int getRand(int l, int u){

	if( l == 0 ){

		return rand() % (u+1); 
	}
	
	return ( rand() % u ) + 1; 
}



//Allocate Resources
void allocate(int idx, struct system_Time *st){

	int i; 
	int t; 
	int rIdx; 

	rIdx = st->pcbTable[idx].requestIDX; 
			
			fprintf(stderr, "Allocating RESOURCE -> P%d:R%d\n", idx, rIdx); 
			
			//t = st->pcbTable[idx].requested[i];
			st->pcbTable[idx].allocated[rIdx] += 1; 

			//Check if resource is shared
			if(st->SysR.sharedResources[rIdx] == 0){
				
				fprintf(stderr, "Non - Shared RESOURCE\n"); 

				//Decrement System Resources
				st->SysR.availableResources[rIdx] = (st->SysR.availableResources[rIdx] - 1); 
			}

			fprintf(stderr, "Shared RESOURCE\n"); 
			
			//Add Resource to User 
		//	st->pcbTable[idx].allocated[rIdx] += 1; 

			//Turn Request Flag false
			st->pcbTable[idx].requestBool = false; 
			//Set Allocated Flag True
			st->pcbTable[idx].allocateBool = true; 

			//Set Corresponding Request to 0
			st->pcbTable[idx].requested[rIdx] = 0; 
			st->pcbTable[idx].requestIDX = 0; 
			
			printArrHead(); 
			//printArr(st->SysR.availableResources, "Available"); 
			fmt(st->SysR.resources, "Sys-Resources"); 
			fmt(st->SysR.availableResources, "Sys-Available"); 
			fmt(st->SysR.sharedResources, "Sys-Shared"); 
			printArrHead(); 
			//printArr(st->pcbTable[idx].allocated, "P%d",idx); 
			fmt(st->pcbTable[idx].allocated, "P%d Allocated", idx); 
			fmt(st->pcbTable[idx].maximum, "P%d Maximum", idx); 

			
}



//Format string for func calls
void fmt(int arr[], char* fmt, ...){

	char buf[100];

	va_list vl; 
	va_start(vl, fmt); 

	vsnprintf(buf, sizeof(buf), fmt, vl); 
	va_end(vl); 

	printArr(arr, buf); 
}



//Format string for func calls
void logPrint(bool printBool, int arr[], char* fmt, ...){

	char buf[100];

	va_list vl; 
	va_start(vl, fmt); 

	vsnprintf(buf, sizeof(buf), fmt, vl); 
	va_end(vl); 

	fprintf(stderr, "%s", buf); 

	if(printBool == true){

		printArr(arr, buf); 
	}
}


//void allocate(int idx, struct system_Time *st){
//bool req_lt_avail( const int * req, const int * avail, const int pnum, const int num_res ){
static bool req_lt_avail( struct system_Time *st, const int idx){

	int i; 
	for(i = 0; i < maxResources; ++i){

		if (st->pcbTable[idx].requested[i] > st->SysR.availableResources[i]){

			fprintf(stderr, "REQ: %d  Avail: %d\n", st->pcbTable[idx].requested[i], st->SysR.availableResources[i]); 
			break; 
		}
	}

	fprintf(stderr, "i:%d\n", i); 

	return (i == maxResources);
}


//Number of current processes
bool deadlock(struct system_Time *st, const int n){

	fprintf(stderr, "Deadlock Algo num Proc: %d\n", n); 
	
	int work[maxResources]; 
	bool finish[n];                   //n = number of processes

	int i ;
	for(i = 0; i < maxResources; ++i){

		work[i] = st->SysR.availableResources[i]; 
	}

	for(i = 0; i < n; ++i){

		finish[i] = false; 
	}

	int p; 

	for(p = 0; p < n; ++p){

		if( finish[p] ) { continue; }
		
		if( req_lt_avail(st, p)){
	
			finish[p] = true; 
			
			for( i = 0; i < maxResources; ++i ){

				work[i] += st->pcbTable[p].allocated[i]; 
			}
			
			p = -1; 
		}
	}

	for(p = 0; p < maxResources; ++p){

		if( ! finish[p] ){

			break; 
		}
	}
	
	fprintf(stderr, "P = %d n = %d\n", p, n); 

	return ( p != n );
}




