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


#include <ccn/ccn.h>
#include <ccn/uri.h>
#include <ccn/keystore.h>
#include <ccn/signing.h>
#include <ccn/schedule.h>
#include <ccn/hashtb.h>


#include "nlsr.h"
#include "nlsr_route.h"
#include "nlsr_lsdb.h"
#include "nlsr_npt.h"
#include "nlsr_adl.h"
#include "nlsr_fib.h"

int
route_calculate(struct ccn_schedule *sched, void *clienth, struct ccn_scheduled_event *ev, int flags)
{

	if(flags == CCN_SCHEDULE_CANCEL)
	{
 	 	return -1;
	}

	nlsr_lock();

	printf("route_calculate called\n");

	if( ! nlsr->is_build_adj_lsa_sheduled )
	{
		/* Calculate Route here */
		print_routing_table();
		print_npt();		

		struct hashtb_param param_me = {0};
		nlsr->map = hashtb_create(sizeof(struct map_entry), &param_me);
		nlsr->rev_map = hashtb_create(sizeof(struct map_entry), &param_me);
		make_map();
		assign_mapping_number();		
		print_map();
		print_rev_map();

		do_old_routing_table_updates();
		clear_old_routing_table();	
		print_routing_table();

		int i;
		int **adj_matrix;
		int map_element=hashtb_n(nlsr->map);
		adj_matrix=malloc(map_element * sizeof(int *));
		for(i = 0; i < map_element; i++)
		{
			adj_matrix[i] = malloc(map_element * sizeof(int));
		}
		make_adj_matrix(adj_matrix,map_element);
		print_adj_matrix(adj_matrix,map_element);

		long int source=get_mapping_no(nlsr->router_name);
		long int *parent=(long int *)malloc(map_element * sizeof(long int));
		long int *dist=(long int *)malloc(map_element * sizeof(long int));

		int num_link=get_no_link_from_adj_matrix(adj_matrix, map_element ,source);
		
		if ( (num_link == 0) || (nlsr->multi_path_face_num <= 1 ) )
		{	
			calculate_path(adj_matrix,parent,dist, map_element, source);		
			print_all_path_from_source(parent,source);
			print_all_next_hop(parent,source);		
			update_routing_table_with_new_route(parent, dist,source);
		}
		else if ( (num_link != 0) && (nlsr->multi_path_face_num >= 1 ) )
		{
			long int *links=(long int *)malloc(num_link*sizeof(long int));
			long int *link_costs=(long int *)malloc(num_link*sizeof(long int));
			get_links_from_adj_matrix(adj_matrix, map_element , links, link_costs, source);
			for ( i=0 ; i < num_link; i++)
			{
				adjust_adj_matrix(adj_matrix, map_element,source,links[i],link_costs[i]);
				calculate_path(adj_matrix,parent,dist, map_element, source);		
				print_all_path_from_source(parent,source);
				print_all_next_hop(parent,source);		
				update_routing_table_with_new_route(parent, dist,source);
			}

			free(links);
			free(link_costs);
		}

		update_npt_with_new_route();

		print_routing_table();
		print_npt();

		for(i = 0; i < map_element; i++)
		{
			free(adj_matrix[i]);
		}
		free(parent);
		free(dist);
		free(adj_matrix);
		hashtb_destroy(&nlsr->map);
		hashtb_destroy(&nlsr->rev_map);
		
	}
	nlsr->is_route_calculation_scheduled=0;

	nlsr_unlock();

	return 0;
}

void 
calculate_path(int **adj_matrix, long int *parent,long int *dist ,long int V, long int S)
{
	int i;
	long int v,u;
	//long int *dist=(long int *)malloc(V * sizeof(long int));
	long int *Q=(long int *)malloc(V * sizeof(long int));
	long int head=0;
	/* Initial the Parent */
	for (i = 0 ; i < V; i++)
	{
		parent[i]=EMPTY_PARENT;
		dist[i]=INF_DISTANCE;
		Q[i]=i;
	} 

	if ( S != NO_MAPPING_NUM )
	{
		dist[S]=0;
		sort_queue_by_distance(Q,dist,head,V);

		while (head < V )
		{
			u=Q[head];
			if(dist[u] == INF_DISTANCE)
			{
				break;
			}

			for(v=0 ; v <V; v++)
			{
				if( adj_matrix[u][v] > 0 ) //
				{
					if ( is_not_explored(Q,v,head+1,V) )
					{
					
						if( dist[u] + adj_matrix[u][v] <  dist[v])
						{
							dist[v]=dist[u] + adj_matrix[u][v] ;
							parent[v]=u;
						}	

					}

				}

			}

			head++;
			sort_queue_by_distance(Q,dist,head,V);
		}
	}
	free(Q);
	//free(dist);	
}

