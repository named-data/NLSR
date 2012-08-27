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
add_adjacent_to_adl(struct ndn_neighbor *nbr)
{
	printf("\nadd_adjacent_to_adl called\n");
	printf("Neighbor: %s Length: %d Face: %d Status: %d\n",ccn_charbuf_as_string(nbr->neighbor),(int)nbr->neighbor->length,nbr->face, nbr->status);

	struct ndn_neighbor *hnbr=(struct ndn_neighbor *)malloc(sizeof(struct ndn_neighbor*));
	
	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res;

   	hashtb_start(nlsr->adl, e);
    	res = hashtb_seek(e, nbr->neighbor->buf , nbr->neighbor->length, 0);

	if(res == HT_NEW_ENTRY )
	{
   
		hnbr = e->data;
		hnbr->neighbor=ccn_charbuf_create();
		ccn_charbuf_append_string(hnbr->neighbor,ccn_charbuf_as_string(nbr->neighbor));

		hnbr->face=nbr->face;
		hnbr->status=nbr->status;
		hnbr->last_lsdb_version=0;
		hnbr->info_interest_timed_out=0;

		struct hashtb_param param_luq = {0};
		hnbr->lsa_update_queue=hashtb_create(200, &param_luq);
	}
	
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
		printf("Neighbor: %s Length: %d Face: %d Status: %d LSDB Version: %ld Interest Timed Out: %d\n",ccn_charbuf_as_string(nbr->neighbor),(int)nbr->neighbor->length,nbr->face, nbr->status, nbr->last_lsdb_version,nbr->info_interest_timed_out);	
		hashtb_next(e);		
	}

	hashtb_end(e);

	printf("\n");
}

void 
update_adjacent_status_to_adl(struct ccn_charbuf *nbr, int status)
{
	printf("update_adjacent_status_to_adl called \n");

	int res;
	struct ndn_neighbor *nnbr;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->adl, e);
   	res = hashtb_seek(e, nbr->buf, nbr->length, 0);
	

	if (res == HT_OLD_ENTRY)
	{
		nnbr=e->data;
		nnbr->status=status;
	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}
	
	hashtb_end(e);
}


void 
update_adjacent_lsdb_version_to_adl(struct ccn_charbuf *nbr, long int version)
{
	printf("update_adjacent_status_to_adl called \n");

	int res;
	struct ndn_neighbor *nnbr;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->adl, e);
	res = hashtb_seek(e, nbr->buf, nbr->length, 0);

	if( res == HT_OLD_ENTRY )
	{
		nnbr=e->data;
		nnbr->last_lsdb_version = version;
		//memcpy(nnbr->last_lsdb_version,version,strlen(version)+1);
	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}
	
	hashtb_end(e);
}

void 
update_adjacent_timed_out_to_adl(struct ccn_charbuf *nbr, int increment)
{
	printf("update_adjacent_timed_out_to_adl called \n");

	int res;
	struct ndn_neighbor *nnbr;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->adl, e);
	res = hashtb_seek(e, nbr->buf, nbr->length, 0);

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
update_adjacent_timed_out_zero_to_adl(struct ccn_charbuf *nbr)
{

	printf("update_adjacent_timed_out_zero_to_adl called \n");
	int time_out_number=get_timed_out_number(nbr);
	update_adjacent_timed_out_to_adl(nbr,-time_out_number);

}


int 
get_timed_out_number(struct ccn_charbuf *nbr)
{

	printf("get_timed_out_number called \n");

	int res,ret=-1;
	struct ndn_neighbor *nnbr;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->adl, e);
	res = hashtb_seek(e, nbr->buf, nbr->length, 0);

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
