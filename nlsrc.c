//1. Make the necessary includes and set up the variables:
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int sockfd;
	int len;
	struct sockaddr_un address;
	int result;
	int byteSend;

	int command_len=0;
	int i;
	for(i=1;i<argc;i++)
		command_len+=(strlen(argv[i])+1);
	char *command=malloc(command_len);
	memset(command,command_len+1,0);
	for(i=1;i<argc;i++)
	{
		memcpy(command+strlen(command),argv[i],strlen(argv[i]));
		if ( i < argc-1 )
			memcpy(command+strlen(command)," ",1);	
	}

	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	address.sun_family = AF_UNIX;
	strcpy(address.sun_path, "/tmp/nlsr_api_server_socket");
	len = sizeof(address);
	result = connect(sockfd, (struct sockaddr *)&address, len);
	if(result == -1) {
		perror("oops nlsrc ");
		exit(1);
	}
	printf("Data to send: %s \n",command);
	byteSend=send(sockfd, command, strlen(command),0);
	close(sockfd);
	exit(0);
}
