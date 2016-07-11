

#include "ipc_calculator.h"

int main(int argc, char *argv[])
{   
    char *title = "===================\n  IPC CALCULATOR  \n===================\n";
    int count = (int) write(STDOUT, title, strlen(title));
    if (count == -1)
        syserr (argv[0], "write() failure"); 
    
    int line_count = 0;
    
    // open file
    int fd = open("config.txt", O_RDONLY|O_SYNC, S_IRUSR);
    if (fd < 0) {
        syserr (argv[0], "open() failure");
    }
    
    char line[255];
    int i = 0;
    struct list* first_element = NULL;
    struct list* last_element = NULL;
    
    while ((count = (int) read(fd, &line[i], 1)) > 0) 
    {
        if(line[i] == '\n')
        {
            line[i] = '\0';
        
            line_count++;
            
            char * str_temp = (char*) malloc(sizeof(char)*i);
            strcpy(str_temp,  line);
            
            if (first_element == NULL)
            {
                first_element = list_create(str_temp);
                last_element = first_element;
            }
            else 
            	last_element =  list_add(str_temp, last_element);
                        
            i = 0;
        }
        else
            i++;
    }
    
    if (count == -1)
        syserr (argv[0], "write() failure");
    
    close(fd);
    
    if(first_element == NULL)
    {
         syserr (argv[0], "file is empty!");
    }
    
    n_operations = line_count - 1;
    
	// the first line is the process count
    childs_count = atoi(first_element->value); 
    
    // allocate memory for the operations and results
    operations = (struct operation*)malloc(sizeof(struct operation)*n_operations);
    results = (float*) malloc(sizeof(float) * n_operations);
    free_child = (bool*) malloc(sizeof(bool) * childs_count);
    
	// take the next list element as first operation
    struct list* list = first_element->next;
    i = 0;
    
    while (list != NULL)
    {
        char* id = strtok(list->value, " "); 
        //NULL must be used to get tokens from the previous string now
        char* val1 = strtok(NULL, " "); 
        char* op = strtok(NULL, " "); 
        char* val2  = strtok(NULL, " ");
        
        if(val1 == NULL || op == NULL || val2 == NULL)
     		syserr (argv[0], "Wrong operation format");
     		        
        operations[i].id = atoi(id) - 1;
        operations[i].val1 = atoi(val1);
        operations[i].val2 = atoi(val2);
        operations[i].operator = op[0];
        i++;
         
        list = list->next; 
    } 
     
    list_free(first_element);
    first_element = NULL;
    
 	// allocate memory for childs
    childs_pid = (int*) malloc(sizeof(int) * childs_count);      
   
  	//=======================
	// Queue creation
  	//=======================
  	int queue_key = ftok(argv[0], 'o');
	if((msg_queue = msgget (queue_key, S_IRUSR | S_IWUSR | IPC_CREAT | IPC_EXCL)) == -1)
		syserr_ext (argv[0], "msg queue create" , __LINE__);
 
 	 
  	//=======================
	// Generate childs
  	//=======================
    pid_t pid;
    for (i = 0; i < childs_count; i++)
    {
    	free_child[i] = true;
        pid = fork();
        if (pid < 0) 
        {
            syserr("fork()", "cannot create a new process");
        }
        else if (pid == 0)
        {
            id_number = i;
            child();
            break;
        }
        else
        {        
            childs_pid[i] = pid;
        }
    }
 
    if(pid != 0)
	{
        parent();
        
	  	//=========================
		// Free IPC
	  	//========================= 
     	msgctl(msg_queue, IPC_RMID, (struct msqid_ds *) NULL); 
	  	
	  	//=========================
		// Free memory
	  	//=========================
	  	
		free(childs_pid);  
		free(free_child);
	  	 
	  	//=========================
		// Write results
	  	//=========================

        fd = open("results.txt", O_CREAT|O_RDWR|O_TRUNC, S_IRUSR|S_IWUSR );
        if (fd < 0)
            syserr ("results.txt", "open() failure");

        // save in a file
        char res[20];
        for(i = 0; i < n_operations; i++)
        {
            sprintf(res, "%.2f\n", results[i]);
            count = (int) write(fd, res, strlen(res));
            if (count == -1)
                syserr ("results", "write() on file failure");
        }
        
        close(fd);
    }

    return 0;
}


struct msg_child mc;
struct operation temp_operation;
struct result temp_result;

int get_first_free_child()
{ 
    if ((msgrcv(msg_queue, &mc, sizeof(struct msg_child) - sizeof(long), MSGID_FREECHILD, 0)) == -1) 
		syserr_ext ("msgrcv", "MSGID_FREECHILD" , __LINE__);
    return mc.id;
}


