#include "header.h"

/* node.c Is a token ring simulation program, and when called n times will form
   a token ring of n length, presuming the provided client/server port values
   create a chain. See README for more information.
*/

//The Frame object which will be passed between nodes.
struct frame{
  char head[2];     //SYN-SYN
  char start[2];    //DLE-STX
  char dst_address; //Destination Address
  char src_address; //Source Address
  char msg[80];     //Message to send
  char end[2];      //DLE-ETX
  int waiting;      //Waiting to be sent flag
};

//Thread Shared Memory
struct sharedMem{
  pthread_mutex_t mutex;       //Pthread Lock Mutex
  char *node_id;               //Node Address
  struct frame *f;             //User Input Frame
  struct frame *forward_frame; //Frame to be forwarded
  int connected;               //Client/Server Connection Flag
  int token;                   //Token Value at node
};

//Thread Data struct to be passed into node_manager
struct cthread{
    int threadid;             //Id of current thread
    struct sharedMem *shared; //Shared Thread Memory
    int port;                 //Port Thread will connect to
};

//Character set that is used to assign node addresses
static char charset[] = "abcdefghijklmnopqrstuvwxyz";

//Function Prototypes, each described individually below.
void *node_manager(void *threadid);
unsigned char * serialize_frame(unsigned char *buffer, struct frame *f);
struct frame * deserialize_frame(unsigned char *buffer, struct frame *f);
void init_shared(struct frame *f, struct frame *forward_frame, struct sharedMem *s);

int main(int argc, char** argv){
  pthread_t t[2];
  int i, port[2], *ret[2];
  struct cthread *threadData[2];
  struct sharedMem *s = malloc(sizeof(struct sharedMem));
  struct frame *f = malloc(sizeof(struct frame)), *forward_frame = malloc(sizeof(struct frame));
  char input[80], dst[4];

  //Get Port Values from args, or error out
  if(argc > 2){
    port[0] = atoi(argv[2]);
    port[1] = atoi(argv[1]);
  }else{
    printf("Please Specify Port Number for Client and Server\n");
    printf("Specify the port numbers for the client and server of this node\nThese will act as a replacement for IP addresses which would be used if not run locally\n");
    return -1;
  }

  //Init Pthread mutex
  if(pthread_mutex_init(&s->mutex, NULL) != 0){
         printf("\n mutex init failed\n");
         return 1;
  }

  //Initialize shared memory
  init_shared(f, forward_frame, s);

  //Initialize thread data and create client/server threads
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

  /*User input loop in form
    sys> Enter msg to provide message
    sys> Enter ^P to provide token
    input>
  */
  while(1){
    //Take in menu choice
    fgets(input, 80, stdin);

    //If user wishes to supply message
    if(strcmp(input, "msg\n") == 0){
      printf("sys> Destination node address\ninput> ");
      fgets(dst, 4, stdin);

      bzero(input,80);
      printf("sys> Enter Message (Max 80 Characters, Extra Will Be Cut)\ninput>");
      fgets(input, 80, stdin);

      //Populate frame data from user input
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
    }
    //Else User may have input ^P to add token
    else if(input[0] == 0x10){
      pthread_mutex_lock(&(s->mutex));
      s->token++;
      pthread_mutex_unlock(&(s->mutex));
    }

    //Clear input buffer and reprint menu
    bzero(input,80);
    printf("\nsys> Enter msg to provide message\nsys> Enter ^P to provide token\ninput>");
  }

  //Join pthreads and destroy mutex before exit. Occurs when ring breaks
  pthread_join(t[0], (void**)&(ret[0]));
  pthread_join(t[1], (void**)&(ret[1]));
  pthread_mutex_destroy(&threadData[0]->shared->mutex);
  return 0;
}

