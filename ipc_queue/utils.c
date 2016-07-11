#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "utils.h"

float process_operation(int val1, int val2, char op)
{
    switch (op) 
    {
        case '+':
            return (float)(val1 + val2);
            break;
        case '-':
            return (float)(val1 - val2);
            break;
        case '*':
            return (float)(val1 * val2);
            break;
        case '/':
            return (float)val1 / val2;
            break;
        default:
            printf("%i %c %i", val1, op, val2);
            syserr("process operation: ", "invalid operator");
            break;
    }
    return -1;
}

void * xmalloc (key_t key, const size_t size)
{
	const int shmid = shmget ( key , size + sizeof ( XMem ) - sizeof ( char ) , 0666| IPC_CREAT );
	if ( shmid == -1)
         syserr ("xmalloc", "shmget");
         
	XMem * ret = ( XMem *) shmat ( shmid , NULL , 0);
	if ( ret == ( void *) -1) 
         syserr ("xmalloc", "shmat");
         
	ret->key = key ;
	ret->shmid = shmid ;
	return ret-> buf ;
}

void xfree (void * ptr)
{
	XMem tmp ;
	XMem * mem = ( XMem *)((( char *) ptr ) - ((( char *)& tmp.buf ) - (( char *)& tmp.key )));
	const int shmid = mem-> shmid ;
	shmdt ( mem );
	shmctl ( shmid , IPC_RMID , NULL );
}
 
struct list *list_create(char* value)
{
    struct list *new = (struct list*) malloc(sizeof(struct list));
    new->value = value;
    new->next = NULL;
    return new;
}

struct list *list_add(char* value, struct list *previous)
{
	if(previous == NULL) 
         syserr ("list_add", "previous list element is NULL!");
	
    struct list *new = (struct list*) malloc(sizeof(struct list));
    new->value = value;
    new->next = NULL;
    previous->next = new;
    return new;
}
 
void list_free(struct list *this)
{
    if (this != NULL) 
    { 
        list_free(this->next);
        free(this->value);
        free(this);
    }
}

char err_str[255];
void syserr(char *prog, char *msg)
{
	if(prog != NULL && msg != NULL && strlen(prog) + strlen(msg) < 200)
	{
		int len = sprintf(err_str, "ERROR:  %s | %s\n", prog, msg);
		write(fileno(stderr), err_str, len);
    }
    exit(1);
}


void syserr_ext(char *prog, char *msg, int line)
{
	if(prog != NULL && msg != NULL && strlen(prog) + strlen(msg) < 200)
	{
		int len = sprintf(err_str,  "ERROR: %s | line: %d | %s\n", prog, line, msg);
		write(fileno(stderr), err_str, len);
	}
    exit(1);
}

void log_msg(char *str)
{
	if(str != NULL && strlen(str) < 200)
	{
		char log_str[255];
		int len = sprintf(log_str,  "%s\n", str);
		write(fileno(stdout), log_str, len);
	}
}

void log_msg_ext(char *str, int i)
{
	if(str != NULL && strlen(str) < 200)
	{
		char log_str[255];
		int len = sprintf(log_str, "[%d] %s\n", i, str);
		write(fileno(stdout), log_str, len);
	}
}





