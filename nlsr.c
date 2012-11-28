#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <assert.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ccn/ccn.h>
#include <ccn/uri.h>
#include <ccn/keystore.h>
#include <ccn/signing.h>
#include <ccn/schedule.h>
#include <ccn/hashtb.h>

#include "nlsr.h"
#include "nlsr_ndn.h"
#include "nlsr_lsdb.h"
#include "utility.h"
#include "nlsr_npl.h"
#include "nlsr_adl.h"
#include "nlsr_npt.h"
#include "nlsr_route.h"


#define ON_ERROR_DESTROY(resval) \
{           \
    if ((resval) < 0) { \
        nlsr_destroy(); \
	exit(1);\
    } \
}


#define ON_ERROR_EXIT(resval) \
{           \
    if ((resval) < 0) { \
        exit(1); \
    } \
}

struct option longopts[] =
{
    { "daemon",      no_argument,       NULL, 'd'},
    { "config_file", required_argument, NULL, 'f'},
    { "api_port",    required_argument, NULL, 'p'},
    { "help",        no_argument,       NULL, 'h'},
    { 0 }
};

static int 
usage(char *progname)
{

    printf("Usage: %s [OPTIONS...]\n\
	NDN routing....\n\
	-d, --daemon        Run in daemon mode\n\
	-f, --config_file   Specify configuration file name\n\
	-p, --api_port      port where api client will connect\n\
	-h, --help          Display this help message\n", progname);

    exit(1);
}

void ndn_rtr_gettime(const struct ccn_gettime *self, struct ccn_timeval *result)
{
    struct timeval now = {0};
    gettimeofday(&now, 0);
    result->s = now.tv_sec;
    result->micros = now.tv_usec;
}

static struct ccn_gettime ndn_rtr_ticker = {
    "timer",
    &ndn_rtr_gettime,
    1000000,
    NULL
};

void
nlsr_lock(void)
{
	nlsr->semaphor=NLSR_LOCKED;
}

void
nlsr_unlock(void)
{
	nlsr->semaphor=NLSR_UNLOCKED;
}

void 
nlsr_stop_signal_handler(int sig)
{
	signal(sig, SIG_IGN);
 	nlsr_destroy();
	exit(0);	
}

void  
daemonize_nlsr(void)
{
	int ret;
	pid_t process_id = 0;
	pid_t sid = 0;
	process_id = fork();
	if (process_id < 0)
	{
		printf("Daemonization failed!\n");
		ON_ERROR_DESTROY(process_id);
	}
	if (process_id > 0)
	{
		printf("Process daemonized. Process id: %d \n", process_id);
		ret=process_id;
		exit(0);
	}
	
	umask(0);
	sid = setsid();
	if(sid < 0)
	{
		ON_ERROR_DESTROY(sid);		
	}
	
	chdir("/");	
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
}

void 
process_command_ccnneighbor(char *command)
{
	if(command==NULL)
	{
		printf(" Wrong Command Format ( ccnneighbor router_name faceX)\n");
		return;
	}
	char *rem;
	const char *sep=" \t\n";
	char *rtr_name,*face;

	rtr_name=strtok_r(command,sep,&rem);
	if(rtr_name==NULL)
	{
		printf(" Wrong Command Format ( ccnneighbor router_name faceX)\n");
		return;
	}

	face=strtok_r(NULL,sep,&rem);
	if(face==NULL)
	{
		printf(" Wrong Command Format ( ccnneighbor router_name faceX)\n");
		return;
	}

	printf("Router: %s face: %s\n",rtr_name,face);
	int face_id;
	int res;
	res=sscanf(face,"face%d",&face_id);

	if(res != 1 )
	{
		printf(" Wrong Command Format ( ccnneighbor router_name faceX) where X is integer\n");
		return;
	}

	if ( rtr_name[strlen(rtr_name)-1] == '/' )
	{
		rtr_name[strlen(rtr_name)-1]='\0';
	}
	struct name_prefix *nbr=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
	nbr->name=(char *)malloc(strlen(rtr_name)+1);
	memset(nbr->name,0,strlen(rtr_name)+1);
	memcpy(nbr->name,rtr_name,strlen(rtr_name)+1);
	nbr->length=strlen(rtr_name)+1;

	add_nbr_to_adl(nbr,face_id);

	free(nbr->name);
	free(nbr);
}

