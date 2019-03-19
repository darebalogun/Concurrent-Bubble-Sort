Authors: Kurt Burton-Rowe (100991883)
	     Dare Balogun (101062340)

# Concurrent Bubble Sort

An implementation of the bubble sort algorithm that sorts the contents of an array using concurrent processes. The program uses shared memory and semaphores to control access to the data. 

Files description:

STATS.c: This contains the program for the bubble sort which computes the result of an array with five elements. The program gives the median, the max and min of the array.

arrayStruct.h: This header file defines an array structure which is used to create the shared memory in the program

semun.h: This header file defines the sem structure of the program which is used to create the shared memory in the program 


Makefile: A make file to help compile all dependecies of the programs


*********Instructions to run:

In order to compile the assignment, you must go to the terminal and access the directory 
location of the assignment files. Once you get into the assignment's folder you can just type 
make and then run myapp afterwards. 
You do not specify any argument except the selection of debug mode.
