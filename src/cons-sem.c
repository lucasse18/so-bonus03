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
#define COUNT_NAME "/prod_cons_sem_count"
#define EMPTY_NAME "/prod_cons_sem_empty"

typedef struct shared_area {
  size_t num;
  char buffer[BUFF_SZ];
} sa_t;

unsigned long blocks = 0;
clock_t begin, end;

void terminate(int arg) {
  end = clock();
  printf("%lu, %lf\n", (blocks * BUFF_SZ)/1048576, (double)(end - begin)/CLOCKS_PER_SEC);
  sem_unlink(COUNT_NAME);
  sem_unlink(EMPTY_NAME);
  exit(arg);
}

int main() {
  begin = clock();
  signal(SIGTERM, terminate);

  key_t key = 1234;
  int shmid = shmget(key, MEM_SZ, 0600|IPC_CREAT);
  void *shared_memory = shmat(shmid, (void*)0, 0);
  sa_t *sa_ptr = (sa_t *) shared_memory;

  //FIXME possibilidade de inicializacao duplicada
  sa_ptr->num = 0;

  sem_t *count = sem_open(COUNT_NAME, O_CREAT, 0600, 0);
  sem_t *empty = sem_open(EMPTY_NAME, O_CREAT, 0600, 1);

  size_t i = 0;
  for(;;) {
    sem_wait(count);
    for(i = sa_ptr->num; i > 0; i--)
      sa_ptr->buffer[i] = '-';
    sa_ptr->num = i;
    sem_post(empty);
    blocks++;
  }
  return 0;
}
