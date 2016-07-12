/** @file utils.h
 *  @brief utils
 */
 
#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
 
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

#define COL_NRM  "\x1B[0m"
#define COL_RED  "\x1B[31m"
#define COL_GRN  "\x1B[32m"
#define COL_YEL  "\x1B[33m"
#define COL_BLU  "\x1B[34m"
#define COL_MAG  "\x1B[35m"
#define COL_CYN  "\x1B[36m"
#define COL_WHT  "\x1B[37m" 

/** 
* @brief Shared memory structure
*/
typedef struct XMem
{
	key_t key ;
	int shmid ;
	char buf [1];
} XMem ;

/** 
* @brief Shared memory allocation
* @param key the shared memory key.
* @param size amount of memory to allocate.
* @return a pointer to the shared memory. 
*/
void * xmalloc ( key_t key , const size_t size );

/** 
* @brief Free the memory allocated with xmalloc
* @param address the memory address returned by xmalloc
*/
void xfree(void * address);

/** 
* @brief prints an error and calls exit
* @param prog the file name
* @param msg the message
*/
void syserr(char *prog, char *msg);

/** 
* @brief prints an error and calls exit
* @param prog the file name
* @param msg the message
* @param line file line
*/
void syserr_ext(char *prog, char *msg, int line);

/** 
* @brief log a message
* @param str the string to print
*/
void log_msg(char *str);

/** 
* @brief log a message
* @param str the string to print
*/
void log_msg_ext(char *str, int i);


/**
* @brief simple string linked list implementation
*/
struct list
{
    char* value;
    struct list *next;
};
 
/**
* @brief creates a new string linked list
*/
struct list *list_create(char* value);

/**
* @brief adds an element at the end of the list_add
* @param value the string to store
* @param previous the previous element of the list
*/
struct list *list_add(char* value, struct list *previous);

/** @brief free the list memory
* @param first_element the pointer to the first element of the list 
*/
void list_free(struct list *first_element);
 
/** @brief Gets the result of related operations.
 * @param val1 operand 1.
 * @param op the operator
 * @param val2 operand 2
 * @return result of operations.
 */
float process_operation(int val1, int val2, char op);


#endif

