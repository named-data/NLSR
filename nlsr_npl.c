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
#include "nlsr_npl.h"
#include "utility.h"


void 
add_name_to_npl(struct name_prefix *np)
{
	struct name_prefix_list_entry *npe=(struct name_prefix_list_entry *)malloc(sizeof(struct name_prefix_list_entry));
	//struct name_prefix *hnp=(struct name_prefix *)malloc(sizeof(struct name_prefix )); //free

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res;

   	hashtb_start(nlsr->npl, e);
    	res = hashtb_seek(e, np->name, np->length, 0);

	if(res == HT_NEW_ENTRY)
	{   
		npe=e->data;
		npe->np=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
		npe->np->length=np->length;
		npe->np->name=(char *)malloc(np->length);
		memcpy(npe->np->name,np->name,np->length);
		npe->name_lsa_id=0;
		//hnp = e->data;
		//hnp->length=np->length;
		//hnp->name=(char *)malloc(np->length); //free
		//memcpy(hnp->name,np->name,np->length);
	}
    	
	hashtb_end(e);

}

int  
does_name_exist_in_npl(struct name_prefix *np)
{
	int ret=0;

	//struct name_prefix_entry *npe;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res;

   	hashtb_start(nlsr->npl, e);
    	res = hashtb_seek(e, np->name, np->length, 0);

	if(res == HT_NEW_ENTRY)
	{   
		hashtb_delete(e);
		ret=0;
	}
	else
	{
		ret=1;
    	}
	hashtb_end(e);

	return ret;

}


long int  
get_lsa_id_from_npl(struct name_prefix *np)
{
	int ret=0;

	struct name_prefix_list_entry *npe;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res;

   	hashtb_start(nlsr->npl, e);
    	res = hashtb_seek(e, np->name, np->length, 0);

	if(res == HT_NEW_ENTRY)
	{   
		hashtb_delete(e);
		ret=0;
	}
	else
	{
		npe=e->data;
		ret=npe->name_lsa_id;
    	}
	hashtb_end(e);

	return ret;

}

void
print_name_prefix_from_npl(void)
{
	if ( nlsr->debugging )
		printf("print_name_prefix_from_npl called \n");	
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"print_name_prefix_from_npl called\n");
	int i, npl_element;
	//struct name_prefix *np;
	struct name_prefix_list_entry *npe;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->npl, e);
	npl_element=hashtb_n(nlsr->npl);

	for(i=0;i<npl_element;i++)
	{
		npe=e->data;
		if ( nlsr->debugging )
			printf("Name Prefix: %s and Length: %d and LSA Id: %ld\n",npe->np->name,npe->np->length,npe->name_lsa_id);
		if ( nlsr->detailed_logging )
			writeLogg(__FILE__,__FUNCTION__,__LINE__,"Name Prefix: %s and Length: %d \n",npe->np->name,npe->np->length);	
		hashtb_next(e);		
	}

	hashtb_end(e);

	if ( nlsr->debugging )
		printf("\n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"\n");
}

void 
update_nlsa_id_for_name_in_npl(struct name_prefix *np, long int nlsa_id)
{
	struct name_prefix_list_entry *npe;
	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res;

   	hashtb_start(nlsr->npl, e);
    	res = hashtb_seek(e, np->name, np->length, 0);

	if(res == HT_OLD_ENTRY)
	{   
		npe=e->data;
		npe->name_lsa_id=nlsa_id;
	}
	else
	{
		hashtb_delete(e);	
	}
    	
	hashtb_end(e);
}

