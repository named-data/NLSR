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
add_npt_entry(char *orig_router, char *name_prefix, int num_face, int *faces, int *route_costs)
{
	if ( strcmp(orig_router,nlsr->router_name)== 0)
	{
		return -1;
	}
	
	struct npt_entry *ne=(struct npt_entry*)malloc(sizeof(struct npt_entry ));
	
	int res,res_nle,res_fle;
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

	
		

		struct hashtb_param param_nle = {0};
		ne->name_list= hashtb_create(sizeof(struct name_list_entry ), &param_nle);

		struct hashtb_enumerator eenle;
    		struct hashtb_enumerator *enle = &eenle;

		hashtb_start(ne->name_list, enle);
		res_nle = hashtb_seek(enle, name_prefix, strlen(name_prefix), 0);

		if(res_nle == HT_NEW_ENTRY )
		{
			struct name_list_entry *nle=(struct name_list_entry *)malloc(sizeof(struct name_list_entry));
			nle=enle->data;
			nle->name=(char *)malloc(strlen(name_prefix)+1);
			memset(nle->name,0,strlen(name_prefix)+1);
			memcpy(nle->name,name_prefix,strlen(name_prefix));

			

		}
		hashtb_end(enle);

		struct hashtb_param param_fle = {0};
		ne->face_list=hashtb_create(sizeof(struct face_list_entry), &param_fle);

		if ( num_face > 0 )
		{
			struct hashtb_enumerator eef;
    			struct hashtb_enumerator *ef = &eef;
    	
    			hashtb_start(ne->face_list, ef);
			int i;						

			for ( i=0; i< num_face ; i ++)
			{
				int face=faces[i];
				res_fle = hashtb_seek(ef, &face, sizeof(face), 0);
				
				if ( res_fle == HT_NEW_ENTRY )
				{
					struct face_list_entry *fle=(struct face_list_entry *)malloc(sizeof(struct face_list_entry));
					fle=ef->data;
					fle->next_hop_face=face;
					fle->route_cost=route_costs[i];
				}
		
			}
			hashtb_end(ef);
		}
		

		//ne->next_hop_face=face;

		/*
		if ( face != NO_FACE )
		{
			add_delete_ccn_face_by_face_id(nlsr->ccn, (const char *)name_prefix, OP_REG, face);
		}
		*/
	}
	else if (res == HT_OLD_ENTRY)
	{
		free(ne);
		struct npt_entry *one;

		one=e->data;
		
		struct hashtb_enumerator eenle;
    		struct hashtb_enumerator *enle = &eenle;

		hashtb_start(one->name_list, enle);
		res_nle = hashtb_seek(enle, name_prefix, strlen(name_prefix), 0);

		if(res_nle == HT_NEW_ENTRY )
		{
			struct name_list_entry *nle=(struct name_list_entry *)malloc(sizeof(struct name_list_entry));
			nle=enle->data;
			nle->name=(char *)malloc(strlen(name_prefix)+1);
			memset(nle->name,0,strlen(name_prefix)+1);
			memcpy(nle->name,name_prefix,strlen(name_prefix));
			/*
			if ( face != NO_FACE )
			{
				add_delete_ccn_face_by_face_id(nlsr->ccn, (const char *)name_prefix, OP_REG, face);
			}
			*/
		}
		else if(res_nle == HT_OLD_ENTRY )
		{
			
		}
		hashtb_end(enle);

		if ( num_face > 0 )
		{
			struct hashtb_enumerator eef;
    			struct hashtb_enumerator *ef = &eef;
    	
    			hashtb_start(one->face_list, ef);
			int i;						

			for ( i=0; i< num_face ; i ++)
			{
				int face=faces[i];
				res_fle = hashtb_seek(ef, &face, sizeof(face), 0);
				
				if ( res_fle == HT_NEW_ENTRY )
				{
					struct face_list_entry *fle=(struct face_list_entry *)malloc(sizeof(struct face_list_entry));
					fle=ef->data;
					fle->next_hop_face=face;
					fle->route_cost=route_costs[i];
				}
		
			}
			hashtb_end(ef);
		}
	

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
			/*
			if (ne->next_hop_face != NO_FACE )
			{
				add_delete_ccn_face_by_face_id(nlsr->ccn, (const char *)name_prefix, OP_UNREG, ne->next_hop_face);
			}
			*/
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
		//ne->next_hop_face == NO_FACE ? printf(" Next Hop Face: NO_NEXT_HOP \n") : printf(" Next Hop Face: %d \n", ne->next_hop_face);
		
		int j, nl_element,face_list_element;
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

		struct face_list_entry *fle;

		struct hashtb_enumerator eef;
    		struct hashtb_enumerator *ef = &eef;
    	
    		hashtb_start(ne->face_list, ef);
		face_list_element=hashtb_n(ne->face_list);
		if ( face_list_element <= 0 )
		{
			printf(" 	Face: No Face \n");
		}
		else
		{
			for(j=0;j<face_list_element;j++)
			{
				fle=ef->data;
				printf(" 	Face: %d Route_Cost: %d \n",fle->next_hop_face,fle->route_cost);
				hashtb_next(ef);	
			}
		}
		hashtb_end(ef);
		
			
		hashtb_next(e);		
	}

	hashtb_end(e);

	printf("\n");
}

