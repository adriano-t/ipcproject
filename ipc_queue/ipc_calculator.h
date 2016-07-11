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
 
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/msg.h>

#include "utils.h"
#define STDIN 0
#define STDOUT 1


enum msg_ids { MSGID_FREECHILD = 1, MSGID_PARENT = 2, MSGID_OFFSET = 3};

struct msg_child
{
	long mtype;
	int id;
};

/**
* @brief A single operation to perform
*/
struct operation
{
	long mtype;
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
	long mtype;
	int id;
	float val;
}; 
 
int msg_queue;

struct operation* operations;
int *childs_pid; 
bool * free_child;
int id_number = -1;
int n_operations = -1;
int childs_count = 0;

void parent();
void child();

float* results;
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

 
 
 
 
 