void
print_all_path_from_source(long int *parent,long int source)
{
	int i, map_element;
	struct map_entry *me;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->map, e);
	map_element=hashtb_n(nlsr->map);

	if ( source != NO_MAPPING_NUM)
	{
		for(i=0;i<map_element;i++)
		{
			me=e->data;
			if(me->mapping != source)
			{
				print_path(parent,(long int)me->mapping);
				printf("\n");
			}
			hashtb_next(e);		
		}
	}
	hashtb_end(e);

}

void 
print_all_next_hop(long int *parent,long int source)
{
	int i, map_element;
	struct map_entry *me;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->map, e);
	map_element=hashtb_n(nlsr->map);

	for(i=0;i<map_element;i++)
	{
		me=e->data;
		if(me->mapping != source)
		{
			
			printf("Dest: %d Next Hop: %ld\n",me->mapping,get_next_hop_from_calculation(parent,me->mapping,source));
		}
		hashtb_next(e);		
	}

	hashtb_end(e);

}

long int
get_next_hop_from_calculation(long int *parent, long int dest,long int source)
{
	long int next_hop;
	while ( parent[dest] != EMPTY_PARENT )
	{
		next_hop=dest;
		dest=parent[dest];

	}

	if ( dest != source )
	{
		next_hop=NO_NEXT_HOP;	
	}
	
	return next_hop;
	
}

void
print_path(long int *parent, long int dest)
{
	if (parent[dest] != EMPTY_PARENT )
		print_path(parent,parent[dest]);

	printf(" %ld",dest);
}

int 
is_not_explored(long int *Q, long int u,long int start, long int element)
{
	int ret=0;
	long int i;
	for(i=start; i< element; i++)
	{
		if ( Q[i] == u )
		{
			ret=1;
			break;
		}
	}
	return ret;
}

void 
sort_queue_by_distance(long int *Q,long int *dist,long int start,long int element)
{
	long int i,j;
	long int temp_u;

	for ( i=start ; i < element ; i ++) 
	{
		for( j=i+1; j<element; j ++)
		{
			if (dist[Q[j]] < dist[Q[i]])
			{
				temp_u=Q[j];
				Q[j]=Q[i];
				Q[i]=temp_u;
			}
		}
	}
	
}

void
print_map(void)
{
	int i, map_element;
	struct map_entry *me;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->map, e);
	map_element=hashtb_n(nlsr->map);

	for(i=0;i<map_element;i++)
	{
		me=e->data;
		printf("Router: %s Mapping Number: %d \n",me->router,me->mapping);
		hashtb_next(e);		
	}

	hashtb_end(e);
}


void
assign_mapping_number(void)
{
	int i, map_element;
	struct map_entry *me;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->map, e);
	map_element=hashtb_n(nlsr->map);

	for(i=0;i<map_element;i++)
	{
		me=e->data;
		me->mapping=i;
		add_rev_map_entry(i,me->router);
		hashtb_next(e);		
	}

	hashtb_end(e);
}

void 
make_map(void)
{
	

	int i, adj_lsdb_element;
	struct alsa *adj_lsa;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->lsdb->adj_lsdb, e);
	adj_lsdb_element=hashtb_n(nlsr->lsdb->adj_lsdb);

	add_map_entry(nlsr->router_name);

	for(i=0;i<adj_lsdb_element;i++)
	{
		adj_lsa=e->data;
		add_adj_data_to_map(adj_lsa->header->orig_router->name,adj_lsa->body,adj_lsa->no_link);
		hashtb_next(e);		
	}

	hashtb_end(e);

}

