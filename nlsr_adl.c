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
#include "nlsr_adl.h"

void
add_adjacent_to_adl(struct name_prefix *np, int face)
{
	printf("\nadd_adjacent_to_adl called\n");
	printf("Neighbor: %s Length: %d Face:%d\n",np->name,np->length,face);
	
	struct ndn_neighbor *nbr=(struct ndn_neighbor *)malloc(sizeof(struct ndn_neighbor));

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res;

   	hashtb_start(nlsr->adl, e);
    	res = hashtb_seek(e, np->name, np->length, 0);

	if(res == HT_NEW_ENTRY )
	{
   
		nbr = e->data;

		nbr->neighbor=(struct name_prefix *)malloc(sizeof( struct name_prefix ));
		nbr->neighbor->name=(char *)malloc(np->length);
		memcpy(nbr->neighbor->name,np->name,np->length);
		nbr->neighbor->length=np->length;
		nbr->face=face;
		nbr->status=NBR_DOWN;
		nbr->info_interest_timed_out=0;
		nbr->lsdb_synch_interval=300;
		nbr->metric=10;
		nbr->is_lsdb_send_interest_scheduled=0;
		
		

		char *time_stamp=get_current_timestamp_micro();
		nbr->last_lsdb_version=(char *)malloc(strlen(time_stamp)+1);
		memcpy(nbr->last_lsdb_version,time_stamp,strlen(time_stamp)+1);
		memset(nbr->last_lsdb_version,'0',strlen(time_stamp));

		nbr->last_lsdb_requested=0;

		//nbr->last_lsdb_requested=(char *)malloc(strlen(time_stamp)+1);
		//memcpy(nbr->last_lsdb_requested,time_stamp,strlen(time_stamp)+1);
		//memset(nbr->last_lsdb_requested,'0',strlen(time_stamp));

	}

    	hashtb_end(e);
}

void 
print_adjacent(struct ndn_neighbor *nbr)
{
	printf("\nprint_adjacent called\n");
	printf("--------Neighbor---------------------------\n");
	printf("	Neighbor: %s \n",nbr->neighbor->name);
	printf("	Length  : %d \n",nbr->neighbor->length);
	printf("	Face    : %d \n",nbr->face);
	printf("	Metric    : %d \n",nbr->metric);
	printf("	Status  : %d \n",nbr->status);
	printf("	LSDB Version: %s \n",nbr->last_lsdb_version);
	printf("	Info Interest Timed Out : %d \n",nbr->info_interest_timed_out);
	printf("	LSDB Synch Interval     : %ld \n",nbr->lsdb_synch_interval);
	printf("	Las Time LSDB Requested: %ld \n",nbr->last_lsdb_requested);

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
		print_adjacent(nbr);	
		hashtb_next(e);		
	}

	hashtb_end(e);

}

void 
update_adjacent_status_to_adl(struct name_prefix *nbr, int status)
{
	printf("update_adjacent_status_to_adl called \n");
	//int ret;
	int res;
	struct ndn_neighbor *nnbr;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->adl, e);
   	res = hashtb_seek(e, nbr->name, nbr->length, 0);


	if (res == HT_OLD_ENTRY)
	{
		nnbr=e->data;
		if ( nnbr->status!=status )
		{
			nnbr->status=status;
			nlsr->adj_build_flag++;
		}
	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}

	hashtb_end(e);
}

void 
update_lsdb_synch_interval_to_adl(struct name_prefix *nbr, long int interval)
{
	printf("uupdate_lsdb_synch_interval_to_adl called \n");

	int res;
	struct ndn_neighbor *nnbr;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->adl, e);
   	res = hashtb_seek(e, nbr->name, nbr->length, 0);


	if (res == HT_OLD_ENTRY)
	{
		nnbr=e->data;
		if ( nnbr->lsdb_synch_interval!= interval )
		{
			nnbr->lsdb_synch_interval=interval;
			
		}
	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}

	hashtb_end(e);
	
	
}


void 
update_adjacent_timed_out_to_adl(struct name_prefix *nbr, int increment)
{
	printf("update_adjacent_timed_out_to_adl called \n");

	int res;
	struct ndn_neighbor *nnbr;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->adl, e);
	res = hashtb_seek(e, nbr->name, nbr->length, 0);

	if( res == HT_OLD_ENTRY )
	{
		nnbr=e->data;
		nnbr->info_interest_timed_out += increment;
	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}

	hashtb_end(e);
}

