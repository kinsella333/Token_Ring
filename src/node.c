#include "header.h"

struct frame{
  char start[2];
  char token[2];
  char dst_address;
  char src_address;
  char msg[80];
  char end[2];
};

struct sharedMem{
  int quit;
  pthread_mutex_t mutex;
  char *node_id;
  struct frame *f;
};

struct cthread{
    int threadid;
    struct sharedMem *shared;
    int port;
};

static char charset[] = "abcdefghijklmnopqrstuvwxyz";


void *node_manager(void *threadid);
unsigned char * serialize_frame(unsigned char *buffer, struct cthread *t);
struct frame * deserialize_frame(unsigned char *buffer, struct cthread *t);

int main(int argc, char** argv){
  pthread_t t[2];
  int i, port[2], *ret[2];
  struct cthread *threadData[2];
  struct sharedMem *s = malloc(sizeof(struct sharedMem));
  struct frame *f = malloc(sizeof(struct frame));
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

  f->start[0] = 0x16;
  f->start[1] = 0x16;
  f->token[0] = 0x10;
  f->token[1] = 0x02;
  f->dst_address = -1;
  f->src_address = -1;
  f->end[0] = 0x10;
  f->end[1] = 0x03;

  s->f = f;

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
    fgets(input, 80, stdin);

    if(strcmp(input, "quit\n") == 0){
      pthread_mutex_lock(&(s->mutex));
      s->quit = 1;
      pthread_mutex_unlock(&(s->mutex));
      break;
    }else if(input[0] == 0x10){
      printf("Destination node address: ");
      fgets(dst, 4, stdin);


      printf("Enter Message (Max 80 Characters)\nmsg:");
      fgets(input, 80, stdin);

      pthread_mutex_lock(&(s->mutex));
      for(i = 0; i < 80; i++){
          s->f->msg[i] = input[i];
      }
      s->f->token[0] = s->f->end[0];
      s->f->token[1] = s->f->end[1];
      s->f->dst_address = dst[0];
      s->f->src_address = *(s->node_id);

      pthread_mutex_unlock(&(s->mutex));
    }
  }

  pthread_join(t[0], (void**)&(ret[0]));
  pthread_join(t[1], (void**)&(ret[1]));
  pthread_mutex_destroy(&threadData[0]->shared->mutex);
  return 0;
}


void *node_manager(void *threadData){
  struct cthread* threadD = threadData;
  int fd, i = 0, choice = 0, size, quit = 0;
  char c;
  unsigned char buffer[88];

  if(threadD->threadid == 0){
    fd = Server(threadD->port);
    printf("This is Server fd: %d From Port: %d\n", fd, threadD->port);
  }else if(threadD->threadid == 1){
    fd = Client(threadD->port);
    printf("This is Client fd: %d From Port: %d\n", fd, threadD->port);
    c = charset[(threadD->port)%20];
    pthread_mutex_lock(&(threadD->shared->mutex));
    threadD->shared->node_id = &c;
    pthread_mutex_unlock(&(threadD->shared->mutex));
    printf("Node Address is: %.*s\n",1,threadD->shared->node_id);
  }

  while(1){
    if(threadD->shared->quit == 1 || quit == 1){
      break;
    }

    //Client
    if(threadD->threadid == 1){
      serialize_frame(buffer, threadD);
      send(fd, buffer, sizeof(buffer), 0);
      pthread_mutex_lock(&(threadD->shared->mutex));
      threadD->shared->f->token[0] = 0x10;
      threadD->shared->f->token[1] = 0x02;
      pthread_mutex_unlock(&(threadD->shared->mutex));
    }

    //Server
    if(threadD->threadid == 0){
      size = recv(fd, buffer, sizeof(buffer), 0);

      if(size == 0){
        printf("Token Ring Broken, Exiting\n");
        quit = 1;

      }else if((buffer[2] != 0x10 || buffer[3] != 0x02)){
        if(buffer[4] == *(threadD->shared->node_id)){
          printf("Recieved Message: %.*s\n", 80, buffer + 6);
          printf("From Address: %.*s\n",1,buffer + 5);
        }else{
          printf("Recieved Message, But not for me\n");
          deserialize_frame(buffer, threadD);
        }

      /*  pthread_mutex_lock(&(threadD->shared->mutex));
        threadD->shared->f->token[0] = 0x10;
        threadD->shared->f->token[1] = 0x02;
        pthread_mutex_unlock(&(threadD->shared->mutex));
      */

      }
    }
    sleep(1);
  }

  pthread_exit(NULL);
  close(fd);
}

unsigned char * serialize_frame(unsigned char *buffer, struct cthread *t){
    int i = 0;

    pthread_mutex_lock(&(t->shared->mutex));
    buffer[0] = t->shared->f->start[0];
    buffer[1] = t->shared->f->start[1];
    buffer[2] = t->shared->f->token[0];
    buffer[3] = t->shared->f->token[1];
    buffer[4] = t->shared->f->dst_address;
    buffer[5] = t->shared->f->src_address;

    for(i = 0; i < 80; i++){
      buffer[i+6] = t->shared->f->msg[i];
    }

    buffer[86] = t->shared->f->end[0];
    buffer[87] = t->shared->f->end[1];

    pthread_mutex_unlock(&(t->shared->mutex));
    return buffer;
}

struct frame * deserialize_frame(unsigned char *buffer, struct cthread *t){
    int i = 0;

    pthread_mutex_lock(&(t->shared->mutex));

    t->shared->f->start[0] = buffer[0];
    t->shared->f->start[1] = buffer[1];
    t->shared->f->token[0] = buffer[2];
    t->shared->f->token[1] = buffer[3];
    t->shared->f->dst_address = buffer[4];
    t->shared->f->src_address = buffer[5];

    for(i = 0; i < 80; i++){
      t->shared->f->msg[i] = buffer[i+6];
    }

    t->shared->f->end[0] = buffer[86];
    t->shared->f->end[1] = buffer[87];

    pthread_mutex_unlock(&(t->shared->mutex));
    return t;
}
