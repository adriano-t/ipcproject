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

// memoria condivisa
# include <sys/ipc.h>
# include <sys/shm.h>
 
// semafori 
#include <sys/types.h>
#include <sys/sem.h> 
 
#include <sys/wait.h>

#include "utils.h"
#define STDIN 0
#define STDOUT 1

/**
* @brief Union to handle the semaphores
*/
union semun
{
	int val;                /// value for SETVAL
	struct semid_ds* buf;   /// buffer for IPC_STAT, IPC_SET
	unsigned short*  array; /// array for GETALL, SETALL
	struct seminfo*  __buf; /// buffer for IPC_INFO
};  
int sem_parent;
int sem_request_result;
int sem_computing;
int sem_wait_data;
    
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
int *childs_pid; 
bool * free_child;
int id_number = -1;
int n_operations = -1;
int childs_count = 0;
int* childs_started;

void parent();
void child();

float* results;
const int SMKEY_OP = 101;
const int SMKEY_RES = 102;
const int SMKEY_STATUS = 103;
const int SMKEY_STARTED = 104;

enum sem_type { MUTEX = 0, RESULT_READY = 1 , WAIT_LAST_CHILD = 1, DATA_READ = 2}; 

struct operation* current_operation;
struct result* current_result;

struct sembuf sops;

/**
 * @brief increases the semaphore by 1
 * @param semid semaphore id
 * @param num semaphore index
 */
void sem_v(int semid, int num);
 
/**
 * @brief decreases the semaphore by 1
 * @param semid semaphore id
 * @param num semaphore index
 */
void sem_p(int semid, int num);

/**
 * @brief returns the first free computing unit
 * if there arent free computing unit, the first is returned (0)
 */
int get_first_free_child();

/**
 * @brief parent process handler
 */
void parent();

/**
 * @brief child process handler
 */
void child();

 
 
#endif
 
 