void 
add_map_entry(char *router)
{

	struct map_entry *me=(struct map_entry*)malloc(sizeof(struct map_entry ));

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res;

   	hashtb_start(nlsr->map, e);
    	res = hashtb_seek(e, router, strlen(router), 0);

	if(res == HT_NEW_ENTRY)
	{
		me=e->data;
		me->router=(char *)malloc(strlen(router)+1);
		memset(me->router,0,strlen(router)+1);
		memcpy(me->router,router,strlen(router));
		me->mapping=0;
	}

	hashtb_end(e);
	
}


void 
add_adj_data_to_map(char *orig_router, char *body, int no_link)
{
	add_map_entry(orig_router);
	
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

		add_map_entry(rtr_id);

		for(i=1;i<no_link;i++)
		{
			rtr_id=strtok_r(NULL,sep,&rem);
			length=strtok_r(NULL,sep,&rem);
			face=strtok_r(NULL,sep,&rem);
			metric=strtok_r(NULL,sep,&rem);

			add_map_entry(rtr_id);

		}
	}
	
	free(lsa_data);
}

int 
get_mapping_no(char *router)
{
	struct map_entry *me;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res;
	int ret;

	int n = hashtb_n(nlsr->map);

	if ( n < 1)
	{
		return NO_MAPPING_NUM;
	}

   	hashtb_start(nlsr->map, e);
    	res = hashtb_seek(e, router, strlen(router), 0);

	if( res == HT_OLD_ENTRY )
	{
		me=e->data;
		ret=me->mapping;
	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
		ret=NO_MAPPING_NUM;
	}

	hashtb_end(e);	

	return ret;

}


void 
add_rev_map_entry(long int mapping_number, char *router)
{

	struct map_entry *me=(struct map_entry*)malloc(sizeof(struct map_entry ));

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res;

   	hashtb_start(nlsr->rev_map, e);
    	res = hashtb_seek(e, &mapping_number, sizeof(mapping_number), 0);

	if(res == HT_NEW_ENTRY)
	{
		me=e->data;
		me->router=(char *)malloc(strlen(router)+1);
		memset(me->router,0,strlen(router)+1);
		memcpy(me->router,router,strlen(router));
		me->mapping=mapping_number;
	}

	hashtb_end(e);
	
}



char * 
get_router_from_rev_map(long int mapping_number)
{

	struct map_entry *me;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res;

   	hashtb_start(nlsr->rev_map, e);
    	res = hashtb_seek(e, &mapping_number, sizeof(mapping_number), 0);

	if(res == HT_OLD_ENTRY)
	{
		me=e->data;
		hashtb_end(e);
		return me->router;
	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
		hashtb_end(e);
	}
	
	return NULL;
}

void
print_rev_map(void)
{
	int i, map_element;
	struct map_entry *me;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->map, e);
	map_element=hashtb_n(nlsr->map);

	for(i=0;i<map_element;i++)
	{
		me=e->data;
		printf("Mapping Number: %d Router: %s  \n",me->mapping,me->router);
		hashtb_next(e);		
	}

	hashtb_end(e);
}


void
assign_adj_matrix_for_lsa(struct alsa *adj_lsa, int **adj_matrix)
{
	int mapping_orig_router=get_mapping_no(adj_lsa->header->orig_router->name);
	int mapping_nbr_router;

	int i;
	char *lsa_data=(char *)malloc(strlen(adj_lsa->body)+1);
	memset(	lsa_data,0,strlen(adj_lsa->body)+1);
	memcpy(lsa_data,adj_lsa->body,strlen(adj_lsa->body)+1);
	char *sep="|";
	char *rem;
	char *rtr_id;
	char *length;
	char *face;
	char *metric;
	
	if(adj_lsa->no_link >0 )
	{
		rtr_id=strtok_r(lsa_data,sep,&rem);
		length=strtok_r(NULL,sep,&rem);
		face=strtok_r(NULL,sep,&rem);
		metric=strtok_r(NULL,sep,&rem);

		mapping_nbr_router=get_mapping_no(rtr_id);
		adj_matrix[mapping_orig_router][mapping_nbr_router]=atoi(metric);

		for(i=1;i<adj_lsa->no_link;i++)
		{
			rtr_id=strtok_r(NULL,sep,&rem);
			length=strtok_r(NULL,sep,&rem);
			face=strtok_r(NULL,sep,&rem);
			metric=strtok_r(NULL,sep,&rem);

			mapping_nbr_router=get_mapping_no(rtr_id);
			adj_matrix[mapping_orig_router][mapping_nbr_router]=atoi(metric);

		}
	}
	
	free(lsa_data);
}