// subroutine of parent
void read_child(int i)
{
	log_msg("[Parent] leggi da coda di messaggi"); 
    if ((msgrcv(msg_queue, &temp_result, sizeof(struct result) - sizeof(long), MSGID_PARENT, 0)) == -1) 
		syserr_ext ("msgrcv", "msg_queue" , __LINE__);
     
	printf("[Parent] recived %f: %d \n", temp_result.val, temp_result.id);
	results[temp_result.id] = temp_result.val;
}


//===========================
// Handle parent operations
//=========================== 
void parent()
{ 
    int i;
   
    int op_id;
    for(op_id = 0; op_id < n_operations; op_id++)
    {
		// preleva l'id del figlio da liberare
		log_msg("[Parent] ottiene l'id del figlio a cui inviare i dati");
    	int i = operations[op_id].id;
    	if(i == -1) 
    		i = get_first_free_child();
    	
		//se il figlio è occupato
		if(!free_child[i])
    		read_child(i);
		
		// inserisce i dati della prossima operazione 
		log_msg("[Parent] inserisce i dati della prossima operazione");
		
		temp_operation.mtype = MSGID_OFFSET + i; // 1 and 2 are taken
		temp_operation.id = op_id;
		temp_operation.val1 = operations[op_id].val1;
		temp_operation.val2 = operations[op_id].val2;
		temp_operation.operator = operations[op_id].operator;
		if ((msgsnd (msg_queue , &temp_operation, sizeof(struct operation) - sizeof(long), 0)) == -1)
  			syserr_ext ("msg_queue", "msgsnd" , __LINE__);
  			
		free_child[i] = false;
    }
        
    
	for(i = 0; i < childs_count; i++)
    { 
		if(!free_child[i])
			read_child(i);
			
		// termina processo 
		log_msg("[Parent] termina processo figlio"); 
		temp_operation.mtype = MSGID_OFFSET + i; // 1 and 2 are taken
		temp_operation.operator = 'k';
		if ((msgsnd (msg_queue , &temp_operation, sizeof(struct operation) - sizeof(long), 0)) == -1)
  			syserr_ext ("msg_queue", "msgsnd" , __LINE__); 
	}
	 
	for(i = 0; i < childs_count; i++)
    	wait(NULL);
    	
	// DEBUG: print the results on screen
	log_msg("----------------------\n Risultati\n------------------------");
    char res[255];
	for(i = 0; i < n_operations; i++)
    {
        sprintf(res, "%.2f\n", results[i]); 
        int count = (int) write(STDOUT, res, strlen(res));
        if (count == -1)
            syserr ("res", "write() failure"); 
    }
}

//===========================
// Handle child operations
//=========================== 
void child()
{
 
 	// operation result
    float res;
    
    while(true)
    {
		// send "child free" to the parent
		mc.mtype = MSGID_FREECHILD;
    	mc.id = id_number;
		log_msg_ext("[Child] dice al padre di essere libero", id_number);
		if ((msgsnd (msg_queue , &mc, sizeof(struct msg_child) - sizeof(long), 0)) == -1)
  			syserr_ext ("msg_queue", "msgsnd" , __LINE__); 
  			
        // legge i dati dalla coda di messaggi  
		log_msg_ext("[Child] legge i dati dalla coda di messaggi", id_number);
	 	if ((msgrcv(msg_queue, &temp_operation, sizeof(struct operation) - sizeof(long), MSGID_OFFSET + id_number, 0)) == -1) 
			syserr_ext ("msg_queue", "msgrcv" , __LINE__);
			 
        int val1 = temp_operation.val1;
        int val2 = temp_operation.val2;
        char op = temp_operation.operator; 
        int op_id = temp_operation.id;
 		printf("RECEIVED child %d: %d , %c , %d , %d \n", id_number, val1, op, val2, op_id);
        // termina col comando k
        if(op == 'k')
        {
			log_msg_ext("[Child] termina", id_number);
            exit(0);
        }
        
        // calcola
        res = process_operation(val1, val2, op);
         
        // scrivi risultato calcolo 
		log_msg_ext("[Child] scrive i risultati del calcolo", id_number);
        temp_result.mtype = MSGID_PARENT;
        temp_result.val = res;
        temp_result.id = op_id;
        
 		printf("[Child] chtemp_result %f: %d \n", res, op_id);
 		
		log_msg_ext("[Child] invia il risultato al padre", id_number);
		if ((msgsnd (msg_queue , &temp_result, sizeof(struct result) - sizeof(long), 0)) == -1)
  			syserr_ext ("msg_queue", "msgsnd" , __LINE__);  
    }
}



