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
#include "nlsr_adl.h"
#include "utility.h"

int
add_npt_entry(char *orig_router, char *name_prefix, int num_face, int *faces, int *route_costs)
{
	if ( strcmp(orig_router,nlsr->router_name)== 0)
	{
		return -1;
	}

	struct npt_entry *ne;//=(struct npt_entry*)calloc(1,sizeof(struct npt_entry ));
	
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
		memcpy(ne->orig_router,orig_router,strlen(orig_router)+1);

	
		

		//struct hashtb_param param_nle = {0};
		ne->name_list= hashtb_create(sizeof(struct name_list_entry ),NULL);

		struct hashtb_enumerator eenle;
    		struct hashtb_enumerator *enle = &eenle;

		hashtb_start(ne->name_list, enle);
		res_nle = hashtb_seek(enle, name_prefix, strlen(name_prefix), 0);

		if(res_nle == HT_NEW_ENTRY )
		{
			struct name_list_entry *nle;//=(struct name_list_entry *)calloc(1,sizeof(struct name_list_entry));
			nle=enle->data;
			nle->name=(char *)calloc(strlen(name_prefix)+1,sizeof(char));
			//memset(nle->name,0,strlen(name_prefix)+1);
			memcpy(nle->name,name_prefix,strlen(name_prefix)+1);

			

		}
		hashtb_end(enle);

		//struct hashtb_param param_fle = {0};
		ne->face_list=hashtb_create(sizeof(struct face_list_entry), NULL);

		if ( num_face > 0 )
		{
			struct hashtb_enumerator eef;
    			struct hashtb_enumerator *ef = &eef;
    	
    			hashtb_start(ne->face_list, ef);
			int i;						

			for ( i=0; i < num_face ; i++)
			{
				int face=faces[i];
				if ( face != NO_FACE && face != ZERO_FACE)
				{
					res_fle = hashtb_seek(ef, &face, sizeof(face), 0);
				
					if ( res_fle == HT_NEW_ENTRY )
					{
						struct face_list_entry *fle;//=(struct face_list_entry *)calloc(1,sizeof(struct face_list_entry));
						fle=ef->data;
						fle->next_hop_face=face;
						fle->route_cost=route_costs[i];
					}
				}
		
			}
			hashtb_end(ef);
		}
		
	}
	else if (res == HT_OLD_ENTRY)
	{
		struct npt_entry *one;

		one=e->data;
		
		struct hashtb_enumerator eenle;
    		struct hashtb_enumerator *enle = &eenle;

		hashtb_start(one->name_list, enle);
		res_nle = hashtb_seek(enle, name_prefix, strlen(name_prefix), 0);

		if(res_nle == HT_NEW_ENTRY )
		{
			struct name_list_entry *nle;//=(struct name_list_entry *)calloc(1,sizeof(struct name_list_entry));
			nle=enle->data;
			nle->name=(char *)malloc(strlen(name_prefix)+1);
			memset(nle->name,0,strlen(name_prefix)+1);
			memcpy(nle->name,name_prefix,strlen(name_prefix));
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
				if ( face != NO_FACE && face != ZERO_FACE)
				{
					res_fle = hashtb_seek(ef, &face, sizeof(face), 0);
				
					if ( res_fle == HT_NEW_ENTRY )
					{
						struct face_list_entry *fle;//=(struct face_list_entry *)calloc(1,sizeof(struct face_list_entry));
						fle=ef->data;
						fle->next_hop_face=face;
						fle->route_cost=route_costs[i];
					}
				}
			}
			hashtb_end(ef);
		}
	

	}
	hashtb_end(e);

	update_ccnd_fib_for_orig_router(orig_router);

	return res;
}

