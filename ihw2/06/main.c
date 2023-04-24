#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define N 6

#define SEM_NAME "/pretty_student_sem"
#define SHM_NAME "/pretty_student_mem"

typedef struct {
    int valentine_type_num[N];
    int lucky;
} Memory;

struct sembuf block = {0, -1, SEM_UNDO};
struct sembuf dismiss = {0, 1, SEM_UNDO};


int main(int argc, char *argv[]) {
	pid_t children[N];
    sem_t *sem;
    int shm_fd;
    
    Memory *memory;

    
	if ((sem = semget(57, 1, IPC_CREAT | 0666)) == -1) {
	  perror("Ошибка semget");
	  exit(1);
	}
	
	if (semctl(sem, 0, SETVAL, 1) == -1) {
	  perror("Ошибка semctl");
	  exit(1);
	}
	
	if ((shm_fd = shmget(57, sizeof(Memory), IPC_CREAT | 0666)) == -1) {
	  perror("Ошибка shmget");
	  exit(1);
	}
	if ((memory = shmat(shm_fd, 0, 0)) == (Memory *) -1) {
	  perror("Ошибка shmat");
	  exit(1);
	}
	
	char* varirants[N] = {
		"Ресторан",
		"Кофе",
		"Кино",
		"Настольная игра",
		"Прогулка",
		"Поездка",
	};
	
	char* names[N] = {
		"Иван",
		"Василий",
		"Антон",
		"Семён",
		"Михаил",
		"Фёдор",
	};
	
	int step = rand() % N;
    memory->lucky = -1;

    for (int i = 0; i < N; i++) {
    	memory->valentine_type_num[i] = (i + step) % N;
        children[i] = fork();
        if (children[i] == -1) {
            perror("fork");
            exit(1);
        } else if (children[i] == 0) {
        	semop(sem, &block, 1);
		    printf("%s: %s\n", names[i], varirants[memory->valentine_type_num[i]]);
		    semop(sem, &dismiss, 1);
		    sleep(4);
		    exit(0);
    	}
	}

	for (int i = 0; i < N; i++) {
		waitpid(children[i], NULL, 0);
	}
	
	
	memory->lucky = rand() % N;
	printf("%s идёт на свидание!\n", names[memory->lucky]);
	
	return 0;
}

