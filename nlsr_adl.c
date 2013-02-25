#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<math.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>
#include <assert.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif


#include <ccn/ccn.h>
#include <ccn/uri.h>
#include <ccn/keystore.h>
#include <ccn/signing.h>
#include <ccn/schedule.h>
#include <ccn/hashtb.h>

#include "nlsr.h"
#include "nlsr_npl.h"
#include "nlsr_adl.h"
#include "utility.h"
#include "nlsr_npt.h"

void 
add_nbr_to_adl(struct name_prefix *new_nbr,int face,char *ip)
{
	struct ndn_neighbor *nbr;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res;

   	hashtb_start(nlsr->adl, e);
    	res = hashtb_seek(e, new_nbr->name, new_nbr->length, 0);

	if(res == HT_NEW_ENTRY )
	{
   
		nbr = e->data;

		nbr->neighbor=(struct name_prefix *)malloc(sizeof( struct name_prefix ));
		nbr->neighbor->name=(char *)malloc(new_nbr->length);
		memcpy(nbr->neighbor->name,new_nbr->name,new_nbr->length);
		nbr->neighbor->length=new_nbr->length;
		nbr->face=face;
		nbr->status=NBR_DOWN;
		nbr->info_interest_timed_out=0;
		nbr->lsdb_interest_timed_out=0;
		nbr->lsdb_random_time_component=(int)(LSDB_SYNCH_INTERVAL/2);
		nbr->lsdb_synch_interval=LSDB_SYNCH_INTERVAL;
		nbr->metric=LINK_METRIC;
		nbr->is_lsdb_send_interest_scheduled=0;
		
		nbr->ip_address=(char *)calloc(strlen(ip)+1,sizeof(char));
		memcpy(nbr->ip_address,ip,strlen(ip)+1);

		char *time_stamp=(char *)malloc(20);
		get_current_timestamp_micro(time_stamp);
		nbr->last_lsdb_version=(char *)calloc(strlen(time_stamp)+1,sizeof(char));
		memcpy(nbr->last_lsdb_version,"0000000000000000",16);
		nbr->last_info_version=(char *)calloc(strlen(time_stamp)+1,sizeof(char));
		memcpy(nbr->last_info_version,"0000000000000000",16);
		free(time_stamp);		

		nbr->last_lsdb_requested=0;
	}

    	hashtb_end(e);
}


void 
print_adjacent(struct ndn_neighbor *nbr)
{
	if ( nlsr->debugging )
	{
		printf("print_adjacent called\n");
		printf("--------Neighbor---------------------------\n");
		printf("	Neighbor: %s \n",nbr->neighbor->name);
		printf("	Length  : %d \n",nbr->neighbor->length);
		printf("	Ip Address: %s \n",nbr->ip_address);
		printf("	Face    : %d \n",nbr->face);
		printf("	Metric    : %d \n",nbr->metric);
		printf("	Status  : %d \n",nbr->status);
		printf("	LSDB Version: %s \n",nbr->last_lsdb_version);
		printf("	Info Version: %s \n",nbr->last_info_version);
		printf("	Info Interest Timed Out : %d \n",nbr->info_interest_timed_out);
		printf("	LSDB Interest Timed Out : %d \n",nbr->lsdb_interest_timed_out);
		printf("	LSDB Synch Interval     : %ld \n",nbr->lsdb_synch_interval);
		printf("	LSDB Random Time comp   : %d \n",nbr->lsdb_random_time_component);
		printf("	Las Time LSDB Requested: %ld \n",nbr->last_lsdb_requested);
		printf("	IS_lsdb_send_interest_scheduled : %d \n",nbr->is_lsdb_send_interest_scheduled);

		printf("\n");
	}

	if ( nlsr->detailed_logging )
	{
		
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"print_adjacent called\n");
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"--------Neighbor---------------------------\n");
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"	Neighbor: %s \n",nbr->neighbor->name);
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"	Length  : %d \n",nbr->neighbor->length);
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"	Face    : %d \n",nbr->face);
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"	Metric    : %d \n",nbr->metric);
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"	Status  : %d \n",nbr->status);
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"	LSDB Version: %s \n",nbr->last_lsdb_version);
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"	Info Version: %s \n",nbr->last_info_version);
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"	Info Interest Timed Out : %d \n",nbr->info_interest_timed_out);
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"	LSDB Interest Timed Out : %d \n",nbr->lsdb_interest_timed_out);
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"	LSDB Synch Interval     : %ld \n",nbr->lsdb_synch_interval);
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"	LSDB Random Time comp   : %d \n",nbr->lsdb_random_time_component);
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"	Las Time LSDB Requested: %ld \n",nbr->last_lsdb_requested);
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"	IS_lsdb_send_interest_scheduled : %d \n",nbr->is_lsdb_send_interest_scheduled);

		writeLogg(__FILE__,__FUNCTION__,__LINE__,"\n");
	}

}