void 
update_ccnd_fib_for_orig_router(char *orig_router)
{

	if ( nlsr->debugging )
	{
		printf("update_ccnd_fib_for_orig_router called\n");
	}

	int res;	
	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    

   	hashtb_start(nlsr->npt, e);
    	res = hashtb_seek(e, orig_router, strlen(orig_router), 0);

	if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}
	else if ( res == HT_OLD_ENTRY )
	{
		struct npt_entry *ne;
		ne=e->data;
		int num_face=hashtb_n(ne->face_list);
		int last_face,first_face;

		int *faces=(int *)malloc(num_face*sizeof(int));
		int *route_costs=(int *)malloc(num_face*sizeof(int));
		
		get_all_faces_for_orig_router_from_npt(orig_router,faces,route_costs,num_face);
		sort_faces_by_distance(faces,route_costs,0,num_face);
		
		if ( nlsr->debugging )
		{
			int m;
			for ( m =0 ; m< num_face ; m++)
			{
				printf("Dest_router: %s Next_Hop_Face: %d Route_cost: %d \n",orig_router,faces[m],route_costs[m]);
			}
		}

		last_face=0;
		if ( nlsr->max_faces_per_prefix == 0) // add all faces available in routing table
		{
			first_face=num_face-1;
		}
		else if( nlsr->max_faces_per_prefix > 0)
		{
			if ( nlsr->max_faces_per_prefix >= num_face)
			{
				first_face=num_face-1;
			}
			else if ( nlsr->max_faces_per_prefix < num_face)
			{
				first_face=nlsr->max_faces_per_prefix-1;
			}
	
		}

		int i,j, nl_element;
		struct name_list_entry *nle;		
		struct hashtb_enumerator eenle;
    		struct hashtb_enumerator *enle = &eenle;

		hashtb_start(ne->name_list, enle);
		nl_element=hashtb_n(ne->name_list);
		
		for (i=0;i<nl_element;i++)
		{
			nle=enle->data;
			
			for( j=num_face-1; j>= 0; j--)
			{

				if ( !( is_neighbor(nle->name) == 1 && get_next_hop_face_from_adl(nle->name) == faces[j] ) )
				{
					add_delete_ccn_face_by_face_id(nlsr->ccn, (const char *)nle->name, OP_UNREG, faces[j],nlsr->router_dead_interval);
				}
			}
			
			
			hashtb_next(enle);
		}
		hashtb_end(enle);
		

		if ( nlsr->debugging )
		{
			printf("First Face Index: %d Last Face Index: %d\n",first_face,last_face);
		}

		hashtb_start(ne->name_list, enle);
		nl_element=hashtb_n(ne->name_list);	

		for (i=0;i<nl_element;i++)
		{
			nle=enle->data;
			
			for( j=first_face; j>= last_face; j--)
			{
				if ( nlsr->debugging )
				{
					printf("Possible FIB Entry name: %s face: %d route cost: %d \n",nle->name,faces[j],route_costs[j]);
				}
				if ( is_active_neighbor(orig_router) == 0 )
				{
					if ( nlsr->debugging )
						printf("Adding face: Name:%s Face: %d\n",nle->name,faces[j]);	
					if ( nlsr->detailed_logging )
						writeLogg(__FILE__,__FUNCTION__,__LINE__,"Adding face: Name:%s Face: %d\n",nle->name,faces[j]);
					add_delete_ccn_face_by_face_id(nlsr->ccn, (const char *)nle->name, OP_REG, faces[j],nlsr->router_dead_interval);	
				}
				else 
				{
					if ( j == last_face &&  is_active_neighbor(nle->name)==0)
					{
						if ( nlsr->debugging )
							printf("Adding face: Name:%s Face: %d\n",nle->name,faces[j]);	
						if ( nlsr->detailed_logging )
							writeLogg(__FILE__,__FUNCTION__,__LINE__,"Adding face: Name:%s Face: %d\n",nle->name,faces[j]);
						add_delete_ccn_face_by_face_id(nlsr->ccn, (const char *)nle->name, OP_REG, faces[j],nlsr->router_dead_interval);
					}
				}
			}
			
			
			hashtb_next(enle);
		}
		hashtb_end(enle);
		


		free(faces);
		free(route_costs);

	}
	hashtb_end(e);

}

