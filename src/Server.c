/*****************************************************************************/
/*                                                                           */
/* This routine establishes a passive open connection.  That is, it creates  */
/* a socket, and passively wait for a connection.  Once a connection request */
/* has been received, it echoes "connected" on the screen, and return        */
/* a file descriptor to be used to communicate with the remote machine.      */
/* Make sure that you change the machine name from "vulcan" to that you    */
/* will be running your process on. Also, change the port number to          */
/* a suitable port number as indicated in the project writeup.               */
/*                                                                           */
/*****************************************************************************/

#include "header.h"

int Server(int port)
{
	struct sockaddr_in myaddr, otheraddr;
	struct hostent *myname;

	int s, fd, otherlength;
	char *ch;
	int hostnamelength;

	pid_t pid;

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) ReportError ("socket");
	bzero(&myaddr, sizeof(myaddr));
	myaddr.sin_family  = AF_INET;
	//My Student Port Number was already being used
	myaddr.sin_port = htons(port);
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(s, &myaddr, sizeof(myaddr));

	listen(s, 1);
	//printf("Server Listening \n");

	otherlength = sizeof(otheraddr);
	fd = accept(s, &otheraddr, &otherlength);

	printf("Connected\n");

	return(fd);
}
