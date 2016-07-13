/** @file ipc_calculator.h
 *  @brief ipc calculator
 */


/*! \mainpage Main Page
 *
 * \section intro_sec Introduction
 *
 * lo script run.sh ha varie funzionalità:
 * - ./run.sh: esegue il programma
 * - ./run.sh compile: compila il codice 
 * - ./run.sh compilerun: compila il codice ed esegue
 * - ./run.sh free: libera le ipc in caso di errore
 * - ./run.sh push messaggio: esegue il commit e il push nel repository git
 */
 

#ifndef IPC_CALCULATOR_H
#define IPC_CALCULATOR_H

#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
 
   
#include <sys/types.h> 
   
#include "utils.h"
#define STDIN 0
#define STDOUT 1

struct thread_info
{
	int id;
	int operation;
};

struct thread_info* info;

pthread_attr_t attr;
pthread_t* threads;
bool * free_threads;
int threads_count = 0; 
/**
* @brief A single operation to perform
*/
struct operation
{
	int id;
    int val1;
    int val2;
    char operator; 
};

/**
* @brief Result of an operation
*/
struct result
{
	int id;
	float val;
};

struct operation* operations; 
int id_number = -1;
int n_operations = -1;

void compute();
void* execute_operation();

float* results;  

/**
 * @brief returns the first free computing unit
 * if there arent free computing unit, the first is returned (0)
 */
int get_first_free_thread();

/**
 * @brief parent process handler
 */
void parent();

/**
 * @brief child process handler
 */
void child();

 
 
#endif
 
 
