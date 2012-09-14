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
#include "nlsr_route.h"

int
add_npt_entry(char *orig_router, char *name_prefix, int face)
{
	if ( strcmp(orig_router,nlsr->router_name)== 0)
	{
		return -1;
	}
	
	struct npt_entry *ne=(struct npt_entry*)malloc(sizeof(struct npt_entry ));
	
	int res,res_nle;
	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    

   	hashtb_start(nlsr->npt, e);
    	res = hashtb_seek(e, orig_router, strlen(orig_router), 0);

	if(res == HT_NEW_ENTRY)
	{
		ne=e->data;

		ne->orig_router=(char *)malloc(strlen(orig_router)+1);
		memset(ne->orig_router,0,strlen(orig_router)+1);
		memcpy(ne->orig_router,orig_router,strlen(orig_router));

	
		struct name_list_entry *nle=(struct name_list_entry *)malloc(sizeof(struct name_list_entry));

		struct hashtb_param param_nle = {0};
		ne->name_list= hashtb_create(sizeof(struct name_list_entry ), &param_nle);

		struct hashtb_enumerator eenle;
    		struct hashtb_enumerator *enle = &eenle;

		hashtb_start(ne->name_list, enle);
		res_nle = hashtb_seek(enle, name_prefix, strlen(name_prefix), 0);

		if(res_nle == HT_NEW_ENTRY )
		{
			nle=enle->data;
			nle->name=(char *)malloc(strlen(name_prefix)+1);
			memset(nle->name,0,strlen(name_prefix)+1);
			memcpy(nle->name,name_prefix,strlen(name_prefix));

			

		}
		hashtb_end(enle);

		ne->next_hop_face=face;

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
		
		struct name_list_entry *nle=(struct name_list_entry *)malloc(sizeof(struct name_list_entry));

		struct hashtb_param param_nle = {0};
		ne->name_list= hashtb_create(sizeof(struct name_list_entry ), &param_nle);

		struct hashtb_enumerator eenle;
    		struct hashtb_enumerator *enle = &eenle;

		hashtb_start(one->name_list, enle);
		res_nle = hashtb_seek(enle, name_prefix, strlen(name_prefix), 0);

		if(res_nle == HT_NEW_ENTRY )
		{
			nle=enle->data;
			nle->name=(char *)malloc(strlen(name_prefix)+1);
			memset(nle->name,0,strlen(name_prefix)+1);
			memcpy(nle->name,name_prefix,strlen(name_prefix));
			
			if ( face != NO_FACE )
			{
				add_delete_ccn_face_by_face_id(nlsr->ccn, (const char *)name_prefix, OP_REG, face);
			}
		}
		else if(res_nle == HT_OLD_ENTRY )
		{
			free(nle);
		}
		hashtb_end(enle);

	

	}
	hashtb_end(e);
	return res;
}

int 
delete_npt_entry(char *orig_router, char *name_prefix)
{
	if ( strcmp(orig_router,nlsr->router_name)== 0)
	{
		return -1;
	}

	struct npt_entry *ne;
	
	int res,res_nle;
	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    

   	hashtb_start(nlsr->npt, e);
    	res = hashtb_seek(e, orig_router, strlen(orig_router), 0);

	if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
		return -1;
	}
	else if (res == HT_OLD_ENTRY)
	{
		ne=e->data;

		struct hashtb_enumerator eenle;
    		struct hashtb_enumerator *enle = &eenle;

		hashtb_start(ne->name_list, enle);
		res_nle = hashtb_seek(enle, name_prefix, strlen(name_prefix), 0);

		if(res_nle == HT_NEW_ENTRY )
		{
			hashtb_delete(enle);
		}
		else if(res_nle == HT_OLD_ENTRY )
		{
			if (ne->next_hop_face != NO_FACE )
			{
				add_delete_ccn_face_by_face_id(nlsr->ccn, (const char *)name_prefix, OP_UNREG, ne->next_hop_face);
			}
			hashtb_delete(enle);
		}

		hashtb_end(enle);

		if ( hashtb_n(ne->name_list) == 0 )
		{
			hashtb_delete(e);
		}
	}
	
	hashtb_end(e);

	return 0;
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
		printf(" Origination Router: %s \n",ne->orig_router);
		ne->next_hop_face == NO_FACE ? printf(" Next Hop Face: NO_NEXT_HOP \n") : printf(" Next Hop Face: %d \n", ne->next_hop_face);
		
		int j, nl_element;
		struct name_list_entry *nle;		
		struct hashtb_enumerator eenle;
    		struct hashtb_enumerator *enle = &eenle;

		hashtb_start(ne->name_list, enle);
		nl_element=hashtb_n(ne->name_list);	

		for (j=0;j<nl_element;j++)
		{
			nle=enle->data;
			printf(" Name Prefix: %s \n",nle->name);
			hashtb_next(enle);
		}
		hashtb_end(enle);
			
		hashtb_next(e);		
	}

	hashtb_end(e);

	printf("\n");
}

