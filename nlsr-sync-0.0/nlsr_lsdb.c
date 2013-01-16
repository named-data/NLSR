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
#include "nlsr_lsdb.h"
#include "utility.h"
#include "nlsr_npl.h"
#include "nlsr_adl.h"
#include "nlsr_route.h"
#include "nlsr_npt.h"
#include "nlsr_sync.h"

void
set_new_lsdb_version(void)
{
//Obaid: this method needs to be modified, no need to free and allocate memory again
	char *time_stamp=(char *)malloc(20);
	memset(time_stamp,0,20);
	get_current_timestamp_micro(time_stamp);
	
	free(nlsr->lsdb->lsdb_version);
	nlsr->lsdb->lsdb_version=(char *)malloc(strlen(time_stamp)+1);
	memset(nlsr->lsdb->lsdb_version,0,strlen(time_stamp)+1);
	memcpy(nlsr->lsdb->lsdb_version,time_stamp,strlen(time_stamp)+1);

	free(time_stamp);
	
}

void 
make_name_lsa_key(char *key, char *orig_router, int ls_type, long int ls_id)
{
	
	char lst[2];
	memset(lst,0,2);
	sprintf(lst,"%d",ls_type);	

	char lsid[10];
	memset(lsid,0,10);
	sprintf(lsid,"%ld",ls_id);
	
	memcpy(key+strlen(key),orig_router,strlen(orig_router));
	memcpy(key+strlen(key),"/",1);
	memcpy(key+strlen(key),lst,strlen(lst));
	memcpy(key+strlen(key),"/",1);
	memcpy(key+strlen(key),lsid,strlen(lsid));
	printf("%s\n", key);
}


void 
make_name_lsa_prefix_for_repo(char *key, char *orig_router, int ls_type, long int ls_id,char *orig_time,char *slice_prefix)
{
	sprintf(key,"%s/%d/%ld/%s%s",slice_prefix, ls_type, ls_id, orig_time, orig_router);
	printf("%s\n",key);
}

void 
make_adj_lsa_prefix_for_repo(char *key, char *orig_router, int ls_type, char *orig_time,char *slice_prefix)
{
	
/*	char lst[2];
	memset(lst,0,2);
	sprintf(lst,"%d",ls_type);
		
	
	memcpy(key+strlen(key),slice_prefix,strlen(slice_prefix));
	memcpy(key+strlen(key),"/",1);
	memcpy(key+strlen(key),lst,strlen(lst));
	memcpy(key+strlen(key),"/",1);
	memcpy(key+strlen(key),orig_time,strlen(orig_time));
	memcpy(key+strlen(key),orig_router,strlen(orig_router));
*/	
	sprintf(key,"%s/%d/%s%s",slice_prefix,ls_type, orig_time, orig_router);	
	printf("Key:%s\n",key);	
}

void 
build_and_install_name_lsas(void)
{
	if ( nlsr->debugging )
		printf("build_and_install_name_lsas called \n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"build_and_install_name_lsas called\n");

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
		struct nlsa *name_lsa=(struct nlsa *)malloc(sizeof( struct nlsa ));
		build_name_lsa(name_lsa,npe->np);
		
		install_name_lsa(name_lsa);
		update_nlsa_id_for_name_in_npl(npe->np,name_lsa->header->ls_id);
		free(name_lsa->header->orig_router->name);
		free(name_lsa->header->orig_router);
		free(name_lsa->header);
		free(name_lsa->name_prefix->name);
		free(name_lsa->name_prefix);
		free(name_lsa);
		hashtb_next(e);		
	}

	hashtb_end(e);	
	
	print_name_prefix_from_npl();

}

