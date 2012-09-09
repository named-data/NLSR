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
#include "utility.h"
#include "nlsr_npl.h"
#include "nlsr_adl.h"
#include "nlsr_route.h"
#include "nlsr_npt.h"

void
set_new_lsdb_version(void)
{
	
	char *time_stamp=(char *)malloc(20);
	memset(time_stamp,0,20);
	get_current_timestamp_micro(time_stamp);
	
	free(nlsr->lsdb->lsdb_version);
	nlsr->lsdb->lsdb_version=(char *)malloc(strlen(time_stamp)+1);
	memset(nlsr->lsdb->lsdb_version,0,strlen(time_stamp)+1);
	memcpy(nlsr->lsdb->lsdb_version,time_stamp,strlen(time_stamp)+1);

	free(time_stamp);
	
}

void 
make_name_lsa_key(char *key, char *orig_router, int ls_type, long int ls_id)
{
	
	
	printf("Orig Router: %s LS Type: %d LS Id: %ld\n",orig_router,ls_type,ls_id);

	char lst[2];
	memset(lst,0,2);
	sprintf(lst,"%d",ls_type);	

	char lsid[10];
	memset(lsid,0,10);
	sprintf(lsid,"%ld",ls_id);
	
	memcpy(key+strlen(key),orig_router,strlen(orig_router));
	memcpy(key+strlen(key),"/",1);
	memcpy(key+strlen(key),lst,strlen(lst));
	memcpy(key+strlen(key),"/",1);
	memcpy(key+strlen(key),lsid,strlen(lsid));

	printf("Key: %s\n",key);
	
}

void 
build_and_install_name_lsas(void)
{
	printf("build_and_install_name_lsas called \n");

	int i, npl_element;
	struct name_prefix *np;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->npl, e);
	npl_element=hashtb_n(nlsr->npl);

	for(i=0;i<npl_element;i++)
	{
		np=e->data;
		struct nlsa *name_lsa=(struct nlsa *)malloc(sizeof( struct nlsa ));
		build_name_lsa(name_lsa,np);
		
		install_name_lsa(name_lsa);
		free(name_lsa->header->orig_router->name);
		free(name_lsa->header->orig_router);
		free(name_lsa->header);
		free(name_lsa->name_prefix->name);
		free(name_lsa->name_prefix);
		free(name_lsa);
		hashtb_next(e);		
	}

	hashtb_end(e);	

}

void 
build_name_lsa(struct nlsa *name_lsa, struct name_prefix *np)
{
	name_lsa->header=(struct nlsa_header *)malloc(sizeof(struct nlsa_header ));
	name_lsa->header->ls_type=LS_TYPE_NAME;

	char *time_stamp=(char *)malloc(20);
	memset(time_stamp,0,20);
	get_current_timestamp_micro(time_stamp);

	name_lsa->header->orig_time=(char *)malloc(strlen(time_stamp)+1); //free 
	memset(name_lsa->header->orig_time,0,strlen(time_stamp)+1);
	memcpy(name_lsa->header->orig_time,time_stamp,strlen(time_stamp)+1);
	
	free(time_stamp);

	name_lsa->header->ls_id=++nlsr->nlsa_id;
	name_lsa->header->orig_router=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
	name_lsa->header->orig_router->name=(char *)malloc(strlen(nlsr->router_name)+1);
	memset(name_lsa->header->orig_router->name,0,strlen(nlsr->router_name)+1);
	memcpy(name_lsa->header->orig_router->name,nlsr->router_name,strlen(nlsr->router_name)+1);
	name_lsa->header->orig_router->length=strlen(nlsr->router_name)+1;
	name_lsa->header->isValid=1;

	
	name_lsa->name_prefix=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
	name_lsa->name_prefix->name=(char *)malloc(np->length);
	memcpy(name_lsa->name_prefix->name,np->name,np->length);
	name_lsa->name_prefix->length=np->length;

}

