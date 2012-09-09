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
#include "nlsr_npt.h"
#include "nlsr_fib.h"

void 
make_npt_key(char *key, char *orig_router, char *name_prefix)
{
	memcpy(key+strlen(key),orig_router,strlen(orig_router));
	memcpy(key+strlen(key),name_prefix,strlen(name_prefix));
}

int
add_npt_entry(char *orig_router, char *name_prefix, int face)
{
	if ( strcmp(orig_router,nlsr->router_name)== 0)
	{
		return -1;
	}
	
	struct npt_entry *ne=(struct npt_entry*)malloc(sizeof(struct npt_entry ));
	
	int res,res_nht;
	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	
	char *key=(char *)malloc(strlen(orig_router)+strlen(name_prefix)+2);
	memset(key,0,strlen(orig_router)+strlen(name_prefix)+2);

	make_npt_key(key,orig_router,name_prefix);

   	hashtb_start(nlsr->npt, e);
    	res = hashtb_seek(e, key, strlen(key), 0);

	if(res == HT_NEW_ENTRY)
	{
		ne=e->data;

		ne->name_prefix=(char *)malloc(strlen(name_prefix)+1);
		memset(ne->name_prefix,0,strlen(name_prefix)+1);
		memcpy(ne->name_prefix,name_prefix,strlen(name_prefix));

		/* Adding Orig Router in Orig Router List */

		struct next_hop_entry *nhe=(struct next_hop_entry *)malloc(sizeof(struct next_hop_entry));

		struct hashtb_param param_nht = {0};
		ne->next_hop_table= hashtb_create(sizeof(struct next_hop_entry ), &param_nht);

		struct hashtb_enumerator eenht;
    		struct hashtb_enumerator *enht = &eenht;

		hashtb_start(ne->next_hop_table, enht);
		res_nht = hashtb_seek(enht, orig_router, strlen(orig_router), 0);

		if(res_nht == HT_NEW_ENTRY )
		{
			nhe=enht->data;
			nhe->orig_router=(char *)malloc(strlen(orig_router)+1);
			memset(nhe->orig_router,0,strlen(orig_router)+1);
			memcpy(nhe->orig_router,orig_router,strlen(orig_router));

			nhe->next_hop_face=face;

		}
		hashtb_end(enht);

		if ( face != NO_FACE )
		{
			add_delete_ccn_face_by_face_id(nlsr->ccn, (const char *)name_prefix, OP_REG, face);
		}
	
	}
	else if (res == HT_OLD_ENTRY)
	{
		free(ne);
		struct npt_entry *one;

		one=e->data;
		
		struct next_hop_entry *nhe=(struct next_hop_entry *)malloc(sizeof(struct next_hop_entry));
		struct hashtb_param param_nht = {0};
		ne->next_hop_table= hashtb_create(sizeof(struct next_hop_entry ), &param_nht);

		struct hashtb_enumerator eenht;
    		struct hashtb_enumerator *enht = &eenht;

		hashtb_start(one->next_hop_table, enht);
		res_nht = hashtb_seek(enht, orig_router, strlen(orig_router), 0);

		if(res_nht == HT_NEW_ENTRY )
		{
			nhe=enht->data;
			nhe->orig_router=(char *)malloc(strlen(orig_router)+1);
			memset(nhe->orig_router,0,strlen(orig_router)+1);
			memcpy(nhe->orig_router,orig_router,strlen(orig_router));

			nhe->next_hop_face=face;

			if ( face != NO_FACE )
			{
				add_delete_ccn_face_by_face_id(nlsr->ccn, (const char *)name_prefix, OP_REG, face);
			}

		}
		else if ( res_nht == HT_OLD_ENTRY)
		{
			free(nhe);
			struct next_hop_entry *onhe;
			onhe=enht->data;

			if(onhe->next_hop_face != face )
			{
				if ( onhe->next_hop_face != NO_FACE )
				{
					add_delete_ccn_face_by_face_id(nlsr->ccn, (const char *)name_prefix, OP_UNREG, onhe->next_hop_face);
				}

				if ( face != NO_FACE )
				{
					add_delete_ccn_face_by_face_id(nlsr->ccn, (const char *)name_prefix, OP_REG, onhe->next_hop_face);
				}

				onhe->next_hop_face=face;

			}			

		}
		hashtb_end(enht);
		
		

	}
	hashtb_end(e);

	free(key);
	return res;
}


void 
print_npt(void)
{
	printf("\n");
	printf("print_npt called\n\n");
	int i, npt_element;
	
	struct npt_entry *ne;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->npt, e);
	npt_element=hashtb_n(nlsr->npt);

	for(i=0;i<npt_element;i++)
	{
		printf("\n");
		printf("----------NPT ENTRY %d------------------\n",i+1);
		ne=e->data;
		printf(" Name Prefix: %s \n",ne->name_prefix);
		
		struct next_hop_entry *nhe;
		struct hashtb_enumerator eenht;
    		struct hashtb_enumerator *enht = &eenht;

		int j, nht_element;
		hashtb_start(ne->next_hop_table, enht);
		nht_element=hashtb_n(ne->next_hop_table);	

		for (j=0;j<nht_element;j++)
		{
			nhe=enht->data;
			printf(" Origination Router: %s \n",nhe->orig_router);
			nhe->next_hop_face == NO_FACE ? printf(" Next Hop Face: NO_NEXT_HOP \n") : printf(" Next Hop Face: %d \n", nhe->next_hop_face);

			hashtb_next(enht);
		}
		hashtb_end(enht);
			
		hashtb_next(e);		
	}

	hashtb_end(e);

	printf("\n");
}
