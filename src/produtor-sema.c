#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>

#define MEM_SZ 4096
#define BUFF_SZ (MEM_SZ - sizeof(size_t))

typedef struct shared_area {
  char buffer[BUFF_SZ];
} sa_t;

unsigned long bytes = 0;

void term_handler(int arg) {
  printf("bytes produzidos: %lu\n", bytes);
  exit(0);
}

int main() {
  signal(SIGTERM, term_handler);

  key_t key = 1234;
  int shmid = shmget(key, MEM_SZ, 0644|IPC_CREAT);
  void *shared_memory = shmat(shmid, (void*)0, 0);

  printf("ID da memoria compartilhada   = %d\n", shmid);
  printf("ADDR da memoria compartilhada = %p\n", shared_memory);

  sa_t *sa_ptr = (sa_t *) shared_memory;
  sem_t *ocupado = sem_open("pro_con_sem_ocupado", O_CREAT, 0644, 0);
  sem_t *livre   = sem_open("pro_con_sem_livre", O_CREAT, 0644, BUFF_SZ);

  size_t i = 0;
  for(;;) {
    sem_wait(livre);
    sa_ptr->buffer[i] = '#';
    sem_post(ocupado);
    i = (i+1)%BUFF_SZ;
    bytes++;
  }
  return 0;
}
