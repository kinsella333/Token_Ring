#include <time.h>
#include "header.h"

int main(int argc, char** argv){
  int n, sfd, rv, count = 0, otherlength, timeout = 0, i = 0;
  int cfd[3], acfd[3];
	fd_set rset;
  struct timeval tv;
  char buf1[256], buf2[256], buf3[256], sbuf[80];
  struct sockaddr_in otheraddr[3];

  sfd = Server();
  sprintf(sbuf, "Hi Client ");
  fcntl(sfd, F_SETFL, O_NONBLOCK);

  pid_t pid = fork();
  if (pid > 0){
    //parent process
    sleep(4);
    while(1){
      otherlength = sizeof(otheraddr[i]);
      bzero(&(otheraddr[i]), otherlength);
      acfd[i] = accept(sfd, &(otheraddr[i]), &otherlength);

      if(acfd[i] > -1){
        i++;
        timeout = 0;

        if(i > 2){
          break;
        }
        continue;
      }

      if(timeout > 10000000000){
        printf("Error Timeout on Accept\n");
        kill(0, SIGKILL);
        return -1;
      }
      timeout++;
    }

    printf("check\n");
    timeout = 0;

    while(timeout < 10){
      send(acfd[0], sbuf, sizeof(sbuf), 0);
      send(acfd[1], sbuf, sizeof(sbuf), 0);
      send(acfd[2], sbuf, sizeof(sbuf), 0);

      if(timeout%1000 == 0){
        printf("Sending\r");
/*
        printf("Error Timeout on Send\n");
        kill(0, SIGKILL);
        return -1;
  */    }
      timeout++;
    }
  }else if (pid == 0){
    cfd[0] = Client();
    cfd[1] = Client();
    cfd[2] = Client();

    if(cfd[0]+cfd[1]+cfd[2] < 3){
      printf("Error on Client Creation\n");
      kill(0, SIGKILL);
    }else{
      printf("Clients fd: %d, %d, %d\n", cfd[0], cfd[1], cfd[2]);
    }

    while(timeout < 10){

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
              timeout = 11;
            }
        }
        if (FD_ISSET(cfd[1], &rset)) {
            if(recv(cfd[1], buf2, sizeof(buf2), 0) > 0){
              strcat(buf2, " Hi I am 2\n");
              printf("%s\n",buf2);
              timeout = 11;
            }
        }
        if (FD_ISSET(cfd[2], &rset)) {
            if(recv(cfd[2], buf3, sizeof(buf3), 0) > 0){
            strcat(buf3, " Hi I am 3\n");
            printf("%s\n",buf3);
            timeout = 11;
          }
        }
      }
      timeout++;
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
