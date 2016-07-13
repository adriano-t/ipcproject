

#include "ipc_calculator.h"
 

int main(int argc, char *argv[])
{   
    char *title = COL_YEL"\n=======================\n   IPC CALCULATOR  \n=======================\n"COL_NRM;
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
            
            char * str_temp = (char*) malloc(sizeof(char) * i);
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
    threads_count = atoi(first_element->value); 
    
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

    threads = (pthread_t*) malloc(sizeof (pthread_t) * threads_count);       // allocate memory for threads
     
    /* Initialize and set thread detached attribute */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
   	free_threads = (bool*) malloc(sizeof (bool) * threads_count);	 
    for(i = 0; i < threads_count; i++)
    	free_threads[i] = true;
    
    compute();
     
  	//=========================
	// Free memory and Thread attributes
  	//=========================
  	pthread_attr_destroy(&attr); 
       
	free(threads);  
	free(free_threads);
	 
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
     
    return 0;
}

int get_first_free_thread()
{
    int i;
	for(i = 0; i < threads_count; i++) 
    	if(free_threads[i])
    		return i; 
    return 0;
}
 int rc;
//===========================
// Handle operations
//=========================== 
void compute()
{
 	void *status;	
    int op_id;
    for(op_id = 0; op_id < n_operations; op_id++)
    {
		// preleva l'id del figlio da liberare
		log_msg("[Parent] preleva l'id dell'operazione");
    	int i = operations[op_id].id;
    	if(i == -1) 
    		i = get_first_free_thread();
    	
		//se il thread è occupato lo attende
		if(!free_threads[i])
		{
			
			log_msg("[Main] joining thread");
			rc = pthread_join(threads[i], &status);
			log_msg("[Main] joined!");
			if (rc)
			{
				printf("ERROR; return code from pthread_join() is %d\n", rc);
				exit(-1);
	      	}
		}
		
		
		log_msg("[Main] fa partire il thread");
		info = (struct thread_info*) malloc(sizeof(struct thread_info));
		info->id = i;
		info->operation = op_id;
		rc = pthread_create(&threads[i], &attr, execute_operation, (void *)info);  
		if (rc) 
		{
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
	  	}
		
    }
        
    //attende che tutti i thread abbiano finito
    int i;
	for(i = 0; i < threads_count; i++)
    {  
		if(!free_threads[i])
		{
			log_msg("[Main] joining thread");
			rc = pthread_join(threads[i], &status);
			log_msg("[Main] joined!");
			if (rc)
			{
				printf("ERROR; return code from pthread_join() is %d\n", rc);
				exit(-1);
	      	}
		}
	} 
	log_msg("[Main] complete!");
	 
	log_msg(COL_YEL"========================\n Risultati\n========================"COL_NRM);
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
void* execute_operation(void *t_info)
{
	struct thread_info* info = (struct thread_info*)t_info;
	free_threads[info->id] = false;
	log_msg_ext("[Thread] esegue il calcolo", info->id);
	int op_id = info->operation;
	results[op_id] = process_operation(operations[op_id].val1, operations[op_id].val2, operations[op_id].operator);
	log_msg_ext("[Thread] termina", info->id); 
	free(info);
	pthread_exit(NULL);
}