void 
make_adj_matrix(int **adj_matrix,int map_element)
{

	init_adj_matrix(adj_matrix,map_element);

	int i, adj_lsdb_element;
	struct alsa *adj_lsa;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->lsdb->adj_lsdb, e);
	adj_lsdb_element=hashtb_n(nlsr->lsdb->adj_lsdb);

	for(i=0;i<adj_lsdb_element;i++)
	{
		adj_lsa=e->data;
		assign_adj_matrix_for_lsa(adj_lsa,adj_matrix);
		hashtb_next(e);		
	}

	hashtb_end(e);

}

void 
init_adj_matrix(int **adj_matrix,int map_element)
{
	int i, j;
	for(i=0;i<map_element;i++)
		for(j=0;j<map_element;j++)
			adj_matrix[i][j]=0;
}

void print_adj_matrix(int **adj_matrix, int map_element)
{
	int i, j;
	for(i=0;i<map_element;i++)
	{
		for(j=0;j<map_element;j++)
			printf("%d ",adj_matrix[i][j]);
		printf("\n");
	}
}


int 
get_no_link_from_adj_matrix(int **adj_matrix,long int V, long int S)
{
	int no_link=0;
	int i;

	for(i=0;i<V;i++)
	{	
		if ( adj_matrix[S][i] > 0 )
		{
			no_link++;
		}
	}
	return no_link;
}

void 
get_links_from_adj_matrix(int **adj_matrix, long int V ,long int *links, long int *link_costs,long int S)
{
	int i,j;
	j=0;
	for (i=0; i <V; i++)
	{
		if ( adj_matrix[S][i] > 0 )
		{
			links[j]=i;
			link_costs[j]=adj_matrix[S][i];
			j++;
		}
	}
}

void adjust_adj_matrix(int **adj_matrix, long int V, long int S, long int link,long int link_cost)
{
	int i;
	for ( i = 0; i < V; i++ )
	{
		if ( i == link )
		{
			adj_matrix[S][i]=link_cost;
		}
		else 
		{
			adj_matrix[S][i]=0;
		}
	}

}

int 
get_number_of_next_hop(char *dest_router)
{
	struct routing_table_entry *rte;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res,ret;

   	hashtb_start(nlsr->routing_table, e);
    	res = hashtb_seek(e, dest_router, strlen(dest_router), 0);

	if( res == HT_OLD_ENTRY )
	{
		rte=e->data;
		ret=hashtb_n(rte->face_list);
		//nhl=rte->face_list;
	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
		ret=NO_NEXT_HOP;
	}

	hashtb_end(e);	

	return ret;
}


int 
get_next_hop(char *dest_router,int *faces, int *route_costs)
{
	struct routing_table_entry *rte;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res,ret;

   	hashtb_start(nlsr->routing_table, e);
    	res = hashtb_seek(e, dest_router, strlen(dest_router), 0);

	if( res == HT_OLD_ENTRY )
	{
		rte=e->data;
		ret=hashtb_n(rte->face_list);
		//nhl=rte->face_list;
		int j,face_list_element;
		struct face_list_entry *fle;

		struct hashtb_enumerator eef;
    		struct hashtb_enumerator *ef = &eef;
    	
    		hashtb_start(rte->face_list, ef);
		face_list_element=hashtb_n(rte->face_list);
		for(j=0;j<face_list_element;j++)
		{
			fle=ef->data;
			//printf(" 	Face: %d Route_Cost: %d \n",fle->next_hop_face,fle->route_cost);
			faces[j]=fle->next_hop_face;
			route_costs[j]=fle->route_cost;
			hashtb_next(ef);	
		}
		hashtb_end(ef);
		
	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
		ret=NO_NEXT_HOP;
	}

	hashtb_end(e);	

	return ret;
}