void 
install_name_lsa(struct nlsa *name_lsa)
{
	
	char lst[2];
	memset(lst,0,2);
	sprintf(lst,"%d",name_lsa->header->ls_type);	

	char lsid[10];
	memset(lsid,0,10);
	sprintf(lsid,"%ld",name_lsa->header->ls_id);
	
	
	char *key=(char *)malloc(strlen(name_lsa->header->orig_router->name)+1+strlen(lst)+1+strlen(lsid)+1);
	memset(key,0,strlen(name_lsa->header->orig_router->name)+1+strlen(lst)+1+strlen(lsid)+1);


	make_name_lsa_key(key, name_lsa->header->orig_router->name,name_lsa->header->ls_type,name_lsa->header->ls_id);	
	printf("Key:%s Length:%d\n",key,(int)strlen(key));

	struct nlsa *new_name_lsa=(struct nlsa*)malloc(sizeof(struct nlsa )); //free

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res;

   	hashtb_start(nlsr->lsdb->name_lsdb, e);
    	res = hashtb_seek(e, key, strlen(key), 0);

	if(res == HT_NEW_ENTRY )
	{
		printf("New Name LSA... Adding to LSDB\n");
		new_name_lsa = e->data;

		new_name_lsa->header=(struct nlsa_header *)malloc(sizeof(struct nlsa_header )); //free
		new_name_lsa->header->ls_type=name_lsa->header->ls_type;

		new_name_lsa->header->orig_time=(char *)malloc(strlen(name_lsa->header->orig_time)+1);
		memset(new_name_lsa->header->orig_time,0,strlen(name_lsa->header->orig_time)+1);
		memcpy(new_name_lsa->header->orig_time,name_lsa->header->orig_time,strlen(name_lsa->header->orig_time)+1);

		new_name_lsa->header->ls_id=name_lsa->header->ls_id;
		new_name_lsa->header->orig_router=(struct name_prefix *)malloc(sizeof(struct name_prefix )); //free
		new_name_lsa->header->orig_router->name=(char *)malloc(name_lsa->header->orig_router->length);
		memcpy(new_name_lsa->header->orig_router->name,name_lsa->header->orig_router->name,name_lsa->header->orig_router->length);
		new_name_lsa->header->orig_router->length=name_lsa->header->orig_router->length;
		new_name_lsa->header->isValid=name_lsa->header->isValid;

	
		new_name_lsa->name_prefix=(struct name_prefix *)malloc(sizeof(struct name_prefix )); //free
		new_name_lsa->name_prefix->name=(char *)malloc(name_lsa->name_prefix->length);
		memcpy(new_name_lsa->name_prefix->name,name_lsa->name_prefix->name,name_lsa->name_prefix->length);
		new_name_lsa->name_prefix->length=name_lsa->name_prefix->length;

		printf("New Name LSA Added....\n");	

		printf("Old Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);
		set_new_lsdb_version();	
		printf("New Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);	

		int next_hop=get_next_hop(new_name_lsa->header->orig_router->name);
		if ( next_hop == NO_NEXT_HOP )
		{
			int check=add_npt_entry(new_name_lsa->header->orig_router->name,new_name_lsa->name_prefix->name,NO_FACE);
			if ( check == HT_NEW_ENTRY )
			{
				printf("Added in npt \n");
			}
		}
		else 
		{
			int check=add_npt_entry(new_name_lsa->header->orig_router->name,new_name_lsa->name_prefix->name,next_hop);
			if ( check == HT_NEW_ENTRY )
			{
				printf("Added in npt \n");
			}

		}

	}
	else if(res == HT_OLD_ENTRY)
	{
		printf("Duplicate Name LSA. Discarded...\n");

	}

    	hashtb_end(e);

	free(key);
}


void 
print_name_lsa(struct nlsa *name_lsa)
{
	
	printf("-----------Name LSA Content---------------\n");
	printf("	Origination Router       :	%s\n",name_lsa->header->orig_router->name);
	printf("	Origination Router Length:	%d\n",name_lsa->header->orig_router->length);
	printf("	LS Type			 :	%d\n",name_lsa->header->ls_type);
	printf("	LS Id			 :	%ld\n",name_lsa->header->ls_id);
	printf("	Origination Time	 :	%s\n",name_lsa->header->orig_time);
	printf("	Is Valid 		 :	%d\n",name_lsa->header->isValid);
	printf("	LSA Data			\n");
	printf("		Name Prefix:	 	:	%s\n",name_lsa->name_prefix->name);
	printf("		Name Prefix Length	:	%d\n",name_lsa->name_prefix->length);

	printf("\n");	
}

void
print_name_lsdb(void)
{
	printf("print_name_lsdb called \n");	
	int i, name_lsdb_element;
	struct nlsa *name_lsa;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->lsdb->name_lsdb, e);
	name_lsdb_element=hashtb_n(nlsr->lsdb->name_lsdb);

	for(i=0;i<name_lsdb_element;i++)
	{
		printf("-----------Name LSA (%d)---------------\n",i+1);
		name_lsa=e->data;
		print_name_lsa(name_lsa);	
		hashtb_next(e);		
	}

	hashtb_end(e);

	printf("\n");
}


void
build_and_install_others_name_lsa(char *orig_router,int ls_type,long int ls_id,char *orig_time, int isValid,char *np)
{
	printf("build_and_install_others_name_lsa called \n");

	struct nlsa *name_lsa=(struct nlsa *)malloc(sizeof( struct nlsa ));
	build_others_name_lsa(name_lsa,orig_router,ls_type,ls_id,orig_time, isValid,np);
	print_name_lsa(name_lsa);
	install_name_lsa(name_lsa);
	print_name_lsdb();

	
	free(name_lsa->header->orig_router->name);
	free(name_lsa->header->orig_router);
	free(name_lsa->header);
	free(name_lsa->name_prefix->name);
	free(name_lsa->name_prefix);
	free(name_lsa);
	
}

void
build_others_name_lsa(struct nlsa *name_lsa, char *orig_router,int ls_type,long int ls_id,char *orig_time, int isValid,char *np)
{
	printf("build_others_name_lsa called \n");

	name_lsa->header=(struct nlsa_header *)malloc(sizeof(struct nlsa_header ));
	name_lsa->header->ls_type=LS_TYPE_NAME;

	name_lsa->header->orig_time=(char *)malloc(strlen(orig_time)+1);
	memset(name_lsa->header->orig_time,0,strlen(orig_time)+1);
	memcpy(name_lsa->header->orig_time,orig_time,strlen(orig_time)+1);

	name_lsa->header->ls_id=ls_id;
	name_lsa->header->orig_router=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
	name_lsa->header->orig_router->name=(char *)malloc(strlen(orig_router)+1);
	memset(name_lsa->header->orig_router->name,0,strlen(orig_router)+1);
	memcpy(name_lsa->header->orig_router->name,orig_router,strlen(orig_router)+1);
	name_lsa->header->orig_router->length=strlen(orig_router)+1;
	name_lsa->header->isValid=isValid;

	name_lsa->name_prefix=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
	name_lsa->name_prefix->name=(char *)malloc(strlen(np)+1);
	memset(name_lsa->name_prefix->name,0,strlen(np)+1);
	memcpy(name_lsa->name_prefix->name,np,strlen(np)+1);
	name_lsa->name_prefix->length=strlen(np)+1;
}


void 
make_adj_lsa_key(char *key,struct alsa *adj_lsa)
{
	memcpy(key+strlen(key),adj_lsa->header->orig_router->name,adj_lsa->header->orig_router->length);
	memcpy(key+strlen(key),"/",1);
	char ls_type[2];
	sprintf(ls_type,"%d",adj_lsa->header->ls_type);
	memcpy(key+strlen(key),ls_type,strlen(ls_type));
	key[strlen(key)]='\0';
}

int
build_and_install_adj_lsa(struct ccn_schedule *sched, void *clienth, struct ccn_scheduled_event *ev, int flags)
{
	printf("build_and_install_adj_lsa called \n");

	printf("adj_build_flag = %d \n",nlsr->adj_build_flag);

	if(nlsr->adj_build_flag > 0)
	{
		printf("is_adj_lsa_build = %d \n",is_adj_lsa_build());
		if ( is_adj_lsa_build()> 0)
		{
			struct alsa *adj_lsa=(struct alsa *)malloc(sizeof( struct alsa ));
			build_adj_lsa(adj_lsa);
			install_adj_lsa(adj_lsa);

			free(adj_lsa->header->orig_router->name);
			free(adj_lsa->header->orig_router);
			free(adj_lsa->header->orig_time);
			free(adj_lsa->header);
			free(adj_lsa->body);
			free(adj_lsa);
			nlsr->adj_build_flag=0;	
			print_adj_lsdb();		
		}
		else
		{
			printf("Can not build adj LSA now\n");
		}
	}
	nlsr->is_build_adj_lsa_sheduled=0;
	return 0;
}


void
build_adj_lsa(struct alsa * adj_lsa)
{
	printf("build_adj_lsa called \n");

	int no_link=no_active_nbr();
	printf("Number of link in Adjacent LSA: %d\n",no_link);

	/*Filling Up Header Data */
	adj_lsa->header=(struct alsa_header *)malloc(sizeof(struct alsa_header ));
	adj_lsa->header->orig_router=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
	adj_lsa->header->orig_router->name=(char *)malloc(strlen(nlsr->router_name)+1);
	memset(adj_lsa->header->orig_router->name,0,strlen(nlsr->router_name)+1);
	memcpy(adj_lsa->header->orig_router->name,nlsr->router_name,strlen(nlsr->router_name)+1);
	adj_lsa->header->orig_router->length=strlen(nlsr->router_name)+1;

	adj_lsa->header->ls_type=(unsigned)LS_TYPE_ADJ;	

	char *time_stamp=(char *)malloc(20);
	memset(time_stamp,0,20);
	get_current_timestamp_micro(time_stamp);

	adj_lsa->header->orig_time=(char *)malloc(strlen(time_stamp)+1); 
	memset(adj_lsa->header->orig_time,0,strlen(time_stamp)+1);
	memcpy(adj_lsa->header->orig_time,time_stamp,strlen(time_stamp)+1);	
	free(time_stamp);


	/* Filling Up Body Data */

	adj_lsa->no_link=no_link;
/*
	struct link *templ=(struct link *)malloc(2*sizeof(struct link));
	adj_lsa->links=templ;

	int i, adl_element;
	struct ndn_neighbor *nbr;
	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	hashtb_start(nlsr->adl, e);
	adl_element=hashtb_n(nlsr->adl);

	for(i=0;i<adl_element;i++)
	{
		nbr=e->data;
		if( nbr->status	== 1 )
		{
			struct link *temp=(struct link *)malloc(sizeof(struct link));
			temp->nbr=(struct name_prefix *)malloc(sizeof(struct name_prefix));
			temp->nbr->name=(char *)malloc(nbr->neighbor->length);
			memset(temp->nbr->name,0,nbr->neighbor->length);		
			memcpy(temp->nbr->name,nbr->neighbor->name,nbr->neighbor->length);

			temp->nbr->length=nbr->neighbor->length;
			temp->face=nbr->face;
			temp->metric=nbr->metric;

			templ=temp;
			templ++;
		}	
		hashtb_next(e);		
	}

	hashtb_end(e);
*/


	struct ccn_charbuf *c=ccn_charbuf_create();
	get_active_nbr_adj_data(c);
	char *data=ccn_charbuf_as_string(c);

	adj_lsa->body=(char *)malloc(strlen(data)+1);
	memset(adj_lsa->body,0,strlen(data)+1);
	memcpy(adj_lsa->body,(char *)data,strlen(data)+1);
	ccn_charbuf_destroy(&c);



	if( !nlsr->is_send_lsdb_interest_scheduled )
	{	
		nlsr->event_send_lsdb_interest= ccn_schedule_event(nlsr->sched, 1000, &send_lsdb_interest, NULL, 0);
		nlsr->is_send_lsdb_interest_scheduled=1;
	}

	nlsr->adj_build_count++;


}


void
install_adj_lsa(struct alsa * adj_lsa)
{
	printf("install_adj_lsa called \n");

	char *key=(char *)malloc(adj_lsa->header->orig_router->length+2+2);
	memset(key,0,adj_lsa->header->orig_router->length+2);
	make_adj_lsa_key(key,adj_lsa);
	printf("Adjacent LSA key: %s \n",key);

	struct alsa *new_adj_lsa=(struct alsa*)malloc(sizeof(struct alsa ));

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res;

   	hashtb_start(nlsr->lsdb->adj_lsdb, e);
    	res = hashtb_seek(e, key, strlen(key), 0);



	if(res == HT_NEW_ENTRY )
	{
		printf("New ADJ LSA... Adding to LSDB\n");
		new_adj_lsa = e->data;

		new_adj_lsa->header=(struct alsa_header *)malloc(sizeof(struct alsa_header ));
		new_adj_lsa->header->ls_type=adj_lsa->header->ls_type;
		new_adj_lsa->header->orig_time=(char *)malloc(strlen(adj_lsa->header->orig_time)+1);
		memcpy(new_adj_lsa->header->orig_time,adj_lsa->header->orig_time,strlen(adj_lsa->header->orig_time)+1);		

		new_adj_lsa->header->orig_router=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
		new_adj_lsa->header->orig_router->name=(char *)malloc(adj_lsa->header->orig_router->length);
		memcpy(new_adj_lsa->header->orig_router->name,adj_lsa->header->orig_router->name,adj_lsa->header->orig_router->length);
		new_adj_lsa->header->orig_router->length=adj_lsa->header->orig_router->length;

		new_adj_lsa->no_link=adj_lsa->no_link;
		
		new_adj_lsa->body=(char *)malloc(strlen(adj_lsa->body)+1);
		memset(new_adj_lsa->body,0,strlen(adj_lsa->body)+1);
		memcpy(new_adj_lsa->body,adj_lsa->body,strlen(adj_lsa->body)+1);

		printf("Old Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);
		set_new_lsdb_version();	
		printf("New Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);

		add_next_hop_router(new_adj_lsa->header->orig_router->name);

		add_next_hop_from_lsa_adj_body(new_adj_lsa->body,new_adj_lsa->no_link);
	}
	else if(res == HT_OLD_ENTRY)
	{
		new_adj_lsa = e->data;
		if(strcmp(adj_lsa->header->orig_time,new_adj_lsa->header->orig_time)<=0)
		{
			printf("Older/Duplicate Adj LSA. Discarded...\n");
		}
		else
		{
			new_adj_lsa = e->data;

			free(new_adj_lsa->header->orig_time);
			new_adj_lsa->header->orig_time=(char *)malloc(strlen(adj_lsa->header->orig_time)+1);
			memcpy(new_adj_lsa->header->orig_time,adj_lsa->header->orig_time,strlen(adj_lsa->header->orig_time)+1);

			new_adj_lsa->no_link=adj_lsa->no_link;
			
			new_adj_lsa->body=(char *)malloc(strlen(adj_lsa->body)+1);
			memset(new_adj_lsa->body,0,strlen(adj_lsa->body)+1);
			memcpy(new_adj_lsa->body,adj_lsa->body,strlen(adj_lsa->body)+1);

			printf("Old Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);
			set_new_lsdb_version();	
			printf("New Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);

			add_next_hop_from_lsa_adj_body(new_adj_lsa->body,new_adj_lsa->no_link);

		}

	}
    	hashtb_end(e);

	if ( !nlsr->is_route_calculation_scheduled )
	{
		nlsr->event_calculate_route = ccn_schedule_event(nlsr->sched, 1000000, &route_calculate, NULL, 0);
		nlsr->is_route_calculation_scheduled=1;
	}
	free(key);
}

void 
print_adj_lsa_body(const char *body, int no_link)
{
	int i=0;
	char *lsa_data=(char *)malloc(strlen(body)+1);
	memset(	lsa_data,0,strlen(body)+1);
	memcpy(lsa_data,body,strlen(body)+1);
	char *sep="|";
	char *rem;
	char *rtr_id;
	char *length;
	char *face;
	char *metric;

	if(no_link >0 )
	{
		rtr_id=strtok_r(lsa_data,sep,&rem);
		length=strtok_r(NULL,sep,&rem);
		face=strtok_r(NULL,sep,&rem);
		metric=strtok_r(NULL,sep,&rem);

		printf("		Link %d	 	\n",i+1);
		printf("		Neighbor		 : %s	\n",rtr_id);
		printf("		Neighbor Length		 : %s	\n",length);
		printf("		Connecting Face		 : %s	\n",face);
		printf("		Metric			 : %s	\n",metric);


		for(i=1;i<no_link;i++)
		{
			rtr_id=strtok_r(NULL,sep,&rem);
			length=strtok_r(NULL,sep,&rem);
			face=strtok_r(NULL,sep,&rem);
			metric=strtok_r(NULL,sep,&rem);
			printf("		Link %d	 	\n",i+1);
			printf("		Neighbor		 : %s	\n",rtr_id);
			printf("		Neighbor Length		 : %s	\n",length);
			printf("		Connecting Face		 : %s	\n",face);
			printf("		Metric			 : %s	\n",metric);

		}
	}

	free(lsa_data);
}

void
print_adj_lsa(struct alsa * adj_lsa)
{

	printf("print_adj_lsa called \n");

	printf("-----------ADJ LSA Content---------------\n");
	printf("	Origination Router       :	%s\n",adj_lsa->header->orig_router->name);
	printf("	Origination Router Length:	%d\n",adj_lsa->header->orig_router->length);
	printf("	LS Type			 :	%d\n",adj_lsa->header->ls_type);
	printf("	Origination Time	 :	%s\n",adj_lsa->header->orig_time);
	printf("	Lsa Data:\n");
	printf("		No of Link	: %d\n",adj_lsa->no_link);

	print_adj_lsa_body(adj_lsa->body,adj_lsa->no_link);

/*
	struct link *templ=adj_lsa->links;
	int i;
	
	for(i=0 ; i< adj_lsa->no_link ; i++)
	{
		printf("		Link %d	 	\n",i+1);
		printf("		Neighbor		 : %s	\n",templ->nbr->name);
		printf("		Neighbor Length		 : %d	\n",templ->nbr->length);
		printf("		Connecting Face		 : %d	\n",templ->face);
		printf("		Metric			 : %d	\n",templ->metric);		

		templ++;
	}
*/
	printf("\n");

}

void
print_adj_lsdb(void)
{
	printf("print_name_lsdb called \n");	
	int i, adj_lsdb_element;
	struct alsa *adj_lsa;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->lsdb->adj_lsdb, e);
	adj_lsdb_element=hashtb_n(nlsr->lsdb->adj_lsdb);

	for(i=0;i<adj_lsdb_element;i++)
	{
		printf("-----------Adj LSA (%d)---------------\n",i+1);
		adj_lsa=e->data;
		print_adj_lsa(adj_lsa);	
		hashtb_next(e);		
	}

	hashtb_end(e);

	printf("\n");
}

void 
build_and_install_others_adj_lsa(char *orig_router,int ls_type,char *orig_time, int no_link,char *data)
{
	printf("build_and_install_others_adj_lsa called \n");	
	struct alsa *adj_lsa=(struct alsa *)malloc(sizeof( struct alsa ));
	build_others_adj_lsa(adj_lsa,orig_router,ls_type,orig_time,no_link,data);
	//print_adj_lsa(adj_lsa);
	install_adj_lsa(adj_lsa);
	

	free(adj_lsa->header->orig_router->name);
	free(adj_lsa->header->orig_router);
	free(adj_lsa->header->orig_time);
	free(adj_lsa->header);
	free(adj_lsa->body);
	free(adj_lsa);

	print_adj_lsdb();

}


void 
build_others_adj_lsa(struct alsa *adj_lsa,char *orig_router,int ls_type,char *orig_time,int no_link,char *data)
{
	printf("build_others_adj_lsa called \n");

	/*Filling Up Header Data */
	adj_lsa->header=(struct alsa_header *)malloc(sizeof(struct alsa_header ));
	adj_lsa->header->orig_router=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
	adj_lsa->header->orig_router->name=(char *)malloc(strlen(orig_router)+1);
	memset(adj_lsa->header->orig_router->name,0,strlen(orig_router)+1);
	memcpy(adj_lsa->header->orig_router->name,orig_router,strlen(orig_router)+1);

	adj_lsa->header->orig_router->length=strlen(orig_router)+1;


	adj_lsa->header->ls_type=(unsigned)LS_TYPE_ADJ;	

	adj_lsa->header->orig_time=(char *)malloc(strlen(orig_time)+1);
	memset(adj_lsa->header->orig_time,0,strlen(orig_time)+1);
	memcpy(adj_lsa->header->orig_time,orig_time,strlen(orig_time)+1);

	adj_lsa->no_link=no_link;

	adj_lsa->body=(char *)malloc(strlen(data)+1);
	memset(adj_lsa->body,0,strlen(data)+1);
	memcpy(adj_lsa->body,(char *)data,strlen(data)+1);

}


long int
get_name_lsdb_num_element(void)
{
	long int num_element;


	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->lsdb->name_lsdb, e);
	num_element=hashtb_n(nlsr->lsdb->name_lsdb);
	hashtb_end(e);

	return num_element; 
}

long int
get_adj_lsdb_num_element(void)
{
	long int num_element;


	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->lsdb->adj_lsdb, e);
	num_element=hashtb_n(nlsr->lsdb->adj_lsdb);
	hashtb_end(e);

	return num_element; 
}

void 
get_name_lsdb_summary(struct ccn_charbuf *name_lsdb_data)
{
	printf("get_name_lsdb_summary called \n");	
	int i, name_lsdb_element;

	struct nlsa *name_lsa;
	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->lsdb->name_lsdb, e);
	name_lsdb_element=hashtb_n(nlsr->lsdb->name_lsdb);

	for(i=0;i<name_lsdb_element;i++)
	{
		name_lsa=e->data;

		ccn_charbuf_append_string(name_lsdb_data,name_lsa->header->orig_router->name);
		ccn_charbuf_append_string(name_lsdb_data,"|");

		char *lst=(char *)malloc(20);
		memset(lst,0,20);
		sprintf(lst,"%d",name_lsa->header->ls_type);
		ccn_charbuf_append_string(name_lsdb_data,lst);
		free(lst);
		ccn_charbuf_append_string(name_lsdb_data,"|");

		char *lsid=(char *)malloc(20);
		memset(lsid,0,20);
		sprintf(lsid,"%ld",name_lsa->header->ls_id);
		ccn_charbuf_append_string(name_lsdb_data,lsid);
		free(lsid);
		ccn_charbuf_append_string(name_lsdb_data,"|");

		ccn_charbuf_append_string(name_lsdb_data,name_lsa->header->orig_time);
		ccn_charbuf_append_string(name_lsdb_data,"|");

		hashtb_next(e);		
	}

	hashtb_end(e);

}


void 
get_adj_lsdb_summary(struct ccn_charbuf *adj_lsdb_data)
{
	printf("get_adj_lsdb_summary called \n");
	int i, adj_lsdb_element;
	struct alsa *adj_lsa;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->lsdb->adj_lsdb, e);
	adj_lsdb_element=hashtb_n(nlsr->lsdb->adj_lsdb);

	for(i=0;i<adj_lsdb_element;i++)
	{
		adj_lsa=e->data;

		ccn_charbuf_append_string(adj_lsdb_data,adj_lsa->header->orig_router->name);
		ccn_charbuf_append_string(adj_lsdb_data,"|");
		
		char *lst=(char *)malloc(20);
		memset(lst,0,20);
		sprintf(lst,"%d",adj_lsa->header->ls_type);
		ccn_charbuf_append_string(adj_lsdb_data,lst);
		free(lst);
		ccn_charbuf_append_string(adj_lsdb_data,"|");

		ccn_charbuf_append_string(adj_lsdb_data,adj_lsa->header->orig_time);
		ccn_charbuf_append_string(adj_lsdb_data,"|");

		hashtb_next(e);		
	}

	hashtb_end(e);
}


