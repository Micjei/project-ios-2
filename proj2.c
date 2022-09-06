#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/sem.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

#define MMAP(pointer){(pointer) = mmap(NULL, sizeof(*(pointer)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);}
#define UNMAP(pointer){munmap((pointer), sizeof((pointer)));}

sem_t *mutex;
sem_t *mutex2;
sem_t *hydrogen_queue;
sem_t *oxygen_queue;
sem_t *line_sem;
sem_t *turnstile;
sem_t *turnstile2;


FILE *pfile;
int *oxygen = NULL;
int *hydrogen = NULL;
int *line = NULL;
int *molecule = NULL;
int *count = NULL;
int *process_o = NULL;
int *process_h = NULL;
bool *end = false;
int *remai_ox = NULL;
int *remai_hyd = NULL;

void semaphore_init(){
    pfile = fopen("proj2.out", "w");
    MMAP(hydrogen); 
    MMAP(oxygen); 
    MMAP(line);
    MMAP(molecule);
    MMAP(count);
    MMAP(process_o);
    MMAP(process_h);
    MMAP(end);	
    MMAP(remai_ox);
    MMAP(remai_hyd);		
       
    	
    if ((hydrogen_queue = sem_open("/xbabus01.ios.proj2.hydrogen_queue", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED){
        fprintf(stderr, "Semaphore ERROR2.\n");
        exit(EXIT_FAILURE);
    }
    if ((oxygen_queue = sem_open("/xbabus01.ios.proj2.oxygen_queue", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED){
        fprintf(stderr, "Semaphore ERROR3.\n");
        exit(EXIT_FAILURE);
    }
    if ((mutex = sem_open("/xbabus01.ios.proj2.mutex", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED){
        fprintf(stderr, "Semaphore ERROR4.\n");
        exit(EXIT_FAILURE);
    }
    if ((mutex2 = sem_open("/xbabus01.ios.proj2.mutex2", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED){
        fprintf(stderr, "Semaphore ERROR5.\n");
        exit(EXIT_FAILURE);
    }
    if ((line_sem = sem_open("/xbabus01.ios.proj2.line_sem", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED){
        fprintf(stderr, "Semaphore ERROR5.\n");
        exit(EXIT_FAILURE);
    }
    if ((turnstile = sem_open("/xbabus01.ios.proj2.turnstile", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED){
        fprintf(stderr, "Semaphore ERROR5.\n");
        exit(EXIT_FAILURE);
    }    
    if ((turnstile2 = sem_open("/xbabus01.ios.proj2.turnstile2", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED){
        fprintf(stderr, "Semaphore ERROR5.\n");
        exit(EXIT_FAILURE);
    }	
}

void cleanup(){
    UNMAP(hydrogen);
    UNMAP(oxygen);
    UNMAP(line);
    UNMAP(molecule);
    UNMAP(count);
    UNMAP(process_o);
    UNMAP(process_h);
    UNMAP(end);
    UNMAP(remai_ox);
    UNMAP(remai_hyd); 
   
    sem_unlink("/xbabus01.ios.proj2.hydrogen_queue");
    sem_unlink("/xbabus01.ios.proj2.oxygen_queue");
    sem_unlink("/xbabus01.ios.proj2.mutex");
    sem_unlink("/xbabus01.ios.proj2.mutex2");
    sem_unlink("/xbabus01.ios.proj2.line_sem");	    
    sem_unlink("/xbabus01.ios.proj2.turnstile"); 
    sem_unlink("/xbabus01.ios.proj2.turnstile2");   

    sem_close(hydrogen_queue);
    sem_close(oxygen_queue);
    sem_close(mutex);
    sem_close(mutex2);
    sem_close(line_sem);
    sem_close(turnstile);
    sem_close(turnstile2);   
 
    if (pfile != NULL) fclose(pfile);		
}

void oxygen_func(int id, int TI, int NO, int NH, int TB){     
    sem_wait(line_sem);
    *line += 1;
    fprintf(pfile,"%d: O %d: started\n", *line, id);
    sem_post(line_sem);
    
    
    if (TI > 0){
        usleep(1000 * (rand() % (TI + 1)));
    }
    else{
        usleep(0);
    }
    sem_wait(mutex);
    *process_o += 1;

    sem_wait(line_sem);    	
    *line += 1;
    fprintf(pfile, "%d: O %d: going to queue\n", *line, id);
    sem_post(line_sem);

    *oxygen += 1;
    if ((*remai_hyd == 1) && (*remai_ox == 1)){
	sem_wait(line_sem);
	*line += 1;
	fprintf(pfile, "%d: O %d: not enough H\n", *line, id);
	sem_post(line_sem);
	sem_post(mutex); 
        exit(0);
    }     
    if (*hydrogen >= 2){
	*molecule += 1;
        sem_post(hydrogen_queue);
        sem_post(hydrogen_queue);
        *hydrogen -= 2;
        sem_post(oxygen_queue);
        *oxygen -= 1;
    }
    else {
        sem_post(mutex);
    }
    if (*process_h == NH && NO > NH/2){
        if (*remai_ox > 0 && *remai_hyd < 2){
	    *end = true;
	    sem_post(oxygen_queue);
	    sem_post(hydrogen_queue);
	    sem_wait(line_sem);
	    *line += 1;
	    fprintf(pfile, "%d: O %d: not enough H\n", *line, id);
	    sem_post(line_sem);
	    sem_post(mutex);
	    exit(0);    		
	}
    }
    if (*remai_hyd == 0 && *remai_ox == 1){
        sem_wait(line_sem);
        *line += 1;
        fprintf(pfile, "%d: O %d: not enough H\n", *line, id);
        sem_post(line_sem);
    }
    sem_wait(oxygen_queue);
	
    if (*end == true){
	sem_post(oxygen_queue);
	sem_wait(line_sem);
	*line += 1;
	fprintf(pfile, "%d: O %d: not enough H\n", *line, id);
	sem_post(line_sem);
	sem_post(mutex);
	exit(0); 
    }
	
    sem_wait(line_sem);
    *line += 1;
    fprintf(pfile,"%d: O %d: Creating molecule %d\n", *line, id, *molecule);
    *remai_ox -= 1;
    sem_post(line_sem);
 
    sem_wait(mutex2);
    *count += 1;
    if (*count == 3){
        if(TB > 0){
 	    usleep(1000 * (rand() % TB));
        }
	else{
	    usleep(0);
	}	
	sem_wait(turnstile2);
        sem_post(turnstile);
    }
    sem_post(mutex2);
    sem_wait(turnstile);
    sem_post(turnstile);

    sem_wait(mutex2);
    *count -= 1;
    if (*count == 0){
        sem_wait(turnstile);
        sem_post(turnstile2);
    }
    sem_post(mutex2);

    sem_wait(turnstile2);
    sem_post(turnstile2);

    sem_wait(line_sem);
    *line += 1;
    fprintf(pfile, "%d: O %d: molecule %d created\n", *line, id, *molecule);
    sem_post(line_sem);

    if (*process_h == NH && NO > NH/2){
        if (*oxygen > 0 && *hydrogen < 2){
            *end = true;
            sem_post(oxygen_queue);
        }
    }
    if (*process_o == NO){
        if (*oxygen == 0 && *hydrogen >= 0){
            *end = true;
            sem_post(hydrogen_queue);
        }
    }
    sem_post(mutex);

}
       
 
void hydrogen_func(int id, int TI, int NO, int NH, int TB){ 
    sem_wait(line_sem);
    *line += 1;
    fprintf(pfile, "%d: H %d: started\n", *line, id);
    sem_post(line_sem);
    
    if (TI > 0){
        usleep(1000 * (rand() % (TI + 1)));
    }
    else{
        usleep(0);
    }
    sem_wait(mutex);
    process_h += 1;	

    sem_wait(line_sem);
    *line += 1;
    fprintf(pfile, "%d: H %d: going to queue\n", *line, id);
    sem_post(line_sem);

    *hydrogen += 1;
    if ((*remai_hyd == 1) && (*remai_ox == 1)){
        sem_wait(line_sem);
        *line += 1;
        fprintf(pfile, "%d: H %d: not enough O or H\n", *line, id);
        sem_post(line_sem);
        sem_post(mutex);
    	exit (0);
    }
    if (*hydrogen >= 2 && *oxygen >= 1){
        *molecule += 1;
	sem_post(hydrogen_queue);
        sem_post(hydrogen_queue);
        *hydrogen -= 2;
        sem_post(oxygen_queue);
        *oxygen -= 1;
    }
    else {
        sem_post(mutex);
    }
    if (*process_h == NO && NO > NH/2){
	if ((*remai_ox == 0 && *remai_hyd > 0) || (*remai_ox == 1 && *remai_hyd == 1)){
	    *end = true;
	    sem_post(oxygen_queue);
	    sem_post(hydrogen_queue);
	    sem_wait(line_sem);
	    *line += 1;
	    fprintf(pfile, "%d: H %d: not enough O or H\n", *line, id);
	    sem_post(line_sem);
	    sem_post(mutex);
	    exit (0);   
	}
    }
    if (*remai_hyd >= 1 && *remai_ox == 0){
	sem_wait(line_sem);
        *line += 1;
        fprintf(pfile, "%d: H %d: not enough O or H\n", *line, id);
        sem_post(line_sem);
     }  
    sem_wait(hydrogen_queue);

    if (*end == true){
	sem_post(hydrogen_queue);
	sem_wait(line_sem);
	*line += 1;
	fprintf(pfile, "%d: H %d: not enough O or H\n", *line, id);
	sem_post(line_sem);
	sem_post(mutex);
	exit(0);
    }

    sem_wait(line_sem);	
    *line += 1;
    fprintf(pfile,"%d: H %d: Creating molecule %d\n", *line, id, *molecule);
    *remai_hyd -= 1;
    sem_post(line_sem);
    
    sem_wait(mutex2);
    *count += 1;
    if (*count == 3){
        if (TB > 0){
	    usleep(1000 * (rand() % TB));
	}
	else{
	    usleep(0);
	}
	sem_wait(turnstile2);
        sem_post(turnstile);
    }
    sem_post(mutex2);
    sem_wait(turnstile);
    sem_post(turnstile);

    sem_wait(mutex2);
    *count -= 1;
    if (*count == 0){
        sem_wait(turnstile);
        sem_post(turnstile2);
    }
    sem_post(mutex2);

    sem_wait(turnstile2);
    sem_post(turnstile2);      
    
    sem_wait(line_sem);
    *line += 1;
    fprintf(pfile, "%d: H %d: molecule %d created\n", *line, id, *molecule);
    sem_post(line_sem);	
}

void validation_of_arguments(char argument[]){
    int lenght = strlen(argument);
    for (int j = 0; j < lenght; j++){
        if ((argument[j] < '0') || (argument[j] > '9')){
            fprintf(stderr, "arguments ERROR.\n");
            cleanup();
            exit(1);
	}
    }	
}  
int main(int argc, char **argv) {
    if ((pfile = fopen("proj2.out","w")) == NULL){
        fprintf(stderr, "ERROR.\n");
	cleanup();
        exit(1);
    }
    if (argc != 5){
        fprintf(stderr, "Invalid number of arguments\n" );
        cleanup();
	exit(1);
    }
    int NO = atoi(argv[1]);
    int NH = atoi(argv[2]);
    int TI = atoi(argv[3]);
    int TB = atoi(argv[4]);
    

    for(int i = 1; i < argc; i++){
        validation_of_arguments(argv[i]);
    } 

    if (TI > 1000 || TI < 0){
        fprintf(stderr, "%d is invalid value of TI\n", TI);
        cleanup();
	exit(1);
    }

    if (TB > 1000 || TB < 0){
        fprintf(stderr, "%d is invalid value of TB\n", TB);
        cleanup();
	exit(1);
    }

    if (NO == 0 && NH  == 0){
        fprintf(stderr,"Zero O and H\n");
	cleaup();
	exit(1);
    }
    semaphore_init();
    setbuf(pfile, NULL);	
  
    *remai_ox = NO;
    *remai_hyd = NH;	  
    for(int id = 1; id <= NO; ++id) {
        pid_t pid = fork();
        if (pid < 0) {
            //chyba
            fprintf(stderr,"ERROR.\n");
            exit(1);
        }
        if (pid == 0) {
            //child process
            oxygen_func(id, TI, NO, NH, TB);
	                
	    	    
	    exit(0);
        }
    }

    for(int id = 1; id <= NH; ++id) {
        pid_t pid = fork();
        if (pid < 0) {
            //chyba
            fprintf(stderr,"ERROR.\n");
            exit(1);
        }
        if (pid == 0) {
            //child process
	    hydrogen_func(id, TI, NO, NH, TB);
	    
            exit(0);
        }
    }

    if (*hydrogen > 0 || *oxygen > 0){
        while(wait(NULL) > 0);
    }
    cleanup();
    exit(0);
}