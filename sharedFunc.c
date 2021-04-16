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
	//	fprintf(stdout, "%d ", st->SysR.resource[i]); 

	}
	
	fprintf(stdout, "\n"); 

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

			while(numArr[j] == r){

				r = getRand(0,19); 
			}	
		}
	
		numArr[i] = r; 

		st->SysR.sharedResources[r] = 1; 
	}
}


void printArr(int arr[], char * name[]){

	
	char blanks[14];
	memset(blanks, ' ', 14); 

	fprintf(stdout, "%sR0  R1  R2  R3  R4  R5  R6  R7  R8  R9 R10 R11 R12 R13 R14 R15 R16 R17 R18 R19\n", blanks); 

	fprintf(stdout, "%10s > ", name); 

	int i; 
	for(i = 0; i < maxResources; ++i){

		fprintf(stdout, "%3d ", arr[i]); 
	}
	
	fprintf(stdout, "\n\n"); 

}


int getRand(int l, int u){

	if( l == 0 ){

		return rand() % (u+1); 
	}
	
	return ( rand() % u ) + 1; 
}