void 
get_lsdb_summary(struct ccn_charbuf *lsdb_data)
{
	struct ccn_charbuf *name_lsdb_data=ccn_charbuf_create();
	struct ccn_charbuf *adj_lsdb_data=ccn_charbuf_create();

	get_name_lsdb_summary(name_lsdb_data);
	get_adj_lsdb_summary(adj_lsdb_data);

	long int num_lsa=get_name_lsdb_num_element() + get_adj_lsdb_num_element();
	char *num_element=(char *)malloc(15);
	memset(num_element,0,15);
	sprintf(num_element,"%ld",num_lsa);

	if( num_lsa > 0)
	{
		ccn_charbuf_append_string(lsdb_data,num_element);
		ccn_charbuf_append_string(lsdb_data,"|");
	}
	if(name_lsdb_data->length>0)
	{
		char *data1=ccn_charbuf_as_string(name_lsdb_data);
		ccn_charbuf_append_string(lsdb_data,(char *)data1);
	}
	if(adj_lsdb_data->length>0)
	{
		char *data2=ccn_charbuf_as_string(adj_lsdb_data);
		ccn_charbuf_append_string(lsdb_data,(char *)data2);
	}
	ccn_charbuf_destroy(&name_lsdb_data);
	ccn_charbuf_destroy(&adj_lsdb_data);
	free(num_element);

}

