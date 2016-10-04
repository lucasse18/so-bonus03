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
  printf("bytes consumidos: %lu\n", bytes);
  exit(0);
}

int main() {
  signal(SIGTERM, term_handler);

  key_t key = 1234;
	int shmid = shmget(key, MEM_SZ, 0666|IPC_CREAT);
	void *shared_memory = shmat(shmid, (void*)0, 0);

	printf("ID da memoria compartilhada   = %d\n", shmid);
	printf("ADDR da memoria compartilhada = %p\n", shared_memory);

	sa_t *sa_ptr = (sa_t *) shared_memory;

  sigset_t s_set;
  sigfillset(&s_set);
  int sig;

  //sincronização entre processos utilizando sinais. sem o uso de sinais o
  //processo consumidor pode fazer uso do semaforo sem que este tenha sido
  //inicializado
  printf("%d aguardando sinal\n", getpid());
  sigwait(&s_set, &sig);
  printf("sinal recebido, continuando\n");

  size_t i = 0;
	for(;;) {
    sem_wait(&(sa_ptr->ocupado));
		sa_ptr->buffer[i] = 'a';
    /*
    for(size_t j; j < BUFF_SZ; j++)
      putchar(sa_ptr->buffer[j]);
    putchar('\n');
    */
    sem_post(&(sa_ptr->livre));
    i = (i+1)%BUFF_SZ;
    bytes++;
	}
  return 0;
}