void
print_adjacent_from_adl(void)
{
	if ( nlsr->debugging )
		printf("print_adjacent_from_adl called \n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"print_adjacent_from_adl called \n");		
	
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

int 
get_adjacent_status(struct name_prefix *nbr)
{

	if ( nlsr->debugging )
		printf("get_adjacent_status called \n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"get_adjacent_status called \n");

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

int 
get_timed_out_number(struct name_prefix *nbr)
{

	if ( nlsr->debugging )
		printf("get_timed_out_number called \n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"get_timed_out_number called \n");


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
}

int 
get_lsdb_interest_timed_out_number(struct name_prefix *nbr)
{

	if ( nlsr->debugging )
		printf("get_lsdb_interest_timed_out_number called \n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"get_lsdb_interest_timed_out_number called \n");

	int res,ret=-1;
	struct ndn_neighbor *nnbr;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->adl, e);
	res = hashtb_seek(e, nbr->name, nbr->length, 0);

	if( res == HT_OLD_ENTRY )
	{
		nnbr=e->data;
		ret=nnbr->lsdb_interest_timed_out;
	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}

	hashtb_end(e);

	return ret;	
}

void 
update_adjacent_timed_out_to_adl(struct name_prefix *nbr, int increment)
{
	if ( nlsr->debugging )
		printf("update_adjacent_timed_out_to_adl called \n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"update_adjacent_timed_out_to_adl called \n");

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
	if ( nlsr->debugging )
		printf("update_adjacent_timed_out_zero_to_adl called \n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"update_adjacent_timed_out_zero_to_adl called \n");
	
	int time_out_number=get_timed_out_number(nbr);
	update_adjacent_timed_out_to_adl(nbr,-time_out_number);

}


void 
update_lsdb_interest_timed_out_to_adl(struct name_prefix *nbr, int increment)
{
	if ( nlsr->debugging )
		printf("update_lsdb_interest_timed_out_to_adl called\n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"update_lsdb_interest_timed_out_to_adl called \n");

	int res;
	struct ndn_neighbor *nnbr;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->adl, e);

	res = hashtb_seek(e, nbr->name, nbr->length, 0);

	if( res == HT_OLD_ENTRY )
	{
		nnbr=e->data;
		nnbr->lsdb_interest_timed_out += increment;
	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}		
	hashtb_end(e);
}

void 
update_lsdb_interest_timed_out_zero_to_adl(struct name_prefix *nbr)
{
	if ( nlsr->debugging )
		printf("update_adjacent_timed_out_zero_to_adl called\n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"update_adjacent_timed_out_zero_to_adl called\n");
	
	int time_out_number=get_lsdb_interest_timed_out_number(nbr);
	update_lsdb_interest_timed_out_to_adl(nbr,-time_out_number);

}

void 
update_adjacent_status_to_adl(struct name_prefix *nbr, int status)
{
	if ( nlsr->debugging )
		printf("update_adjacent_status_to_adl called\n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"update_adjacent_status_to_adl called \n");

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
delete_nbr_from_adl(struct name_prefix *nbr)
{
	if ( nlsr->debugging )
		printf("delete_nbr_from_adl called\n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"delete_nbr_from_adl called \n");

	int res;
	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->adl, e);
   	res = hashtb_seek(e, nbr->name, nbr->length, 0);


	if (res == HT_OLD_ENTRY)
	{
		struct ndn_neighbor *nbr=e->data;
		/*free(nbr->neighbor->name);
		free(nbr->neighbor);
		free(nbr->last_lsdb_version);
		free(nbr->last_info_version);
		free(nbr->ip_address);
		*/
		destroy_nbr_component(nbr);
		hashtb_delete(e);
			
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
	if ( nlsr->debugging )
		printf("update_lsdb_synch_interval_to_adl called\n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"update_lsdb_synch_interval_to_adl called \n");

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
			nnbr->lsdb_random_time_component=(int)(interval/2);

		}
	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}

	hashtb_end(e);
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
		else if ( (nbr->status == 0) && (nbr->info_interest_timed_out >= nlsr->interest_retry || nbr->lsdb_interest_timed_out >= nlsr->interest_retry))
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


int 
get_next_hop_face_from_adl(char *nbr)
{
	int res;
	int connecting_face=NO_FACE;
	struct ndn_neighbor *nnbr;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->adl, e);
	res = hashtb_seek(e, nbr, strlen(nbr)+1, 0);

	if( res == HT_OLD_ENTRY )
	{
		nnbr=e->data;
		connecting_face=nnbr->face;
		
	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}

	hashtb_end(e);
	return connecting_face;
}

void
update_face_to_adl_for_nbr(char *nbr, int face)
{
	int res;
	struct ndn_neighbor *nnbr;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->adl, e);
	res = hashtb_seek(e, nbr, strlen(nbr)+1, 0);

	if( res == HT_OLD_ENTRY )
	{
		nnbr=e->data;
		nnbr->face=face;
		
	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}

	hashtb_end(e);
}

int 
is_neighbor(char *nbr)
{
	int ret=0;

	int res;
	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->adl, e);
	res = hashtb_seek(e, nbr, strlen(nbr)+1, 0);

	if( res == HT_OLD_ENTRY )
	{
		ret=1;	
	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}

	hashtb_end(e);

	return ret;
}

int 
is_active_neighbor(char *nbr)
{
	int ret=0;

	int res;
	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->adl, e);
	res = hashtb_seek(e, nbr, strlen(nbr)+1, 0);

	if( res == HT_OLD_ENTRY )
	{
		struct ndn_neighbor *nnbr;
		nnbr=e->data;
		if (nnbr->status == NBR_ACTIVE )
		{
			ret=1;	
		}
	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}

	hashtb_end(e);

	return ret;
}


void 
get_host_name_from_command_string(struct name_prefix *name_part,
												char *nbr_name_uri, int offset)
{

	
	
	int res,i;
	int len=0;
	const unsigned char *comp_ptr1;
	size_t comp_size;

	struct ccn_charbuf *name=ccn_charbuf_create();
	name = ccn_charbuf_create();
    	res = ccn_name_from_uri(name,nbr_name_uri);
    	if (res < 0) {
        	fprintf(stderr, "Bad ccn URI: %s\n", nbr_name_uri);
        	exit(1);
    	}	

	//struct ccn_indexbuf cid={0};

    	//struct ccn_indexbuf *components=&cid;
	struct ccn_indexbuf *components=ccn_indexbuf_create();
    	ccn_name_split (name, components);

	for(i=components->n-2;i> (0+offset);i--)
	{
		res=ccn_name_comp_get(name->buf, components,i,&comp_ptr1, &comp_size);
		len+=1;
		len+=(int)comp_size;	
	}
	len++;

	char *neighbor=(char *)malloc(len);
	memset(neighbor,0,len);

	for(i=components->n-2;i> (0+offset);i--)
	{
		res=ccn_name_comp_get(name->buf, components,i,&comp_ptr1, &comp_size);
		if ( i != components->n-2)
		memcpy(neighbor+strlen(neighbor),".",1);
		memcpy(neighbor+strlen(neighbor),(char *)comp_ptr1,
													strlen((char *)comp_ptr1));

	}

	name_part->name=(char *)calloc(strlen(neighbor)+1,sizeof(char));
	memcpy(name_part->name,neighbor,strlen(neighbor)+1);
	name_part->length=strlen(neighbor)+1;

	// 01/31/2013
	free(neighbor);
	ccn_charbuf_destroy(&name);
	ccn_indexbuf_destroy(&components);
}


void
destroy_nbr_component(struct ndn_neighbor *nbr)
{
	if ( nbr->neighbor->name )
		free(nbr->neighbor->name);
	if ( nbr->neighbor )
		free(nbr->neighbor);
	if ( nbr->last_lsdb_version)
		free(nbr->last_lsdb_version);
	if ( nbr->last_info_version)
		free(nbr->last_info_version);
	if ( nbr->ip_address)
		free(nbr->ip_address);
}

void
destroy_adl(void)
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
		destroy_nbr_component(nbr);
		hashtb_next(e);		
	}

	hashtb_end(e);
	
	if( nlsr->adl )
		hashtb_destroy(&nlsr->adl);

}


