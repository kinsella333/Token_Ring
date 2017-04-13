#include "header.h"

struct frame{
  char head[2];
  char start[2];
  char dst_address;
  char src_address;
  char msg[80];
  char end[2];
  char token;
  int waiting;
};

struct sharedMem{
  pthread_mutex_t mutex;
  char *node_id;
  struct frame *f;
  struct frame *forward_frame;
  int connected;
};

struct cthread{
    int threadid;
    struct sharedMem *shared;
    int port;
};

static char charset[] = "abcdefghijklmnopqrstuvwxyz";


void *node_manager(void *threadid);
unsigned char * serialize_frame(unsigned char *buffer, struct frame *f);
struct frame * deserialize_frame(unsigned char *buffer, struct frame *f);

int main(int argc, char** argv){
  pthread_t t[2];
  int i, port[2], *ret[2];
  struct cthread *threadData[2];
  struct sharedMem *s = malloc(sizeof(struct sharedMem));
  struct frame *f = malloc(sizeof(struct frame)), *forward_frame = malloc(sizeof(struct frame));
  char input[80], dst[4];

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

  f->head[0] = 0x16;
  f->head[1] = 0x16;
  f->start[0] = 0x10;
  f->start[1] = 0x02;
  f->dst_address = -1;
  f->src_address = -1;
  f->end[0] = 0x10;
  f->end[1] = 0x03;
  f->token = 0;
  f->waiting = 0;

  s->f = f;
  s->forward_frame = forward_frame;
  s->connected = 0;

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

  sleep(2);
  while(1){
    fgets(input, 80, stdin);

    if(strcmp(input, "msg\n") == 0){
      printf("sys> Destination node address\ninput> ");
      fgets(dst, 4, stdin);

      bzero(input,80);
      printf("sys> Enter Message (Max 80 Characters, Extra Will Be Cut)\ninput>");
      fgets(input, 80, stdin);

      pthread_mutex_lock(&(s->mutex));
      for(i = 0; i < 80; i++){
          s->f->msg[i] = input[i];
      }
      s->f->start[0] = s->f->end[0];
      s->f->start[1] = s->f->end[1];
      s->f->dst_address = dst[0];
      s->f->src_address = *(s->node_id);
      s->f->waiting = 1;

      pthread_mutex_unlock(&(s->mutex));
    }else if(input[0] == 0x10){

      pthread_mutex_lock(&(s->mutex));
      s->f->token++;
      pthread_mutex_unlock(&(s->mutex));
    }
    bzero(input,80);
    printf("\nsys> Enter msg to provide message\nsys> Enter ^P to provide token\ninput>");
  }

  pthread_join(t[0], (void**)&(ret[0]));
  pthread_join(t[1], (void**)&(ret[1]));
  pthread_mutex_destroy(&threadData[0]->shared->mutex);
  return 0;
}


void *node_manager(void *threadData){
  struct cthread* threadD = threadData;
  int fd, i = 0, choice = 0, size;
  char c, msg[100];
  unsigned char buffer[88];

  if(threadD->threadid == 0){
    fd = Server(threadD->port);
    printf("sys> Server Created\n");
  }else if(threadD->threadid == 1){
    fd = Client(threadD->port);
    printf("sys> Client Created\n");
    c = charset[(threadD->port)%20];

    pthread_mutex_lock(&(threadD->shared->mutex));
    threadD->shared->node_id = &c;
    pthread_mutex_unlock(&(threadD->shared->mutex));
  }
  pthread_mutex_lock(&(threadD->shared->mutex));
  threadD->shared->connected++;

  if(threadD->shared->connected > 1){
    printf("sys> Node Address is: %.*s\n",1,threadD->shared->node_id);
    printf("\nsys> Enter msg to provide message\nsys> Enter ^P to provide token\ninput>");
  }
  pthread_mutex_unlock(&(threadD->shared->mutex));

  while(1){

    //Client
    if(threadD->threadid == 1){
      pthread_mutex_lock(&(threadD->shared->mutex));

      if(threadD->shared->f->token > 0){
        if(threadD->shared->forward_frame->waiting == 1){
          serialize_frame(buffer, threadD->shared->forward_frame);
          send(fd, buffer, sizeof(buffer), 0);
          threadD->shared->forward_frame = malloc(sizeof(struct frame));
          threadD->shared->forward_frame->waiting = 0;
        }else if(threadD->shared->f->waiting == 1){
          serialize_frame(buffer, threadD->shared->f);
          send(fd, buffer, sizeof(buffer), 0);
          threadD->shared->f->start[0] = 0x10;
          threadD->shared->f->start[1] = 0x02;
          threadD->shared->f->waiting = 0;
        }
        threadD->shared->f->token--;
      }
      pthread_mutex_unlock(&(threadD->shared->mutex));
    }

    //Server
    if(threadD->threadid == 0){
      size = recv(fd, buffer, sizeof(buffer), 0);

      if((buffer[2] != 0x10 || buffer[3] != 0x02)){

        if(buffer[5] == *(threadD->shared->node_id)){
          printf("Error Node does not exist\n");
        }else{
          pthread_mutex_lock(&(threadD->shared->mutex));
          threadD->shared->f->token++;
          pthread_mutex_unlock(&(threadD->shared->mutex));

          if(buffer[4] == *(threadD->shared->node_id)){
            pthread_mutex_lock(&(threadD->shared->mutex));
            printf("\nRecieved Message: %.*s\n", 80, buffer + 6);
            printf("From Node Address: (%.*s)\n",1,buffer + 5);
            printf("Node (%.*s)",1,threadD->shared->node_id);
            printf(" currently has %d tokens\n", threadD->shared->f->token);
            pthread_mutex_unlock(&(threadD->shared->mutex));
          }else{
            printf("Recieved Message, But not for me\n");
            pthread_mutex_lock(&(threadD->shared->mutex));

            deserialize_frame(buffer, threadD->shared->forward_frame);
            threadD->shared->forward_frame->waiting = 1;
            pthread_mutex_unlock(&(threadD->shared->mutex));
          }
          printf("\nsys> Enter msg to provide message\nsys> Enter ^P to provide token\ninput>");
        }
      }
    }
    sleep(1);
  }

  pthread_exit(NULL);
  close(fd);
}

unsigned char * serialize_frame(unsigned char *buffer, struct frame *f){
    int i = 0;

    buffer[0] = f->head[0];
    buffer[1] = f->head[1];
    buffer[2] = f->start[0];
    buffer[3] = f->start[1];
    buffer[4] = f->dst_address;
    buffer[5] = f->src_address;

    for(i = 0; i < 80; i++){
      buffer[i+6] = f->msg[i];
    }

    buffer[86] = f->end[0];
    buffer[87] = f->end[1];

    return buffer;
}

struct frame * deserialize_frame(unsigned char *buffer, struct frame *f){
    int i = 0;

    f->head[0] = buffer[0];
    f->head[1] = buffer[1];
    f->start[0] = buffer[2];
    f->start[1] = buffer[3];
    f->dst_address = buffer[4];
    f->src_address = buffer[5];

    for(i = 0; i < 80; i++){
      f->msg[i] = buffer[i+6];
    }

    f->end[0] = buffer[86];
    f->end[1] = buffer[87];

    return f;
}
