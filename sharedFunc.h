/*
 * Author: Nick House
 * Project: Resource Management
 * File Name: sharedFunc.h
 */

#ifndef SHAREDFUNC_H
#define SHAREDFUNC_H

void semWait();  
void semSignal();
void initResourcArr();
void allocate(); 
void printArrHead(); 
void printArr(); 
void fmt(int arr[], char*fmt, ...); 
int shmidSem; 

#endif