void 
process_command_ccnname(char *command)
{

	if(command==NULL)
	{
		printf(" Wrong Command Format ( ccnname /name/prefix)\n");
		return;
	}
	char *rem;
	const char *sep=" \t\n";
	char *name;
	name=strtok_r(command,sep,&rem);
	if(name==NULL)
	{
		printf(" Wrong Command Format ( ccnname /name/prefix/ )\n");
		return;
	}

	printf("Name Prefix: %s \n",name);

	if ( name[strlen(name)-1] == '/' )
		name[strlen(name)-1]='\0';

	struct name_prefix *np=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
	np->name=(char *)malloc(strlen(name)+1);
	memset(np->name,0,strlen(name)+1);
	memcpy(np->name,name,strlen(name)+1);
	np->length=strlen(name)+1;

	add_name_to_npl(np);

	free(np->name);
	free(np);
}


void 
process_command_router_name(char *command)
{
	if(command==NULL)
	{
		printf(" Wrong Command Format ( router-name /router/name )\n");
		return;
	}
	char *rem;
	const char *sep=" \t\n";
	char *rtr_name;

	rtr_name=strtok_r(command,sep,&rem);
	if(rtr_name==NULL)
	{
		printf(" Wrong Command Format ( router-name /router/name )\n");
		return;
	}
	

	if ( rtr_name[strlen(rtr_name)-1] == '/' )
		rtr_name[strlen(rtr_name)-1]='\0';

	nlsr->router_name=(char *)malloc(strlen(rtr_name)+1);
	memset(nlsr->router_name,0,strlen(rtr_name)+1);
	memcpy(nlsr->router_name,rtr_name,strlen(rtr_name)+1);


}

void 
process_command_lsdb_synch_interval(char *command)
{
	if(command==NULL)
	{
		printf(" Wrong Command Format ( lsdb-synch-interval secs )\n");
		return;
	}
	char *rem;
	const char *sep=" \t\n";
	char *secs;
	long int seconds;
	
	secs=strtok_r(command,sep,&rem);
	if(secs==NULL)
	{
		printf(" Wrong Command Format ( lsdb-synch-interval secs)\n");
		return;
	}

	seconds=atoi(secs);
	if ( seconds >= 120 && seconds <= 3600 )
	{
		nlsr->lsdb_synch_interval=seconds;
	}

}


void 
process_command_interest_retry(char *command)
{
	if(command==NULL)
	{
		printf(" Wrong Command Format ( interest-retry number )\n");
		return;
	}
	char *rem;
	const char *sep=" \t\n";
	char *retry;
	long int retry_number;
	
	retry=strtok_r(command,sep,&rem);
	if(retry==NULL)
	{
		printf(" Wrong Command Format ( interest-retry number)\n");
		return;
	}

	retry_number=atoi(retry);
	if ( retry_number >= 1 && retry_number<=10 )
	{
		nlsr->interest_retry=retry_number;
	}

}

void 
process_command_interest_resend_time(char *command)
{
	if(command==NULL)
	{
		printf(" Wrong Command Format ( interest-resend-time secs )\n");
		return;
	}
	char *rem;
	const char *sep=" \t\n";
	char *secs;
	long int seconds;
	
	secs=strtok_r(command,sep,&rem);
	if(secs==NULL)
	{
		printf(" Wrong Command Format ( interest-resend-time secs)\n");
		return;
	}

	seconds=atoi(secs);
	if ( seconds <= 60 && seconds >= 1 )
	{
		nlsr->interest_resend_time=seconds;
	}
}


void 
process_command_lsa_refresh_time(char *command)
{
	if(command==NULL)
	{
		printf(" Wrong Command Format ( lsa-refresh-time secs )\n");
		return;
	}
	char *rem;
	const char *sep=" \t\n";
	char *secs;
	long int seconds;
	
	secs=strtok_r(command,sep,&rem);
	if(secs==NULL)
	{
		printf(" Wrong Command Format ( lsa-refresh-time secs)\n");
		return;
	}

	seconds=atoi(secs);
	if ( seconds >= 240 && seconds <= 3600 )
	{
		nlsr->lsa_refresh_time=seconds;
	}

}

