#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>

#define MEM_SZ 4096
#define BUFF_SZ (MEM_SZ - sizeof(size_t))

typedef struct shared_area {
  char buffer[BUFF_SZ];
} sa_t;

unsigned long bytes = 0;
clock_t begin, end;

void terminate(int arg) {
  end = clock();
  printf("%lu, %lf\n", bytes/1048576, (double)(end - begin)/CLOCKS_PER_SEC);
  exit(0);
}

int main() {
  begin = clock();
  signal(SIGTERM, terminate);

  key_t key = 1234;
  int shmid = shmget(key, MEM_SZ, 0600|IPC_CREAT);
  void *shared_memory = shmat(shmid, (void*)0, 0);
  sa_t *sa_ptr = (sa_t *) shared_memory;

  sem_t *ocupado = sem_open("pro_con_sem_ocupado", O_CREAT, 0600, 0);
  sem_t *livre   = sem_open("pro_con_sem_livre", O_CREAT, 0600, BUFF_SZ);

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