int 
delete_npt_entry_by_router_and_name_prefix(char *orig_router, char *name_prefix)
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
			struct name_list_entry *nle;

			nle=enle->data;
	
			int j;
			int num_face=hashtb_n(ne->face_list);
			int last_face,first_face;

			int *faces=(int *)malloc(num_face*sizeof(int));
			int *route_costs=(int *)malloc(num_face*sizeof(int));
		
			get_all_faces_for_orig_router_from_npt(orig_router,faces,route_costs,num_face);
			sort_faces_by_distance(faces,route_costs,0,num_face);
		

			last_face=0;
			if ( nlsr->max_faces_per_prefix == 0) // add all faces available in routing table
			{
				first_face=num_face-1;
			}
			else if( nlsr->max_faces_per_prefix > 0)
			{
				if ( nlsr->max_faces_per_prefix >= num_face)
				{
					first_face=num_face-1;
				}
				else if ( nlsr->max_faces_per_prefix < num_face)
				{
					first_face=nlsr->max_faces_per_prefix-1;
				}
	
			}
			for( j=first_face; j>= last_face; j--)
			{

				if ( is_active_neighbor(orig_router) == 0 )
				{
					if ( nlsr->debugging )
						printf("Deleting face: Name:%s Face: %d\n",nle->name,faces[j]);	
					if ( nlsr->detailed_logging )
						writeLogg(__FILE__,__FUNCTION__,__LINE__,"Deleting face: Name:%s Face: %d\n",nle->name,faces[j]);
					add_delete_ccn_face_by_face_id(nlsr->ccn, (const char *)nle->name, OP_UNREG, faces[j],nlsr->router_dead_interval);	
				}
				else 
				{
					if ( j == last_face && is_active_neighbor(nle->name)==0)
					{
						if ( nlsr->debugging )
							printf("Deleting face: Name:%s Face: %d\n",nle->name,faces[j]);	
						if ( nlsr->detailed_logging )
							writeLogg(__FILE__,__FUNCTION__,__LINE__,"Deleting face: Name:%s Face: %d\n",nle->name,faces[j]);
						add_delete_ccn_face_by_face_id(nlsr->ccn, (const char *)nle->name, OP_UNREG, faces[j],nlsr->router_dead_interval);
					}
				}
				
			}
			
		}

		hashtb_end(enle);
	}
	
	hashtb_end(e);

	return 0;
}

void 
print_npt(void)
{

	if ( nlsr->debugging )
	{
		printf("\n");
		printf("print_npt called\n");
	}
	if ( nlsr->detailed_logging )
	{
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"\n");
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"print_npt called\n");
	}
	int i, npt_element;
	
	struct npt_entry *ne;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->npt, e);
	npt_element=hashtb_n(nlsr->npt);

	for(i=0;i<npt_element;i++)
	{
		if ( nlsr->debugging )
		{
			printf("\n");
			printf("----------NPT ENTRY %d------------------\n",i+1);
		}
		if ( nlsr->detailed_logging )
		{
			writeLogg(__FILE__,__FUNCTION__,__LINE__,"\n");
			writeLogg(__FILE__,__FUNCTION__,__LINE__,"----------NPT ENTRY %d------------------\n",i+1);
		}
		ne=e->data;
		if ( nlsr->debugging )
			printf(" Origination Router: %s \n",ne->orig_router);
		if ( nlsr->detailed_logging )
			writeLogg(__FILE__,__FUNCTION__,__LINE__," Origination Router: %s \n",ne->orig_router);
		
		int j, nl_element,face_list_element;
		struct name_list_entry *nle;		
		struct hashtb_enumerator eenle;
    		struct hashtb_enumerator *enle = &eenle;

		hashtb_start(ne->name_list, enle);
		nl_element=hashtb_n(ne->name_list);	

		for (j=0;j<nl_element;j++)
		{
			nle=enle->data;
			if ( nlsr->debugging )
				printf(" Name Prefix: %s \n",nle->name);
			if ( nlsr->detailed_logging )
				writeLogg(__FILE__,__FUNCTION__,__LINE__," Name Prefix: %s \n",nle->name);
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
			if ( nlsr->debugging )
				printf(" 	Face: No Face \n");
			if ( nlsr->detailed_logging )
				writeLogg(__FILE__,__FUNCTION__,__LINE__," 	Face: No Face \n");
			
		}
		else
		{
			for(j=0;j<face_list_element;j++)
			{
				fle=ef->data;
				if ( nlsr->debugging )
					printf(" 	Face: %d Route_Cost: %f \n",fle->next_hop_face,fle->route_cost);
				if ( nlsr->detailed_logging )
					writeLogg(__FILE__,__FUNCTION__,__LINE__," 	Face: %d Route_Cost: %d \n",fle->next_hop_face,fle->route_cost);
				hashtb_next(ef);	
			}
		}
		hashtb_end(ef);
		
			
		hashtb_next(e);		
	}

	hashtb_end(e);

	if ( nlsr->debugging )
		printf("\n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"\n");
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
					delete_npt_entry_by_router_and_name_prefix(ne->orig_router,nle->name);		
					hashtb_next(enle);
				}
				hashtb_end(enle);				

			} 

		}
		//hashtb_destroy(&ne->name_list);
		//hashtb_destroy(&ne->face_list);
		destroy_name_list(ne->name_list);
		destroy_face_list(ne->face_list);
		hashtb_delete(e);		
	}
	else if ( res == HT_NEW_ENTRY )
	{
		hashtb_delete(e);
	}
	hashtb_end(e);	
}


