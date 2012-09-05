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


#include <ccn/ccn.h>
#include <ccn/uri.h>
#include <ccn/keystore.h>
#include <ccn/signing.h>
#include <ccn/schedule.h>
#include <ccn/hashtb.h>

#include "nlsr.h"
#include "nlsr_ndn.h"
#include "nlsr_lsdb.h"
#include "nlsr_adl.h"
#include "utility.h"


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
my_lock(void)
{
	nlsr->semaphor=1;
}

void 
my_unlock(void)
{
	nlsr->semaphor=0;
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
	
	nlsr->router_name->name=(char *)malloc(strlen(rtr_name)+1);
	memcpy(nlsr->router_name->name,rtr_name,strlen(rtr_name)+1);
	nlsr->router_name->length=strlen(rtr_name)+1;

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
	memcpy(np->name,name,strlen(name)+1);
	np->length=strlen(name)+1;
	
	add_name_prefix_to_npl(np);

	free(np);
	
		
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

	struct name_prefix *np=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
	np->name=(char *)malloc(strlen(rtr_name)+1);
	memcpy(np->name,rtr_name,strlen(rtr_name)+1);
	np->length=strlen(rtr_name)+1;

	add_adjacent_to_adl(np,face_id);

	free(np);
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
add_name_prefix_to_npl(struct name_prefix *np)
{

	
	printf("\nadd_name_prefix called\n");
	printf("Name Prefix: %s and length: %d \n",np->name,np->length);

	struct name_prefix *hnp=(struct name_prefix *)malloc(sizeof(struct name_prefix ));

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res;

   	hashtb_start(nlsr->npl, e);
    	res = hashtb_seek(e, np->name, np->length, 0);

	if(res == HT_NEW_ENTRY)
	{   

		hnp = e->data;
		hnp->length=np->length;
		hnp->name=(char *)malloc(np->length);
		memcpy(hnp->name,np->name,np->length);
	}
    	
	hashtb_end(e);


	printf("\n");

}

void
print_name_prefix_from_npl(void)
{
	printf("print_name_prefix_from_npl called \n");	
	int i, npl_element;
	struct name_prefix *np;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->npl, e);
	npl_element=hashtb_n(nlsr->npl);

	for(i=0;i<npl_element;i++)
	{
		np=e->data;
		printf("Name Prefix: %s and Length: %d \n",np->name,np->length);	
		hashtb_next(e);		
	}

	hashtb_end(e);

	printf("\n");
}


void 
nlsr_destroy( void )
{

	printf("Freeing Allocated Memory....\n");	
	/* Destroying every hash table attached to each neighbor in ADL before destorying ADL */	

	int i, element;
	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
	/*destroying NAME LSDB */
	struct nlsr *name_lsa;
	hashtb_start(nlsr->lsdb->name_lsdb, e);
	element=hashtb_n(nlsr->lsdb->name_lsdb);

	for(i=0;i<element;i++)
	{
		name_lsa=e->data;
		free(name_lsa);	
		hashtb_next(e);		
	}

	hashtb_end(e);    	
	hashtb_destroy(&nlsr->lsdb->name_lsdb);


	/*destroying ADJ LSDB */
	struct alsr *adj_lsa;
	hashtb_start(nlsr->lsdb->adj_lsdb, e);
	element=hashtb_n(nlsr->lsdb->adj_lsdb);

	for(i=0;i<element;i++)
	{
		adj_lsa=e->data;
		free(adj_lsa);	
		hashtb_next(e);		
	}

	hashtb_end(e);
	

	hashtb_destroy(&nlsr->lsdb->adj_lsdb);

	/* Destroying NPL */
	struct ccn_charbuf *np;
	hashtb_start(nlsr->npl, e);
	element=hashtb_n(nlsr->npl);

	for(i=0;i<element;i++)
	{
		np=e->data;	
		free(np);
		hashtb_next(e);		
	}
	hashtb_end(e);
	hashtb_destroy(&nlsr->npl);

	/* Destroying ADL */
	struct ndn_neighbor *nbr;
	hashtb_start(nlsr->adl, e);
	element=hashtb_n(nlsr->adl);

	for(i=0;i<element;i++)
	{
		nbr=e->data;
		free(nbr);	
		hashtb_next(e);		
	}
	hashtb_end(e);
	hashtb_destroy(&nlsr->adl);


	
	ccn_schedule_destroy(&nlsr->sched);
	ccn_destroy(&nlsr->ccn);
	free(nlsr);

	printf("Finished freeing allocated memory\n");

}

