#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>

#define N 6

#define SEM_NAME "/pretty_student_sem"
#define SHM_NAME "/pretty_student_mem"

typedef struct {
    int valentine_type_num[N];
    int lucky;
} Memory;


int main(int argc, char *argv[]) {
	pid_t children[N];
    sem_t *sem;
    int shm_fd;
    
    Memory *memory;

    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0644);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }

    if (ftruncate(shm_fd, sizeof(Memory)) == -1) {
        perror("ftruncate");
        exit(1);
    }

    memory = (Memory *) mmap(NULL, sizeof(Memory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (memory == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sem == MAP_FAILED) {
        perror("sem mmap");
        exit(1);
    }
    if (sem_init(sem, 1, 0) == -1) {
        perror("sem_init");
        exit(1);
    }
    sem_post(sem);
    
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
			sem_wait(sem);
		    printf("%s: %s\n", names[i], varirants[memory->valentine_type_num[i]]);
		    sem_post(sem);
		    sleep(4);
		    exit(0);
    	}
	}

	for (int i = 0; i < N; i++) {
		waitpid(children[i], NULL, 0);
	}
	
	
	memory->lucky = rand() % N;
	printf("%s идёт на свидание!\n", names[memory->lucky]);

	munmap(memory, sizeof(Memory));
	shm_unlink(SHM_NAME);
	sem_destroy(sem);
	
	return 0;
}
