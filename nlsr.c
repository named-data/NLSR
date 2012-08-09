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

	struct name_prefix *np=(struct name_prefix *)malloc(sizeof(struct name_prefix *));
	np->name=(char *)malloc(strlen(name_prefix)+1);
	memcpy(np->name,name_prefix,strlen(name_prefix)+1);
	np->length=strlen(name_prefix)+1;

	add_name_prefix_to_npl(np);
	/* Debugging Purpose */
	print_name_prefix_from_npl();

	free(np->name);
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

	struct ndn_neighbor *nbr=(struct ndn_neighbor *)malloc(sizeof(struct ndn_neighbor*));
	nbr->neighbor=(struct name_prefix *)malloc(sizeof(struct name_prefix *));
	nbr->neighbor->name=(char *)malloc(strlen(rtr_name)+1);
	memcpy(nbr->neighbor->name,rtr_name,strlen(rtr_name)+1);
	nbr->neighbor->length=strlen(rtr_name)+1;
	nbr->face=face_id;
	nbr->status=0;	

	add_adjacent_to_adl(nbr);
	print_adjacent_from_adl();

	free(nbr->neighbor->name);
	free(nbr->neighbor);
	free(nbr);

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

void
add_name_prefix_to_npl(struct name_prefix *np)
{

	
	printf("\nadd_name_prefix called\n");
	printf("Name Prefix: %s and length: %d \n",np->name,np->length);

	struct name_prefix *hnp=(struct name_prefix *)malloc(sizeof(struct name_prefix *));

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res;

   	hashtb_start(nlsr->npl, e);
    	res = hashtb_seek(e, np->name, strlen(np->name), 0);
   
	hnp = e->data;
	hnp->name=(char *)malloc(np->length);
    	memcpy(hnp->name,np->name,np->length);
	hnp->length=np->length;

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
add_adjacent_to_adl(struct ndn_neighbor *nbr)
{
	printf("\nadd_adjacent_to_adl called\n");
	printf("Neighbor: %s Length: %d Face: %d Status: %d\n",nbr->neighbor->name,nbr->neighbor->length,nbr->face, nbr->status);

	struct ndn_neighbor *hnbr=(struct ndn_neighbor *)malloc(sizeof(struct ndn_neighbor*));
	
	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res;

   	hashtb_start(nlsr->adl, e);
    	res = hashtb_seek(e, nbr->neighbor->name , nbr->neighbor->length, 0);
   
	hnbr = e->data;

	hnbr->neighbor=(struct name_prefix *)malloc(sizeof(struct name_prefix *));
	hnbr->neighbor->name=(char *)malloc(nbr->neighbor->length);
	memcpy(hnbr->neighbor->name,nbr->neighbor->name,nbr->neighbor->length);
	hnbr->neighbor->length=nbr->neighbor->length;
	hnbr->face=nbr->face;
	hnbr->status=nbr->status;
	
    	hashtb_end(e);

	printf("\n");


}

void
print_adjacent_from_adl(void)
{
	printf("print_adjacent_from_adl called \n");	
	int i, adl_element;
	struct ndn_neighbor *nbr;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->adl, e);
	adl_element=hashtb_n(nlsr->adl);

	for(i=0;i<adl_element;i++)
	{
		nbr=e->data;
		printf("Neighbor: %s Length: %d Face: %d Status: %d\n",nbr->neighbor->name,nbr->neighbor->length,nbr->face, nbr->status);	
		hashtb_next(e);		
	}

	hashtb_end(e);

	printf("\n");
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

	nlsr->adl=hashtb_create(sizeof(struct ndn_neighbor), &param_adl);
	nlsr->npl = hashtb_create(sizeof(struct name_prefix), &param_npl);
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