void
delete_orig_router_from_npt(char *orig_router)
{
	int res,num_face,num_prefix;
	struct npt_entry *ne;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->npt, e);
	res = hashtb_seek(e, orig_router, strlen(orig_router), 0);

	if ( res == HT_OLD_ENTRY )
	{
		ne=e->data;
		num_prefix=hashtb_n(ne->name_list);
		if ( num_prefix > 0 )
		{
			num_face=hashtb_n(ne->face_list);
			
			if ( num_face > 0  )
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
		
					int k,face_list_element;
					struct face_list_entry *fle;

					struct hashtb_enumerator eef;
    					struct hashtb_enumerator *ef = &eef;
    	
    					hashtb_start(ne->face_list, ef);
					face_list_element=hashtb_n(ne->face_list);
					for(k=0;k<face_list_element;k++)
					{
						fle=ef->data;
						//printf(" 	Face: %d Route_Cost: %d \n",fle->next_hop_face,fle->route_cost);
						//add_delete_ccn_face_by_face_id(nlsr->ccn, (const char *)nle->name, OP_UNREG , next_hop_face);
						hashtb_next(ef);	
					}
					hashtb_end(ef);

						//add_delete_ccn_face_by_face_id(nlsr->ccn, (const char *)nle->name, OP_UNREG , next_hop_face);
					
					hashtb_next(enle);
				}
				hashtb_end(enle);				

			} 

		}
		hashtb_destroy(&ne->name_list);
		hashtb_destroy(&ne->face_list);
		hashtb_delete(e);		
	}
	else if ( res == HT_NEW_ENTRY )
	{
		hashtb_delete(e);
	}
	hashtb_end(e);	
}

/*

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

*/


void
add_face_to_npt_by_face_id(char *dest_router, int face_id, int route_cost)
{
	printf("add_face_to_npt_by_face_id called\n");

	printf("Dest Router: %s Face: %d Route_cost: %d \n",dest_router, face_id, route_cost);
	int res,res1;
	struct npt_entry *ne;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->npt, e);
	res = hashtb_seek(e, dest_router, strlen(dest_router), 0);

	if ( res == HT_OLD_ENTRY )
	{
		printf("Dest Router Found \n");
		ne=e->data;
		
		struct hashtb_enumerator eef;
    		struct hashtb_enumerator *ef = &eef;
    	
    		hashtb_start(ne->face_list, ef);
		res1=hashtb_seek(ef, &face_id, sizeof(face_id), 0);

		if ( res1 == HT_OLD_ENTRY )
		{
			printf("Face Found \n");
			struct face_list_entry *fle;//=(struct face_list_entry *)malloc(sizeof(struct face_list_entry));
			fle=ef->data;
			fle->next_hop_face=face_id;
			fle->route_cost=route_cost;
		}
		else if ( res1 == HT_NEW_ENTRY )
		{
			printf("Face Not Found \n");
			struct face_list_entry *fle=(struct face_list_entry *)malloc(sizeof(struct face_list_entry));
			fle=ef->data;
			fle->next_hop_face=face_id;
			fle->route_cost=route_cost;
			//hashtb_delete(ef);
		}
		hashtb_end(ef);
	}
	else if (res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}
	
	hashtb_end(e);
}


void
add_new_fib_entries_to_npt()
{
	printf("add_new_fib_entries_to_npt called\n");
	int i,j, rt_element,face_list_element;
	
	struct routing_table_entry *rte;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->routing_table, e);
	rt_element=hashtb_n(nlsr->routing_table);

	for(i=0;i<rt_element;i++)
	{
		//printf("----------Routing Table Entry %d------------------\n",i+1);
		rte=e->data;
		//printf(" Destination Router: %s \n",rte->dest_router);
		//rte->next_hop_face == NO_NEXT_HOP ? printf(" Next Hop Face: NO_NEXT_HOP \n") : printf(" Next Hop Face: %d \n", rte->next_hop_face);

		struct face_list_entry *fle;

		struct hashtb_enumerator eef;
    		struct hashtb_enumerator *ef = &eef;
    	
    		hashtb_start(rte->face_list, ef);
		face_list_element=hashtb_n(rte->face_list);
		if ( face_list_element <= 0 )
		{
			printf(" 	Face: No Face \n");
		}
		else
		{
			for(j=0;j<face_list_element;j++)
			{
				fle=ef->data;
				//printf(" 	Face: %d Route_Cost: %d \n",fle->next_hop_face,fle->route_cost);
				add_face_to_npt_by_face_id(rte->dest_router,fle->next_hop_face,fle->route_cost);
				hashtb_next(ef);	
			}
		}
		hashtb_end(ef);

		hashtb_next(e);		
	}

	hashtb_end(e);

}


