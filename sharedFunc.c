/*
 * Author: Nick House
 * Project: Resource Management
 * File Name: sharedFunc.c
 */

#include "headers.h"
#include "shared.h"
#include "sharedFunc.h"

//=== Sem Vars ===//
//struct sembuf sops;  
//FILE *file; 
//void openfile();
//void closefile(); 

void setShmid(struct system_Time * ptr){
	
	st = ptr; 
}


void setSemID(int semID){

	shmidSem = semID;
}

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

//void openfile(){
//
//	file = fopen("logfile_P5", "a");

//	if( file == NULL ){
//
//		perror("sharedFunc: ERROR: Failed to Open logfile_P5 "); 
//		exit(EXIT_FAILURE); 
//	}
//}

//void closefile(){
//	
//	fclose(file); 
//}


void initResourceArr( struct system_Time * st){

	int i; 
	int r; 

	st->grantedReq = 0; 
	
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


//void printArrHead(){
	
//	if(st->fileLength > 99950){ return; }

//	openfile(); 

//	fprintf(stdout, "\n                  R0  R1  R2  R3  R4  R5  R6  R7  R8  R9 R10 R11 R12 R13 R14 R15 R16 R17 R18 R19\n");//, blanks); 
//	fprintf(file, "\n                  R0  R1  R2  R3  R4  R5  R6  R7  R8  R9 R10 R11 R12 R13 R14 R15 R16 R17 R18 R19\n");//, blanks); 
///
//	st->fileLength++;
	
//	closefile(); 
//}

//void printArr(int arr[], char name[]){
//	
//	if(st->fileLength > 99950){ return; }
//
///	openfile(); 
//
//	fprintf(stdout, "%15s:", name); 
//	fprintf(file, "%15s:", name); 
///
//	int i; 
//	for(i = 0; i < maxResources; ++i){
//
//		fprintf(stdout, "%3d ", arr[i]); 
//		fprintf(file, "%3d ", arr[i]); 
//	}
//	
//	fprintf(stdout, "\n"); 
//	fprintf(file, "\n"); 
//
//	st->fileLength += 3; 
//
//	closefile();
//
//}//


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
			
	st->pcbTable[idx].allocated[rIdx] += 1; 

	//Check if resource is shared
	if(st->SysR.sharedResources[rIdx] == 0){
				
		//Decrement System Resources
		st->SysR.availableResources[rIdx] = (st->SysR.availableResources[rIdx] - 1); 
	}
			
	//Turn Request Flag false
	st->pcbTable[idx].requestBool = false; 
	
	//Set Allocated Flag True
	st->pcbTable[idx].allocateBool = true; 
	st->grantedReq++; 

	//Set Corresponding Request to 0
	st->pcbTable[idx].requested[rIdx] = 0; 
	st->pcbTable[idx].requestIDX = 0; 
			
	
//	if( st->fileLength < 99950 ){

//		fprintf(stderr, "Master: Allocating P%d Resource R%d at Time: %f\n", idx, rIdx, getTime()); 
//		st->fileLength++; 
//	}

			
}



////Format string for func calls
//void fmt(int arr[], char* string, ...){
//
//	char buf[100];
//
//	va_list vl; 
//	va_start(vl, string); 
//
//	vsnprintf(buf, sizeof(buf), string, vl); 
//	va_end(vl); 
//
//	printArr(arr, buf); 
//}



//Format string for func calls
//void logPrint(char* string, ...){
//
//	if( st->fileLength > 99950 ){ return; } 
//	
//	char buf[100];
//
//	va_list vl; 
//	va_start(vl, string); 
//
//	vsnprintf(buf, sizeof(buf), string, vl); 
//	va_end(vl); 
//
//	fprintf(stderr, "%s at Time: %f", buf, getTime()); 
//	fprintf(file, "%s at Time: %f", buf, getTime()); 
///	st->fileLength++; 
//}


//Check if resources are requested are avaiable
static bool req_lt_avail( struct system_Time *st, const int idx){

	int i; 
	for(i = 0; i < maxResources; ++i){

		if (st->pcbTable[idx].requested[i] > st->SysR.availableResources[i]){

			break; 
		}
	}


	return (i == maxResources);
}


//Number of current processes
bool deadlock(struct system_Time *st, const int n){

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
	
	return ( p != n );
}



float getTime(){

	float decimal = st->nanoSeconds; 
	decimal = decimal/1000000000;

	float second = st->seconds; 

	float localT = second+decimal; 

	return localT; 
}
