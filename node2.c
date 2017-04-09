#include <time.h>
#include "header.h"

int main(int argc, char** argv){
  int n, sfd, rv, count = 0, otherlength;
  int cfd[3];
	fd_set rset;
  struct timeval tv;
  char buf1[256], buf2[256], buf3[256], sbuf[80];
  struct sockaddr_in otheraddr[3];

  sfd = Server();
  sprintf(sbuf, "Hi Client ");

  pid_t pid = fork();
  if (pid == 0){
    // child process
    while(count < 3){
      otherlength = sizeof(otheraddr[count]);
      if(accept(sfd, &otheraddr[count], &otherlength) != -1){
      	printf("Connected\n");
        count++;
      }
    }
    while(1){
      send(sfd, sbuf, sizeof(sbuf), 0);
    }
  }else if (pid > 0){
    cfd[0] = Client();
    cfd[1] = Client();
    cfd[2] = Client();

    while(1){

      FD_ZERO(&rset);
      FD_SET(cfd[0], &rset);
      FD_SET(cfd[1], &rset);
      FD_SET(cfd[2], &rset);

      n = cfd[2] + 1;

      tv.tv_sec = 10;
      tv.tv_usec = 500000;

      rv = select(n, &rset, NULL, NULL, &tv);

      if (rv == -1) {
        perror("select"); // error occurred in select()
      } else if (rv == 0) {
        printf("Timeout occurred!  No data after 10.5 seconds.\n");
      }else {
        // one or both of the descriptors have data
        if (FD_ISSET(cfd[0], &rset)){
            if(recv(cfd[0], buf1, sizeof(buf1), 0) > 0){
              strcat(buf1, " Hi I am 1\n");
              printf("%s\n",buf1);
            }
        }
        if (FD_ISSET(cfd[1], &rset)) {
            if(recv(cfd[1], buf2, sizeof(buf2), 0) > 0){
              strcat(buf2, " Hi I am 2\n");
              printf("%s\n",buf2);
            }
        }
        if (FD_ISSET(cfd[2], &rset)) {
            if(recv(cfd[2], buf3, sizeof(buf3), 0) > 0){
            strcat(buf3, " Hi I am 3\n");
            printf("%s\n",buf3);
          }
        }
      }
    }

  }else{
    // fork failed
    printf("fork() failed!\n");
    return 1;
  }

  close(sfd);
  close(cfd[0]);
  close(cfd[1]);
  close(cfd[2]);

  return 0;
}