void 
process_command_router_dead_interval(char *command)
{
	if(command==NULL)
	{
		printf(" Wrong Command Format ( router-dead-interval secs )\n");
		return;
	}
	char *rem;
	const char *sep=" \t\n";
	char *secs;
	long int seconds;
	
	secs=strtok_r(command,sep,&rem);
	if(secs==NULL)
	{
		printf(" Wrong Command Format ( router-dead-interval secs)\n");
		return;
	}

	seconds=atoi(secs);
	if ( seconds >= 360 && seconds <= 5400 )
	{
		nlsr->router_dead_interval=seconds;
	}

}

void 
process_command_multi_path_face_num(char *command)
{
	if(command==NULL)
	{
		printf(" Wrong Command Format ( multi-path-face-num n )\n");
		return;
	}
	char *rem;
	const char *sep=" \t\n";
	char *num;
	long int number;
	
	num=strtok_r(command,sep,&rem);
	if(num==NULL)
	{
		printf(" Wrong Command Format ( multi-path-face-num n)\n");
		return;
	}

	number=atoi(num);
	if ( number >= 0 && number <= 60 )
	{
		nlsr->multi_path_face_num=number;
	}

}

void 
process_command_logdir(char *command)
{
	if(command==NULL)
	{
		printf(" Wrong Command Format ( logdir /path/to/logdir )\n");
		return;
	}
	char *rem;
	const char *sep=" \t\n";
	char *dir;

	dir=strtok_r(command,sep,&rem);
	if(dir==NULL)
	{
		printf(" Wrong Command Format ( logdir /path/to/logdir/ )\n");
		return;
	}
	
	nlsr->logDir=(char *)malloc(strlen(dir)+1);
	memset(nlsr->logDir,0,strlen(dir)+1);
	memcpy(nlsr->logDir,dir,strlen(dir));
}

void 
process_command_detailed_log(char *command)
{
	if(command==NULL)
	{
		printf(" Wrong Command Format ( detailed-log on/off )\n");
		return;
	}
	char *rem;
	const char *sep=" \t\n";
	char *on_off;

	on_off=strtok_r(command,sep,&rem);
	if(on_off==NULL)
	{
		printf(" Wrong Command Format ( detailed-log on/off )\n");
		return;
	}
	
	if ( strcmp(on_off,"ON") == 0  || strcmp(on_off,"on") == 0)
	{
		nlsr->detailed_logging=1;
	}
}

void 
process_command_debug(char *command)
{
	if(command==NULL)
	{
		printf(" Wrong Command Format ( debug on/off )\n");
		return;
	}
	char *rem;
	const char *sep=" \t\n";
	char *on_off;

	on_off=strtok_r(command,sep,&rem);
	if(on_off==NULL)
	{
		printf(" Wrong Command Format ( debug on/off )\n");
		return;
	}
	
	if ( strcmp(on_off,"ON") == 0 || strcmp(on_off,"on") == 0 )
	{
		nlsr->debugging=1;
	}
}

void 
process_conf_command(char *command)
{
	const char *separators=" \t\n";
	char *remainder=NULL;
	char *cmd_type=NULL;

	if(command==NULL || strlen(command)==0 || command[0]=='!')
		return;	

	cmd_type=strtok_r(command,separators,&remainder);

	if(!strcmp(cmd_type,"router-name") )
	{
		process_command_router_name(remainder);
	}
	else if(!strcmp(cmd_type,"ccnneighbor") )
	{
		process_command_ccnneighbor(remainder);
	} 
	else if(!strcmp(cmd_type,"ccnname") )
	{
		process_command_ccnname(remainder);
	}
	else if(!strcmp(cmd_type,"lsdb-synch-interval") )
	{
		process_command_lsdb_synch_interval(remainder);
	}
	else if(!strcmp(cmd_type,"interest-retry") )
	{
		process_command_interest_retry(remainder);
	}
	else if(!strcmp(cmd_type,"interest-resend-time") )
	{
		process_command_interest_resend_time(remainder);
	}
	else if(!strcmp(cmd_type,"lsa-refresh-time") )
	{
		process_command_lsa_refresh_time(remainder);
	}
	else if(!strcmp(cmd_type,"router-dead-interval") )
	{
		process_command_router_dead_interval(remainder);
	}
	else if(!strcmp(cmd_type,"multi-path-face-num") )
	{
		process_command_multi_path_face_num(remainder);
	}
	else if(!strcmp(cmd_type,"logdir") )
	{
			process_command_logdir(remainder);
	}
	else if(!strcmp(cmd_type,"detailed-log") )
	{
			process_command_detailed_log(remainder);
	}
	else if(!strcmp(cmd_type,"debug") )
	{
			process_command_debug(remainder);
	}
	else 
	{
		printf("Wrong configuration Command %s \n",cmd_type);
	}
}


