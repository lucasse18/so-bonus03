#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <signal.h>
#include <time.h>

#define MEM_SZ 4096
#define BUFF_SZ (MEM_SZ - sizeof(size_t))

typedef struct shared_area{
  size_t num;
  char buffer[BUFF_SZ];
} sa_t;

unsigned long blocks = 0;
clock_t begin, end;

void terminate(int arg) {
  end = clock();
  printf("%lu, %lf\n", (blocks * BUFF_SZ)/1048576, (double)(end - begin)/CLOCKS_PER_SEC);
  exit(0);
}

int main() {
  begin = clock();
  signal(SIGTERM, terminate);

  key_t key = 4321;
  int shmid = shmget(key, MEM_SZ, 0600|IPC_CREAT);
  void *shared_memory = shmat(shmid, (void*)0, 0);
  sa_t *sa_ptr = (sa_t *) shared_memory;

  //FIXME possibilidade de inicializacao duplicada
  sa_ptr->num = 0;

  size_t i;
  for(;;) {
    if(sa_ptr->num > 0) {
      for(i = sa_ptr->num; i > 0; i--)
        sa_ptr->buffer[i] = '-';
      sa_ptr->num = 0;
      blocks++;
    }
  }
  return 0;
}
