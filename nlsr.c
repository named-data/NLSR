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
	memcpy(nlsr->router_name,rtr_name,strlen(rtr_name)+1);	
	printf("Router Name: %s\n",nlsr->router_name);
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
	char *name_prefix;
	name_prefix=strtok_r(command,sep,&rem);
	if(name_prefix==NULL)
	{
		printf(" Wrong Command Format ( ccnname /name/prefix/ )\n");
		return;
	}

	printf("Name Prefix: %s \n",name_prefix);
}

void 
process_command_ccnneighbor(char *command)
{
	if(command==NULL)
	{
		printf(" Wrong Command Format ( ccnneighbor router_id faceX)\n");
		return;
	}
	char *rem;
	const char *sep=" \t\n";
	char *rtr_id,*face;

	rtr_id=strtok_r(command,sep,&rem);
	if(rtr_id==NULL)
	{
		printf(" Wrong Command Format ( ccnneighbor router_id faceX)\n");
		return;
	}

	face=strtok_r(NULL,sep,&rem);
	if(face==NULL)
	{
		printf(" Wrong Command Format ( ccnneighbor router_id faceX)\n");
		return;
	}

	printf("Router: %s face: %s\n",rtr_id,face);

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

int 
main(int argc, char *argv[])
{
    	int res;
    	char *config_file;
	int daemon_mode;
	struct hashtb_param param_adl = {0};
	struct hashtb_param param_npl = {0};
	
	nlsr=(struct nlsr *)malloc(sizeof(struct nlsr));

	nlsr->adl=hashtb_create(200, &param_adl);
	nlsr->npl = hashtb_create(200, &param_npl);
	nlsr->in_interest.p = &incoming_interest;
	nlsr->in_content.p = &incoming_content;
	nlsr->is_synch_init=1;

	struct ccn_charbuf *router_prefix;	
    	
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

	nlsr->sched = ccn_schedule_create(nlsr, &ndn_rtr_ticker);

	while(1)
	{
		ccn_schedule_run(nlsr->sched);
        	res = ccn_run(nlsr->ccn, 500);

	}

	
	
	hashtb_destroy(&nlsr->adl);
	hashtb_destroy(&nlsr->npl);
	ccn_schedule_destroy(&nlsr->sched);
	ccn_destroy(&nlsr->ccn);
	free(nlsr);

	return 0;
}