void
delete_orig_router_from_npt(char *orig_router,int next_hop_face)
{
	int res;
	struct npt_entry *ne;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->npt, e);
	res = hashtb_seek(e, orig_router, strlen(orig_router), 0);

	if ( res == HT_OLD_ENTRY )
	{
		ne=e->data;
		if ( next_hop_face == ne->next_hop_face )
		{
			if ( next_hop_face != NO_NEXT_HOP )
			{
				int j, nl_element;
				struct name_list_entry *nle;		
				struct hashtb_enumerator eenle;
    				struct hashtb_enumerator *enle = &eenle;

				hashtb_start(ne->name_list, enle);
				nl_element=hashtb_n(ne->name_list);	

				for (j=0;j<nl_element;j++)
				{
					nle=enle->data;
					add_delete_ccn_face_by_face_id(nlsr->ccn, (const char *)nle->name, OP_UNREG , next_hop_face);
					hashtb_next(enle);
				}
				hashtb_end(enle);				

			} 

		}
		hashtb_destroy(&ne->name_list);
		hashtb_delete(e);		
	}
	else if ( res == HT_NEW_ENTRY )
	{
		hashtb_delete(e);
	}
	hashtb_end(e);	
}


void 
update_npt_with_new_route(char * orig_router,int next_hop_face)
{
	int res;
	struct npt_entry *ne;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->npt, e);
	res = hashtb_seek(e, orig_router, strlen(orig_router), 0);

	if ( res == HT_OLD_ENTRY )
	{
		ne=e->data;

		if ( next_hop_face != ne->next_hop_face )
		{
			int j, nl_element;
			struct name_list_entry *nle;		
			struct hashtb_enumerator eenle;
    			struct hashtb_enumerator *enle = &eenle;

			hashtb_start(ne->name_list, enle);
			nl_element=hashtb_n(ne->name_list);	

			for (j=0;j<nl_element;j++)
			{
				nle=enle->data;
				if (ne->next_hop_face != NO_FACE )
				{
					add_delete_ccn_face_by_face_id(nlsr->ccn, (const char *)nle->name, OP_UNREG , ne->next_hop_face);
				}
				if (next_hop_face != NO_FACE )
				{
					add_delete_ccn_face_by_face_id(nlsr->ccn, (const char *)nle->name, OP_REG , next_hop_face);
				}
				hashtb_next(enle);
			}
			hashtb_end(enle);
			ne->next_hop_face=next_hop_face;	
		}
		
	}
	else if ( res == HT_NEW_ENTRY )
	{
		hashtb_delete(e);
	}
	hashtb_end(e);
}

void 
destroy_all_face_by_nlsr(void)
{
	int i, npt_element;
	
	struct npt_entry *ne;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->npt, e);
	npt_element=hashtb_n(nlsr->npt);

	for(i=0;i<npt_element;i++)
	{
		ne=e->data;
		
		int j, nl_element;
		struct name_list_entry *nle;		
		struct hashtb_enumerator eenle;
    		struct hashtb_enumerator *enle = &eenle;

		hashtb_start(ne->name_list, enle);
		nl_element=hashtb_n(ne->name_list);	

		for (j=0;j<nl_element;j++)
		{
			nle=enle->data;
			if ( ne->next_hop_face != NO_FACE)
			{
				add_delete_ccn_face_by_face_id(nlsr->ccn, (const char *)nle->name, OP_UNREG , ne->next_hop_face);
			}
			//printf(" Name Prefix: %s Face: %d \n",nle->name,ne->next_hop_face);
			hashtb_next(enle);
		}
		hashtb_end(enle);
			
		hashtb_next(e);		
	}

	hashtb_end(e);

	printf("\n");
}