void
add_next_hop_router(char *dest_router)
{
	if ( strcmp(dest_router,nlsr->router_name)== 0)
	{
		return ;
	}

	struct routing_table_entry *rte=(struct routing_table_entry *)malloc(sizeof(struct routing_table_entry));

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res;

   	hashtb_start(nlsr->routing_table, e);
    	res = hashtb_seek(e, dest_router, strlen(dest_router), 0);

	if( res == HT_NEW_ENTRY )
	{
		rte=e->data;
		rte->dest_router=(char *)malloc(strlen(dest_router)+1);
		memset(rte->dest_router,0,strlen(dest_router)+1);
		memcpy(rte->dest_router,dest_router,strlen(dest_router));
		//rte->next_hop_face=NO_NEXT_HOP;
		struct hashtb_param param_fle = {0};
		rte->face_list=hashtb_create(sizeof(struct face_list_entry), &param_fle);

		add_npt_entry(dest_router, dest_router, 0, NULL, NULL);
	}
	hashtb_end(e);

}

void 
add_next_hop_from_lsa_adj_body(char *body, int no_link)
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


		add_next_hop_router(rtr_id);

		for(i=1;i<no_link;i++)
		{
			rtr_id=strtok_r(NULL,sep,&rem);
			length=strtok_r(NULL,sep,&rem);
			face=strtok_r(NULL,sep,&rem);
			metric=strtok_r(NULL,sep,&rem);

			add_next_hop_router(rtr_id);
	
		}
	}

	free(lsa_data);


}

void 
update_routing_table(char * dest_router,int next_hop_face, int route_cost)
{
	int res,res1;
	struct routing_table_entry *rte;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->routing_table, e);
	res = hashtb_seek(e, dest_router, strlen(dest_router), 0);

	if( res == HT_OLD_ENTRY )
	{
		rte=e->data;

		struct hashtb_enumerator eef;
    		struct hashtb_enumerator *ef = &eef;
    	
    		hashtb_start(rte->face_list, ef);
		res1 = hashtb_seek(ef, &next_hop_face, sizeof(next_hop_face), 0);	
		if( res1 == HT_NEW_ENTRY)
		{
			struct face_list_entry *fle=(struct face_list_entry *)malloc(sizeof(struct face_list_entry));
			fle=ef->data;
			fle->next_hop_face=next_hop_face;
			fle->route_cost=route_cost;						
		}
		else if ( res1 == HT_OLD_ENTRY )
		{
			struct face_list_entry *fle;
			fle=ef->data;
			fle->route_cost=route_cost;
		}
		hashtb_end(ef);
		
		/*
		//updating the face for the router prefix itself
		if ( (rte->next_hop_face != NO_FACE  || rte->next_hop_face != NO_NEXT_HOP ) && is_neighbor(dest_router)==0 )
		{
			add_delete_ccn_face_by_face_id(nlsr->ccn, (const char *)dest_router, OP_UNREG, rte->next_hop_face);
		}
		if ( (next_hop_face != NO_FACE  || next_hop_face != NO_NEXT_HOP ) && is_neighbor(dest_router)==0 )
		{
			add_delete_ccn_face_by_face_id(nlsr->ccn, (const char *)dest_router, OP_REG, next_hop_face);
		}

		rte->next_hop_face=next_hop_face;
		*/
	}
	else if ( res == HT_OLD_ENTRY )
	{
		hashtb_delete(e);
	}
	
	hashtb_end(e);
	
}

void 
print_routing_table(void)
{
	printf("\n");
	printf("print_routing_table called\n");
	int i,j, rt_element,face_list_element;
	
	struct routing_table_entry *rte;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->routing_table, e);
	rt_element=hashtb_n(nlsr->routing_table);

	for(i=0;i<rt_element;i++)
	{
		printf("----------Routing Table Entry %d------------------\n",i+1);
		rte=e->data;
		printf(" Destination Router: %s \n",rte->dest_router);
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


int
delete_empty_rte(struct ccn_schedule *sched, void *clienth, struct ccn_scheduled_event *ev, int flags)
{
	printf("delete_empty_rte called\n");
	printf("Router: %s \n",(char *)ev->evdata);
	if(flags == CCN_SCHEDULE_CANCEL)
	{
 	 	return -1;
	}

	//struct routing_table_entry *rte;
	int res;
	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->routing_table, e);
	res = hashtb_seek(e, (char *)ev->evdata, strlen((char *)ev->evdata), 0);
	
	if ( res == HT_OLD_ENTRY )
	{
		//rte=e->data;

		/*		
	
		if ( (rte->next_hop_face != NO_FACE  || rte->next_hop_face != NO_NEXT_HOP) && is_neighbor(ev->evdata)==0 )
		{
			add_delete_ccn_face_by_face_id(nlsr->ccn, (const char *)ev->evdata, OP_UNREG, rte->next_hop_face);
		}

		*/
		hashtb_delete(e);
	}
	else if ( res == HT_NEW_ENTRY )
	{
		hashtb_delete(e);
	}

	print_routing_table();
	
	return 0;
}