// Thread method, controls both client and server threads.
// Takes in a cthread variable as threadData.
void *node_manager(void *threadData){
  struct cthread* threadD = threadData;
  int fd, i = 0, choice = 0, size;
  char c, msg[100];
  unsigned char buffer[88];

  //Initialize Client and Server Threads using the provided code.
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

  //Once connected, print the node address and the menu options
  pthread_mutex_lock(&(threadD->shared->mutex));
  threadD->shared->connected++;
  if(threadD->shared->connected > 1){
    printf("sys> Node Address is: %.*s\n",1,threadD->shared->node_id);
    printf("\nsys> Enter msg to provide message\nsys> Enter ^P to provide token\ninput>");
  }
  pthread_mutex_unlock(&(threadD->shared->mutex));

  //Main thread loop both sending and receiving frames from other nodes.
  while(1){

    //If Client thread
    if(threadD->threadid == 1){

      //Only execute if node has tokens
      pthread_mutex_lock(&(threadD->shared->mutex));
      if(threadD->shared->token > 0){

        //If the node needs to forward a message down the chain
        if(threadD->shared->forward_frame->waiting == 1){
          serialize_frame(buffer, threadD->shared->forward_frame);
          send(fd, buffer, sizeof(buffer), 0);
          threadD->shared->forward_frame = malloc(sizeof(struct frame));
          threadD->shared->forward_frame->waiting = 0;
          threadD->shared->token--;
        }
        //if the node needs to send message received from user
        else if(threadD->shared->f->waiting == 1){
          serialize_frame(buffer, threadD->shared->f);
          send(fd, buffer, sizeof(buffer), 0);
          threadD->shared->f->start[0] = 0x10;
          threadD->shared->f->start[1] = 0x02;
          threadD->shared->f->waiting = 0;
          threadD->shared->token--;
        }
      }
      pthread_mutex_unlock(&(threadD->shared->mutex));
    }

    //If Server thread
    if(threadD->threadid == 0){
      //Wait to receive
      size = recv(fd, buffer, sizeof(buffer), 0);

      //If the DLE-STX bit has been changed, operate on buffer
      if((buffer[2] != 0x10 || buffer[3] != 0x02)){
        //If the frame has gone all the way around the chain,
        //the Destination must not exist on the chain
        if(buffer[5] == *(threadD->shared->node_id)){
          printf("Error Node does not exist\n");
        }else{
          //If the Destination Address is this node, then print message and
          //accept token.
          if(buffer[4] == *(threadD->shared->node_id)){
            pthread_mutex_lock(&(threadD->shared->mutex));
            threadD->shared->token++;

            printf("\nsys> Received Message: %.*s\n", 80, buffer + 6);
            printf("sys> From Node Address: (%.*s)\n",1,buffer + 5);
            printf("sys> Node (%.*s)",1,threadD->shared->node_id);
            printf(" currently has %d tokens\n", threadD->shared->token);

            pthread_mutex_unlock(&(threadD->shared->mutex));
          }
          //If message is not for this node, forward it to the next in the chain.
          else{
            printf("\nsys> Received Message, But not for me\n");
            pthread_mutex_lock(&(threadD->shared->mutex));
            threadD->shared->token++;

            deserialize_frame(buffer, threadD->shared->forward_frame);
            threadD->shared->forward_frame->waiting = 1;
            pthread_mutex_unlock(&(threadD->shared->mutex));
          }
        }
      }
    }
    sleep(1);
  }

  pthread_exit(NULL);
  close(fd);
}

//Simple struct serialize function to store frame in a character buffer
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

//Simple struct deserialize function to store buffer data in a frame
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

//Initializing funtion for the sharedMem struct.
void init_shared(struct frame *f, struct frame *forward_frame, struct sharedMem *s){
  f->head[0] = 0x16;
  f->head[1] = 0x16;
  f->start[0] = 0x10;
  f->start[1] = 0x02;
  f->dst_address = -1;
  f->src_address = -1;
  f->end[0] = 0x10;
  f->end[1] = 0x03;
  f->waiting = 0;

  s->f = f;
  s->forward_frame = forward_frame;
  s->connected = 0;
  s->token = 0;
}