int 
check_is_new_name_lsa(char *orig_router,char *lst,char *lsid,char *orig_time)
{
	int ret=0;
	struct ccn_charbuf *key=ccn_charbuf_create();
	ccn_charbuf_append_string(key,orig_router);
	ccn_charbuf_append_string(key,"/");
	ccn_charbuf_append_string(key,lst);
	ccn_charbuf_append_string(key,"/");
	ccn_charbuf_append_string(key,lsid);

	int res;
	struct nlsa *name_lsa;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->lsdb->name_lsdb, e);
	res = hashtb_seek(e, ccn_charbuf_as_string(key), key->length, 0);

	if( res == HT_NEW_ENTRY )
	{
		hashtb_delete(e);
		ret=1;

	}
	else if(res == HT_OLD_ENTRY)
	{
		name_lsa=e->data;
		if( strcmp ( orig_time , name_lsa->header->orig_time ) > 0 )
		{
			ret=1;
		}
	}

	hashtb_end(e);

	ccn_charbuf_destroy(&key);
	
	return ret;
}

int 
check_is_new_adj_lsa(char *orig_router,char *lst,char *orig_time)
{
	int ret=0;
	struct ccn_charbuf *key=ccn_charbuf_create();
	ccn_charbuf_append_string(key,orig_router);
	ccn_charbuf_append_string(key,"/");
	ccn_charbuf_append_string(key,lst);

	int res;
	struct alsa *adj_lsa;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->lsdb->adj_lsdb, e);
	res = hashtb_seek(e, ccn_charbuf_as_string(key), key->length, 0);

	if( res == HT_NEW_ENTRY )
	{
		hashtb_delete(e);
		ret=1;

	}
	else if(res == HT_OLD_ENTRY)
	{
		adj_lsa=e->data;
		if( strcmp ( orig_time , adj_lsa->header->orig_time ) > 0 )
		{
			ret=1;
		}
	}

	hashtb_end(e);
	
	ccn_charbuf_destroy(&key);

	return ret;
}

