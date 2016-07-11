

#include "ipc_calculator.h"

void sem_v(int semid, int num)
{
	sops.sem_op = 1;
	sops.sem_num = num;
	if( semop (semid, &sops , 1) == -1)
		syserr("semaphore", "V");
}

void sem_p(int semid, int num)
{
	sops.sem_op = -1;
	sops.sem_num = num;
	if( semop (semid, &sops , 1) == -1)
		syserr("semaphore", "P");
}

  	
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

    childs_pid = (int*) malloc(sizeof (int) * childs_count);       // allocate memory for childs
  
  
  	//=======================
	// Semaphores
  	//=======================
  	
  	
  	//------------
	// Childs
  	//------------
  	// allocate a zero-initialized array for the semaphores
  	unsigned short* sem_child_init_values = (unsigned short*) calloc(childs_count, sizeof(unsigned short));
  	
    union semun arg;
	arg.array = sem_child_init_values;
    
    // semafori di sincronizzazione: indica se il figlio i-esimo sta ancora facendo il calcolo
    if (( sem_computing = semget (ftok(argv[0], 'a') , childs_count, IPC_CREAT | 0777)) == -1) 
		syserr_ext (argv[0], " semget " , __LINE__); 
	if (semctl(sem_computing, childs_count, SETALL, arg) == -1) 
		syserr_ext (argv[0], " semctl " , __LINE__); 
	
    // semafori di sincronizzazione: dice se il figlio i-esimo si deve mettere in attesa 
    // per ricevere i dati relativi al calcolo
    if (( sem_wait_data = semget (ftok(argv[0], 'b') , childs_count, IPC_CREAT | 0777)) == -1) 
		syserr_ext (argv[0], " semget " , __LINE__);
	if (semctl(sem_wait_data, childs_count, SETALL, arg) == -1) 
		syserr_ext (argv[0], " semctl " , __LINE__); 
	
    // semafori di sincronizzazione: dice se il padre deve mettersi in attesa del figlio i-esimo
    // per ricevere il risultato dell'ultimo calcolo eseguito
    if (( sem_request_result = semget (ftok(argv[0], 'c') , childs_count, IPC_CREAT  | 0777)) == -1)  
		syserr_ext (argv[0], " semget " , __LINE__);
	if (semctl(sem_request_result, childs_count, SETALL, arg) == -1) 
		syserr_ext (argv[0], " semctl " , __LINE__); 
	
	free(sem_child_init_values);
	
	
  	//------------
	// Parent
  	//------------
  	
	// 0: mutex		1: result_ready		2: data_read
	if (( sem_parent = semget (ftok(argv[0], 'd') , 3, IPC_CREAT | 0777)) == -1) {
		syserr_ext (argv[0], " semget " , __LINE__);
	}

    unsigned short sem_parent_init_values[3] = {1, 0, 0};
	arg.array = sem_parent_init_values; 
	semctl(sem_parent, 3, SETALL, arg);
  
  	
  	//=======================
	// Pipe
  	//=======================
  	
  	if(pipe(pipe_to_parent) == -1)
  		syserr_ext (argv[0], "parent pipe creation" , __LINE__);
  		
  	if(pipe(pipe_to_child) == -1)
  		syserr_ext (argv[0], "child pipe creation" , __LINE__);
  	

    free_child = (bool*) malloc(sizeof (bool) * childs_count);	 
    for(i = 0; i < childs_count; i++)
    	free_child[i] = true;
    
  	//=======================
	// Generate childs
  	//=======================
    pid_t pid;
    for (i = 0; i < childs_count; i++)
    {
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
		// Free memory and IPCs
	  	//=========================
	  	
		free(childs_pid);  
		free(free_child);
	
        // delete semaphores
		if (semctl(sem_computing, 0, IPC_RMID, NULL) == -1) 
			syserr(argv[0], "Error deleting sem_computing!"); 
		if (semctl(sem_wait_data, 0, IPC_RMID, NULL) == -1) 
			syserr(argv[0], "Error deleting sem_wait_data!"); 
		if (semctl(sem_request_result, 0, IPC_RMID, NULL) == -1) 
			syserr(argv[0], "Error deleting sem_request_result!"); 
		if (semctl(sem_parent, 0, IPC_RMID, NULL) == -1) 
			syserr(argv[0], "Error deleting sem_parent!");

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

int get_first_free_child()
{
    int i;
	for(i = 0; i < childs_count; i++) 
    	if(free_child[i])
    		return i; 
    return 0;
}


struct operation temp_operation;
struct result temp_result;

// subroutine of parent
void wait_child(int i)
{
	
	// attende che abbia finito il calcolo 
	log_msg("[Parent] Attende che il figlio abbia finito il calcolo");
	sem_p(sem_computing, i);
	
	// richiede eventuali dati precedenti 
	log_msg("[Parent] richiede eventuali dati precedenti");
	sem_v(sem_request_result, i);
	
	//aspetta che i dati siano pronti da leggere 
	log_msg("[Parent] aspetta che i dati siano pronti da leggere");
	sem_p(sem_parent, RESULT_READY);
	
	log_msg("[Parent] leggi da pipe");
	if (read (pipe_to_parent [0] , &temp_result, sizeof(struct result) ) == -1)
		syserr_ext ("pipe", "read" , __LINE__);
	results[temp_result.id] = temp_result.val;
	
}



//===========================
// Handle parent operations
//=========================== 
void parent()
{

	//===========================
	// Pipe handling
	//=========================== 
	// close read
	if(close (pipe_to_child[0]) == -1) 
  		syserr_ext ("pipe", "close" , __LINE__);
  	// close write
	if(close (pipe_to_parent[1]) == -1) 
  		syserr_ext ("pipe", "close" , __LINE__);
  		
	// wait the last child
	log_msg("[Parent] aspetta l'ultimo figlio");
	char r[1];
 	for(int i = 0; i < childs_count; i++) 
		if (read (pipe_to_parent [0] , r, 1) == -1)
			syserr_ext ("pipe", "read" , __LINE__);
			
    int op_id;
    for(op_id = 0; op_id < n_operations; op_id++)
    {
		// preleva l'id del figlio da liberare
		log_msg("[Parent] preleva l'id del figlio da liberare");
    	int i = operations[op_id].id;
    	if(i == -1) 
    		i = get_first_free_child();
    	
		//se il figlio è occupato
    	if(!free_child[i])
			wait_child(i);
		
		// inserisce i dati della prossima operazione 
		log_msg("[Parent] inserisce i dati della prossima operazione");
		
		temp_operation.id = op_id;
		temp_operation.val1 = operations[op_id].val1;
		temp_operation.val2 = operations[op_id].val2;
		temp_operation.operator = operations[op_id].operator;
		if (write (pipe_to_child [1] , &temp_operation, sizeof(struct operation) ) == -1)
  			syserr_ext ("pipe", "write" , __LINE__);
				
		
		free_child[i] = false;
		
		// libera il figlio bloccato per fargli eseguire un operazione
		log_msg("[Parent] libera il figlio bloccato per fargli eseguire un operazione");
		sem_v(sem_wait_data, i);
		
		// aspetta che il figlio li abbia letti 
		log_msg("[Parent] aspetta che il figlio legga i dati");
		sem_p(sem_parent, DATA_READ);
    }
        
    
    int i;
	for(i = 0; i < childs_count; i++)
    {
		if(!free_child[i])
			wait_child(i);
			
		// termina processo 
		log_msg("[Parent] termina processo");
		 
		temp_operation.operator = 'k';
		if (write (pipe_to_child [1] , &temp_operation, sizeof(struct operation) ) == -1)
  			syserr_ext ("pipe", "write" , __LINE__);
				
		// libera il figlio bloccato
		log_msg("[Parent] libera il figlio bloccato");
		sem_v(sem_wait_data, i);
		
		// aspetta che il figlio abbia letto
		log_msg("[Parent] aspetta che il figlio abbia letto");
		sem_p(sem_parent, DATA_READ);
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
	//===========================
	// Pipe handling
	//=========================== 
	// close read
	if(close (pipe_to_parent[0]) == -1)
  		syserr_ext ("pipe", "close" , __LINE__);
  	// close write
	if(close (pipe_to_child[1]) == -1)
  		syserr_ext ("pipe", "close" , __LINE__);
  		
  		
 	// operation result
    float res;
     
    // send "child ready" to the parent 
  	if (write (pipe_to_parent [1] , "1", 1) == -1)
		syserr_ext ("pipe", "write" , __LINE__); 
    
    while(true)
    {
        // si mette in attesa di essere chiamato per il calcolo
		log_msg_ext("[Child] si mette in attesa di essere chiamato per il calcolo", id_number);
        sem_p(sem_wait_data, id_number);
                
        // legge i dati dalla pipe
		log_msg_ext("[Child] leggi operazione da pipe", id_number);
		if (read (pipe_to_child [0] , &temp_operation, sizeof(struct operation) ) == -1)
			syserr_ext ("pipe", "read" , __LINE__);
			
		results[temp_result.id] = temp_result.val;
        int val1 = temp_operation.val1;
        int val2 = temp_operation.val2;
        char op = temp_operation.operator; 
        int op_id = temp_operation.id;
        
        // avvisa che ho finito di leggere 
		log_msg_ext("[Child] avvisa che ho finito di leggere  i dati", id_number);
        sem_v(sem_parent, DATA_READ);
        
        
        // termina col comando k
        if(op == 'k')
        {
			log_msg_ext("[Child] termina", id_number);
            exit(0);
        }
        
        // calcola
        res = process_operation(val1, val2, op);
        
        // avvisa di aver terminato il calcolo
		log_msg_ext("[Child] calcolo terminato", id_number);
        sem_v(sem_computing, id_number);
        
        // attende che il padre richieda i dati
		log_msg_ext("[Child] attende che il padre richieda i dati", id_number);
        sem_p(sem_request_result, id_number);
        
        // scrivi risultato calcolo 
		log_msg_ext("[Child] scrive i risultati del calcolo", id_number);
        temp_result.val = res;
        temp_result.id = op_id;
		if (write (pipe_to_parent [1] , &temp_result, sizeof(struct result) ) == -1)
  			syserr_ext ("pipe", "write" , __LINE__);
        
        // dice al padre che i dati sono pronti per essere letti
		log_msg_ext("[Child] avvisa che i dati sono pronti per essere letti", id_number);
        sem_v(sem_parent, RESULT_READY);
    }
}



