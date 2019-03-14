#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include "arrayStruct.h"
#include "semun.h"

// Declare semaphore functions
static int set_semvalue(int sem_id);
static void del_semvalue(int sem_id);
static int semaphore_p(int sem_id);
static int semaphore_v(int sem_id);
bool done = false;
void end(int sig){
    done = true;
}

// Array of semaphores we have 4 critical sections 
// 3 to protect critical sections in data and one to protect swap flag used to check if processes finished
static int sem_id[4];

int main(){

    struct sigaction act;

	act.sa_handler = end;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	sigaction(SIGINT, &act, 0);
 
	void *shared_memory = (void *)0;

	// Array data structure
	struct arrayStruct *array;

	// Shared memory ID
	int shmid;

	// Create shared memory
	srand((unsigned int)getpid());

	shmid = shmget((key_t)1234, sizeof(struct arrayStruct), 0666 | IPC_CREAT);

	if (shmid == -1) {
		fprintf(stderr, "shmget failed\n");
		exit(EXIT_FAILURE);
	}

	shared_memory = shmat(shmid, (void *)0, 0);

	if (shared_memory == (void *)-1) {
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);
	}

	array = (struct arrayStruct *)shared_memory;

    bool debug = false;

    // Ask user for 5 distinct numbers

    for (int i = 0; i < 5; i++){
        int n;
        printf("Please enter integer %d: ", i+1);
        scanf("%d", &n);
        (array->data)[i] = n;
    }

    bool valid = false;
    while (1){
        char c;
        fflush(stdin);
        printf("Run in debug mode? (Y/N)");
        scanf("%c", &c);
        if (c == 'Y' | c == 'y'){
            debug = true;
            valid = true;
            break;
        } else if (c == 'N' | c == 'n'){
            valid = true;
            break;
        } 
    }

    // Set flag to one as initialization
    for (int i = 0; i < 4; i++){
        array->swap[i] = 1;
    } 
    
    // Create 4 semaphores to protect 4 critical sections
    for (int i = 0; i < 3; i++){
        sem_id[i] = semget((key_t)1234, 1, 0666 | IPC_CREAT);
    }

    // Process IDs
    pid_t pid[4], wpid;

    //Create 4 child processes
	for (int i = 0; i < 4; i++){
		pid[i] = fork();
		if (pid[i] == 0){
			break;
		}
	}

    // Process P1
    if (pid[0] == 0){
        bool swap;
        for (;;){
            swap = false;

            //P1 needs first semaphore only
            if(!semaphore_p(sem_id[0]))
                exit(EXIT_FAILURE);

            // Sort numbers
            if (array->data[0] < array->data[1]){
                int temp = array->data[0];
                array->data[0] = array->data[1];
                array->data[1] = temp;
                swap = true;
            } 

            // Release first semaphore
            if(!semaphore_v(sem_id[0]))
                exit(EXIT_FAILURE);

            // Get 4th semaphore to set flags and check if done
            if(!semaphore_p(sem_id[3]))
                exit(EXIT_FAILURE);

            array->swap[0] = 0;

            if (swap){
                if (debug)
                    printf("Process P1: Swapped\n");
                array->swap[1] = 1;
            } else {
                if (debug)
                    printf("Process P1: No swap\n");
                int sum = 0; 
                for (int i = 0; i < 4; i++){
                    sum = sum + array->swap[i];
                }
                if (sum == 0){
                    kill(getppid(), SIGINT);
                    exit(EXIT_SUCCESS);
                }
            }

            // Release 4th semaphore
            if(!semaphore_v(sem_id[3]))
                exit(EXIT_FAILURE);
            
        }
        

    // Process P2    
    } else if (pid[1] == 0) {
        bool swap;
        for (;;){
            swap = false;

            //Process P2 needs the first and second sempaphore
            if(!semaphore_p(sem_id[0]))
                exit(EXIT_FAILURE);
            if(!semaphore_p(sem_id[1]))
                exit(EXIT_FAILURE);

            // Sort numbers
            if (array->data[1] < array->data[2]){
                int temp = array->data[1];
                array->data[1] = array->data[2];
                array->data[2] = temp;
                swap = true;
            } 

            // Release semaphores
            if(!semaphore_v(sem_id[1]))
                exit(EXIT_FAILURE);
            if(!semaphore_v(sem_id[0]))
                exit(EXIT_FAILURE);


            // Get 4th semaphore to set flags and check if done
            if(!semaphore_p(sem_id[3]))
                exit(EXIT_FAILURE);

            array->swap[1] = 0;
            if (swap){
                if (debug)
                    printf("Process P2: Swapped\n");
                array->swap[0] = 1;
                array->swap[2] = 1;
            } else { 
                if (debug)
                    printf("Process P2: No swap\n");
                int sum = 0; 
                for (int i = 0; i < 4; i++){
                    sum = sum + array->swap[i];
                }
                if (sum == 0){
                    kill(getppid(), SIGINT);
                    exit(EXIT_SUCCESS);
                }
            }

            // Release semaphore
            if(!semaphore_v(sem_id[3]))
                exit(EXIT_FAILURE);
        }

    // Process P3            
    } else if (pid[2] == 0){
        bool swap;
        for (;;){
            swap = false;
            // Process P3 needs the second and third semaphores
            if(!semaphore_p(sem_id[1]))
                    exit(EXIT_FAILURE);
            if(!semaphore_p(sem_id[2]))
                    exit(EXIT_FAILURE);

            // Sort numbers
            if (array->data[2] < array->data[3]){
                int temp = array->data[2];
                array->data[2] = array->data[3];
                array->data[3] = temp;
                swap = true;
            } 

            // Release sempahore
            if(!semaphore_v(sem_id[2]))
                exit(EXIT_FAILURE);
            if(!semaphore_v(sem_id[1]))
                exit(EXIT_FAILURE);

            // Get 4th semaphore to set flags and check if done
            if(!semaphore_p(sem_id[3]))
                exit(EXIT_FAILURE);

            array->swap[2] = 0;
            if (swap){
                if (debug)
                    printf("Process P3: Swapped\n");
                array->swap[1] = 1;
                array->swap[3] = 1;
            } else { 
                if (debug)
                    printf("Process P3: No swap\n");
                int sum = 0; 
                for (int i = 0; i < 4; i++){
                    sum = sum + array->swap[i];
                }
                if (sum == 0){
                    kill(getppid(), SIGINT);
                    exit(EXIT_SUCCESS);
                }
            }

            if(!semaphore_v(sem_id[3]))
                exit(EXIT_FAILURE);
        }

    // Process P4
    } else if (pid[3] == 0){
        bool swap;
        for (;;){
            swap = false;
            if(!semaphore_p(sem_id[2]))
                exit(EXIT_FAILURE);

            // Sort numbers
            if (array->data[3] < array->data[4]){
                int temp = array->data[3];
                array->data[3] = array->data[4];
                array->data[4] = temp;
                swap = true;
            } 

            if(!semaphore_v(sem_id[2]))
                exit(EXIT_FAILURE);

            // Get 4th semaphore to set flags and check if done
            if(!semaphore_p(sem_id[3]))
                exit(EXIT_FAILURE);

            array->swap[3] = 0;
            if (swap){
                if (debug)
                    printf("Process P4: Swapped\n");
                array->swap[2] = 1;
            } else {
                if (debug)
                    printf("Process P4: No swap\n");
                int sum = 0; 
                for (int i = 0; i < 4; i++){
                    sum = sum + array->swap[i];
                }
                if (sum == 0){
                    kill(getppid(), SIGINT);
                    exit(EXIT_SUCCESS);
                }
            }
            if(!semaphore_v(sem_id[3]))
                exit(EXIT_FAILURE);
        }

    }

    //Initialize semaphores
    for (int i = 0; i < 4; i++){
        if (!set_semvalue(sem_id[i])) {
            fprintf(stderr, "Failed to initialize semaphore\n");
            exit(EXIT_FAILURE);
        }
    }

    (void) signal(SIGINT, end);
    pause();
    if(done){
        //kill all children
        printf("Array[0]: %d\n", array->data[0]);
        printf("Array[1]: %d\n", array->data[1]);
        printf("Array[2]: %d\n", array->data[2]);
        printf("Array[3]: %d\n", array->data[3]);
        printf("Array[4]: %d\n", array->data[4]);

    exit(EXIT_SUCCESS);
    } 

}

// Initialize the semaphore to 1
static int set_semvalue(int sem_id) {
    union semun sem_union;
    sem_union.val = 1;

    if (semctl(sem_id, 0, SETVAL, sem_union) == -1) 
        return(0);
    
    return(1);
}

static void del_semvalue(int sem_id) {
    union semun sem_union;

    if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
        fprintf(stderr, "Failed to delete semaphore\n");
}

static int semaphore_p(int sem_id) {
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = -1; /* P() */
    sem_b.sem_flg = SEM_UNDO;

    if (semop(sem_id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_p failed\n");
        return(0);
    }

    return(1);
}

static int semaphore_v(int sem_id){
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = 1; /* V() */
    sem_b.sem_flg = SEM_UNDO;
    
    if (semop(sem_id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_v failed\n");
        return(0);
    }

    return(1);
}


    