void 
get_name_lsa_data(struct ccn_charbuf *lsa_data, struct name_prefix *lsaId)
{
	printf("get_name_lsa_data called \n");

	struct nlsa *name_lsa=(struct nlsa*)malloc(sizeof(struct nlsa ));

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res;

   	hashtb_start(nlsr->lsdb->name_lsdb, e);
    	res = hashtb_seek(e, lsaId->name, lsaId->length-1, 0);

	if( res == HT_OLD_ENTRY )
	{
		name_lsa=e->data;
		printf("NAME LSA found\n");

		ccn_charbuf_append_string(lsa_data,name_lsa->header->orig_router->name);
		ccn_charbuf_append_string(lsa_data,"|");

		char *temp_length=(char *)malloc(20);
		memset(temp_length,0,20);
		sprintf(temp_length,"%d",name_lsa->header->orig_router->length);
		ccn_charbuf_append_string(lsa_data,temp_length);
		free(temp_length);
		ccn_charbuf_append_string(lsa_data,"|");

		char *temp_ltype=(char *)malloc(20);
		memset(temp_ltype,0,20);
		sprintf(temp_ltype,"%d",name_lsa->header->ls_type);
		ccn_charbuf_append_string(lsa_data,temp_ltype);
		free(temp_ltype);
		ccn_charbuf_append_string(lsa_data,"|");

		char *temp_lsid=(char *)malloc(20);
		memset(temp_lsid,0,20);
		sprintf(temp_lsid,"%ld",name_lsa->header->ls_id);
		ccn_charbuf_append_string(lsa_data,temp_lsid);
		free(temp_lsid);
		ccn_charbuf_append_string(lsa_data,"|");

		ccn_charbuf_append_string(lsa_data,name_lsa->header->orig_time);
		ccn_charbuf_append_string(lsa_data,"|");

		char *temp_valid=(char *)malloc(20);
		memset(temp_valid,0,20);
		sprintf(temp_valid,"%d",name_lsa->header->isValid);
		ccn_charbuf_append_string(lsa_data,temp_valid);
		free(temp_valid);
		ccn_charbuf_append_string(lsa_data,"|");

		ccn_charbuf_append_string(lsa_data,name_lsa->name_prefix->name);
		ccn_charbuf_append_string(lsa_data,"|");

		char *temp_npl=(char *)malloc(20);
		memset(temp_npl,0,20);
		sprintf(temp_npl,"%d",name_lsa->name_prefix->length);
		ccn_charbuf_append_string(lsa_data,temp_npl);
		free(temp_npl);
		ccn_charbuf_append_string(lsa_data,"|");

	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}

	hashtb_end(e);
}