int 
readConfigFile(const char *filename)
{
	FILE *cfg;
	char buf[1024];
	int len;

	cfg=fopen(filename, "r");

	if(cfg == NULL)
	{
		printf("\nConfiguration File does not exists\n");
		exit(1);	
	}

	while(fgets((char *)buf, sizeof(buf), cfg))
	{
		len=strlen(buf);
		if(buf[len-1] == '\n')
		buf[len-1]='\0';
		if ( buf[0] != '#' && buf[0] != '!') 
			process_conf_command(buf);	
	}

	fclose(cfg);

	return 0;
}

char *
process_api_client_command(char *command)
{
	char *msg;
	msg=(char *)malloc(100);	
	memset(msg,100,0);
	//strcpy(msg,"Action Carried Out for NLSR Api Client");

	const char *sep=" \t\n";
	char *rem=NULL;
	char *cmd_type=NULL;
	char *op_type=NULL;
	char *name=NULL;
	char *face=NULL;
	int face_id;
	int res;

	op_type=strtok_r(command,sep,&rem);
	cmd_type=strtok_r(NULL,sep,&rem);
	name=strtok_r(NULL,sep,&rem);
	if ( name[strlen(name)-1] == '/' )
		name[strlen(name)-1]='\0';

	struct name_prefix *np=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
	np->name=(char *)malloc(strlen(name)+1);
	memset(np->name,0,strlen(name)+1);
	memcpy(np->name,name,strlen(name)+1);
	np->length=strlen(name)+1;

	if ( strcmp(cmd_type,"name")!= 0 )
	{
		face=strtok_r(NULL,sep,&rem);
		sscanf(face,"face%d",&face_id);
	}
	
	if ( strcmp(cmd_type,"name")== 0 )
	{
		if ( strcmp(op_type,"del") == 0 ) 
		{
			res=does_name_exist_in_npl(np);
			if ( res == 0)
			{
				sprintf(msg,"Name %s does not exist !!",name);
			}
			else
			{
				long int ls_id=get_lsa_id_from_npl(np);
				if ( ls_id != 0 )
				{
					make_name_lsa_invalid(np,LS_TYPE_NAME,ls_id);
					sprintf(msg,"Name %s has been deleted and Advertised.",name);
				}
				else 
				{
					sprintf(msg,"Name %s does not have an Name LSA yet !!",name);
				}
			}			
		}
		else if ( strcmp(op_type,"add") == 0 )
		{
			res=does_name_exist_in_npl(np);
			if ( res == 0)
			{
				add_name_to_npl(np);
				build_and_install_single_name_lsa(np);
				sprintf(msg,"Name %s has been added to advertise.",name);
			}
			else
			{
				sprintf(msg,"Name %s has already been advertised from this router !!",name);
			}
		} 
	}
	else if ( strcmp(cmd_type,"neighbor") == 0 )
	{
		if ( strcmp(op_type,"del") == 0 ) 
		{
			res=is_neighbor(np->name);
			if ( res == 0)
			{
				sprintf(msg,"Neighbor %s does not exist !!",name);
			}
			else
			{
				update_adjacent_status_to_adl(np,NBR_DOWN);
				delete_nbr_from_adl(np);
				if(!nlsr->is_build_adj_lsa_sheduled)
				{
					nlsr->event_build_adj_lsa = ccn_schedule_event(nlsr->sched, 1000, &build_and_install_adj_lsa, NULL, 0);
					nlsr->is_build_adj_lsa_sheduled=1;		
				}
				sprintf(msg,"Neighbor %s has been deleted from adjacency list.",name);	
			}
		}
		else if ( strcmp(op_type,"add") == 0 )
		{
			res=is_neighbor(np->name);
			if ( res == 0 )
			{
				add_nbr_to_adl(np,face_id);
				sprintf(msg,"Neighbor %s has been added to adjacency list.",name);
			}
			else
			{
				sprintf(msg,"Neighbor %s already exists in adjacency list.",name);
			}
		}
	}
		

	return msg;
}

