#include <time.h>
#include "header.h"


struct sharedMem{
  int choice;
  pthread_mutex_t mutex;
};

struct cthread{
    int threadid;
    struct sharedMem *shared;
    int port;
};

void *node_manager(void *threadid);

int main(int argc, char** argv){
  pthread_t t[2];
  int i, port[2], *ret[2];
  struct cthread *threadData[2];
  struct sharedMem *s = malloc(sizeof(struct sharedMem));
  char input[32];

  if(argc > 2){
    port[0] = atoi(argv[2]);
    port[1] = atoi(argv[1]);
  }else{
    printf("Please Specify Port Number for Client and Server\n");
    printf("Specify the port numbers for the client and server of this node\nThese will act as a replacement for IP addresses which would be used if not run locally\n");
    return -1;
  }

  if(pthread_mutex_init(&s->mutex, NULL) != 0){
         printf("\n mutex init failed\n");
         return 1;
    }

  for (i=0; i<2; i++){
    threadData[i] = malloc(sizeof(struct cthread));
    threadData[i]->threadid = i;
    threadData[i]->port = port[i];
    threadData[i]->shared = s;

    if(pthread_create( &t[i], NULL, node_manager, (void*)threadData[i]) < 0){
        perror("could not create node\n");
        return 1;
    }
  }

  while(1){
    fgets(input, 32, stdin);
    input[strcspn(input, "\n" )] = '\0';

    if(strcmp(input, "quit") == 0){
      pthread_mutex_lock(&(s->mutex));
      s->choice = 1;
      pthread_mutex_unlock(&(s->mutex));
      break;
    }else if(){

    }
  }

  pthread_join(t[0], (void**)&(ret[0]));
  pthread_join(t[1], (void**)&(ret[1]));
  pthread_mutex_destroy(&threadData[0]->shared->mutex);
  return 0;
}


void *node_manager(void *threadData){
  struct cthread* threadD = threadData;
  int fd, i = 0, choice = 0;
  char buf[80];

  if(threadD->threadid == 0){
    fd = Server(threadD->port);
    printf("This is Server fd: %d From Node: %d\n", fd, threadD->port);
  }else if(threadD->threadid == 1){
    fd = Client(threadD->port);
    printf("This is Client fd: %d From Node: %d\n", fd, threadD->port);
  }

  while(1){
    if(threadD->shared->choice == 1){
      break;
    }
  }

  pthread_exit(NULL);
  close(fd);
}
