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


void printArr(int arr[]){

	int i; 
	for(i = 0; i < maxResources; ++i){

		fprintf(stdout, "%d ", arr[i]); 
	}
	
	fprintf(stdout, "\n"); 

}


int getRand(int l, int u){

	if( l == 0 ){

		return rand() % (u+1); 
	}
	
	return ( rand() % u ) + 1; 
}