void
delete_face_from_npt_by_face_id(char *dest_router, int face_id)
{
	printf("delete_face_from_npt_by_face_id called\n");

	int res,res1;
	struct npt_entry *ne;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->npt, e);
	res = hashtb_seek(e, dest_router, strlen(dest_router), 0);

	if ( res == HT_OLD_ENTRY )
	{
		ne=e->data;
		
		struct hashtb_enumerator eef;
    		struct hashtb_enumerator *ef = &eef;
    	
    		hashtb_start(ne->face_list, ef);
		res1=hashtb_seek(ef, &face_id, sizeof(face_id), 0);

		if ( res1 == HT_OLD_ENTRY )
		{
			hashtb_delete(ef);
		}
		else if ( res1 == HT_NEW_ENTRY )
		{
			hashtb_delete(ef);
		}
		hashtb_end(ef);
	}
	else if (res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}
	
	hashtb_end(e);
}

int
delete_old_face_from_npt(struct ccn_schedule *sched, void *clienth, struct ccn_scheduled_event *ev, int flags)
{
	if(flags == CCN_SCHEDULE_CANCEL)
	{
 	 	return -1;
	}
	
	nlsr_lock();

	printf("delete_old_face_from_npt called\n");
	if ( ev->evdata != NULL )
	{	
		printf("Event Data: %s \n",(char *)ev->evdata);	
		char *sep="|";
		char *rem;
		char *orig_router;
		char *faceid;
		int face_id;

		char *face_data=(char *)malloc(strlen((char *)ev->evdata)+1);
		memset(face_data,0,strlen((char *)ev->evdata)+1);
		memcpy(face_data+strlen(face_data),(char *)ev->evdata,strlen((char *)ev->evdata));

		orig_router=strtok_r(face_data,sep,&rem);
		faceid=strtok_r(NULL,sep,&rem);
		face_id=atoi(faceid);
		printf("Orig Router: %s Face: %d \n",orig_router,face_id);

		delete_face_from_npt_by_face_id(orig_router,face_id);		
	}

	nlsr_unlock();
	
	return 0;
}

void 
clean_old_fib_entries_from_npt()
{
	
	printf("clean_old_fib_entries_from_npt called\n\n");
	int i, npt_element;
	
	struct npt_entry *ne;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->npt, e);
	npt_element=hashtb_n(nlsr->npt);

	for(i=0;i<npt_element;i++)
	{
		ne=e->data;
		
		int j,k, nl_element,face_list_element;
		struct face_list_entry *fle;

		struct hashtb_enumerator eef;
    		struct hashtb_enumerator *ef = &eef;
    	
    		hashtb_start(ne->face_list, ef);
		face_list_element=hashtb_n(ne->face_list);
		if ( face_list_element <= 0 )
		{
			printf(" 	Face: No Face \n");
		}
		else
		{
			for(j=0;j<face_list_element;j++)
			{
				fle=ef->data;
				int check=does_face_exist_for_router(ne->orig_router,fle->next_hop_face);
				if ( check == 0 )
				{
					struct name_list_entry *nle;		
					struct hashtb_enumerator eenle;
    					struct hashtb_enumerator *enle = &eenle;

					hashtb_start(ne->name_list, enle);
					nl_element=hashtb_n(ne->name_list);	

					for (k=0;k<nl_element;k++)
					{
						nle=enle->data;

						//delete all the fib entries here		

						hashtb_next(enle);
					}
					hashtb_end(enle);

					char faceid[20];
					memset(faceid,0,20);
					sprintf(faceid,"%d",fle->next_hop_face);
					char *evdata=(char *)malloc(strlen(ne->orig_router)+strlen(faceid)+2);
					memset(evdata,0,strlen(ne->orig_router)+strlen(faceid)+2);					
					memcpy(evdata+strlen(evdata),ne->orig_router,strlen(ne->orig_router));	
					memcpy(evdata+strlen(evdata),"|",1);
					memcpy(evdata+strlen(evdata),faceid,strlen(faceid));					
	
					nlsr->event = ccn_schedule_event(nlsr->sched, 1, &delete_old_face_from_npt, (void *)evdata, 0);					
					
				}
				
				hashtb_next(ef);	
			}
		}
		hashtb_end(ef);
		
			
		hashtb_next(e);		
	}

	hashtb_end(e);

}

void
update_npt_with_new_route()
{
	clean_old_fib_entries_from_npt();
	add_new_fib_entries_to_npt();
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
			/*
			if ( ne->next_hop_face != NO_FACE)
			{
				add_delete_ccn_face_by_face_id(nlsr->ccn, (const char *)nle->name, OP_UNREG , ne->next_hop_face);
			}
			*/
			//printf(" Name Prefix: %s Face: %d \n",nle->name,ne->next_hop_face);
			hashtb_next(enle);
		}
		hashtb_end(enle);
			
		hashtb_next(e);		
	}

	hashtb_end(e);

	printf("\n");
}
