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
void printArrHead(); 
void printArr(); 
int shmidSem; 

#endif