int
nlsr_api_server_poll(long int time_out_micro_sec, int ccn_fd)
{
	struct timeval timeout;
	if ( time_out_micro_sec < 0 )
	{
		timeout.tv_sec=1;
		timeout.tv_usec=0;
	}
	else
	{
		time_out_micro_sec=(long int)time_out_micro_sec*0.4;
		timeout.tv_sec=time_out_micro_sec / 1000000;
		timeout.tv_usec=time_out_micro_sec % 1000000;
	}

	
	int fd;
	int nread;
	int result;
	fd_set testfds;
	unsigned int client_len;
	int client_sockfd;
	char recv_buffer[1024];
	bzero(recv_buffer,1024);
	struct sockaddr_in client_address;

	testfds=nlsr->readfds;
	result = select(FD_SETSIZE, &testfds, NULL,NULL, &timeout);
	
	for(fd = 0; fd < FD_SETSIZE; fd++) 
	{
		if(FD_ISSET(fd,&testfds)) 
		{
			if ( fd == ccn_fd )
			{
				return 0;
			}			
			else if(fd == nlsr->nlsr_api_server_sock_fd)
			{
				client_len = sizeof(client_address);
				client_sockfd = accept(nlsr->nlsr_api_server_sock_fd,(struct sockaddr *)&client_address, &client_len);
				FD_SET(client_sockfd, &nlsr->readfds);
			}
			else
			{   
					
				ioctl(fd, FIONREAD, &nread);
				if(nread == 0) 
				{
					close(fd);
					FD_CLR(fd, &nlsr->readfds);
				}
				else 
				{
					recv(fd, recv_buffer, 1024, 0);
					printf("Received Data from NLSR API cleint: %s \n",recv_buffer);
					char *msg=process_api_client_command(recv_buffer);
					send(fd, msg, strlen(msg),0);
					free(msg);
					close(fd);
					FD_CLR(fd, &nlsr->readfds);
				}
			}
		}
	}

	return 0;
}

void 
nlsr_destroy( void )
{
	if ( nlsr->debugging )
	{
		printf("Freeing Allocated Memory....\n");
	}	
	writeLogg(__FILE__,__FUNCTION__,__LINE__,"Freeing Allocated Memory....\n");	
	/* Destroying all face created by nlsr in CCND */
	destroy_all_face_by_nlsr();	

	/* Destroying every hash table attached to each neighbor in ADL before destorying ADL */	
	hashtb_destroy(&nlsr->npl);
	hashtb_destroy(&nlsr->adl);	
	hashtb_destroy(&nlsr->lsdb->name_lsdb);
	hashtb_destroy(&nlsr->lsdb->adj_lsdb);
	hashtb_destroy(&nlsr->pit_alsa);

	//To Do: has to destroy the face_list one by one 	

	hashtb_destroy(&nlsr->routing_table);


	int i, npt_element;
	struct npt_entry *ne;
	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	hashtb_start(nlsr->npt, e);
	npt_element=hashtb_n(nlsr->npt);
	for(i=0;i<npt_element;i++)
	{
		ne=e->data;
		hashtb_destroy(&ne->name_list);
		hashtb_destroy(&ne->face_list);	
		hashtb_next(e);		
	}

	hashtb_end(e);
	hashtb_destroy(&nlsr->npt);


	close(nlsr->nlsr_api_server_sock_fd);	

	ccn_schedule_destroy(&nlsr->sched);
	ccn_destroy(&nlsr->ccn);

	free(nlsr->lsdb->lsdb_version);
	free(nlsr->lsdb);
	free(nlsr->router_name);
	free(nlsr);
	if ( nlsr->debugging )
	{
		printf("Finished freeing allocated memory\n");
	}
	writeLogg(__FILE__,__FUNCTION__,__LINE__,"Finished freeing allocated memory\n");

}