void
init_nlsr(void)
{
	struct hashtb_param param_adl = {0};
	struct hashtb_param param_npl = {0};

	struct hashtb_param param_adj_lsdb = {0};
	struct hashtb_param param_name_lsdb = {0};
	
	nlsr=(struct nlsr *)malloc(sizeof(struct nlsr));

	nlsr->adl=hashtb_create(sizeof(struct ndn_neighbor), &param_adl);
	nlsr->npl = hashtb_create(sizeof(struct name_prefix ), &param_npl);
	
	nlsr->in_interest.p = &incoming_interest;
	nlsr->in_content.p = &incoming_content;

	nlsr->lsdb=(struct linkStateDatabase *)malloc(sizeof(struct linkStateDatabase ));
	char *time_stamp=get_current_timestamp_micro();
	nlsr->lsdb->version=(char *)malloc(strlen(time_stamp)+1);
	memcpy(nlsr->lsdb->version,time_stamp,strlen(time_stamp)+1);
	memset(nlsr->lsdb->version,'0',strlen(time_stamp));
	

	nlsr->lsdb->adj_lsdb = hashtb_create(sizeof(struct alsa), &param_adj_lsdb);
	nlsr->lsdb->name_lsdb = hashtb_create(sizeof(struct nlsa), &param_name_lsdb);

	nlsr->router_name=(struct name_prefix *)malloc(sizeof(struct name_prefix ));

	nlsr->is_synch_init=1;
	nlsr->nlsa_id=0;
	nlsr->adj_build_flag=0;
	nlsr->adj_build_count=0;
	nlsr->is_build_adj_lsa_sheduled=0;
	nlsr->is_send_lsdb_interest_scheduled=0;	

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
	//int daemon_mode;
		

	init_nlsr();
    	
	while ((res = getopt_long(argc, argv, "df:h", longopts, 0)) != -1) 
	{
        	switch (res) 
		{
			case 'd':
				//daemon_mode = 1;
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
	res=ccn_name_from_uri(router_prefix,nlsr->router_name->name);		
	if(res<0)
		{
			fprintf(stderr, "Bad ccn URI: %s\n",nlsr->router_name->name);
			exit(1);
		}

	ccn_name_append_str(router_prefix,"nlsr");
	nlsr->in_interest.data=nlsr->router_name->name;
	res=ccn_set_interest_filter(nlsr->ccn,router_prefix,&nlsr->in_interest);
	if ( res < 0 )
		{
			fprintf(stderr,"Failed to register interest for router\n");
			exit(1);
		}

	/* Debugging purpose */	
	print_name_prefix_from_npl();
	print_adjacent_from_adl();
	
	printf("\n");
	printf("Router Name: %s\n",nlsr->router_name->name);
	printf("Time in MicroSec: %s \n",get_current_timestamp_micro());
	printf("LSDB Version: %s\n",nlsr->lsdb->version);
	printf("\n");

	build_and_install_name_lsas();
	print_name_lsdb();	

	nlsr->sched = ccn_schedule_create(nlsr, &ndn_rtr_ticker);
	nlsr->event_send_info_interest = ccn_schedule_event(nlsr->sched, 500, &send_info_interest, NULL, 0);	



	while(1)
	{
		if(nlsr->semaphor == 0)
		{	
			ccn_schedule_run(nlsr->sched);
        		res = ccn_run(nlsr->ccn, 500);
		}

	}
	
	nlsr_destroy();

	return 0;
}