void 
update_adjacent_timed_out_zero_to_adl(struct name_prefix *nbr)
{
	printf("update_adjacent_timed_out_zero_to_adl called \n");
	int time_out_number=get_timed_out_number(nbr);
	update_adjacent_timed_out_to_adl(nbr,-time_out_number);

}


void 
update_adjacent_lsdb_version_to_adl(struct name_prefix *nbr, char *version)
{
	printf("update_adjacent_timed_out_to_adl called \n");

	int res;
	struct ndn_neighbor *nnbr;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->adl, e);
	res = hashtb_seek(e, nbr->name, nbr->length, 0);

	if( res == HT_OLD_ENTRY )
	{
		nnbr=e->data;
		free(nnbr->last_lsdb_version);
		nnbr->last_lsdb_version=(char *)malloc(strlen(version)+1);
		memset(nnbr->last_lsdb_version,0,strlen(version)+1);
		memcpy(nnbr->last_lsdb_version,version,strlen(version)+1);
	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}

	hashtb_end(e);

}

int 
get_timed_out_number(struct name_prefix *nbr)
{
	printf("get_timed_out_number called \n");

	int res,ret=-1;
	struct ndn_neighbor *nnbr;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->adl, e);
	res = hashtb_seek(e, nbr->name, nbr->length, 0);

	if( res == HT_OLD_ENTRY )
	{
		nnbr=e->data;
		ret=nnbr->info_interest_timed_out;
	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}

	hashtb_end(e);

	return ret;

	return 0;	
}

int
is_adj_lsa_build(void)
{
	int ret=0;

	int nbr_count=0;	

	int i, adl_element;
	struct ndn_neighbor *nbr;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->adl, e);
	adl_element=hashtb_n(nlsr->adl);

	for(i=0;i<adl_element;i++)
	{
		nbr=e->data;
		if(nbr->status	== 1 )
		{
			nbr_count++;
		}
		else if (nbr->info_interest_timed_out >= nlsr->interest_retry)
		{
			nbr_count++;
		}
		hashtb_next(e);		
	}

	hashtb_end(e);
	if(nbr_count == adl_element)
		ret=1;
	
	return ret;
}

int 
no_active_nbr(void)
{
	int i, adl_element;
	int no_link=0;
	struct ndn_neighbor *nbr;
	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	hashtb_start(nlsr->adl, e);
	adl_element=hashtb_n(nlsr->adl);

	for(i=0;i<adl_element;i++)
	{
		nbr=e->data;
		if( nbr->status	== 1 )
			no_link++;
		hashtb_next(e);		
	}

	hashtb_end(e);

	return no_link;

}

long int 
len_active_nbr_data(void)
{
	int i, adl_element;
	int no_link=0;
	long int len=0;
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
			char *temp_face=(char *)malloc(20);
			char *temp_metric=(char *)malloc(20);
			char *temp_length=(char *)malloc(20);
			
			len+=strlen(nbr->neighbor->name);

			memset(	temp_face,0,20);
			sprintf(temp_face,"%d",nbr->face);
			len+=strlen(temp_face);	

			memset(	temp_metric,0,20);
			sprintf(temp_face,"%d",nbr->metric);
			len+=strlen(temp_metric);

			memset(	temp_length,0,20);
			sprintf(temp_length,"%d",nbr->neighbor->length);
			len+=strlen(temp_length);
		
			no_link++;

			free(temp_face);
			free(temp_metric);
			free(temp_length);

		}
		hashtb_next(e);		
	}

	hashtb_end(e);

	len=len+ no_link*4+1;

	return len;

}

void 
get_active_nbr_adj_data(struct ccn_charbuf *c)
{

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
			ccn_charbuf_append_string(c,nbr->neighbor->name);
			ccn_charbuf_append_string(c,"|");
			
			char *temp_length=(char *)malloc(20);
			memset(temp_length,0,20);
			sprintf(temp_length,"%d",nbr->neighbor->length);
			ccn_charbuf_append_string(c,temp_length);
			free(temp_length);
			ccn_charbuf_append_string(c,"|");

			char *temp_face=(char *)malloc(20);
			memset(temp_face,0,20);
			sprintf(temp_face,"%d",nbr->face);
			ccn_charbuf_append_string(c,temp_face);
			free(temp_face);
			ccn_charbuf_append_string(c,"|");

			char *temp_metric=(char *)malloc(20);
			memset(temp_metric,0,20);
			sprintf(temp_metric,"%d",nbr->metric);
			ccn_charbuf_append_string(c,temp_metric);
			free(temp_metric);
			ccn_charbuf_append_string(c,"|");

		}
		hashtb_next(e);		
	}

	hashtb_end(e);
}