void 
build_and_install_single_name_lsa(struct name_prefix *np)
{
	if ( nlsr->debugging )
		printf("build_and_install_single_name_lsa called \n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"build_and_install_single_name_lsa called\n");
	
	struct nlsa *name_lsa=(struct nlsa *)malloc(sizeof( struct nlsa ));
	build_name_lsa(name_lsa,np);
		
	install_name_lsa(name_lsa);
	update_nlsa_id_for_name_in_npl(np,name_lsa->header->ls_id);

	free(name_lsa->header->orig_router->name);
	free(name_lsa->header->orig_router);
	free(name_lsa->header);
	free(name_lsa->name_prefix->name);
	free(name_lsa->name_prefix);
	free(name_lsa);
	
	print_name_prefix_from_npl();

}

void 
build_name_lsa(struct nlsa *name_lsa, struct name_prefix *np)
{
	name_lsa->header=(struct nlsa_header *)malloc(sizeof(struct nlsa_header ));
	name_lsa->header->ls_type=LS_TYPE_NAME;

	char *time_stamp=(char *)malloc(20);
	memset(time_stamp,0,20);
	get_current_timestamp_micro(time_stamp);

	name_lsa->header->orig_time=(char *)malloc(strlen(time_stamp)+1); //free 
	memset(name_lsa->header->orig_time,0,strlen(time_stamp)+1);
	memcpy(name_lsa->header->orig_time,time_stamp,strlen(time_stamp)+1);
	
	free(time_stamp);

	name_lsa->header->ls_id=++nlsr->nlsa_id;
	name_lsa->header->orig_router=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
	name_lsa->header->orig_router->name=(char *)malloc(strlen(nlsr->router_name)+1);
	memset(name_lsa->header->orig_router->name,0,strlen(nlsr->router_name)+1);
	memcpy(name_lsa->header->orig_router->name,nlsr->router_name,strlen(nlsr->router_name)+1);
	name_lsa->header->orig_router->length=strlen(nlsr->router_name)+1;
	name_lsa->header->isValid=1;

	
	name_lsa->name_prefix=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
	name_lsa->name_prefix->name=(char *)malloc(np->length);
	memcpy(name_lsa->name_prefix->name,np->name,np->length);
	name_lsa->name_prefix->length=np->length;

}

void 
install_name_lsa(struct nlsa *name_lsa)
{

	char *time_stamp=(char *)malloc(20);
	memset(time_stamp,0,20);
	get_current_timestamp_micro(time_stamp);
	//long int lsa_life_time=get_time_diff(time_stamp,name_lsa->header->orig_time);

	//printf("time difference: %ld \n",lsa_life_time);
	

		char lst[2];
		memset(lst,0,2);
		sprintf(lst,"%d",name_lsa->header->ls_type);	

		char lsid[10];
		memset(lsid,0,10);
		sprintf(lsid,"%ld",name_lsa->header->ls_id);
	
	
		char *key=(char *)malloc(strlen(name_lsa->header->orig_router->name)+1+strlen(lst)+1+strlen(lsid)+1);
		memset(key,0,strlen(name_lsa->header->orig_router->name)+1+strlen(lst)+1+strlen(lsid)+1);


		make_name_lsa_key(key, name_lsa->header->orig_router->name,name_lsa->header->ls_type,name_lsa->header->ls_id);
		if ( nlsr->debugging )
			printf("Key:%s Length:%d\n",key,(int)strlen(key));
		if ( nlsr->detailed_logging )
			writeLogg(__FILE__,__FUNCTION__,__LINE__,"Key:%s Length:%d\n",key,(int)strlen(key));	
		

		struct nlsa *new_name_lsa=(struct nlsa*)malloc(sizeof(struct nlsa )); //free

		struct hashtb_enumerator ee;
    		struct hashtb_enumerator *e = &ee; 	
    		int res;

   		hashtb_start(nlsr->lsdb->name_lsdb, e);
    		res = hashtb_seek(e, key, strlen(key), 0);

		if(res == HT_NEW_ENTRY )
		{
		
			if ( nlsr->debugging )
				printf("New Name LSA... Adding to LSDB\n");
			if ( nlsr->detailed_logging )
				writeLogg(__FILE__,__FUNCTION__,__LINE__,"New Name LSA... Adding to LSDB\n");


			new_name_lsa = e->data;

			new_name_lsa->header=(struct nlsa_header *)malloc(sizeof(struct nlsa_header )); //free
			new_name_lsa->header->ls_type=name_lsa->header->ls_type;

			new_name_lsa->header->orig_time=(char *)malloc(strlen(name_lsa->header->orig_time)+1);
			memset(new_name_lsa->header->orig_time,0,strlen(name_lsa->header->orig_time)+1);
			memcpy(new_name_lsa->header->orig_time,name_lsa->header->orig_time,strlen(name_lsa->header->orig_time)+1);

			new_name_lsa->header->ls_id=name_lsa->header->ls_id;
			new_name_lsa->header->orig_router=(struct name_prefix *)malloc(sizeof(struct name_prefix )); //free
			new_name_lsa->header->orig_router->name=(char *)malloc(name_lsa->header->orig_router->length);
			memcpy(new_name_lsa->header->orig_router->name,name_lsa->header->orig_router->name,name_lsa->header->orig_router->length);
			new_name_lsa->header->orig_router->length=name_lsa->header->orig_router->length;
			new_name_lsa->header->isValid=name_lsa->header->isValid;

	
			new_name_lsa->name_prefix=(struct name_prefix *)malloc(sizeof(struct name_prefix )); //free
			new_name_lsa->name_prefix->name=(char *)malloc(name_lsa->name_prefix->length);
			memcpy(new_name_lsa->name_prefix->name,name_lsa->name_prefix->name,name_lsa->name_prefix->length);
			new_name_lsa->name_prefix->length=name_lsa->name_prefix->length;

			if ( nlsr->debugging )
			{
				printf("New Name LSA Added....\n");	
				printf("Old Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);
			}
			if ( nlsr->detailed_logging )
			{
				writeLogg(__FILE__,__FUNCTION__,__LINE__,"New Name LSA Added....\n");	
				writeLogg(__FILE__,__FUNCTION__,__LINE__,"Old Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);
			}
			set_new_lsdb_version();	
			if ( nlsr->debugging )
				printf("New Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);
			if ( nlsr->detailed_logging )
				writeLogg(__FILE__,__FUNCTION__,__LINE__,"New Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);	

			
			int num_next_hop=get_number_of_next_hop(new_name_lsa->header->orig_router->name);
			if ( num_next_hop < 0 )
			{
				int check=add_npt_entry(new_name_lsa->header->orig_router->name,new_name_lsa->name_prefix->name,NO_NEXT_HOP,NULL,NULL);
				if ( check == HT_NEW_ENTRY )
				{
					if ( nlsr->debugging )
						printf("Added in npt \n");
					if ( nlsr->detailed_logging )
						writeLogg(__FILE__,__FUNCTION__,__LINE__,"Added in npt \n");
				}
			}
			else 
			{
				int *faces=malloc(num_next_hop*sizeof(int));
				int *route_costs=malloc(num_next_hop*sizeof(int));			
				int next_hop=get_next_hop(new_name_lsa->header->orig_router->name,faces,route_costs);
				if ( nlsr->debugging )
				{
					printf("Printing from install_name_lsa \n");
					int j;
					for(j=0;j<num_next_hop;j++)
						printf("Face: %d Route Cost: %d \n",faces[j],route_costs[j]);
				}
				if ( nlsr->detailed_logging )
				{
					writeLogg(__FILE__,__FUNCTION__,__LINE__,"Printing from install_name_lsa \n");
					int j;
					for(j=0;j<num_next_hop;j++)
						writeLogg(__FILE__,__FUNCTION__,__LINE__,"Face: %d Route Cost: %d \n",faces[j],route_costs[j]);
				}
				int check=add_npt_entry(new_name_lsa->header->orig_router->name,new_name_lsa->name_prefix->name,next_hop,faces,route_costs);
				if ( check == HT_NEW_ENTRY )
				{
					if ( nlsr->debugging )
						printf("Added in npt \n");
					if ( nlsr->detailed_logging )
						writeLogg(__FILE__,__FUNCTION__,__LINE__,"Added in npt \n");
				}
				free(faces);
				free(route_costs);

			}
			
			writeLogg(__FILE__,__FUNCTION__,__LINE__," Name-LSA\n");
			writeLogg(__FILE__,__FUNCTION__,__LINE__," Adding name lsa\n");
			write_log_for_name_lsa(new_name_lsa);
			writeLogg(__FILE__,__FUNCTION__,__LINE__," name_lsa_end\n");
		
			free(time_stamp);

		}
		else if(res == HT_OLD_ENTRY)
		{
			new_name_lsa=e->data;
			if(strcmp(name_lsa->header->orig_time,new_name_lsa->header->orig_time)<0)
			{
				if ( nlsr->debugging )
					printf("Older Adj LSA. Discarded... \n");
				if ( nlsr->detailed_logging )
					writeLogg(__FILE__,__FUNCTION__,__LINE__,"Older Adj LSA. Discarded...\n");
			}
			else if( strcmp(name_lsa->header->orig_time,new_name_lsa->header->orig_time) == 0 )
			{
				if ( nlsr->debugging )
					printf("Duplicate Adj LSA. Discarded... \n");
				if ( nlsr->detailed_logging )
					writeLogg(__FILE__,__FUNCTION__,__LINE__,"Duplicate Adj LSA. Discarded...\n");
			}
			else 
			{
				if ( name_lsa->header->isValid == 0 )
				{
					// have to call to delete npt table entry
					delete_npt_entry_by_router_and_name_prefix(new_name_lsa->header->orig_router->name,new_name_lsa->name_prefix->name);
				
					if ( strcmp(name_lsa->header->orig_router->name,nlsr->router_name)!= 0)
					{
						writeLogg(__FILE__,__FUNCTION__,__LINE__," Name-LSA\n");
						writeLogg(__FILE__,__FUNCTION__,__LINE__," Deleting name lsa\n");
						write_log_for_name_lsa(new_name_lsa);
						writeLogg(__FILE__,__FUNCTION__,__LINE__," name_lsa_end\n");
						
						hashtb_delete(e);
						if ( nlsr->debugging )
							printf("isValid bit not set for Router %s so LSA Deleted from LSDB\n",name_lsa->header->orig_router->name);
						if ( nlsr->detailed_logging )
							writeLogg(__FILE__,__FUNCTION__,__LINE__,"isValid bit not set for Router %s so LSA Deleted from LSDB\n",name_lsa->header->orig_router->name);
					}
					else 
					{
						new_name_lsa->header->isValid=name_lsa->header->isValid;
						free(new_name_lsa->header->orig_time);
						new_name_lsa->header->orig_time=(char *)malloc(strlen(name_lsa->header->orig_time)+1);
						memset(new_name_lsa->header->orig_time,0,strlen(name_lsa->header->orig_time)+1);
						memcpy(new_name_lsa->header->orig_time,name_lsa->header->orig_time,strlen(name_lsa->header->orig_time)+1);
					}
					if ( nlsr->debugging )
						printf("Old Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);
					if ( nlsr->detailed_logging )
						writeLogg(__FILE__,__FUNCTION__,__LINE__,"Old Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);
					set_new_lsdb_version();	
					if ( nlsr->debugging )
						printf("New Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);
					if ( nlsr->detailed_logging )
						writeLogg(__FILE__,__FUNCTION__,__LINE__,"New Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);
				}
				else
				{
					int is_npt_update=0;
					if ( strcmp(new_name_lsa->name_prefix->name,name_lsa->name_prefix->name) != 0 )
					{
						is_npt_update=1;
						delete_npt_entry_by_router_and_name_prefix(new_name_lsa->header->orig_router->name,new_name_lsa->name_prefix->name);
					}

					// copying LSA content with header
					writeLogg(__FILE__,__FUNCTION__,__LINE__," Name-LSA\n");
					writeLogg(__FILE__,__FUNCTION__,__LINE__," Deleting name lsa\n");
					write_log_for_name_lsa(new_name_lsa);
					writeLogg(__FILE__,__FUNCTION__,__LINE__," name_lsa_end\n");					


					free(new_name_lsa->header->orig_time);
					new_name_lsa->header->orig_time=(char *)malloc(strlen(name_lsa->header->orig_time)+1);
					memset(new_name_lsa->header->orig_time,0,strlen(name_lsa->header->orig_time)+1);
					memcpy(new_name_lsa->header->orig_time,name_lsa->header->orig_time,strlen(name_lsa->header->orig_time)+1);
				
					new_name_lsa->header->isValid=name_lsa->header->isValid;

					free(new_name_lsa->name_prefix->name);
					free(new_name_lsa->name_prefix);
					new_name_lsa->name_prefix=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
					new_name_lsa->name_prefix->name=(char *)malloc(name_lsa->name_prefix->length);
					memcpy(new_name_lsa->name_prefix->name,name_lsa->name_prefix->name,name_lsa->name_prefix->length);
					new_name_lsa->name_prefix->length=name_lsa->name_prefix->length;


					writeLogg(__FILE__,__FUNCTION__,__LINE__," Name-LSA\n");
					writeLogg(__FILE__,__FUNCTION__,__LINE__," Adding name lsa\n");
					write_log_for_name_lsa(new_name_lsa);
					writeLogg(__FILE__,__FUNCTION__,__LINE__," name_lsa_end\n");

					if ( nlsr->debugging )
						printf("Old Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);
					if ( nlsr->detailed_logging )
						writeLogg(__FILE__,__FUNCTION__,__LINE__,"Old Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);

					set_new_lsdb_version();	

					if ( nlsr->debugging )
						printf("New Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);
					if ( nlsr->detailed_logging )
						writeLogg(__FILE__,__FUNCTION__,__LINE__,"New Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);


					if( is_npt_update == 1 )
					{
						//struct hashtb *face_list;
						int num_next_hop=get_number_of_next_hop(new_name_lsa->header->orig_router->name);
						if ( num_next_hop < 0 )
						{
							
							int check=add_npt_entry(new_name_lsa->header->orig_router->name,new_name_lsa->name_prefix->name,NO_NEXT_HOP,NULL,NULL);
							if ( check == HT_NEW_ENTRY )
							{
								if ( nlsr->debugging )
									printf("Added in npt \n");
								if ( nlsr->detailed_logging )
									writeLogg(__FILE__,__FUNCTION__,__LINE__,"Added in npt \n");
							}
						}
						else 
						{
							int *faces=malloc(num_next_hop*sizeof(int));
							int *route_costs=malloc(num_next_hop*sizeof(int));			
							int next_hop=get_next_hop(new_name_lsa->header->orig_router->name,faces,route_costs);
							
							if ( nlsr->debugging )
							{
								printf("Printing from install_name_lsa \n");
								int j;
								for(j=0;j<num_next_hop;j++)
								printf("Face: %d Route Cost: %d \n",faces[j],route_costs[j]);
							}
							if ( nlsr->detailed_logging )
							{
								writeLogg(__FILE__,__FUNCTION__,__LINE__,"Printing from install_name_lsa \n");
								int j;
								for(j=0;j<num_next_hop;j++)
									writeLogg(__FILE__,__FUNCTION__,__LINE__,"Face: %d Route Cost: %d \n",faces[j],route_costs[j]);
							}

							
							int check=add_npt_entry(new_name_lsa->header->orig_router->name,new_name_lsa->name_prefix->name,next_hop,faces,route_costs);
							if ( check == HT_NEW_ENTRY )
							{
								//printf("Added in npt \n");
								if ( nlsr->debugging )
									printf("Added in npt \n");
								if ( nlsr->detailed_logging )
									writeLogg(__FILE__,__FUNCTION__,__LINE__,"Added in npt \n");
							}
							free(faces);
							free(route_costs);
							
						}
						
					}
				}
			}
		
		}

    		hashtb_end(e);

		free(key);
}

void 
write_log_for_name_lsa(struct nlsa *name_lsa)
{
	
	writeLogg(__FILE__,__FUNCTION__,__LINE__,"-----------Name LSA Content---------------\n");
	writeLogg(__FILE__,__FUNCTION__,__LINE__,"	Origination Router: %s\n",name_lsa->header->orig_router->name);
	writeLogg(__FILE__,__FUNCTION__,__LINE__,"	Origination Router Length:	%d\n",name_lsa->header->orig_router->length);
	writeLogg(__FILE__,__FUNCTION__,__LINE__,"	LS Type:	%d\n",name_lsa->header->ls_type);
	writeLogg(__FILE__,__FUNCTION__,__LINE__,"	LS Id:	%ld\n",name_lsa->header->ls_id);
	writeLogg(__FILE__,__FUNCTION__,__LINE__,"	Origination Time:	%s\n",name_lsa->header->orig_time);
	writeLogg(__FILE__,__FUNCTION__,__LINE__,"	Is Valid:	%d\n",name_lsa->header->isValid);
	writeLogg(__FILE__,__FUNCTION__,__LINE__,"	LSA Data			\n");
	writeLogg(__FILE__,__FUNCTION__,__LINE__,"		Name Prefix: %s\n",name_lsa->name_prefix->name);
	writeLogg(__FILE__,__FUNCTION__,__LINE__,"		Name Prefix Length:	%d\n",name_lsa->name_prefix->length);
	writeLogg(__FILE__,__FUNCTION__,__LINE__,"\n");	
}

void 
print_name_lsa(struct nlsa *name_lsa)
{

	if ( nlsr->debugging )
	{
		printf("-----------Name LSA Content---------------\n");
		printf("	Origination Router       :	%s\n",name_lsa->header->orig_router->name);
		printf("	Origination Router Length:	%d\n",name_lsa->header->orig_router->length);
		printf("	LS Type			 :	%d\n",name_lsa->header->ls_type);
		printf("	LS Id			 :	%ld\n",name_lsa->header->ls_id);
		printf("	Origination Time	 :	%s\n",name_lsa->header->orig_time);
		printf("	Is Valid 		 :	%d\n",name_lsa->header->isValid);
		printf("	LSA Data			\n");
		printf("		Name Prefix:	 	:	%s\n",name_lsa->name_prefix->name);
		printf("		Name Prefix Length	:	%d\n",name_lsa->name_prefix->length);

		printf("\n");	
	}
}

void
print_name_lsdb(void)
{
	if ( nlsr->debugging )
		printf("print_name_lsdb called \n");	
	int i, name_lsdb_element;
	struct nlsa *name_lsa;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->lsdb->name_lsdb, e);
	name_lsdb_element=hashtb_n(nlsr->lsdb->name_lsdb);

	for(i=0;i<name_lsdb_element;i++)
	{
		if ( nlsr->debugging )
			printf("-----------Name LSA (%d)---------------\n",i+1);
		name_lsa=e->data;
		print_name_lsa(name_lsa);	
		hashtb_next(e);		
	}

	hashtb_end(e);

	if ( nlsr->debugging )
		printf("\n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"\n");
}


void
build_and_install_others_name_lsa(char *orig_router,int ls_type,long int ls_id,char *orig_time, int isValid,char *np)
{
	if ( nlsr->debugging )
		printf("build_and_install_others_name_lsa called \n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"build_and_install_others_name_lsa called \n");

	struct nlsa *name_lsa=(struct nlsa *)malloc(sizeof( struct nlsa ));
	build_others_name_lsa(name_lsa,orig_router,ls_type,ls_id,orig_time, isValid,np);
	print_name_lsa(name_lsa);
	install_name_lsa(name_lsa);
	print_name_lsdb();
	print_npt();
	
	free(name_lsa->header->orig_router->name);
	free(name_lsa->header->orig_router);
	free(name_lsa->header);
	free(name_lsa->name_prefix->name);
	free(name_lsa->name_prefix);
	free(name_lsa);
	
}

void
build_others_name_lsa(struct nlsa *name_lsa, char *orig_router,int ls_type,long int ls_id,char *orig_time, int isValid,char *np)
{
	if ( nlsr->debugging )
		printf("build_others_name_lsa called\n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"build_others_name_lsa called \n");

	name_lsa->header=(struct nlsa_header *)malloc(sizeof(struct nlsa_header ));
	name_lsa->header->ls_type=LS_TYPE_NAME;

	name_lsa->header->orig_time=(char *)malloc(strlen(orig_time)+1);
	memset(name_lsa->header->orig_time,0,strlen(orig_time)+1);
	memcpy(name_lsa->header->orig_time,orig_time,strlen(orig_time)+1);

	name_lsa->header->ls_id=ls_id;
	name_lsa->header->orig_router=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
	name_lsa->header->orig_router->name=(char *)malloc(strlen(orig_router)+1);
	memset(name_lsa->header->orig_router->name,0,strlen(orig_router)+1);
	memcpy(name_lsa->header->orig_router->name,orig_router,strlen(orig_router)+1);
	name_lsa->header->orig_router->length=strlen(orig_router)+1;
	name_lsa->header->isValid=isValid;

	name_lsa->name_prefix=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
	name_lsa->name_prefix->name=(char *)malloc(strlen(np)+1);
	memset(name_lsa->name_prefix->name,0,strlen(np)+1);
	memcpy(name_lsa->name_prefix->name,np,strlen(np)+1);
	name_lsa->name_prefix->length=strlen(np)+1;
}




void 
make_adj_lsa_key(char *key,struct alsa *adj_lsa)
{
	memcpy(key+strlen(key),adj_lsa->header->orig_router->name,adj_lsa->header->orig_router->length);
	memcpy(key+strlen(key),"/",1);
	char ls_type[2];
	sprintf(ls_type,"%d",adj_lsa->header->ls_type);
	memcpy(key+strlen(key),ls_type,strlen(ls_type));
	key[strlen(key)]='\0';
}

int
build_and_install_adj_lsa(struct ccn_schedule *sched, void *clienth, struct ccn_scheduled_event *ev, int flags)
{
	if(flags == CCN_SCHEDULE_CANCEL)
	{
 	 	return -1;
	}

	nlsr_lock();

	if ( nlsr->debugging )
		printf("build_and_install_adj_lsa called \n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"build_and_install_adj_lsa called \n");
	
	if ( nlsr->debugging )
		printf("adj_build_flag = %d \n",nlsr->adj_build_flag);
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"adj_build_flag = %d \n",nlsr->adj_build_flag);

	if(nlsr->adj_build_flag > 0)
	{
		if ( nlsr->debugging )
			printf("is_adj_lsa_build = %d \n",is_adj_lsa_build());
		if ( nlsr->detailed_logging )
			writeLogg(__FILE__,__FUNCTION__,__LINE__,"is_adj_lsa_build = %d \n",is_adj_lsa_build());

		if ( is_adj_lsa_build()> 0)
		{
			struct alsa *adj_lsa=(struct alsa *)malloc(sizeof( struct alsa ));
			build_adj_lsa(adj_lsa);
			install_adj_lsa(adj_lsa);

			char lst[2];
			memset(lst,0,2);
			sprintf(lst,"%d",LS_TYPE_ADJ);			

			char *repo_con_name=(char *)malloc(strlen(nlsr->slice_prefix)+strlen(adj_lsa->header->orig_time)+strlen(adj_lsa->header->orig_router->name) + strlen(lst) + 5);
			memset(repo_con_name, 0, strlen(nlsr->slice_prefix)+strlen(adj_lsa->header->orig_time)+strlen(adj_lsa->header->orig_router->name) + strlen(lst) + 5);	
			make_adj_lsa_prefix_for_repo(repo_con_name, adj_lsa->header->orig_router->name,LS_TYPE_ADJ,adj_lsa->header->orig_time,nlsr->slice_prefix);
			if ( nlsr->debugging )
				printf("Adj LSA Repo Key: %s \n",repo_con_name);
			
			char *key=(char *)malloc(adj_lsa->header->orig_router->length+2+2);
			memset(key,0,adj_lsa->header->orig_router->length+2);
			make_adj_lsa_key(key,adj_lsa);
			if ( nlsr->debugging )
				printf("Adj LSA: %s \n",key);
			struct name_prefix *lsaid=(struct name_prefix *)malloc(sizeof(struct name_prefix));
			lsaid->name=(char *)malloc(strlen(key)+1);
			memset(lsaid->name, 0, strlen(key)+1);
			memcpy(lsaid->name,key,strlen(key));
			lsaid->length=strlen(key)+1;

		
			write_adj_lsa_to_repo(repo_con_name, lsaid);
		
			free(adj_lsa->header->orig_router->name);
			free(adj_lsa->header->orig_router);
			free(adj_lsa->header->orig_time);
			free(adj_lsa->header);
			free(adj_lsa->body);
			free(adj_lsa);
			nlsr->adj_build_flag=0;	
			print_adj_lsdb();		
		}
		else
		{
			if ( nlsr->debugging )
				printf("Can not build adj LSA now\n");
			if ( nlsr->detailed_logging )
				writeLogg(__FILE__,__FUNCTION__,__LINE__,"Can not build adj LSA now\n");
		}
	}
	nlsr->is_build_adj_lsa_sheduled=0;

	nlsr_unlock();

	return 0;
}


void
build_adj_lsa(struct alsa * adj_lsa)
{
	if ( nlsr->debugging )
		printf("build_adj_lsa called \n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"build_adj_lsa called  \n");

	int no_link=no_active_nbr();
	
	//printf("Number of link in Adjacent LSA: %d\n",no_link);

	/*Filling Up Header Data */
	adj_lsa->header=(struct alsa_header *)malloc(sizeof(struct alsa_header ));
	adj_lsa->header->orig_router=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
	adj_lsa->header->orig_router->name=(char *)malloc(strlen(nlsr->router_name)+1);
	memset(adj_lsa->header->orig_router->name,0,strlen(nlsr->router_name)+1);
	memcpy(adj_lsa->header->orig_router->name,nlsr->router_name,strlen(nlsr->router_name)+1);
	adj_lsa->header->orig_router->length=strlen(nlsr->router_name)+1;

	adj_lsa->header->ls_type=(unsigned)LS_TYPE_ADJ;	

	char *time_stamp=(char *)malloc(20);
	memset(time_stamp,0,20);
	get_current_timestamp_micro(time_stamp);

	adj_lsa->header->orig_time=(char *)malloc(strlen(time_stamp)+1); 
	memset(adj_lsa->header->orig_time,0,strlen(time_stamp)+1);
	memcpy(adj_lsa->header->orig_time,time_stamp,strlen(time_stamp)+1);	
	free(time_stamp);


	/* Filling Up Body Data */

	adj_lsa->no_link=no_link;


	struct ccn_charbuf *c=ccn_charbuf_create();
	get_active_nbr_adj_data(c);
	char *data=ccn_charbuf_as_string(c);

	adj_lsa->body=(char *)malloc(strlen(data)+1);
	memset(adj_lsa->body,0,strlen(data)+1);
	memcpy(adj_lsa->body,(char *)data,strlen(data)+1);
	ccn_charbuf_destroy(&c);



	/*if( !nlsr->is_send_lsdb_interest_scheduled )
	{	
		nlsr->event_send_lsdb_interest= ccn_schedule_event(nlsr->sched, 1000, &send_lsdb_interest, NULL, 0);
		nlsr->is_send_lsdb_interest_scheduled=1;
	}*/

	nlsr->adj_build_count++;


}


void
install_adj_lsa(struct alsa * adj_lsa)
{

	if ( nlsr->debugging )
		printf("install_adj_lsa called \n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"install_adj_lsa called  \n");
	

	char *time_stamp=(char *)malloc(20);
	memset(time_stamp,0,20);
	get_current_timestamp_micro(time_stamp);
	//long int lsa_life_time=get_time_diff(time_stamp,adj_lsa->header->orig_time);

	//printf("time difference: %ld \n",lsa_life_time);


		char *key=(char *)malloc(adj_lsa->header->orig_router->length+2+2);
		memset(key,0,adj_lsa->header->orig_router->length+2);
		make_adj_lsa_key(key,adj_lsa);
		//printf("Adjacent LSA key: %s \n",key);

		struct alsa *new_adj_lsa=(struct alsa*)malloc(sizeof(struct alsa ));

		struct hashtb_enumerator ee;
    		struct hashtb_enumerator *e = &ee; 	
    		int res;

   		hashtb_start(nlsr->lsdb->adj_lsdb, e);
    		res = hashtb_seek(e, key, strlen(key), 0);



		if(res == HT_NEW_ENTRY)
		{
			if ( adj_lsa->no_link > 0)
			{
				if ( nlsr->debugging )
					printf("New ADJ LSA... Adding to LSDB\n");
				if ( nlsr->detailed_logging )
					writeLogg(__FILE__,__FUNCTION__,__LINE__,"New ADJ LSA... Adding to LSDB\n");
				new_adj_lsa = e->data;

				new_adj_lsa->header=(struct alsa_header *)malloc(sizeof(struct alsa_header ));
				new_adj_lsa->header->ls_type=adj_lsa->header->ls_type;
				new_adj_lsa->header->orig_time=(char *)malloc(strlen(adj_lsa->header->orig_time)+1);
				memcpy(new_adj_lsa->header->orig_time,adj_lsa->header->orig_time,strlen(adj_lsa->header->orig_time)+1);		

				new_adj_lsa->header->orig_router=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
				new_adj_lsa->header->orig_router->name=(char *)malloc(adj_lsa->header->orig_router->length);
				memcpy(new_adj_lsa->header->orig_router->name,adj_lsa->header->orig_router->name,adj_lsa->header->orig_router->length);
				new_adj_lsa->header->orig_router->length=adj_lsa->header->orig_router->length;

				new_adj_lsa->no_link=adj_lsa->no_link;
		
				new_adj_lsa->body=(char *)malloc(strlen(adj_lsa->body)+1);
				memset(new_adj_lsa->body,0,strlen(adj_lsa->body)+1);
				memcpy(new_adj_lsa->body,adj_lsa->body,strlen(adj_lsa->body)+1);
			
				add_next_hop_router(new_adj_lsa->header->orig_router->name);
				add_next_hop_from_lsa_adj_body(new_adj_lsa->body,new_adj_lsa->no_link);

				writeLogg(__FILE__,__FUNCTION__,__LINE__," Adj-LSA\n");
				writeLogg(__FILE__,__FUNCTION__,__LINE__," Adding adj lsa\n");
				write_log_for_adj_lsa(new_adj_lsa);
				writeLogg(__FILE__,__FUNCTION__,__LINE__," adj_lsa_end\n");
			}
			else 
			{
				hashtb_delete(e);
			}

			if ( nlsr->debugging )
				printf("Old Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);
			if ( nlsr->detailed_logging )
				writeLogg(__FILE__,__FUNCTION__,__LINE__,"Old Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);

			set_new_lsdb_version();	

			if ( nlsr->debugging )
				printf("New Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);
			if ( nlsr->detailed_logging )
				writeLogg(__FILE__,__FUNCTION__,__LINE__,"New Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);

		}
		else if(res == HT_OLD_ENTRY)
		{
			new_adj_lsa = e->data;
			if(strcmp(adj_lsa->header->orig_time,new_adj_lsa->header->orig_time)<=0)
			{
				if ( nlsr->debugging )
					printf("Older/Duplicate Adj LSA. Discarded...\n");
				if ( nlsr->detailed_logging )
					writeLogg(__FILE__,__FUNCTION__,__LINE__,"Older/Duplicate Adj LSA. Discarded...\n");
			}
			else
			{

				if ( adj_lsa->no_link > 0)
				{				
					//new_adj_lsa = e->data;

					writeLogg(__FILE__,__FUNCTION__,__LINE__," Adj-LSA\n");
					writeLogg(__FILE__,__FUNCTION__,__LINE__," Deleting adj lsa\n");
					write_log_for_adj_lsa(new_adj_lsa);
					writeLogg(__FILE__,__FUNCTION__,__LINE__," adj_lsa_end\n");

					free(new_adj_lsa->header->orig_time);
					new_adj_lsa->header->orig_time=(char *)malloc(strlen(adj_lsa->header->orig_time)+1);
					memcpy(new_adj_lsa->header->orig_time,adj_lsa->header->orig_time,strlen(adj_lsa->header->orig_time)+1);

					new_adj_lsa->no_link=adj_lsa->no_link;
				
					new_adj_lsa->body=(char *)malloc(strlen(adj_lsa->body)+1);
					memset(new_adj_lsa->body,0,strlen(adj_lsa->body)+1);
					memcpy(new_adj_lsa->body,adj_lsa->body,strlen(adj_lsa->body)+1);

					add_next_hop_from_lsa_adj_body(new_adj_lsa->body,new_adj_lsa->no_link);

					writeLogg(__FILE__,__FUNCTION__,__LINE__," Adj-LSA\n");
					writeLogg(__FILE__,__FUNCTION__,__LINE__," Adding adj lsa\n");
					write_log_for_adj_lsa(new_adj_lsa);
					writeLogg(__FILE__,__FUNCTION__,__LINE__," adj_lsa_end\n");
				}
				else
				{
					writeLogg(__FILE__,__FUNCTION__,__LINE__," Adj-LSA\n");
					writeLogg(__FILE__,__FUNCTION__,__LINE__," Deleting adj lsa\n");
					write_log_for_adj_lsa(new_adj_lsa);
					writeLogg(__FILE__,__FUNCTION__,__LINE__," adj_lsa_end\n");
					
					hashtb_delete(e);
				}
				
				if ( nlsr->debugging )
					printf("Old Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);
				if ( nlsr->detailed_logging )
					writeLogg(__FILE__,__FUNCTION__,__LINE__,"Old Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);

				set_new_lsdb_version();	

				if ( nlsr->debugging )
					printf("New Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);
				if ( nlsr->detailed_logging )
					writeLogg(__FILE__,__FUNCTION__,__LINE__,"New Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);
			}
	
		}
	    	hashtb_end(e);

		if ( !nlsr->is_route_calculation_scheduled )
		{
			nlsr->event_calculate_route = ccn_schedule_event(nlsr->sched, 1000000, &route_calculate, NULL, 0);
			nlsr->is_route_calculation_scheduled=1;
		}


		free(key);

	free(time_stamp);
}

void 
write_log_for_adj_lsa_body(const char *body, int no_link)
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

		writeLogg(__FILE__,__FUNCTION__,__LINE__,"		Link %d	 	\n",i+1);
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"		Adjacent Router: %s	\n",rtr_id);
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"		Adjacent Router Length: %s	\n",length);
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"		Connecting Face: %s	\n",face);
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"		Metric: %s	\n",metric);


		for(i=1;i<no_link;i++)
		{
			rtr_id=strtok_r(NULL,sep,&rem);
			length=strtok_r(NULL,sep,&rem);
			face=strtok_r(NULL,sep,&rem);
			metric=strtok_r(NULL,sep,&rem);
			writeLogg(__FILE__,__FUNCTION__,__LINE__,"		Link %d	 	\n",i+1);
			writeLogg(__FILE__,__FUNCTION__,__LINE__,"		Adjacent Router: %s	\n",rtr_id);
			writeLogg(__FILE__,__FUNCTION__,__LINE__,"		Adjacent Router Length: %s	\n",length);
			writeLogg(__FILE__,__FUNCTION__,__LINE__,"		Connecting Face: %s	\n",face);
			writeLogg(__FILE__,__FUNCTION__,__LINE__,"		Metric: %s	\n",metric);

		}
	}

	free(lsa_data);
}


void
write_log_for_adj_lsa(struct alsa * adj_lsa)
{
	writeLogg(__FILE__,__FUNCTION__,__LINE__,"-----------Adj LSA Content---------------\n");
	writeLogg(__FILE__,__FUNCTION__,__LINE__,"	Origination Router: %s\n",adj_lsa->header->orig_router->name);
	writeLogg(__FILE__,__FUNCTION__,__LINE__,"	Origination Router Length: %d\n",adj_lsa->header->orig_router->length);
	writeLogg(__FILE__,__FUNCTION__,__LINE__,"	LS Type:	%d\n",adj_lsa->header->ls_type);
	writeLogg(__FILE__,__FUNCTION__,__LINE__,"	Origination Time: %s\n",adj_lsa->header->orig_time);
	writeLogg(__FILE__,__FUNCTION__,__LINE__,"	Lsa Data:\n");
	writeLogg(__FILE__,__FUNCTION__,__LINE__,"		No of Link: %d\n",adj_lsa->no_link);

	write_log_for_adj_lsa_body(adj_lsa->body,adj_lsa->no_link);
	
	writeLogg(__FILE__,__FUNCTION__,__LINE__,"\n");

}

void 
print_adj_lsa_body(const char *body, int no_link)
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

		printf("		Link %d	 	\n",i+1);
		printf("		Neighbor		 : %s	\n",rtr_id);
		printf("		Neighbor Length		 : %s	\n",length);
		printf("		Connecting Face		 : %s	\n",face);
		printf("		Metric			 : %s	\n",metric);


		for(i=1;i<no_link;i++)
		{
			rtr_id=strtok_r(NULL,sep,&rem);
			length=strtok_r(NULL,sep,&rem);
			face=strtok_r(NULL,sep,&rem);
			metric=strtok_r(NULL,sep,&rem);
			printf("		Link %d	 	\n",i+1);
			printf("		Neighbor		 : %s	\n",rtr_id);
			printf("		Neighbor Length		 : %s	\n",length);
			printf("		Connecting Face		 : %s	\n",face);
			printf("		Metric			 : %s	\n",metric);

		}
	}

	free(lsa_data);
}

void
print_adj_lsa(struct alsa * adj_lsa)
{
	if ( nlsr->debugging )
	{
		printf("-----------ADJ LSA Content---------------\n");
		printf("	Origination Router       :	%s\n",adj_lsa->header->orig_router->name);
		printf("	Origination Router Length:	%d\n",adj_lsa->header->orig_router->length);
		printf("	LS Type			 :	%d\n",adj_lsa->header->ls_type);
		printf("	Origination Time	 :	%s\n",adj_lsa->header->orig_time);
		printf("	Lsa Data:\n");
		printf("		No of Link	: %d\n",adj_lsa->no_link);

		print_adj_lsa_body(adj_lsa->body,adj_lsa->no_link);
		printf("\n");
	}

}

void
print_adj_lsdb(void)
{
	if ( nlsr->debugging )
		printf("print_name_lsdb called \n");	
	int i, adj_lsdb_element;
	struct alsa *adj_lsa;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->lsdb->adj_lsdb, e);
	adj_lsdb_element=hashtb_n(nlsr->lsdb->adj_lsdb);

	for(i=0;i<adj_lsdb_element;i++)
	{
		if ( nlsr->debugging )
			printf("-----------Adj LSA (%d)---------------\n",i+1);
		adj_lsa=e->data;
		print_adj_lsa(adj_lsa);	
		hashtb_next(e);		
	}

	hashtb_end(e);

	if ( nlsr->debugging )
		printf("\n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"\n");
}

void 
build_and_install_others_adj_lsa(char *orig_router,int ls_type,char *orig_time, int no_link,char *data)
{
	if ( nlsr->debugging )
		printf("build_and_install_others_adj_lsa called \n");	
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"build_and_install_others_adj_lsa called \n");
	struct alsa *adj_lsa=(struct alsa *)malloc(sizeof( struct alsa ));
	build_others_adj_lsa(adj_lsa,orig_router,ls_type,orig_time,no_link,data);
	//print_adj_lsa(adj_lsa);
	install_adj_lsa(adj_lsa);
	

	free(adj_lsa->header->orig_router->name);
	free(adj_lsa->header->orig_router);
	free(adj_lsa->header->orig_time);
	free(adj_lsa->header);
	free(adj_lsa->body);
	free(adj_lsa);

	print_adj_lsdb();

}


void 
build_others_adj_lsa(struct alsa *adj_lsa,char *orig_router,int ls_type,char *orig_time,int no_link,char *data)
{
	//printf("build_others_adj_lsa called \n");
	if ( nlsr->debugging )
		printf("build_others_adj_lsa called  \n");	
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"build_others_adj_lsa called  \n");

	/*Filling Up Header Data */
	adj_lsa->header=(struct alsa_header *)malloc(sizeof(struct alsa_header ));
	adj_lsa->header->orig_router=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
	adj_lsa->header->orig_router->name=(char *)malloc(strlen(orig_router)+1);
	memset(adj_lsa->header->orig_router->name,0,strlen(orig_router)+1);
	memcpy(adj_lsa->header->orig_router->name,orig_router,strlen(orig_router)+1);

	adj_lsa->header->orig_router->length=strlen(orig_router)+1;


	adj_lsa->header->ls_type=(unsigned)LS_TYPE_ADJ;	

	adj_lsa->header->orig_time=(char *)malloc(strlen(orig_time)+1);
	memset(adj_lsa->header->orig_time,0,strlen(orig_time)+1);
	memcpy(adj_lsa->header->orig_time,orig_time,strlen(orig_time)+1);

	adj_lsa->no_link=no_link;

	adj_lsa->body=(char *)malloc(strlen(data)+1);
	memset(adj_lsa->body,0,strlen(data)+1);
	memcpy(adj_lsa->body,(char *)data,strlen(data)+1);

}


long int
get_name_lsdb_num_element(void)
{
	long int num_element;


	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->lsdb->name_lsdb, e);
	num_element=hashtb_n(nlsr->lsdb->name_lsdb);
	hashtb_end(e);

	return num_element; 
}

long int
get_adj_lsdb_num_element(void)
{
	long int num_element;


	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->lsdb->adj_lsdb, e);
	num_element=hashtb_n(nlsr->lsdb->adj_lsdb);
	hashtb_end(e);

	return num_element; 
}

void 
get_name_lsdb_summary(struct ccn_charbuf *name_lsdb_data)
{
	//printf("get_name_lsdb_summary called \n");
	if ( nlsr->debugging )
		printf("get_name_lsdb_summary called  \n");	
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"get_name_lsdb_summary called  \n");
	
	int i, name_lsdb_element;

	struct nlsa *name_lsa;
	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->lsdb->name_lsdb, e);
	name_lsdb_element=hashtb_n(nlsr->lsdb->name_lsdb);

	for(i=0;i<name_lsdb_element;i++)
	{
		name_lsa=e->data;

		ccn_charbuf_append_string(name_lsdb_data,name_lsa->header->orig_router->name);
		ccn_charbuf_append_string(name_lsdb_data,"|");

		char *lst=(char *)malloc(20);
		memset(lst,0,20);
		sprintf(lst,"%d",name_lsa->header->ls_type);
		ccn_charbuf_append_string(name_lsdb_data,lst);
		free(lst);
		ccn_charbuf_append_string(name_lsdb_data,"|");

		char *lsid=(char *)malloc(20);
		memset(lsid,0,20);
		sprintf(lsid,"%ld",name_lsa->header->ls_id);
		ccn_charbuf_append_string(name_lsdb_data,lsid);
		free(lsid);
		ccn_charbuf_append_string(name_lsdb_data,"|");

		ccn_charbuf_append_string(name_lsdb_data,name_lsa->header->orig_time);
		ccn_charbuf_append_string(name_lsdb_data,"|");

		hashtb_next(e);		
	}

	hashtb_end(e);

}


void 
get_adj_lsdb_summary(struct ccn_charbuf *adj_lsdb_data)
{
	if ( nlsr->debugging )
		printf("get_adj_lsdb_summary called  \n");	
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"get_adj_lsdb_summary called  \n");
	int i, adj_lsdb_element;
	struct alsa *adj_lsa;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->lsdb->adj_lsdb, e);
	adj_lsdb_element=hashtb_n(nlsr->lsdb->adj_lsdb);

	for(i=0;i<adj_lsdb_element;i++)
	{
		adj_lsa=e->data;

		ccn_charbuf_append_string(adj_lsdb_data,adj_lsa->header->orig_router->name);
		ccn_charbuf_append_string(adj_lsdb_data,"|");
		
		char *lst=(char *)malloc(20);
		memset(lst,0,20);
		sprintf(lst,"%d",adj_lsa->header->ls_type);
		ccn_charbuf_append_string(adj_lsdb_data,lst);
		free(lst);
		ccn_charbuf_append_string(adj_lsdb_data,"|");

		ccn_charbuf_append_string(adj_lsdb_data,adj_lsa->header->orig_time);
		ccn_charbuf_append_string(adj_lsdb_data,"|");

		hashtb_next(e);		
	}

	hashtb_end(e);
}


void 
get_lsdb_summary(struct ccn_charbuf *lsdb_data)
{
	struct ccn_charbuf *name_lsdb_data=ccn_charbuf_create();
	struct ccn_charbuf *adj_lsdb_data=ccn_charbuf_create();

	get_name_lsdb_summary(name_lsdb_data);
	get_adj_lsdb_summary(adj_lsdb_data);

	long int num_lsa=get_name_lsdb_num_element() + get_adj_lsdb_num_element();
	char *num_element=(char *)malloc(15);
	memset(num_element,0,15);
	sprintf(num_element,"%ld",num_lsa);

	if( num_lsa > 0)
	{
		ccn_charbuf_append_string(lsdb_data,num_element);
		ccn_charbuf_append_string(lsdb_data,"|");
	}
	if(name_lsdb_data->length>0)
	{
		char *data1=ccn_charbuf_as_string(name_lsdb_data);
		ccn_charbuf_append_string(lsdb_data,(char *)data1);
	}
	if(adj_lsdb_data->length>0)
	{
		char *data2=ccn_charbuf_as_string(adj_lsdb_data);
		ccn_charbuf_append_string(lsdb_data,(char *)data2);
	}
	ccn_charbuf_destroy(&name_lsdb_data);
	ccn_charbuf_destroy(&adj_lsdb_data);
	free(num_element);

}

int 
check_is_new_name_lsa(char *orig_router,char *lst,char *lsid,char *orig_time)
{
	int ret=0;
	struct ccn_charbuf *key=ccn_charbuf_create();
	ccn_charbuf_append_string(key,orig_router);
	ccn_charbuf_append_string(key,"/");
	ccn_charbuf_append_string(key,lst);
	ccn_charbuf_append_string(key,"/");
	ccn_charbuf_append_string(key,lsid);

	int res;
	struct nlsa *name_lsa;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->lsdb->name_lsdb, e);
	res = hashtb_seek(e, ccn_charbuf_as_string(key), key->length, 0);

	if( res == HT_NEW_ENTRY )
	{
		hashtb_delete(e);
		ret=1;

	}
	else if(res == HT_OLD_ENTRY)
	{
		name_lsa=e->data;
		if( strcmp ( orig_time , name_lsa->header->orig_time ) > 0 )
		{
			ret=1;
		}
	}

	hashtb_end(e);

	ccn_charbuf_destroy(&key);
	
	return ret;
}

int 
check_is_new_adj_lsa(char *orig_router,char *lst,char *orig_time)
{
	int ret=0;
	struct ccn_charbuf *key=ccn_charbuf_create();
	ccn_charbuf_append_string(key,orig_router);
	ccn_charbuf_append_string(key,"/");
	ccn_charbuf_append_string(key,lst);

	int res;
	struct alsa *adj_lsa;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;

	hashtb_start(nlsr->lsdb->adj_lsdb, e);
	res = hashtb_seek(e, ccn_charbuf_as_string(key), key->length, 0);

	if( res == HT_NEW_ENTRY )
	{
		hashtb_delete(e);
		ret=1;

	}
	else if(res == HT_OLD_ENTRY)
	{
		adj_lsa=e->data;
		if( strcmp ( orig_time , adj_lsa->header->orig_time ) > 0 )
		{
			ret=1;
		}
	}

	hashtb_end(e);
	
	ccn_charbuf_destroy(&key);

	return ret;
}

void 
get_name_lsa_data(struct ccn_charbuf *lsa_data, struct name_prefix *lsaId)
{
	//printf("get_name_lsa_data called \n");
	if ( nlsr->debugging )
		printf("get_name_lsa_data called  \n");	
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"get_name_lsa_data called  \n");

	struct nlsa *name_lsa=(struct nlsa*)malloc(sizeof(struct nlsa ));

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res;

   	hashtb_start(nlsr->lsdb->name_lsdb, e);
    	res = hashtb_seek(e, lsaId->name, lsaId->length-1, 0);

	if( res == HT_OLD_ENTRY )
	{
		name_lsa=e->data;
		//printf("NAME LSA found\n");

		if ( nlsr->debugging )
			printf("NAME LSA found  \n");	
		if ( nlsr->detailed_logging )
			writeLogg(__FILE__,__FUNCTION__,__LINE__,"Name LSA found  \n");

		ccn_charbuf_append_string(lsa_data,name_lsa->header->orig_router->name);
		ccn_charbuf_append_string(lsa_data,"|");

		char *temp_length=(char *)malloc(20);
		memset(temp_length,0,20);
		sprintf(temp_length,"%d",name_lsa->header->orig_router->length);
		ccn_charbuf_append_string(lsa_data,temp_length);
		free(temp_length);
		ccn_charbuf_append_string(lsa_data,"|");

		char *temp_ltype=(char *)malloc(20);
		memset(temp_ltype,0,20);
		sprintf(temp_ltype,"%d",name_lsa->header->ls_type);
		ccn_charbuf_append_string(lsa_data,temp_ltype);
		free(temp_ltype);
		ccn_charbuf_append_string(lsa_data,"|");

		char *temp_lsid=(char *)malloc(20);
		memset(temp_lsid,0,20);
		sprintf(temp_lsid,"%ld",name_lsa->header->ls_id);
		ccn_charbuf_append_string(lsa_data,temp_lsid);
		free(temp_lsid);
		ccn_charbuf_append_string(lsa_data,"|");

		ccn_charbuf_append_string(lsa_data,name_lsa->header->orig_time);
		ccn_charbuf_append_string(lsa_data,"|");

		char *temp_valid=(char *)malloc(20);
		memset(temp_valid,0,20);
		sprintf(temp_valid,"%d",name_lsa->header->isValid);
		ccn_charbuf_append_string(lsa_data,temp_valid);
		free(temp_valid);
		ccn_charbuf_append_string(lsa_data,"|");

		ccn_charbuf_append_string(lsa_data,name_lsa->name_prefix->name);
		ccn_charbuf_append_string(lsa_data,"|");

		char *temp_npl=(char *)malloc(20);
		memset(temp_npl,0,20);
		sprintf(temp_npl,"%d",name_lsa->name_prefix->length);
		ccn_charbuf_append_string(lsa_data,temp_npl);
		free(temp_npl);
		ccn_charbuf_append_string(lsa_data,"|");

	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}

	hashtb_end(e);
}

void 
get_adj_lsa_data(struct ccn_charbuf *lsa_data,struct name_prefix *lsaId)
{
	if ( nlsr->debugging )
		printf("get_adj_lsa_data called  \n");	
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"get_adj_lsa_data called  \n");

	struct alsa *adj_lsa=(struct alsa*)malloc(sizeof(struct alsa ));

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res;

   	hashtb_start(nlsr->lsdb->adj_lsdb, e);
    	res = hashtb_seek(e, lsaId->name, lsaId->length-1, 0);

	if( res == HT_OLD_ENTRY )
	{
		adj_lsa=e->data;
		//printf("Adj LSA found\n");

		if ( nlsr->debugging )
			printf("Adj LSA found  \n");	
		if ( nlsr->detailed_logging )
			writeLogg(__FILE__,__FUNCTION__,__LINE__,"Adj LSA found  \n");

		ccn_charbuf_append_string(lsa_data,adj_lsa->header->orig_router->name);
		ccn_charbuf_append_string(lsa_data,"|");

		char *temp_length=(char *)malloc(20);
		memset(temp_length,0,20);
		sprintf(temp_length,"%d",adj_lsa->header->orig_router->length);
		ccn_charbuf_append_string(lsa_data,temp_length);
		free(temp_length);
		ccn_charbuf_append_string(lsa_data,"|");

		char *temp_ltype=(char *)malloc(20);
		memset(temp_ltype,0,20);
		sprintf(temp_ltype,"%d",adj_lsa->header->ls_type);
		ccn_charbuf_append_string(lsa_data,temp_ltype);
		free(temp_ltype);
		ccn_charbuf_append_string(lsa_data,"|");

		ccn_charbuf_append_string(lsa_data,adj_lsa->header->orig_time);
		ccn_charbuf_append_string(lsa_data,"|");

		char *temp_nl=(char *)malloc(20);
		memset(temp_nl,0,20);
		sprintf(temp_nl,"%d",adj_lsa->no_link);
		ccn_charbuf_append_string(lsa_data,temp_nl);
		free(temp_nl);
		ccn_charbuf_append_string(lsa_data,"|");

		ccn_charbuf_append_string(lsa_data,adj_lsa->body);


	}
	else if(res == HT_NEW_ENTRY)
	{
		hashtb_delete(e);
	}

	hashtb_end(e);
}

void
make_name_lsa_invalid(struct name_prefix *np,int ls_type, long int ls_id)
{

	if ( nlsr->debugging )
		printf("make_name_lsa_invalid called  \n");	
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"make_name_lsa_invalid called  \n");


	char lst[2];
	memset(lst,0,2);
	sprintf(lst,"%d",ls_type);	

	char lsid[10];
	memset(lsid,0,10);
	sprintf(lsid,"%ld",ls_id);
	
	
	char *key=(char *)malloc(strlen(np->name)+1+strlen(lst)+1+strlen(lsid)+1);
	memset(key,0,strlen(np->name)+1+strlen(lst)+1+strlen(lsid)+1);


	make_name_lsa_key(key, np->name,ls_type,ls_id);	
	printf("Key:%s Length:%d\n",key,(int)strlen(key));

	struct nlsa *nlsa;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
	 	
    	int res;

   	hashtb_start(nlsr->lsdb->name_lsdb, e);
    	res = hashtb_seek(e, key,strlen(key) , 0);

	if( res == HT_OLD_ENTRY )
	{
		nlsa=e->data;
	
		nlsa->header->isValid=0;

		writeLogg(__FILE__,__FUNCTION__,__LINE__," Name-LSA\n");
		writeLogg(__FILE__,__FUNCTION__,__LINE__," Deleting name lsa\n");
		write_log_for_name_lsa(nlsa);
		writeLogg(__FILE__,__FUNCTION__,__LINE__," name_lsa_end\n");

		hashtb_delete(e);
	}
	else if( res == HT_NEW_ENTRY )
	{
		hashtb_delete(e);
	}
	hashtb_end(e);
	
	if ( nlsr->debugging )
		printf("Old Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"Old Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);

	set_new_lsdb_version();	

	if ( nlsr->debugging )
		printf("New Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"New Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);

}

int 
delete_name_lsa(struct ccn_schedule *sched, void *clienth, struct ccn_scheduled_event *ev, int flags)
{
	//printf("delete_name_lsa called \n");

	if ( nlsr->debugging )
		printf("delete_name_lsa called  \n");	
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"delete_name_lsa called  \n");
	
	if(flags == CCN_SCHEDULE_CANCEL)
	{
 	 	return -1;
	}



	nlsr_lock();

	if ( nlsr->debugging )
		printf("LSA Key: %s \n",(char *)ev->evdata);
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"LSA Key: %s \n",(char *)ev->evdata);

	struct nlsa *nlsa;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
	 	
    	int res;

   	hashtb_start(nlsr->lsdb->name_lsdb, e);
    	res = hashtb_seek(e, (char *)ev->evdata, strlen((char *)ev->evdata), 0);

	if( res == HT_OLD_ENTRY )
	{
		nlsa=e->data;
		delete_npt_entry_by_router_and_name_prefix(nlsa->header->orig_router->name, nlsa->name_prefix->name);

		writeLogg(__FILE__,__FUNCTION__,__LINE__," Name-LSA\n");
		writeLogg(__FILE__,__FUNCTION__,__LINE__," Deleting name lsa\n");
		write_log_for_name_lsa(nlsa);
		writeLogg(__FILE__,__FUNCTION__,__LINE__," name_lsa_end\n");

		hashtb_delete(e);
	}
	else if( res == HT_NEW_ENTRY )
	{
		hashtb_delete(e);
	}
	hashtb_end(e);
	
	if ( nlsr->debugging )
		printf("Old Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"Old Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);

	set_new_lsdb_version();	

	if ( nlsr->debugging )
		printf("New Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"New Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);
	
	//print_name_lsdb();
	
	nlsr_unlock();

	return 0;
}

int 
delete_adj_lsa(struct ccn_schedule *sched, void *clienth, struct ccn_scheduled_event *ev, int flags)
{
	//printf("delete_adj_lsa called \n");

	if ( nlsr->debugging )
		printf("delete_adj_lsa called  \n");	
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"delete_adj_lsa called  \n");
	
	if(flags == CCN_SCHEDULE_CANCEL)
	{
 	 	return -1;
	}
	nlsr_lock();

	//printf("LSA Key: %s \n",(char *)ev->evdata);

	if ( nlsr->debugging )
		printf("LSA Key: %s \n",(char *)ev->evdata);
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"LSA Key: %s \n",(char *)ev->evdata);

	struct alsa *alsa;
	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res;

   	hashtb_start(nlsr->lsdb->adj_lsdb, e);
    	res = hashtb_seek(e, (char *)ev->evdata, strlen((char *)ev->evdata), 0);

	if( res == HT_OLD_ENTRY )
	{
		alsa=e->data;
		writeLogg(__FILE__,__FUNCTION__,__LINE__," Adj-LSA\n");
		writeLogg(__FILE__,__FUNCTION__,__LINE__," Deleting adj lsa\n");
		write_log_for_adj_lsa(alsa);
		writeLogg(__FILE__,__FUNCTION__,__LINE__," adj_lsa_end\n");
		
		hashtb_delete(e);
	}
	else if( res == HT_OLD_ENTRY )
	{
		hashtb_delete(e);
	}
	hashtb_end(e);

	if ( nlsr->debugging )
		printf("Old Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"Old Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);

	set_new_lsdb_version();	

	if ( nlsr->debugging )
		printf("New Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"New Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);


	if ( !nlsr->is_route_calculation_scheduled)
	{
		nlsr->event_calculate_route = ccn_schedule_event(nlsr->sched, 1000000, &route_calculate, NULL, 0);
		nlsr->is_route_calculation_scheduled=1;
	}

	//print_adj_lsdb();

	nlsr_unlock();

	return 0;
}

void
refresh_name_lsdb(void)
{
	//printf("refresh_name_lsdb called \n");

	if ( nlsr->debugging )
		printf("refresh_name_lsdb called  \n");	
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"refresh_name_lsdb called  \n");

	//int lsa_change_count=0;

	char *time_stamp=(char *)malloc(20);
	memset(time_stamp,0,20);
	get_current_timestamp_micro(time_stamp);
	
	long int lsa_life_time;
		
	int i, name_lsdb_element;
	struct nlsa *name_lsa;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->lsdb->name_lsdb, e);
	name_lsdb_element=hashtb_n(nlsr->lsdb->name_lsdb);

	for(i=0;i<name_lsdb_element;i++)
	{
		name_lsa=e->data;

		lsa_life_time=get_time_diff(time_stamp,name_lsa->header->orig_time);
		if ( nlsr->debugging )
			printf("LSA Life Time: %ld \n",lsa_life_time);	
		if ( nlsr->detailed_logging )
			writeLogg(__FILE__,__FUNCTION__,__LINE__,"LSA Life Time: %ld \n",lsa_life_time);
		
		if ( strcmp(name_lsa->header->orig_router->name,nlsr->router_name) == 0)
		{
			if ( lsa_life_time > nlsr->lsa_refresh_time )
			{
				if ( name_lsa->header->isValid == NAME_LSA_VALID )					
				{
					if ( nlsr->debugging )
						printf("Own Name LSA need to be refrshed\n");
					if ( nlsr->detailed_logging )
						writeLogg(__FILE__,__FUNCTION__,__LINE__,"Own Name LSA need to be refrshed\n");

					writeLogg(__FILE__,__FUNCTION__,__LINE__," Name-LSA\n");
					writeLogg(__FILE__,__FUNCTION__,__LINE__," Deleting name lsa\n");
					write_log_for_name_lsa(name_lsa);
					writeLogg(__FILE__,__FUNCTION__,__LINE__," name_lsa_end\n");
					

					char *current_time_stamp=(char *)malloc(20);
					memset(current_time_stamp,0,20);
					get_current_timestamp_micro(current_time_stamp);

					free(name_lsa->header->orig_time);
					name_lsa->header->orig_time=(char *)malloc(strlen(current_time_stamp)+1); //free 
					memset(name_lsa->header->orig_time,0,strlen(current_time_stamp)+1);
					memcpy(name_lsa->header->orig_time,current_time_stamp,strlen(current_time_stamp)+1);

					writeLogg(__FILE__,__FUNCTION__,__LINE__," Name-LSA\n");
					writeLogg(__FILE__,__FUNCTION__,__LINE__," Adding name lsa\n");
					write_log_for_name_lsa(name_lsa);
					writeLogg(__FILE__,__FUNCTION__,__LINE__," name_lsa_end\n");
	
					free(current_time_stamp);
				}
				else 
				{
					char lst[2];
					memset(lst,0,2);
					sprintf(lst,"%d",name_lsa->header->ls_type);	

					char lsid[10];
					memset(lsid,0,10);
					sprintf(lsid,"%ld",name_lsa->header->ls_id);
	
	
					char *key=(char *)malloc(strlen(name_lsa->header->orig_router->name)+1+strlen(lst)+1+strlen(lsid)+1);
					memset(key,0,strlen(name_lsa->header->orig_router->name)+1+strlen(lst)+1+strlen(lsid)+1);


					make_name_lsa_key(key, name_lsa->header->orig_router->name,name_lsa->header->ls_type,name_lsa->header->ls_id);	
					//printf("Key:%s Length:%d\n",key,(int)strlen(key));
				
					nlsr->event = ccn_schedule_event(nlsr->sched, 10, &delete_name_lsa, (void *)key, 0);
				}

				if ( nlsr->debugging )
					printf("Old Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);
				if ( nlsr->detailed_logging )
					writeLogg(__FILE__,__FUNCTION__,__LINE__,"Old Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);

				set_new_lsdb_version();	

				if ( nlsr->debugging )
					printf("New Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);
				if ( nlsr->detailed_logging )
					writeLogg(__FILE__,__FUNCTION__,__LINE__,"New Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);

							

				print_name_lsdb();
				//lsa_change_count++;
			}	
		}
		else 
		{
			if ( lsa_life_time > nlsr->router_dead_interval )
			{
				if ( nlsr->debugging )
				 	printf("Others Name LSA need to be deleted\n");
				if ( nlsr->detailed_logging )
					writeLogg(__FILE__,__FUNCTION__,__LINE__,"Others Name LSA need to be deleted\n");

				char lst[2];
				memset(lst,0,2);
				sprintf(lst,"%d",name_lsa->header->ls_type);	

				char lsid[10];
				memset(lsid,0,10);
				sprintf(lsid,"%ld",name_lsa->header->ls_id);
	
	
				char *key=(char *)malloc(strlen(name_lsa->header->orig_router->name)+1+strlen(lst)+1+strlen(lsid)+1);
				memset(key,0,strlen(name_lsa->header->orig_router->name)+1+strlen(lst)+1+strlen(lsid)+1);


				make_name_lsa_key(key, name_lsa->header->orig_router->name,name_lsa->header->ls_type,name_lsa->header->ls_id);	
				//printf("Key:%s Length:%d\n",key,(int)strlen(key));
				
				nlsr->event = ccn_schedule_event(nlsr->sched, 10, &delete_name_lsa, (void *)key, 0);
				//lsa_change_count++;
			}
		}

		hashtb_next(e);		
	}

	hashtb_end(e);
	
	free(time_stamp);

	
}

void
refresh_adj_lsdb(void)
{
	//printf("refresh_adj_lsdb called \n");

	if ( nlsr->debugging )
		printf("refresh_adj_lsdb called  \n");	
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"refresh_adj_lsdb called  \n");

	char *time_stamp=(char *)malloc(20);
	memset(time_stamp,0,20);
	get_current_timestamp_micro(time_stamp);
	
	long int lsa_life_time;
		
	int i, adj_lsdb_element;
	struct alsa *adj_lsa;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->lsdb->adj_lsdb, e);
	adj_lsdb_element=hashtb_n(nlsr->lsdb->adj_lsdb);

	for(i=0;i<adj_lsdb_element;i++)
	{
		adj_lsa=e->data;

		lsa_life_time=get_time_diff(time_stamp,adj_lsa->header->orig_time);
		//printf("LSA Life Time: %ld \n",lsa_life_time);	

		if ( nlsr->debugging )
			printf("LSA Life Time: %ld \n",lsa_life_time);	
		if ( nlsr->detailed_logging )
			writeLogg(__FILE__,__FUNCTION__,__LINE__,"LSA Life Time: %ld \n",lsa_life_time);		

		if ( strcmp(adj_lsa->header->orig_router->name,nlsr->router_name) == 0)
		{
			if ( lsa_life_time > nlsr->lsa_refresh_time )
			{
				//printf("Own Adj LSA need to be refrshed\n");
				if ( nlsr->debugging )
					printf("Own Adj LSA need to be refrshed\n");
				if ( nlsr->detailed_logging )
					writeLogg(__FILE__,__FUNCTION__,__LINE__,"Own Adj LSA need to be refrshed\n");

				writeLogg(__FILE__,__FUNCTION__,__LINE__," Adj-LSA\n");
				writeLogg(__FILE__,__FUNCTION__,__LINE__," Deleting adj lsa\n");
				write_log_for_adj_lsa(adj_lsa);
				writeLogg(__FILE__,__FUNCTION__,__LINE__," adj_lsa_end\n");

				char *current_time_stamp=(char *)malloc(20);
				memset(current_time_stamp,0,20);
				get_current_timestamp_micro(current_time_stamp);

				free(adj_lsa->header->orig_time);
				adj_lsa->header->orig_time=(char *)malloc(strlen(current_time_stamp)+1); //free 
				memset(adj_lsa->header->orig_time,0,strlen(current_time_stamp)+1);
				memcpy(adj_lsa->header->orig_time,current_time_stamp,strlen(current_time_stamp)+1);
	
				free(current_time_stamp);

				writeLogg(__FILE__,__FUNCTION__,__LINE__," Adj-LSA\n");
				writeLogg(__FILE__,__FUNCTION__,__LINE__," Adding adj lsa\n");
				write_log_for_adj_lsa(adj_lsa);
				writeLogg(__FILE__,__FUNCTION__,__LINE__," adj_lsa_end\n");

				if ( nlsr->debugging )
					printf("Old Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);
				if ( nlsr->detailed_logging )
					writeLogg(__FILE__,__FUNCTION__,__LINE__,"Old Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);

				set_new_lsdb_version();	

				if ( nlsr->debugging )
					printf("New Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);
				if ( nlsr->detailed_logging )
					writeLogg(__FILE__,__FUNCTION__,__LINE__,"New Version Number of LSDB: %s \n",nlsr->lsdb->lsdb_version);

				print_adj_lsdb();
			}	
		}
		else 
		{
			if ( lsa_life_time > nlsr->router_dead_interval )
			{
				//printf("Others Adj LSA need to be deleted\n");

				if ( nlsr->debugging )
				 	printf("Others Adj LSA need to be deleted\n");
				if ( nlsr->detailed_logging )
					writeLogg(__FILE__,__FUNCTION__,__LINE__,"Others Adj LSA need to be deleted\n");
				
				char *key=(char *)malloc(adj_lsa->header->orig_router->length+2+2);
				memset(key,0,adj_lsa->header->orig_router->length+2);
				make_adj_lsa_key(key,adj_lsa);
				//printf("Adjacent LSA key: %s \n",key);				
				nlsr->event = ccn_schedule_event(nlsr->sched, 10, &delete_adj_lsa, (void *)key, 0);
			}
		}

		

		hashtb_next(e);		
	}

	hashtb_end(e);
	
	free(time_stamp);
}

int
refresh_lsdb(struct ccn_schedule *sched, void *clienth, struct ccn_scheduled_event *ev, int flags)
{
	if(flags == CCN_SCHEDULE_CANCEL)
	{
 	 	return -1;
	}

	nlsr_lock();

	//printf("refresh_lsdb called \n");

	if ( nlsr->debugging )
		printf("refresh_lsdb called\n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"refresh_lsdb called\n");
	
	refresh_name_lsdb();
	refresh_adj_lsdb();

	nlsr->event = ccn_schedule_event(nlsr->sched, 60000000, &refresh_lsdb, NULL, 0);
	
	nlsr_unlock();
	return 0;
}

void
write_adj_lsa_to_repo(char *repo_content_prefix, struct name_prefix *lsa_id)
{
	
	printf("Content Prefix: %s\n",repo_content_prefix);
	
	struct ccn_charbuf *lsa_data=ccn_charbuf_create();		
	get_adj_lsa_data(lsa_data,lsa_id);
	printf("Name LSA Data: %s \n",ccn_charbuf_as_string(lsa_data));	

	write_data_to_repo(ccn_charbuf_as_string(lsa_data), repo_content_prefix);

	ccn_charbuf_destroy(&lsa_data);
}

void
write_name_lsa_to_repo(char *repo_content_prefix, struct name_prefix *lsa_id)
{
	printf("Content Prefix: %s\n",repo_content_prefix);
	
	struct ccn_charbuf *lsa_data=ccn_charbuf_create();		
	get_name_lsa_data(lsa_data,lsa_id);
	printf("Name LSA Data: %s \n",ccn_charbuf_as_string(lsa_data));	

	write_data_to_repo(ccn_charbuf_as_string(lsa_data), repo_content_prefix);

	ccn_charbuf_destroy(&lsa_data);
}


void
write_name_lsdb_to_repo(char *slice_prefix)
{
	int i, name_lsdb_element;

	struct nlsa *name_lsa;
	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->lsdb->name_lsdb, e);
	name_lsdb_element=hashtb_n(nlsr->lsdb->name_lsdb);

	for(i=0;i<name_lsdb_element;i++)
	{
		name_lsa=e->data;

		char lst[2];
		memset(lst,0,2);
		sprintf(lst,"%d",name_lsa->header->ls_type);	

		char lsid[10];
		memset(lsid,0,10);
		sprintf(lsid,"%ld",name_lsa->header->ls_id);
	
	
		char *key=(char *)malloc(strlen(name_lsa->header->orig_router->name)+1+strlen(lst)+1+strlen(lsid)+1);
		memset(key,0,strlen(name_lsa->header->orig_router->name)+1+strlen(lst)+1+strlen(lsid)+1);


		make_name_lsa_key(key, name_lsa->header->orig_router->name,name_lsa->header->ls_type,name_lsa->header->ls_id);
		printf("Name LSA Key: %s \n",key);


		char *repo_key=(char *)malloc(strlen(slice_prefix)+1+strlen(name_lsa->header->orig_router->name)+1+strlen(lst)+1+strlen(lsid)+1+strlen(name_lsa->header->orig_time)+1);
		memset(repo_key,0,strlen(slice_prefix)+1+strlen(name_lsa->header->orig_router->name)+1+strlen(lst)+1+strlen(lsid)+1+strlen(name_lsa->header->orig_time)+1);	
		make_name_lsa_prefix_for_repo(repo_key, name_lsa->header->orig_router->name,name_lsa->header->ls_type,name_lsa->header->ls_id,name_lsa->header->orig_time,slice_prefix);
		
		printf("Name LSA Repo Key: %s \n",repo_key);

		struct name_prefix *lsaid=(struct name_prefix *)malloc(sizeof(struct name_prefix));
		lsaid->name=(char *)malloc(strlen(key)+1);
		memset(lsaid->name, 0, strlen(key)+1);
		memcpy(lsaid->name,key,strlen(key));
		lsaid->length=strlen(key)+1;

		
		write_name_lsa_to_repo(repo_key, lsaid);

		free(key);
		free(repo_key);
		free(lsaid->name);
		free(lsaid);

		hashtb_next(e);		
	}

	hashtb_end(e);
	

}
