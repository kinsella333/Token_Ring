#include <time.h>
#include "header.h"

struct cthread{
    int threadid;
    int nodeID;
    int snode;
};

void *node_manager(void *threadid);
void *node_init(void *threadData);

int main(int argc, char** argv){
  pthread_t t[3];
  int i;
  struct cthread *threadData[3];

  threadData[0] = malloc(sizeof(struct cthread));
  threadData[1] = malloc(sizeof(struct cthread));
  threadData[2] = malloc(sizeof(struct cthread));

  for (i=0; i<3; i++){
    threadData[i]->nodeID = i;
    if(argc > 1) threadData[i]->snode = atoi(argv[1]);

    if(pthread_create( &t[i], NULL, node_manager, (void*)threadData[i]) < 0){
        perror("could not create node\n");
        return 1;
    }
    sleep(15);
  }
  pthread_exit(NULL);
  return 0;
}

void *node_manager(void *threadData){
  struct cthread* threadD[2];
  struct cthread* temp = (struct cthread*)threadData;

  threadD[0] = malloc(sizeof(struct cthread));
  threadD[1] = malloc(sizeof(struct cthread));
  threadD[0]->nodeID = temp->nodeID;
  threadD[1]->nodeID = temp->nodeID;
  threadD[0]->snode = temp->snode;
  threadD[1]->snode = temp->snode;

  int fd, i;
  char buf[80];
  pthread_t t[2];

  for (i=0; i<2; i++){
    threadD[i]->threadid = i;

    if(pthread_create( &t[i], NULL, node_init, (void*)threadD[i]) < 0){
        perror("could not create Client/Server thread\n");
        return 1;
    }
  }
  pthread_exit(NULL);
}

void *node_init(void *threadData){
  struct cthread* threadD = threadData;
  int fd, i;
  int *fds;
  char buf[80];

  //printf("Node: %d Thread: %d\n", threadD->nodeID, threadD->threadid);

  if(threadD->threadid == 0){
    fds = Server();
    printf("This is Server fd: %d From Node: %d\n", *fds, threadD->nodeID);
    close(*fds);
    close(*(fds));
  }else if(threadD->threadid == 1){
    fd = Client();
    printf("This is Client fd: %d From Node: %d\n", fd, threadD->nodeID);
    close(fd);
  }

/*
  if(threadD->threadid == 0 && threadD->nodeID == threadD->snode){
    sprintf(buf, "Hi Client From Node: %d\n", threadD->nodeID);
    send(fd, buf, sizeof(buf), 0);
  }else if(threadD->threadid == 1 && threadD->nodeID == threadD->snode){
    if(recv(fd, buf, sizeof(buf),0)==0){
      close(fd);
      perror("Recieve Error");
    }
    printf(buf);
    bzero(buf, 80);
  }
  */
}