char *
get_nbr_lsdb_version(char *nbr)
{
	printf("get_timed_out_number called \n");

	char *version=NULL;

	int res;
	struct ndn_neighbor *nnbr;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->adl, e);
	res = hashtb_seek(e, nbr, strlen(nbr)+1, 0);
	
	if (res == HT_OLD_ENTRY)
	{
		nnbr=e->data;
		version=(char *)malloc(strlen(nnbr->last_lsdb_version)+1);
		memset(version,0,strlen(nnbr->last_lsdb_version)+1);
		memcpy(version,nnbr->last_lsdb_version,strlen(nnbr->last_lsdb_version)+1);
	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}

	hashtb_end(e);

	return version;
}

void 
update_adjacent_last_lsdb_requested_to_adl(char *nbr, long int timestamp)
{
	printf("update_adjacent_last_lsdb_requested_to_adl called \n");

	int res;
	struct ndn_neighbor *nnbr;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->adl, e);
	res = hashtb_seek(e, nbr, strlen(nbr)+1, 0);

	if( res == HT_OLD_ENTRY )
	{
		nnbr=e->data;
		nnbr->last_lsdb_requested=timestamp;
		
	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}

	hashtb_end(e);

}

void 
adjust_adjacent_last_lsdb_requested_to_adl(char *nbr, long int sec)
{
	printf("update_adjacent_last_lsdb_requested_to_adl called \n");

	int res;
	struct ndn_neighbor *nnbr;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->adl, e);
	res = hashtb_seek(e, nbr, strlen(nbr)+1, 0);

	if( res == HT_OLD_ENTRY )
	{
		nnbr=e->data;
		nnbr->last_lsdb_requested=nnbr->last_lsdb_requested-sec;
		
	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}

	hashtb_end(e);

}


long int 
get_nbr_last_lsdb_requested(char *nbr)
{
	printf("get_timed_out_number called \n");

	long int last_lsdb_requested=0;

	int res;
	struct ndn_neighbor *nnbr;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->adl, e);
	res = hashtb_seek(e, nbr, strlen(nbr)+1, 0);
	
	if (res == HT_OLD_ENTRY)
	{
		nnbr=e->data;
		last_lsdb_requested=nnbr->last_lsdb_requested;
	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}

	hashtb_end(e);

	return last_lsdb_requested;
}

long int
get_nbr_time_diff_lsdb_req(char *nbr)
{
	printf("get_nbr_time_diff_lsdb_req called \n");

	long int time_diff=get_lsdb_synch_interval(nbr)+1;	
	int res;
	struct ndn_neighbor *nnbr;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->adl, e);
	res = hashtb_seek(e, nbr, strlen(nbr)+1, 0);
	
	if (res == HT_OLD_ENTRY)
	{
		nnbr=e->data;
		
		if (nnbr->last_lsdb_requested == 0)
			time_diff=get_lsdb_synch_interval(nbr)+1;
		else time_diff=get_current_time_sec() - get_nbr_last_lsdb_requested(nbr);
		
	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}

	hashtb_end(e);

	return time_diff;
}

long int 
get_lsdb_synch_interval(char *nbr)
{
	printf("get_lsdb_synch_interval called \n");

	long int lsdb_synch_interval=300;	
	

	int res;
	struct ndn_neighbor *nnbr;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->adl, e);
	res = hashtb_seek(e, nbr, strlen(nbr)+1, 0);
	
	if (res == HT_OLD_ENTRY)
	{
		nnbr=e->data;
		lsdb_synch_interval=nnbr->lsdb_synch_interval;
	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}

	hashtb_end(e);

	return lsdb_synch_interval;

}

void 
set_is_lsdb_send_interest_scheduled_to_zero(char *nbr)
{
	printf("set_is_lsdb_send_interest_scheduled_to_zero called \n");

	int res;
	struct ndn_neighbor *nnbr;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->adl, e);
	res = hashtb_seek(e, nbr, strlen(nbr)+1, 0);

	if (res == HT_OLD_ENTRY)
	{
		nnbr=e->data;
		nnbr->is_lsdb_send_interest_scheduled=0;
	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}

	hashtb_end(e);

}


int 
get_adjacent_status(struct name_prefix *nbr)
{
	printf("get_adjacent_status called \n");

	int res;
	int status=-1;
	struct ndn_neighbor *nnbr;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->adl, e);
	res = hashtb_seek(e, nbr->name, nbr->length, 0);

	if (res == HT_OLD_ENTRY)
	{
		nnbr=e->data;
		status=nnbr->status;
	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}

	hashtb_end(e);

	return status;

}

