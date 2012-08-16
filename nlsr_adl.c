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
	hnbr->last_lsdb_version=(char *)malloc(15);

	hnbr->neighbor->length=nbr->neighbor->length;
	hnbr->face=nbr->face;
	hnbr->status=nbr->status;
	memcpy(hnbr->last_lsdb_version,"00000000000000",14);
	memcpy(hnbr->last_lsdb_version+strlen(hnbr->last_lsdb_version),"\0",1);

	struct hashtb_param param_luq = {0};
	hnbr->lsa_update_queue=hashtb_create(200, &param_luq);
	
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
		printf("Neighbor: %s Length: %d Face: %d Status: %d LSDB Version: %s \n",nbr->neighbor->name,nbr->neighbor->length,nbr->face, nbr->status, nbr->last_lsdb_version);	
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

	assert( res == HT_OLD_ENTRY);

	nnbr=e->data;
	nnbr->status=status;
	
	hashtb_end(e);
}


void 
update_adjacent_lsdb_version_to_adl(struct ccn_charbuf *nbr, char *version)
{
	printf("update_adjacent_status_to_adl called \n");

	int res;
	struct ndn_neighbor *nnbr;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->adl, e);
   	res = hashtb_seek(e, nbr->buf, nbr->length, 0);

	assert( res == HT_OLD_ENTRY);

	nnbr=e->data;
	memcpy(nnbr->last_lsdb_version,version,strlen(version)+1);
	
	hashtb_end(e);
}
