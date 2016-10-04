#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <errno.h>
#include <signal.h>
#include <semaphore.h>
#include <unistd.h>

#define MEM_SZ 4096
//igualar BUFF_SZ nas duas implementacoes
#define BUFF_SZ (MEM_SZ - sizeof(size_t) - (2 * sizeof(sem_t)))

typedef struct shared_area {
	sem_t ocupado;
  sem_t livre;
	char buffer[BUFF_SZ];
} sa_t;

unsigned long bytes = 0;

void term_handler(int arg) {
  printf("bytes produzidos: %lu\n", bytes);
  exit(0);
}

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    printf("o produtor deve executar apos o consumidor e receber seu pid\n");
    exit(1);
  }

  signal(SIGTERM, term_handler);

  key_t key = 1234;
	int shmid = shmget(key, MEM_SZ, 0666|IPC_CREAT);
  if(shmid == -1) {
    perror("shmget falhou");
    exit(-1);
  }

	void *shared_memory = shmat(shmid, (void*)0, 0);
  if (shared_memory == (void *) -1) {
    perror("shmat falhou");
    exit(-1);
  }

	printf("ID da memoria compartilhada   = %d\n", shmid);
	printf("ADDR da memoria compartilhada = %p\n", shared_memory);

	sa_t *sa_ptr = (sa_t *) shared_memory;
  sem_init(&(sa_ptr->ocupado), 1, 0);
  sem_init(&(sa_ptr->livre), 1, BUFF_SZ);

	for(size_t i = 0; i < BUFF_SZ; i++)
		sa_ptr->buffer[i] = 'a';

  pid_t cons_pid = atoi(argv[1]);
  printf("%d enviando sinal para %d\n", getpid(), cons_pid);
  kill(cons_pid, SIGTERM);

  size_t i = 0;
	for(;;) {
    sem_wait(&(sa_ptr->livre));
    sa_ptr->buffer[i] = 'b';
    /*
    for(size_t j; j < BUFF_SZ; j++)
      putchar(sa_ptr->buffer[j]);
    putchar('\n');
    */
    sem_post(&(sa_ptr->ocupado));
    i = (i+1)%BUFF_SZ;
    bytes++;
	}
  return 0;
}
