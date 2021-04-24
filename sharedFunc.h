/*
 * Author: Nick House
 * Project: Resource Management
 * File Name: sharedFunc.h
 */

#ifndef SHAREDFUNC_H
#define SHAREDFUNC_H

void semWait();  
void semSignal();
void setSemID(); 
void initResourcArr();
void allocate(); 
void printArrHead(); 
void printArr();
void setLogfile(); 
void fmt(int arr[], char*fmt, ...);
void logPrint(bool printArr, int arr[], char * fmt, ...);  
void setShmid(); 
float getTime(); 

FILE * file; 
struct sembuf sops; 
struct system_Time * st; 

//Deadlock
static bool req_lt_avail(); 								
bool deadlock();                       //Stuct sysTime and number of current processes

#endif
