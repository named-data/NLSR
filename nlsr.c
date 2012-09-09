#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>
#include <assert.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <sys/types.h>
#include <signal.h>



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



struct option longopts[] =
{
    { "daemon",      no_argument,       NULL, 'd'},
    { "config_file", required_argument, NULL, 'f'},
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
nlsr_stop_signal_handler(int sig)
{
	signal(sig, SIG_IGN);
 	nlsr_destroy();	
	//exit(0);
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
	nlsr->lsdb_synch_interval=seconds;

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
	char *secs;
	long int seconds;
	
	secs=strtok_r(command,sep,&rem);
	if(secs==NULL)
	{
		printf(" Wrong Command Format ( interest-retry number)\n");
		return;
	}

	seconds=atoi(secs);
	nlsr->interest_retry=seconds;

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
	nlsr->interest_resend_time=seconds;

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
		process_conf_command(buf);	
	}

	fclose(cfg);

	return 0;
}

void 
nlsr_destroy( void )
{

	printf("Freeing Allocated Memory....\n");	
	
	/* Destroying every hash table attached to each neighbor in ADL before destorying ADL */	
	hashtb_destroy(&nlsr->npl);
	hashtb_destroy(&nlsr->adl);	
	hashtb_destroy(&nlsr->lsdb->name_lsdb);
	hashtb_destroy(&nlsr->lsdb->adj_lsdb);
	hashtb_destroy(&nlsr->pit_alsa);
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
		hashtb_destroy(&ne->next_hop_table);	
		hashtb_next(e);		
	}

	hashtb_end(e);
	hashtb_destroy(&nlsr->npt);

	
	ccn_schedule_destroy(&nlsr->sched);
	ccn_destroy(&nlsr->ccn);

	free(nlsr->lsdb->lsdb_version);
	free(nlsr->lsdb);
	free(nlsr->router_name);
	free(nlsr);

	printf("Finished freeing allocated memory\n");

}


void
init_nlsr(void)
{
	if (signal(SIGQUIT, nlsr_stop_signal_handler ) == SIG_ERR) 
	{
		perror("SIGQUIT install error\n");
		exit(1);
	}
	if (signal(SIGTERM, nlsr_stop_signal_handler ) == SIG_ERR) 
	{
		perror("SIGTERM install error\n");
		exit(1);
    	}
 	if (signal(SIGINT, nlsr_stop_signal_handler ) == SIG_ERR)
	{
		perror("SIGTERM install error\n");
		exit(1);
	}

	nlsr=(struct nlsr *)malloc(sizeof(struct nlsr));
	
	struct hashtb_param param_adl = {0};
	nlsr->adl=hashtb_create(sizeof(struct ndn_neighbor), &param_adl);
	struct hashtb_param param_npl = {0};
	nlsr->npl = hashtb_create(sizeof(struct name_prefix), &param_npl);
	struct hashtb_param param_pit_alsa = {0};	
	nlsr->pit_alsa = hashtb_create(sizeof(struct pneding_interest), &param_pit_alsa);
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

	nlsr->lsdb_synch_interval = LSDB_SYNCH_INTERVAL;
	nlsr->interest_retry = INTEREST_RETRY;
	nlsr->interest_resend_time = INTEREST_RESEND_TIME;

	nlsr->semaphor=0;
}


int 
main(int argc, char *argv[])
{
    	int res;
    	char *config_file;
	int daemon_mode;

	init_nlsr();	
    	
	while ((res = getopt_long(argc, argv, "df:h", longopts, 0)) != -1) 
	{
        	switch (res) 
		{
			case 'd':
				daemon_mode = 1;
				break;
			case 'f':
				config_file = optarg;
				break;
			case 'h':
			default:
				usage(argv[0]);
		}
    	}

	readConfigFile(config_file);

	nlsr->ccn=ccn_create();
	if(ccn_connect(nlsr->ccn, NULL) == -1)
	{
		fprintf(stderr,"Could not connect to ccnd\n");
		exit(1);
	}
	struct ccn_charbuf *router_prefix;	
	router_prefix=ccn_charbuf_create(); 
	res=ccn_name_from_uri(router_prefix,nlsr->router_name);		
	if(res<0)
	{
		fprintf(stderr, "Bad ccn URI: %s\n",nlsr->router_name);
		exit(1);
	}

	ccn_name_append_str(router_prefix,"nlsr");
	nlsr->in_interest.data=nlsr->router_name;
	res=ccn_set_interest_filter(nlsr->ccn,router_prefix,&nlsr->in_interest);
	if ( res < 0 )
	{
		fprintf(stderr,"Failed to register interest for router\n");
		exit(1);
	}
	ccn_charbuf_destroy(&router_prefix);
	
	printf("Router Name : %s\n",nlsr->router_name);
	printf("lsdb_version: %s\n",nlsr->lsdb->lsdb_version);

	print_name_prefix_from_npl();
	print_adjacent_from_adl();
	build_and_install_name_lsas();
	print_name_lsdb();	

	nlsr->sched = ccn_schedule_create(nlsr, &ndn_rtr_ticker);
	nlsr->event_send_info_interest = ccn_schedule_event(nlsr->sched, 1, &send_info_interest, NULL, 0);

	while(1)
	{	
		if( nlsr->sched != NULL )
		{
			ccn_schedule_run(nlsr->sched);
		}
		if(nlsr->ccn != NULL)
		{
        		res = ccn_run(nlsr->ccn, 500);
		}
		if (!(nlsr->sched && nlsr->ccn))
		{	      
			break;
		}

	}

	return 0;
}

