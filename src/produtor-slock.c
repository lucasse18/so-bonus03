#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <errno.h>
#include <semaphore.h>

#define MEM_SZ 4096
//igualar BUFF_SZ nas duas implementações
#define BUFF_SZ (MEM_SZ - sizeof(size_t) - (2 * sizeof(sem_t)))

typedef struct shared_area{
	size_t num;
	char buffer[BUFF_SZ];
} sa_t;

int main() {
  size_t i;
	key_t key = 1234;
	sa_t *sa_ptr;

	int shmid = shmget(key, MEM_SZ, 0666|IPC_CREAT);
	if (shmid == -1) {
    perror("shmget falhou.");
		exit(-1);
	}

	void *shared_memory = shmat(shmid, (void*)0, 0);
	if (shared_memory == (void *) -1) {
		perror("shmat falhou.");
		exit(-1);
	}

	printf("ID da memoria compartilhada   = %d\n", shmid);
	printf("ADDR da memoria compartilhada = %p\n", shared_memory);

	sa_ptr = (sa_t *) shared_memory;

	sa_ptr->num=0;
	for(i = 0; i < BUFF_SZ; i++)
		sa_ptr->buffer[i] = 0;

	for(;;) {
		if (sa_ptr->num ==  0) {
			for(i = 0; i < BUFF_SZ; i++)
				sa_ptr->buffer[i] = '#';
			printf("Produziu %zd bytes\n", i);
			sa_ptr->num = i;
		}
	}
  return 0;
}
