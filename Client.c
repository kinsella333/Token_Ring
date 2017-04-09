/*****************************************************************************/
/*                                                                           */
/* This routine establishes an active open connection.  That is, it creates  */
/* a socket, and connects using it to a remote machine. The routine returns  */
/* a file descriptor to be used to communicate with the remote machine.      */
/* Make sure that you change the machine name from "vulcan" to that of   */
/* the remote machine.  Also, change the port number to a suitable port      */
/* number as indicated in the project writeup.                               */
/*                                                                           */
/*****************************************************************************/

#include "header.h"

int Client()
{
	int s;
	int n;
	int code;
	FILE *fp;
	char * ch, thishostname[256];

	struct hostent *otherhost;
	struct sockaddr_in otheraddr;

	pid_t pid;

	bzero(&otheraddr, sizeof(otheraddr));

	otheraddr.sin_family = AF_INET;
	otheraddr.sin_port = htons(32088);
	otheraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	s = socket(AF_INET, SOCK_STREAM, 0);

	//otherhost = gethostbyname(otherhostname);
	//bcopy(otherhost->h_addr_list[0], &otheraddr.sin_addr, otherhost->h_length);

	n = connect(s, &otheraddr, sizeof(otheraddr));

	if ( n < 0)
		return(n);
	else
		return(s);
}