void
init_api_server(int ccn_fd)
{
	int server_sockfd;
	int server_len;
	struct sockaddr_in server_address;
	unsigned int yes=1;	

	server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

	int flags = fcntl(server_sockfd, F_GETFL, 0);
	fcntl(server_sockfd, F_SETFL, O_NONBLOCK|flags);

	if (setsockopt(server_sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0) 
	{
       		ON_ERROR_DESTROY(-1);
       	}

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_address.sin_port = nlsr->api_port;

	server_len = sizeof(server_address);
	bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
	listen(server_sockfd, 100);
	FD_ZERO(&nlsr->readfds);
	FD_SET(server_sockfd, &nlsr->readfds);
	FD_SET(ccn_fd, &nlsr->readfds);
	nlsr->nlsr_api_server_sock_fd=server_sockfd;

}

int 
init_nlsr(void)
{
	if (signal(SIGQUIT, nlsr_stop_signal_handler ) == SIG_ERR) 
	{
		perror("SIGQUIT install error\n");
		return -1;
	}
	if (signal(SIGTERM, nlsr_stop_signal_handler ) == SIG_ERR) 
	{
		perror("SIGTERM install error\n");
		return -1;
    	}
 	if (signal(SIGINT, nlsr_stop_signal_handler ) == SIG_ERR)
	{
		perror("SIGTERM install error\n");
		return -1;
	}

	nlsr=(struct nlsr *)malloc(sizeof(struct nlsr));
	
	struct hashtb_param param_adl = {0};
	nlsr->adl=hashtb_create(sizeof(struct ndn_neighbor), &param_adl);
	struct hashtb_param param_npl = {0};
	nlsr->npl = hashtb_create(sizeof(struct name_prefix_list_entry), &param_npl);
	struct hashtb_param param_pit_alsa = {0};	
	nlsr->pit_alsa = hashtb_create(sizeof(struct pending_interest), &param_pit_alsa);
	struct hashtb_param param_npt = {0};	
	nlsr->npt = hashtb_create(sizeof(struct npt_entry), &param_npt);
	struct hashtb_param param_rte = {0};	
	nlsr->routing_table = hashtb_create(sizeof(struct routing_table_entry), &param_rte);

	nlsr->in_interest.p = &incoming_interest;
	nlsr->in_content.p = &incoming_content;

	nlsr->lsdb=(struct linkStateDatabase *)malloc(sizeof(struct linkStateDatabase));

	char *time_stamp=(char *)malloc(20);
	memset(time_stamp,0,20);
	get_current_timestamp_micro(time_stamp);
	nlsr->lsdb->lsdb_version=(char *)malloc(strlen(time_stamp)+1);
	memset(nlsr->lsdb->lsdb_version,'0',strlen(time_stamp));
	free(time_stamp);
	
	struct hashtb_param param_adj_lsdb = {0};
	nlsr->lsdb->adj_lsdb = hashtb_create(sizeof(struct alsa), &param_adj_lsdb);
	struct hashtb_param param_name_lsdb = {0};
	nlsr->lsdb->name_lsdb = hashtb_create(sizeof(struct nlsa), &param_name_lsdb);
	
	


	nlsr->is_synch_init=1;
	nlsr->nlsa_id=0;
	nlsr->adj_build_flag=0;
	nlsr->adj_build_count=0;
	nlsr->is_build_adj_lsa_sheduled=0;
	nlsr->is_send_lsdb_interest_scheduled=0;
	nlsr->is_route_calculation_scheduled=0;	

	nlsr->detailed_logging=0;
	nlsr->debugging=0;

	nlsr->lsdb_synch_interval = LSDB_SYNCH_INTERVAL;
	nlsr->interest_retry = INTEREST_RETRY;
	nlsr->interest_resend_time = INTEREST_RESEND_TIME;
	nlsr->lsa_refresh_time=LSA_REFRESH_TIME;
	nlsr->router_dead_interval=ROUTER_DEAD_INTERVAL;
	nlsr->multi_path_face_num=MULTI_PATH_FACE_NUM;
	nlsr->semaphor=NLSR_UNLOCKED;

	nlsr->api_port=API_PORT;

	return 0;
}


int 
main(int argc, char *argv[])
{
    	int res, ret;
    	char *config_file;
	int daemon_mode=0;
	int port=0;

	

	while ((res = getopt_long(argc, argv, "df:p:h", longopts, 0)) != -1) 
	{
        	switch (res) 
		{
			case 'd':
				daemon_mode = 1;
				break;
			case 'f':
				config_file = optarg;
				break;
			case 'p':
				port = atoi(optarg);
				break;
			case 'h':
			default:
				usage(argv[0]);
		}
    	}

	ret=init_nlsr();	
    	ON_ERROR_EXIT(ret);

	if ( port !=0 )
		nlsr->api_port=port;

	readConfigFile(config_file);
	if ( daemon_mode == 1 )
	{
		daemonize_nlsr();
	}
	
	startLogging(nlsr->logDir);
	
	nlsr->ccn=ccn_create();
	int ccn_fd=ccn_connect(nlsr->ccn, NULL);
	if(ccn_fd == -1)
	{
		fprintf(stderr,"Could not connect to ccnd\n");
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"Could not connect to ccnd\n");
		ON_ERROR_DESTROY(-1);
	}

	init_api_server(ccn_fd);
	
	struct ccn_charbuf *router_prefix;	
	router_prefix=ccn_charbuf_create(); 
	res=ccn_name_from_uri(router_prefix,nlsr->router_name);		
	if(res<0)
	{
		fprintf(stderr, "Bad ccn URI: %s\n",nlsr->router_name);
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"Bad ccn URI: %s\n",nlsr->router_name);
		ON_ERROR_DESTROY(res);
	}

	ccn_name_append_str(router_prefix,"nlsr");
	nlsr->in_interest.data=nlsr->router_name;
	res=ccn_set_interest_filter(nlsr->ccn,router_prefix,&nlsr->in_interest);
	if ( res < 0 )
	{
		fprintf(stderr,"Failed to register interest for router\n");
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"Failed to register interest for router\n");
		ON_ERROR_DESTROY(res);
	}
	ccn_charbuf_destroy(&router_prefix);
	
	if ( nlsr->debugging )
		printf("Router Name : %s\n",nlsr->router_name);
	writeLogg(__FILE__,__FUNCTION__,__LINE__,"Router Name : %s\n",nlsr->router_name);
	if ( nlsr->debugging )
		printf("lsdb_version: %s\n",nlsr->lsdb->lsdb_version);
	writeLogg(__FILE__,__FUNCTION__,__LINE__,"lsdb_version: %s\n",nlsr->lsdb->lsdb_version);

	print_name_prefix_from_npl();
	print_adjacent_from_adl();
	build_and_install_name_lsas();
	print_name_lsdb();	

	nlsr->sched = ccn_schedule_create(nlsr, &ndn_rtr_ticker);
	nlsr->event_send_info_interest = ccn_schedule_event(nlsr->sched, 1, &send_info_interest, NULL, 0);
	nlsr->event = ccn_schedule_event(nlsr->sched, 60000000, &refresh_lsdb, NULL, 0);

	
	while(1)
	{	
		if ( nlsr->semaphor == NLSR_UNLOCKED  )
		{
			if( nlsr->sched != NULL )
			{
				long int micro_sec=ccn_schedule_run(nlsr->sched);
				res=nlsr_api_server_poll(micro_sec,ccn_fd);
				ON_ERROR_DESTROY(res);
			}
			if(nlsr->ccn != NULL)
			{
        			res = ccn_run(nlsr->ccn, 0);
			}
			if (!(nlsr->sched && nlsr->ccn))
			{	      
				break;
			}
		}

	}
	

	return 0;
}