void
add_face_to_npt_by_face_id(char *dest_router, int face_id, int route_cost)
{
	if ( nlsr->debugging )
	{
		printf("add_face_to_npt_by_face_id called\n");
		printf("Dest Router: %s Face: %d Route_cost: %d \n",dest_router, face_id, route_cost);
	}
	if ( nlsr->detailed_logging )
	{
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"add_face_to_npt_by_face_id called\n");
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"Dest Router: %s Face: %d Route_cost: %d \n",dest_router, face_id, route_cost);
	}

	
	int res,res1;
	struct npt_entry *ne;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->npt, e);
	res = hashtb_seek(e, dest_router, strlen(dest_router), 0);

	if ( res == HT_OLD_ENTRY )
	{
		if ( nlsr->debugging )
			printf("Dest Router Found \n");
		if ( nlsr->detailed_logging )
			writeLogg(__FILE__,__FUNCTION__,__LINE__,"Dest Router Found \n");
	
		ne=e->data;
		
		struct hashtb_enumerator eef;
    		struct hashtb_enumerator *ef = &eef;
    	
    		hashtb_start(ne->face_list, ef);
		res1=hashtb_seek(ef, &face_id, sizeof(face_id), 0);

		if ( res1 == HT_OLD_ENTRY )
		{
			if ( nlsr->debugging )
				printf("Face Found \n");
			if ( nlsr->detailed_logging )
				writeLogg(__FILE__,__FUNCTION__,__LINE__,"Face Found \n");
			struct face_list_entry *fle;
			fle=ef->data;
			fle->next_hop_face=face_id;
			fle->route_cost=route_cost;
		}
		else if ( res1 == HT_NEW_ENTRY )
		{
			if ( nlsr->debugging )
				printf("Face Not Found \n");
			if ( nlsr->detailed_logging )
				writeLogg(__FILE__,__FUNCTION__,__LINE__,"Face Not Found \n");
			struct face_list_entry *fle;//=(struct face_list_entry *)malloc(sizeof(struct face_list_entry));
			fle=ef->data;
			fle->next_hop_face=face_id;
			fle->route_cost=route_cost;

			


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
add_new_fib_entries_to_npt(void)
{
	if ( nlsr->debugging )
		printf("add_new_fib_entries_to_npt called\n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"add_new_fib_entries_to_npt called\n");
	int i,j, rt_element,face_list_element;
	
	struct routing_table_entry *rte;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->routing_table, e);
	rt_element=hashtb_n(nlsr->routing_table);

	for(i=0;i<rt_element;i++)
	{
		rte=e->data;

		struct face_list_entry *fle;

		struct hashtb_enumerator eef;
    		struct hashtb_enumerator *ef = &eef;
    	
    		hashtb_start(rte->face_list, ef);
		face_list_element=hashtb_n(rte->face_list);
		if ( face_list_element <= 0 )
		{
			if ( nlsr->debugging )
				printf(" Face: No Face \n");
			if ( nlsr->detailed_logging )
				writeLogg(__FILE__,__FUNCTION__,__LINE__," Face: No Face \n");
		}
		else
		{
			for(j=0;j<face_list_element;j++)
			{
				fle=ef->data;
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
	if ( nlsr->debugging )
		printf("delete_face_from_npt_by_face_id\n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"delete_face_from_npt_by_face_id\n");

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


void 
clean_old_fib_entries_from_npt(void)
{
	
	
	if ( nlsr->debugging )
		printf("clean_old_fib_entries_from_npt called\n\n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"clean_old_fib_entries_from_npt called\n\n");
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
			if ( nlsr->debugging )
				printf(" Face: No Face \n");
			if ( nlsr->detailed_logging )
				writeLogg(__FILE__,__FUNCTION__,__LINE__," 	Face: No Face \n");
			
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
						if( is_neighbor(nle->name) == 0 )
						{
							if ( nlsr->debugging )
								printf("Deleting face: Name:%s Face: %d\n",nle->name,fle->next_hop_face);
							if ( nlsr->detailed_logging )
								writeLogg(__FILE__,__FUNCTION__,__LINE__,"Deleting face: Name:%s Face: %d\n",nle->name,fle->next_hop_face);		
							add_delete_ccn_face_by_face_id(nlsr->ccn, (const char *)nle->name, OP_UNREG, fle->next_hop_face,nlsr->router_dead_interval);
						}						
		

						hashtb_next(enle);
					}
					hashtb_end(enle);


					hashtb_delete(ef);
					j++;
						
				}
				else 
				{
					struct name_list_entry *nle;		
					struct hashtb_enumerator eenle;
    					struct hashtb_enumerator *enle = &eenle;

					hashtb_start(ne->name_list, enle);
					nl_element=hashtb_n(ne->name_list);	

					for (k=0;k<nl_element;k++)
					{
						nle=enle->data;
						if(  is_active_neighbor(ne->orig_router) && get_next_hop_face_from_adl(	ne->orig_router ) != fle->next_hop_face )
						{
							if ( nlsr->debugging )
								printf("Deleting face: Name:%s Face: %d\n",nle->name,fle->next_hop_face);
							if ( nlsr->detailed_logging )
								writeLogg(__FILE__,__FUNCTION__,__LINE__,"Deleting face: Name:%s Face: %d\n",nle->name,fle->next_hop_face);		
							add_delete_ccn_face_by_face_id(nlsr->ccn, (const char *)nle->name, OP_UNREG, fle->next_hop_face,nlsr->router_dead_interval);
						}						
		

						hashtb_next(enle);
					}
					hashtb_end(enle);
					
					hashtb_next(ef);
				}	
			}
		}
		hashtb_end(ef);
		
			
		hashtb_next(e);		
	}

	hashtb_end(e);

}

void
update_npt_with_new_route(void)
{
	if ( nlsr->debugging )
		printf("update_npt_with_new_route called\n");
		
	clean_old_fib_entries_from_npt();
	print_npt();
	add_new_fib_entries_to_npt();
	print_npt();	

	int i, npt_element;
	
	struct npt_entry *ne;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->npt, e);
	npt_element=hashtb_n(nlsr->npt);

	for(i=0;i<npt_element;i++)
	{
	
		ne=e->data;
		update_ccnd_fib_for_orig_router(ne->orig_router);
		hashtb_next(e);
	}
	
	hashtb_end(e);
}



void 
sort_faces_by_distance(int *faces,int *route_costs,int start,int element)
{
	int i,j;
	int temp_cost;
	int temp_face;

	for ( i=start ; i < element ; i ++) 
	{
		for( j=i+1; j<element; j ++)
		{
			if (route_costs[j] < route_costs[i] )
			{
				temp_cost=route_costs[j];
				route_costs[j]=route_costs[i];
				route_costs[i]=temp_cost;

				temp_face=faces[j];
				faces[j]=faces[i];
				faces[i]=temp_face;
			}
		}
	}
	
}

void
get_all_faces_for_orig_router_from_npt(char *orig_router, int *faces, int *route_costs, int num_faces)
{

	int res,face_list_element,j;	
	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    

   	hashtb_start(nlsr->npt, e);
    	res = hashtb_seek(e, orig_router, strlen(orig_router), 0);

	if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}
	else if ( res == HT_OLD_ENTRY )
	{
		struct npt_entry *ne;
		ne=e->data;
		
		struct face_list_entry *fle;

		struct hashtb_enumerator eef;
    		struct hashtb_enumerator *ef = &eef;
    	
    		hashtb_start(ne->face_list, ef);
		face_list_element=hashtb_n(ne->face_list);
		for(j=0;j<face_list_element;j++)
		{
			fle=ef->data;
			faces[j]=fle->next_hop_face;
			route_costs[j]=fle->route_cost;
			hashtb_next(ef);	
		}
		hashtb_end(ef);
		

	}
	hashtb_end(e);

}

void 
destroy_faces_by_orig_router(char *orig_router)
{

	int res;	
	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    

   	hashtb_start(nlsr->npt, e);
    	res = hashtb_seek(e, orig_router, strlen(orig_router), 0);

	if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}
	else if ( res == HT_OLD_ENTRY )
	{
		struct npt_entry *ne;
		ne=e->data;
		int num_face=hashtb_n(ne->face_list);
		int last_face,first_face;

		int *faces=(int *)malloc(num_face*sizeof(int));
		int *route_costs=(int *)malloc(num_face*sizeof(int));
		
		get_all_faces_for_orig_router_from_npt(orig_router,faces,route_costs,num_face);
		sort_faces_by_distance(faces,route_costs,0,num_face);
		last_face=0;
		if ( nlsr->max_faces_per_prefix == 0)
		{
			first_face=num_face-1;
		}
		else if( nlsr->max_faces_per_prefix > 0)
		{
			if ( nlsr->max_faces_per_prefix >= num_face)
			{
				first_face=num_face-1;
			}
			else if ( nlsr->max_faces_per_prefix < num_face)
			{
				first_face=nlsr->max_faces_per_prefix-1;
			}
	
		}

		int i,j, nl_element;
		struct name_list_entry *nle;		
		struct hashtb_enumerator eenle;
    		struct hashtb_enumerator *enle = &eenle;

		hashtb_start(ne->name_list, enle);
		nl_element=hashtb_n(ne->name_list);	

		for (i=0;i<nl_element;i++)
		{
			nle=enle->data;
			
			for( j=first_face; j>= last_face; j--)
			{
				if ( is_active_neighbor(orig_router) == 0 )
				{
					if ( nlsr->debugging )
						printf("Deleting face: Name:%s Face: %d\n",nle->name,faces[j]);
					if ( nlsr->detailed_logging )
						writeLogg(__FILE__,__FUNCTION__,__LINE__,"Deleting face: Name:%s Face: %d\n",nle->name,faces[j]);
					add_delete_ccn_face_by_face_id(nlsr->ccn, (const char *)nle->name, OP_UNREG, faces[j],nlsr->router_dead_interval);	
				}
				else 
				{
					if ( j == last_face && is_active_neighbor(nle->name)==0)
					{
						if ( nlsr->debugging )
							printf("Deleting face: Name:%s Face: %d\n",nle->name,faces[j]);
						if ( nlsr->detailed_logging )
							writeLogg(__FILE__,__FUNCTION__,__LINE__,"Deleting face: Name:%s Face: %d\n",nle->name,faces[j]);
						add_delete_ccn_face_by_face_id(nlsr->ccn, (const char *)nle->name, OP_UNREG, faces[j],nlsr->router_dead_interval);
					}
				}
			}

			hashtb_next(enle);
		}
		hashtb_end(enle);
		


		free(faces);
		free(route_costs);

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
		destroy_faces_by_orig_router(ne->orig_router);
		hashtb_next(e);		
	}

	hashtb_end(e);

	if ( nlsr->debugging )
		printf("\n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"\n");
}

void
destroy_name_list(struct hashtb *name_list)
{
	int j,nl_element;
	struct name_list_entry *nle;		
	struct hashtb_enumerator eenle;
    	struct hashtb_enumerator *enle = &eenle;

	hashtb_start(name_list, enle);
	nl_element=hashtb_n(name_list);	

	for (j=0;j<nl_element;j++)
	{
		nle=enle->data;
		free(nle->name);
		hashtb_next(enle);
	}
	hashtb_end(enle);

	hashtb_destroy(&name_list);	
}

void
destroy_face_list(struct hashtb *face_list)
{
	hashtb_destroy(&face_list);
}

void 
destroy_npt(void)
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
		destroy_name_list(ne->name_list);
		destroy_face_list(ne->face_list);	
		hashtb_next(e);		
	}
	hashtb_end(e);

	hashtb_destroy(&nlsr->npt);
}