void 
clear_old_routing_table(void)
{
	printf("clear_old_routing_table called\n");
	int i,rt_element;
	
	struct routing_table_entry *rte;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->routing_table, e);
	rt_element=hashtb_n(nlsr->routing_table);

	for(i=0;i<rt_element;i++)
	{
		rte=e->data;
		hashtb_destroy(&rte->face_list);
		struct hashtb_param param_fle = {0};
		rte->face_list=hashtb_create(sizeof(struct face_list_entry), &param_fle);

		hashtb_next(e);		
	}

	hashtb_end(e);	
}


void 
do_old_routing_table_updates(void)
{
	printf("do_old_routing_table_updates called\n");
	int i, rt_element;
	int mapping_no;	

	struct routing_table_entry *rte;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->routing_table, e);
	rt_element=hashtb_n(nlsr->routing_table);

	for(i=0;i<rt_element;i++)
	{
		rte=e->data;
		mapping_no=get_mapping_no(rte->dest_router);
		if ( mapping_no == NO_MAPPING_NUM)
		{		
			delete_orig_router_from_npt(rte->dest_router);
			char *router=(char *)malloc(strlen(rte->dest_router)+1);
			memset(router,0,strlen(rte->dest_router)+1);
			memcpy(router,rte->dest_router,strlen(rte->dest_router)+1);
			nlsr->event = ccn_schedule_event(nlsr->sched, 1, &delete_empty_rte, (void *)router , 0);
		}
		hashtb_next(e);		
	}

	hashtb_end(e);	
}



void 
update_routing_table_with_new_route(long int *parent, long int *dist,long int source)
{
	printf("update_routing_table_with_new_route called\n");
	int i, map_element;
	struct map_entry *me;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->rev_map, e);
	map_element=hashtb_n(nlsr->rev_map);

	for(i=0;i<map_element;i++)
	{
		me=e->data;
		if(me->mapping != source)
		{
			
			char *orig_router=get_router_from_rev_map(me->mapping);
			if (orig_router != NULL )
			{
				int next_hop_router_num=get_next_hop_from_calculation(parent,me->mapping,source);
				//printf(" Next hop router Num: %d ",next_hop_router_num);
				if ( next_hop_router_num == NO_NEXT_HOP )
				{
					//update_npt_with_new_route(orig_router,NO_FACE);
					printf ("\nOrig_router: %s Next Hop Face: %d \n",orig_router,NO_FACE);
				}
				else 
				{
					char *next_hop_router=get_router_from_rev_map(next_hop_router_num);
					//printf("Next hop router name: %s \n",next_hop_router);
					int next_hop_face=get_next_hop_face_from_adl(next_hop_router);
					//update_npt_with_new_route(orig_router,next_hop_face);
					update_routing_table(orig_router,next_hop_face,dist[me->mapping]);
					printf ("Orig_router: %s Next Hop Face: %d \n",orig_router,next_hop_face);

				}
			}
		}
		hashtb_next(e);		
	}

	hashtb_end(e);
}

int 
does_face_exist_for_router(char *dest_router, int face_id)
{
	int ret=0;

	int res,res1;
	struct routing_table_entry *rte;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->routing_table, e);
	res = hashtb_seek(e, dest_router, strlen(dest_router), 0);

	if( res == HT_OLD_ENTRY )
	{
		rte=e->data;
		struct hashtb_enumerator eef;
    		struct hashtb_enumerator *ef = &eef;
    	
    		hashtb_start(rte->face_list, ef);
		res1 = hashtb_seek(ef, &face_id, sizeof(face_id), 0);	
		if( res1 == HT_OLD_ENTRY)
		{
			ret=1;									
		}
		else if ( res1 == HT_OLD_ENTRY )
		{
			hashtb_delete(ef);
		}
		hashtb_end(ef);
	}
	else if( res == HT_NEW_ENTRY )
	{
		hashtb_delete(e);
	} 
	
	hashtb_end(e);

	return ret;
}
