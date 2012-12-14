//1. Make the necessary includes and set up the variables:
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <getopt.h>

struct option longopts[] =
{
    { "server_ip",    required_argument, NULL, 's'},
    { "server_port",    required_argument, NULL, 'p'},
    { 0 }
};

static int 
usage(char *progname)
{
    printf("Usage: %s [OPTIONS...]\n\
	NLSR Api client....\n\
	nlsrc -s server_ip -p server_port add|del name|neighbor name_prefix [faceX] \n\
	add/del, -- adding/deleting operation\n\
	name/neighbor, -- Operation for name/neighbor\n\
	name_name, -- Name prefix for name/neighbor\n\
	faceX, -- Face Id for neighbor if third argument is neighbor\n", progname);

    exit(1);
}

int main(int argc, char *argv[])
{
	int sockfd;
	int len;
	struct sockaddr_in address;
	int result;
	int byteSend;
	char *server_address, *server_port;	

	int command_len=0;
	int i;

	if (argc < 8 )
		usage(argv[0]);
	if ( strcmp(argv[6],"neighbor") == 0 && argc <9 )
		usage(argv[0]);

	while ((result = getopt_long(argc, argv, "s:p:", longopts, 0)) != -1) 
	{
        	switch (result) 
		{
			case 's':
				server_address = optarg;
				break;
			case 'p':
				server_port = optarg;
				break;
		}
    	}	


	char recv_buffer[1024];
	bzero(recv_buffer,1024);

	for(i=5;i<argc;i++)
		command_len+=(strlen(argv[i])+1);
	char *command=malloc(command_len);
	memset(command,command_len+1,0);
	for(i=5;i<argc;i++)
	{
		memcpy(command+strlen(command),argv[i],strlen(argv[i]));
		if ( i < argc-1 )
			memcpy(command+strlen(command)," ",1);	
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	//address.sun_family = AF_UNIX;
	//strcpy(address.sun_path, "/tmp/nlsr_api_server_socket");
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(server_address);
	address.sin_port = atoi(server_port);

	len = sizeof(address);
	result = connect(sockfd, (struct sockaddr *)&address, len);
	if(result == -1) 
	{
		perror("oops nlsrc ");
		exit(1);
	}
	printf("Command to send: %s \n",command);
	byteSend=send(sockfd, command, strlen(command),0);
	recv(sockfd, recv_buffer, 1024, 0);
	printf("%s\n",recv_buffer);
	free(command);
	close(sockfd);
	exit(0);
}