void 
get_adj_lsa_data(struct ccn_charbuf *lsa_data,struct name_prefix *lsaId)
{
	printf("get_adj_lsa_data called \n");

	struct alsa *adj_lsa=(struct alsa*)malloc(sizeof(struct alsa ));

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res;

   	hashtb_start(nlsr->lsdb->adj_lsdb, e);
    	res = hashtb_seek(e, lsaId->name, lsaId->length-1, 0);

	if( res == HT_OLD_ENTRY )
	{
		adj_lsa=e->data;
		printf("NAME LSA found\n");

		ccn_charbuf_append_string(lsa_data,adj_lsa->header->orig_router->name);
		ccn_charbuf_append_string(lsa_data,"|");

		char *temp_length=(char *)malloc(20);
		memset(temp_length,0,20);
		sprintf(temp_length,"%d",adj_lsa->header->orig_router->length);
		ccn_charbuf_append_string(lsa_data,temp_length);
		free(temp_length);
		ccn_charbuf_append_string(lsa_data,"|");

		char *temp_ltype=(char *)malloc(20);
		memset(temp_ltype,0,20);
		sprintf(temp_ltype,"%d",adj_lsa->header->ls_type);
		ccn_charbuf_append_string(lsa_data,temp_ltype);
		free(temp_ltype);
		ccn_charbuf_append_string(lsa_data,"|");

		ccn_charbuf_append_string(lsa_data,adj_lsa->header->orig_time);
		ccn_charbuf_append_string(lsa_data,"|");

		char *temp_nl=(char *)malloc(20);
		memset(temp_nl,0,20);
		sprintf(temp_nl,"%d",adj_lsa->no_link);
		ccn_charbuf_append_string(lsa_data,temp_nl);
		free(temp_nl);
		ccn_charbuf_append_string(lsa_data,"|");

		ccn_charbuf_append_string(lsa_data,adj_lsa->body);


	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}

	hashtb_end(e);
}
