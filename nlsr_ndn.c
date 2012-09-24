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
#include "nlsr_npl.h"
#include "nlsr_adl.h"
#include "nlsr_lsdb.h"
#include "utility.h"

int
appendLifetime(struct ccn_charbuf *cb, int lifetime) 
{
	unsigned char buf[sizeof(int32_t)];
	int32_t dreck = lifetime << 12;
	int pos = sizeof(int32_t);
	int res = 0;
	while (dreck > 0 && pos > 0) 
	{
		pos--;
		buf[pos] = dreck & 255;
		dreck = dreck >> 8;
	}
	res |= ccnb_append_tagged_blob(cb, CCN_DTAG_InterestLifetime, buf+pos, sizeof(buf)-pos);
	return res;
}


void 
get_nbr(struct name_prefix *nbr,struct ccn_closure *selfp, struct ccn_upcall_info *info)
{

	printf("get_nbr called\n");
	int res,i;
	int nlsr_position=0;
	int name_comps=(int)info->interest_comps->n;
	int len=0;

	for(i=0;i<name_comps;i++)
	{
		res=ccn_name_comp_strcmp(info->interest_ccnb,info->interest_comps,i,"nlsr");
		if( res == 0)
		{
			nlsr_position=i;
			break;
		}	
	}


	const unsigned char *comp_ptr1;
	size_t comp_size;
	for(i=0;i<nlsr_position;i++)
	{
		res=ccn_name_comp_get(info->interest_ccnb, info->interest_comps,i,&comp_ptr1, &comp_size);
		len+=1;
		len+=(int)comp_size;	
	}
	len++;

	char *neighbor=(char *)malloc(len);
	memset(neighbor,0,len);

	for(i=0; i<nlsr_position;i++)
	{
		res=ccn_name_comp_get(info->interest_ccnb, info->interest_comps,i,&comp_ptr1, &comp_size);
		memcpy(neighbor+strlen(neighbor),"/",1);
		memcpy(neighbor+strlen(neighbor),(char *)comp_ptr1,strlen((char *)comp_ptr1));

	}

	nbr->name=(char *)malloc(strlen(neighbor)+1);
	memcpy(nbr->name,neighbor,strlen(neighbor)+1);
	nbr->length=strlen(neighbor)+1;

	printf("Neighbor: %s Length: %d\n",nbr->name,nbr->length);
	

}

void 
get_lsa_identifier(struct name_prefix *lsaId,struct ccn_closure *selfp, struct ccn_upcall_info *info, int offset)
{

	printf("get_lsa_identifier called\n");
	int res,i;
	int nlsr_position=0;
	int name_comps=(int)info->interest_comps->n;
	int len=0;

	for(i=0;i<name_comps;i++)
	{
		res=ccn_name_comp_strcmp(info->interest_ccnb,info->interest_comps,i,"nlsr");
		if( res == 0)
		{
			nlsr_position=i;
			break;
		}	
	}


	const unsigned char *comp_ptr1;
	size_t comp_size;
	for(i=nlsr_position+3+offset;i<info->interest_comps->n-1;i++)
	{
		res=ccn_name_comp_get(info->interest_ccnb, info->interest_comps,i,&comp_ptr1, &comp_size);
		len+=1;
		len+=(int)comp_size;	
	}
	len++;

	char *neighbor=(char *)malloc(len);
	memset(neighbor,0,len);

	for(i=nlsr_position+3+offset; i<info->interest_comps->n-1;i++)
	{
		res=ccn_name_comp_get(info->interest_ccnb, info->interest_comps,i,&comp_ptr1, &comp_size);
		memcpy(neighbor+strlen(neighbor),"/",1);
		memcpy(neighbor+strlen(neighbor),(char *)comp_ptr1,strlen((char *)comp_ptr1));

	}

	lsaId->name=(char *)malloc(strlen(neighbor)+1);
	memset(lsaId->name,0,strlen(neighbor)+1);
	memcpy(lsaId->name,neighbor,strlen(neighbor)+1);
	lsaId->length=strlen(neighbor)+1;

	printf("LSA Identifier: %s Length: %d\n",lsaId->name,lsaId->length-1);


}

int 
get_ls_type(struct ccn_closure *selfp, struct ccn_upcall_info *info)
{
	int res,i;
	int nlsr_position=0;
	int name_comps=(int)info->interest_comps->n;

	int ret=0;

	for(i=0;i<name_comps;i++)
	{
		res=ccn_name_comp_strcmp(info->interest_ccnb,info->interest_comps,i,"nlsr");
		if( res == 0)
		{
			nlsr_position=i;
			break;
		}	
	}


	const unsigned char *comp_ptr1;
	size_t comp_size;
	res=ccn_name_comp_get(info->interest_ccnb, info->interest_comps,nlsr_position+2,&comp_ptr1, &comp_size);

	ret=atoi((char *)comp_ptr1);

	return ret;	

}

void 
get_lsdb_version(char *lsdb_version,struct ccn_closure *selfp, struct ccn_upcall_info *info )
{
	const unsigned char *comp_ptr1;
	size_t comp_size;
	ccn_name_comp_get(info->content_ccnb, info->content_comps,info->content_comps->n-2,&comp_ptr1, &comp_size);
	memcpy(lsdb_version,(char *)comp_ptr1,(int)comp_size);

}


/* Call back function registered in ccnd to get all interest coming to NLSR application */

enum ccn_upcall_res 
incoming_interest(struct ccn_closure *selfp,
        enum ccn_upcall_kind kind, struct ccn_upcall_info *info)
{

    nlsr_lock();    

    switch (kind) {
        case CCN_UPCALL_FINAL:
            break;
        case CCN_UPCALL_INTEREST:
		// printing the name prefix for which it received interest
            	printf("Interest Received for name: "); 
	    	struct ccn_charbuf*c;
		c=ccn_charbuf_create();
		ccn_uri_append(c,info->interest_ccnb,info->pi->offset[CCN_PI_E_Name],0);
		printf("%s\n",ccn_charbuf_as_string(c));
		ccn_charbuf_destroy(&c);

		process_incoming_interest(selfp, info);
		
		break;

        default:
            break;
    }

     nlsr_unlock();

    return CCN_UPCALL_RESULT_OK;
}

/* Function for processing incoming interest and reply with content/NACK content */

void 
process_incoming_interest(struct ccn_closure *selfp, struct ccn_upcall_info *info)
{
	printf("process_incoming_interest called \n");
	const unsigned char *comp_ptr1;
	size_t comp_size;
	int res,i;
	int nlsr_position=0;
	int name_comps=(int)info->interest_comps->n;

	for(i=0;i<name_comps;i++)
	{
		res=ccn_name_comp_strcmp(info->interest_ccnb,info->interest_comps,i,"nlsr");
		if( res == 0)
		{
			nlsr_position=i;
			break;
		}	
	}

	res=ccn_name_comp_get(info->interest_ccnb, info->interest_comps,nlsr_position+1,&comp_ptr1, &comp_size);

	printf("Det= %s \n",comp_ptr1);

	if(!strcmp((char *)comp_ptr1,"info"))
	{
		process_incoming_interest_info(selfp,info);
	}
	if(!strcmp((char *)comp_ptr1,"lsdb"))
	{
		process_incoming_interest_lsdb(selfp,info);
	}
	if(!strcmp((char *)comp_ptr1,"lsa"))
	{
		process_incoming_interest_lsa(selfp,info);
	}
}

void 
process_incoming_interest_info(struct ccn_closure *selfp, struct ccn_upcall_info *info)
{
	printf("process_incoming_interest_info called \n");

	printf("Sending Info Content back.....\n");
	int res;
	struct ccn_charbuf *data=ccn_charbuf_create();
    	struct ccn_charbuf *name=ccn_charbuf_create();
    	struct ccn_signing_params sp=CCN_SIGNING_PARAMS_INIT;

	res=ccn_charbuf_append(name, info->interest_ccnb + info->pi->offset[CCN_PI_B_Name],info->pi->offset[CCN_PI_E_Name] - info->pi->offset[CCN_PI_B_Name]);
	if (res >= 0)
	{
		sp.template_ccnb=ccn_charbuf_create();
		ccn_charbuf_append_tt(sp.template_ccnb,CCN_DTAG_SignedInfo, CCN_DTAG);
		ccnb_tagged_putf(sp.template_ccnb, CCN_DTAG_FreshnessSeconds, "%ld", 10);
       	 	sp.sp_flags |= CCN_SP_TEMPL_FRESHNESS;
		ccn_charbuf_append_closer(sp.template_ccnb);


		char *raw_data=(char *)malloc(16);
		memset(raw_data,0,16);
		sprintf(raw_data,"%ld", nlsr->lsdb_synch_interval);	

		res= ccn_sign_content(nlsr->ccn, data, name, &sp, raw_data,strlen(raw_data)); 
		if(res >= 0)
			printf("Signing Content is successful \n");

    		res=ccn_put(nlsr->ccn,data->buf,data->length);		
		if(res >= 0)
		printf("Sending Info Content is successful \n");

		printf("Info Content sending done....\n");


		struct name_prefix *nbr=(struct name_prefix * )malloc(sizeof(struct name_prefix *));
		get_lsa_identifier(nbr,selfp,info,-1);
		printf("Neighbor : %s Length : %d Status : %d\n",nbr->name,nbr->length,get_adjacent_status(nbr));


		if( get_adjacent_status(nbr) == 0 && get_timed_out_number(nbr)>=nlsr->interest_retry )
		{
			send_info_interest_to_neighbor(nbr);
		}

		free(nbr);
		free(raw_data);
		ccn_charbuf_destroy(&sp.template_ccnb);
	}

	ccn_charbuf_destroy(&data);
	ccn_charbuf_destroy(&name);

}


void 
process_incoming_interest_lsdb(struct ccn_closure *selfp, struct ccn_upcall_info *info)
{
	printf("process_incoming_interest_lsdb called \n");

	int l,res;
	const unsigned char *exclbase;
	size_t size;
	struct ccn_buf_decoder decoder;
	struct ccn_buf_decoder *d;
	const unsigned char *comp;
	int dbcmp=0;

	l = info->pi->offset[CCN_PI_E_Exclude] - info->pi->offset[CCN_PI_B_Exclude];
	if (l > 0) 
	{
		comp = NULL;
		size = 0;
		exclbase = info->interest_ccnb + info->pi->offset[CCN_PI_B_Exclude];
		d = ccn_buf_decoder_start(&decoder, exclbase, l);
		if (ccn_buf_match_dtag(d, CCN_DTAG_Exclude)) 
		{
			ccn_buf_advance(d);
			if (ccn_buf_match_dtag(d, CCN_DTAG_Any))
				ccn_buf_advance_past_element(d);
			if (ccn_buf_match_dtag(d, CCN_DTAG_Component)) 
			{
				ccn_buf_advance(d);
				ccn_buf_match_blob(d, &comp, &size);
				ccn_buf_check_close(d);			


			}
			ccn_buf_check_close(d);
		}
		if (comp != NULL)
		{
			printf("LSDB Version in Exclusion Filter is %s\n",comp);
			printf("LSDB Version of own NLSR is: %s \n",nlsr->lsdb->lsdb_version);
			dbcmp=strcmp(nlsr->lsdb->lsdb_version,(char *)comp);
		}
		/* Now comp points to the start of your potential number, and size is its length */
	}
	else
	{
		printf("LSDB Version in Exclusion Filter is: None Added\n");
		dbcmp=1;		

	}

	struct ccn_charbuf *data=ccn_charbuf_create();
	struct ccn_charbuf *name=ccn_charbuf_create();
	struct ccn_signing_params sp=CCN_SIGNING_PARAMS_INIT;
	
	ccn_charbuf_append(name, info->interest_ccnb + info->pi->offset[CCN_PI_B_Name],info->pi->offset[CCN_PI_E_Name] - info->pi->offset[CCN_PI_B_Name]);

	sp.template_ccnb=ccn_charbuf_create();
	ccn_charbuf_append_tt(sp.template_ccnb,CCN_DTAG_SignedInfo, CCN_DTAG);
	ccnb_tagged_putf(sp.template_ccnb, CCN_DTAG_FreshnessSeconds, "%ld", 10);
        sp.sp_flags |= CCN_SP_TEMPL_FRESHNESS;
        ccn_charbuf_append_closer(sp.template_ccnb);


	if(dbcmp>0)
	{
		printf("Has Updated Database than Neighbor\n");
		printf("Sending LSDB Summary of Updated LSDB Content...\n");			

		ccn_name_append_str(name,nlsr->lsdb->lsdb_version);

		struct ccn_charbuf *lsdb_data=ccn_charbuf_create();
		get_lsdb_summary(lsdb_data);

		char *raw_data=ccn_charbuf_as_string(lsdb_data);

		//printf("Content Data to be sent: %s \n",raw_data);		

		if( nlsr->is_build_adj_lsa_sheduled == 1 || strlen((char *)raw_data) == 0 )
		{
			 res= ccn_sign_content(nlsr->ccn, data, name, &sp, "WAIT" , strlen("WAIT"));
		}
		else
		{
			res= ccn_sign_content(nlsr->ccn, data, name, &sp, raw_data , strlen(raw_data));
		}
		if(res >= 0)
			printf("Signing LSDB Summary of Updated LSDB Content is successful \n");

	    	res=ccn_put(nlsr->ccn,data->buf,data->length);

		if(res >= 0)
			printf("Sending LSDB Summary of Updated LSDB Content is successful \n");
		
		ccn_charbuf_destroy(&lsdb_data);
	}
	else
	{
		printf("Does not have Updated Database than Neighbor\n");
		
		printf("Sending NACK Content.....\n");	
		res= ccn_sign_content(nlsr->ccn, data, name, &sp, "NACK", strlen("NACK")); 
		if(res >= 0)
			printf("Signing NACK Content is successful \n");

	    	res=ccn_put(nlsr->ccn,data->buf,data->length);

		if(res >= 0)
			printf("Sending NACK Content is successful \n");

		
	}

	ccn_charbuf_destroy(&data);
	ccn_charbuf_destroy(&name);
	ccn_charbuf_destroy(&sp.template_ccnb);


}


void 
process_incoming_interest_lsa(struct ccn_closure *selfp, struct ccn_upcall_info *info)
{
	printf("process_incoming_interest_lsa called \n");

	int res;

	struct name_prefix *lsaId=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
	get_lsa_identifier(lsaId,selfp,info,0);

	printf("LSA Identifier: %s Length: %d\n",lsaId->name,lsaId->length);
	int ls_type=get_ls_type(selfp, info);

	struct ccn_charbuf *lsa_data=ccn_charbuf_create();

	if ( ls_type == LS_TYPE_NAME )
	{
		printf("Interest Received for NAME LSA\n");
		get_name_lsa_data(lsa_data,lsaId);
	}
	else if ( ls_type == LS_TYPE_ADJ )
	{
		printf("Interest Received for ADJ LSA\n");
		get_adj_lsa_data(lsa_data,lsaId);
	}

	char *rdata=ccn_charbuf_as_string(lsa_data);
	char *raw_data=(char *)malloc(strlen(rdata)+1);
	memset(raw_data,0,strlen(rdata)+1);
	memcpy(raw_data,(char *)rdata,strlen(rdata)+1);
	//printf("Content Data to be sent: %s\n",raw_data);

	struct ccn_charbuf *data=ccn_charbuf_create();
	struct ccn_charbuf *name=ccn_charbuf_create();
	struct ccn_signing_params sp=CCN_SIGNING_PARAMS_INIT;

	ccn_charbuf_append(name, info->interest_ccnb + info->pi->offset[CCN_PI_B_Name],info->pi->offset[CCN_PI_E_Name] - info->pi->offset[CCN_PI_B_Name]); 

	sp.template_ccnb=ccn_charbuf_create();
	ccn_charbuf_append_tt(sp.template_ccnb,CCN_DTAG_SignedInfo, CCN_DTAG);
	ccnb_tagged_putf(sp.template_ccnb, CCN_DTAG_FreshnessSeconds, "%ld", 10);
        sp.sp_flags |= CCN_SP_TEMPL_FRESHNESS;
        ccn_charbuf_append_closer(sp.template_ccnb);

	res= ccn_sign_content(nlsr->ccn, data, name, &sp, raw_data , strlen(raw_data)); 
	if(res >= 0)
		printf("Signing LSA Content is successful \n");

	res=ccn_put(nlsr->ccn,data->buf,data->length);
	if(res >= 0)
		printf("Sending LSA Content is successful \n");



	ccn_charbuf_destroy(&data);
	ccn_charbuf_destroy(&name);
	ccn_charbuf_destroy(&sp.template_ccnb);
	ccn_charbuf_destroy(&lsa_data);

	free(raw_data);
	free(lsaId);
}

/* Call back function registered in ccnd to get all content coming to NLSR application */

enum ccn_upcall_res incoming_content(struct ccn_closure* selfp,
        enum ccn_upcall_kind kind, struct ccn_upcall_info* info)
{

     nlsr_lock();

    switch(kind) {
        case CCN_UPCALL_FINAL:
            break;
        case CCN_UPCALL_CONTENT:
            	printf("Content Received for Name: ");  
		struct ccn_charbuf*c;
		c=ccn_charbuf_create();
		ccn_uri_append(c,info->interest_ccnb,info->pi->offset[CCN_PI_E],0);
		printf("%s\n",ccn_charbuf_as_string(c));
		ccn_charbuf_destroy(&c);

		process_incoming_content(selfp,info);

	    break;
        case CCN_UPCALL_INTEREST_TIMED_OUT:
		printf("Interest Timed Out Received for Name: ");
		
		struct ccn_charbuf*ito;
		ito=ccn_charbuf_create();
		ccn_uri_append(ito,info->interest_ccnb,info->pi->offset[CCN_PI_E],0);
		printf("%s\n",ccn_charbuf_as_string(ito));
		ccn_charbuf_destroy(&ito);

		process_incoming_timed_out_interest(selfp,info);

	    break;
        default:
            fprintf(stderr, "Unexpected response of kind %d\n", kind);
            //return CCN_UPCALL_RESULT_ERR;
	    break;
    }
    
     nlsr_unlock();

    return CCN_UPCALL_RESULT_OK;
}


void 
process_incoming_content(struct ccn_closure *selfp, struct ccn_upcall_info* info)
{
	printf("process_incoming_content called \n");

	const unsigned char *comp_ptr1;
	size_t comp_size;
	int res,i;
	int nlsr_position=0;
	int name_comps=(int)info->interest_comps->n;

	for(i=0;i<name_comps;i++)
	{
		res=ccn_name_comp_strcmp(info->interest_ccnb,info->interest_comps,i,"nlsr");
		if( res == 0)
		{
			nlsr_position=i;
			break;
		}	
	}

	res=ccn_name_comp_get(info->interest_ccnb, info->interest_comps,nlsr_position+1,&comp_ptr1, &comp_size);

	printf("Det= %s \n",comp_ptr1);

	if(!strcmp((char *)comp_ptr1,"info"))
	{
		process_incoming_content_info(selfp,info);
	}
	if(!strcmp((char *)comp_ptr1,"lsdb"))
	{
		process_incoming_content_lsdb(selfp,info);
	}
	if(!strcmp((char *)comp_ptr1,"lsa"))
	{
		process_incoming_content_lsa(selfp,info);
	}

}


void 
process_incoming_content_info(struct ccn_closure *selfp, struct ccn_upcall_info* info)
{
	printf("process_incoming_content_info called \n");

	struct name_prefix *nbr=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
	get_nbr(nbr,selfp,info);

	printf("Info Content Received For Neighbor: %s Length:%d\n",nbr->name,nbr->length);


	const unsigned char *ptr;
	size_t length;
	ccn_content_get_value(info->content_ccnb, info->pco->offset[CCN_PCO_E_Content]-info->pco->offset[CCN_PCO_B_Content], info->pco, &ptr, &length);
	//printf("Content data: %s\n",ptr);

	long int interval=atoi((char *)ptr);



	update_adjacent_timed_out_zero_to_adl(nbr);	
	update_adjacent_status_to_adl(nbr,NBR_ACTIVE);
	update_lsdb_synch_interval_to_adl(nbr,interval);
	print_adjacent_from_adl();



	if(!nlsr->is_build_adj_lsa_sheduled)
	{
		printf("Scheduling Build and Install Adj LSA...\n");
		nlsr->event_build_adj_lsa = ccn_schedule_event(nlsr->sched, 100000, &build_and_install_adj_lsa, NULL, 0);
		nlsr->is_build_adj_lsa_sheduled=1;		
	}
	else
	{
		printf("Build and Install Adj LSA already scheduled\n");
	}


	free(nbr);


}


void 
process_incoming_content_lsdb(struct ccn_closure *selfp, struct ccn_upcall_info* info)
{
	printf("process_incoming_content_lsdb called \n");

	const unsigned char *ptr;
	size_t length;
	ccn_content_get_value(info->content_ccnb, info->pco->offset[CCN_PCO_E_Content]-info->pco->offset[CCN_PCO_B_Content], info->pco, &ptr, &length);
	//printf("Content data: %s\n",ptr);

	if( (strcmp("NACK",(char *)ptr) != 0 ) && (strcmp("WAIT",(char *)ptr) != 0 ) )
	{
		struct name_prefix *nbr=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
		get_nbr(nbr,selfp,info);

		char *nl;
		int num_element;
		int i;
		char *rem;
		const char *sep="|";
		char *orig_router;
		char *lst;
		int ls_type;
		char *lsid;
		long int ls_id;
		char *orig_time;

		nl=strtok_r((char *)ptr,sep,&rem);
		num_element=atoi(nl);		

		for(i = 0 ; i < num_element ; i++)
		{
			orig_router=strtok_r(NULL,sep,&rem);
			lst=strtok_r(NULL,sep,&rem);
			ls_type=atoi(lst);
			printf("Orig Router: %s ls Type: %d",orig_router,ls_type);			

			if(ls_type == LS_TYPE_NAME)
			{
				lsid=strtok_r(NULL,sep,&rem);
				ls_id=atoi(lsid);
				orig_time=strtok_r(NULL,sep,&rem);
				printf(" LS Id: %ld  Orig Time: %s\n",ls_id ,orig_time);
				int is_new_name_lsa=check_is_new_name_lsa(orig_router,lst,lsid,orig_time);
				if ( is_new_name_lsa == 1 )
				{
					printf("New NAME LSA.....\n");
					send_interest_for_name_lsa(nbr,orig_router,lst,lsid);	
				}
				else 
				{
					printf("Name LSA already exists in LSDB\n");
				}
			}
			else
			{
				orig_time=strtok_r(NULL,sep,&rem);
				printf(" Orig Time: %s\n",orig_time);
				int is_new_adj_lsa=check_is_new_adj_lsa(orig_router,lst,orig_time);
				if ( is_new_adj_lsa == 1 )
				{
					printf("New ADJ LSA.....\n");
					send_interest_for_adj_lsa(nbr,orig_router,lst);
				}
				else
				{
					printf("ADJ LSA already exists in LSDB\n");
				}
			}

		}

		char *lsdb_version=(char *)malloc(20);
		memset(lsdb_version,0,20);
		get_lsdb_version(lsdb_version,selfp,info);

		printf("Old LSDB Version of Neighbor: %s is :%s\n",nbr->name,get_nbr_lsdb_version(nbr->name));		
		update_adjacent_lsdb_version_to_adl(nbr,lsdb_version);
		printf("New LSDB Version of Neighbor: %s is :%s\n",nbr->name,get_nbr_lsdb_version(nbr->name));

		update_lsdb_interest_timed_out_zero_to_adl(nbr);

		free(lsdb_version);
		free(nbr);	
	}
	else if (strcmp("WAIT",(char *)ptr) == 0)
	{
		struct name_prefix *nbr=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
		get_nbr(nbr,selfp,info);
		long int interval=get_lsdb_synch_interval(nbr->name);
		adjust_adjacent_last_lsdb_requested_to_adl(nbr->name,(long int)interval/2);

		update_lsdb_interest_timed_out_zero_to_adl(nbr);
		free(nbr);
	}
	else 
	{
		printf("NACK Content Received\n");
		struct name_prefix *nbr=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
		get_nbr(nbr,selfp,info);
		update_lsdb_interest_timed_out_zero_to_adl(nbr);
	}
}


void 
process_incoming_content_lsa(struct ccn_closure *selfp, struct ccn_upcall_info* info)
{
	printf("process_incoming_content_lsa called \n");

	char *sep="|";
	char *rem;
	char *orig_router;
	char *orl;
	int orig_router_length;
	char *lst;
	int ls_type;
	char *lsid;
	long int ls_id;
	char *isvld;
	int isValid;
	char *num_link;
	int no_link;
	char *np;
	char *np_length;
	int name_length;
	char *data;
	char *orig_time;

	const unsigned char *ptr;
	size_t length;
	ccn_content_get_value(info->content_ccnb, info->pco->offset[CCN_PCO_E_Content]-info->pco->offset[CCN_PCO_B_Content], info->pco, &ptr, &length);
	//printf("Content data Received: %s\n",ptr);

	

	
	

	printf("LSA Data\n");

	if( strlen((char *) ptr ) > 0 )
	{

		orig_router=strtok_r((char *)ptr,sep,&rem);
		orl=strtok_r(NULL,sep,&rem);
		orig_router_length=atoi(orl);

		printf("	Orig Router Name  : %s\n",orig_router);
		printf("	Orig Router Length: %d\n",orig_router_length);

		lst=strtok_r(NULL,sep,&rem);		
		ls_type=atoi(lst);

		printf("	LS Type  : %d\n",ls_type);

		if ( ls_type == LS_TYPE_NAME )
		{
			lsid=strtok_r(NULL,sep,&rem);
			ls_id=atoi(lsid);
			orig_time=strtok_r(NULL,sep,&rem);
			isvld=strtok_r(NULL,sep,&rem);
			isValid=atoi(isvld);
			np=strtok_r(NULL,sep,&rem);
			np_length=strtok_r(NULL,sep,&rem);
			name_length=atoi(np_length);
			printf("	LS ID  : %ld\n",ls_id);
			printf("	isValid  : %d\n",isValid);
			printf("	Name Prefix : %s\n",np);
			printf("	Orig Time   : %s\n",orig_time);
			printf("	Name Prefix length: %d\n",name_length);

			build_and_install_others_name_lsa(orig_router,ls_type,ls_id,orig_time,isValid,np);

		}
		else if ( ls_type == LS_TYPE_ADJ )
		{
			orig_time=strtok_r(NULL,sep,&rem);
			num_link=strtok_r(NULL,sep,&rem);
			no_link=atoi(num_link);
			data=rem;

			printf("	No Link  : %d\n",no_link);
			printf("	Data  : %s\n",data);

			build_and_install_others_adj_lsa(orig_router,ls_type,orig_time,no_link,data);
		}
	}
}


void
process_incoming_timed_out_interest(struct ccn_closure* selfp, struct ccn_upcall_info* info)
{
	printf("process_incoming_timed_out_interest called \n");
	int res,i;
	int nlsr_position=0;
	int name_comps=(int)info->interest_comps->n;

	for(i=0;i<name_comps;i++)
	{
		res=ccn_name_comp_strcmp(info->interest_ccnb,info->interest_comps,i,"nlsr");
		if( res == 0)
		{
			nlsr_position=i;
			break;
		}	
	}

	if(ccn_name_comp_strcmp(info->interest_ccnb,info->interest_comps,nlsr_position+1,"info") == 0)
	{
		process_incoming_timed_out_interest_info(selfp,info);
	}
	if(ccn_name_comp_strcmp(info->interest_ccnb,info->interest_comps,nlsr_position+1,"lsdb") == 0)
	{
		process_incoming_timed_out_interest_lsdb(selfp,info);
	}
	if(ccn_name_comp_strcmp(info->interest_ccnb,info->interest_comps,nlsr_position+1,"lsa") == 0)
	{
		process_incoming_timed_out_interest_lsa(selfp,info);
	}
}

void
process_incoming_timed_out_interest_info(struct ccn_closure* selfp, struct ccn_upcall_info* info)
{
	printf("process_incoming_timed_out_interest_info called \n");

	struct name_prefix *nbr=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
	get_nbr(nbr,selfp,info);

	printf("Info Interest Timed Out for for Neighbor: %s Length:%d\n",nbr->name,nbr->length);


	update_adjacent_timed_out_to_adl(nbr,1);
	print_adjacent_from_adl();	
	int timed_out=get_timed_out_number(nbr);


	if(timed_out<nlsr->interest_retry && timed_out>0) // use configured variables 
	{
		printf("Neighbor: %s Info Interest Timed Out: %d times\n",nbr->name,timed_out);
		send_info_interest_to_neighbor(nbr);
	}
	else
	{
		printf("Neighbor: %s Info Interest Timed Out: %d times\n",nbr->name,timed_out);		
		update_adjacent_status_to_adl(nbr,NBR_DOWN);
		if(!nlsr->is_build_adj_lsa_sheduled)
		{
			nlsr->event_build_adj_lsa = ccn_schedule_event(nlsr->sched, 1000, &build_and_install_adj_lsa, NULL, 0);
			nlsr->is_build_adj_lsa_sheduled=1;		
		}
	}

	free(nbr);


}

void
process_incoming_timed_out_interest_lsdb(struct ccn_closure* selfp, struct ccn_upcall_info* info)
{
	printf("process_incoming_timed_out_interest_lsdb called \n");

	struct name_prefix *nbr=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
	get_nbr(nbr,selfp,info);

	printf("LSDB Interest Timed Out for for Neighbor: %s Length:%d\n",nbr->name,nbr->length);

	update_lsdb_interest_timed_out_to_adl(nbr,1);

	int interst_timed_out_num=get_lsdb_interest_timed_out_number(nbr);

	printf("Interest Timed out number : %d Interest Retry: %d \n",interst_timed_out_num,nlsr->interest_retry);

	if( interst_timed_out_num >= nlsr->interest_retry )
	{
		update_adjacent_status_to_adl(nbr,NBR_DOWN);
		if(!nlsr->is_build_adj_lsa_sheduled)
		{
			nlsr->event_build_adj_lsa = ccn_schedule_event(nlsr->sched, 1000, &build_and_install_adj_lsa, NULL, 0);
			nlsr->is_build_adj_lsa_sheduled=1;		
		}
	}
	free(nbr->name);
	free(nbr);
}

void
process_incoming_timed_out_interest_lsa(struct ccn_closure* selfp, struct ccn_upcall_info* info)
{
	printf("process_incoming_timed_out_interest_lsa called \n");
	
}

int
send_info_interest(struct ccn_schedule *sched, void *clienth, struct ccn_scheduled_event *ev, int flags)
{
	if(flags == CCN_SCHEDULE_CANCEL)
	{
 	 	return -1;
	}

         nlsr_lock();

	printf("send_info_interest called \n");
	printf("\n");

	int adl_element,i;
	struct ndn_neighbor *nbr;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->adl, e);
	adl_element=hashtb_n(nlsr->adl);

	for(i=0;i<adl_element;i++)
	{
		nbr=e->data;
		send_info_interest_to_neighbor(nbr->neighbor);
		hashtb_next(e);		
	}
	hashtb_end(e);

	 nlsr_unlock();

	return 0;
}

void 
send_info_interest_to_neighbor(struct name_prefix *nbr)
{
	printf("send_info_interest_to_neighbor called \n");

	int res;
	char info_str[5];
	char nlsr_str[5];
	
	memset(&nlsr_str,0,5);
	sprintf(nlsr_str,"nlsr");
	memset(&info_str,0,5);
	sprintf(info_str,"info");
	
	
	struct ccn_charbuf *name;	
	name=ccn_charbuf_create();

	char *int_name=(char *)malloc(strlen(nbr->name)+1+strlen(nlsr_str)+1+strlen(info_str)+strlen(nlsr->router_name)+1);
	memset(int_name,0,strlen(nbr->name)+1+strlen(nlsr_str)+1+strlen(info_str)+strlen(nlsr->router_name)+1);
	memcpy(int_name+strlen(int_name),nbr->name,strlen(nbr->name));
	memcpy(int_name+strlen(int_name),"/",1);
	memcpy(int_name+strlen(int_name),nlsr_str,strlen(nlsr_str));
	memcpy(int_name+strlen(int_name),"/",1);
	memcpy(int_name+strlen(int_name),info_str,strlen(info_str));
	memcpy(int_name+strlen(int_name),nlsr->router_name,strlen(nlsr->router_name));
	

	res=ccn_name_from_uri(name,int_name);
	if ( res >=0 )
	{
		/* adding InterestLifeTime and InterestScope filter */

		struct ccn_charbuf *templ;
		templ = ccn_charbuf_create();

		ccn_charbuf_append_tt(templ, CCN_DTAG_Interest, CCN_DTAG);
		ccn_charbuf_append_tt(templ, CCN_DTAG_Name, CCN_DTAG);
		ccn_charbuf_append_closer(templ); /* </Name> */
		ccn_charbuf_append_tt(templ, CCN_DTAG_Scope, CCN_DTAG);
		ccn_charbuf_append_tt(templ, 1, CCN_UDATA);
		ccn_charbuf_append(templ, "2", 1); //scope of interest: 2 (not further than next host)
		ccn_charbuf_append_closer(templ); /* </Scope> */

		appendLifetime(templ,nlsr->interest_resend_time);
		ccn_charbuf_append_closer(templ); /* </Interest> */
		/* Adding InterestLifeTime and InterestScope filter done */
	
		printf("Sending info interest on name prefix : %s \n",int_name);

		res=ccn_express_interest(nlsr->ccn,name,&(nlsr->in_content),templ);

		if ( res >= 0 )
			printf("Info interest sending Successfull .... \n");	
		ccn_charbuf_destroy(&templ);
	}
	ccn_charbuf_destroy(&name);
	free(int_name);

}


int 
send_lsdb_interest(struct ccn_schedule *sched, void *clienth, struct ccn_scheduled_event *ev, int flags)
{
	printf("send_lsdb_interest called \n");	

	if(flags == CCN_SCHEDULE_CANCEL)
	{
 	 	return -1;
	}

	 nlsr_lock();

	int i, adl_element;
	struct ndn_neighbor *nbr;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->adl, e);
	adl_element=hashtb_n(nlsr->adl);

	for(i=0;i<adl_element;i++)
	{
		nbr=e->data;

		if(nbr->status == NBR_ACTIVE)
		{	
			if(nbr->is_lsdb_send_interest_scheduled == 0)
			{
				long int time_diff=get_nbr_time_diff_lsdb_req(nbr->neighbor->name);	
				printf("Time since last time LSDB requested : %ld Seconds for Neighbor: %s \n",time_diff,nbr->neighbor->name);		

				if( time_diff >= ( get_lsdb_synch_interval(nbr->neighbor->name) + get_nbr_random_time_component(nbr->neighbor->name) ) )
				{
					nbr->is_lsdb_send_interest_scheduled=1;
					send_lsdb_interest_to_nbr(nbr->neighbor);
				}
			}
		}
		hashtb_next(e);		
	}

	hashtb_end(e);
	nlsr->event_send_lsdb_interest= ccn_schedule_event(nlsr->sched, 30000000, &send_lsdb_interest, NULL, 0);

	 nlsr_unlock();

	return 0;
}

void 
send_lsdb_interest_to_nbr(struct name_prefix *nbr)
{
	printf("send_lsdb_interest_to_nbr called \n");	

	char *last_lsdb_version=get_nbr_lsdb_version(nbr->name);

	if(last_lsdb_version !=NULL)
	{
		printf("Last LSDB Version: %s \n",last_lsdb_version);


		struct ccn_charbuf *name;
		int res;
		char lsdb_str[5];
		char nlsr_str[5];

		memset(&nlsr_str,0,5);
		sprintf(nlsr_str,"nlsr");
		memset(&lsdb_str,0,5);
		sprintf(lsdb_str,"lsdb");		
		//make and send interest with exclusion filter as last_lsdb_version
		printf("Sending interest for name prefix:%s/%s/%s\n",nbr->name,nlsr_str,lsdb_str);	
		name=ccn_charbuf_create();
		res=ccn_name_from_uri(name,nbr->name);

		if( res >= 0)
		{
			ccn_name_append_str(name,nlsr_str);
			ccn_name_append_str(name,lsdb_str);
			/* adding Exclusion filter */

			struct ccn_charbuf *templ;
			templ = ccn_charbuf_create();

			struct ccn_charbuf *c;
			c = ccn_charbuf_create();


			ccn_charbuf_append_tt(templ, CCN_DTAG_Interest, CCN_DTAG);
			ccn_charbuf_append_tt(templ, CCN_DTAG_Name, CCN_DTAG);
			ccn_charbuf_append_closer(templ); /* </Name> */
			ccn_charbuf_append_tt(templ, CCN_DTAG_Exclude, CCN_DTAG);
			ccnb_tagged_putf(templ, CCN_DTAG_Any, "");
			ccn_charbuf_reset(c);
			ccn_charbuf_putf(c, "%s", last_lsdb_version);
			
			ccnb_append_tagged_blob(templ, CCN_DTAG_Component, c->buf, c->length);
			ccn_charbuf_append_closer(templ); /* </Exclude> */
			ccn_charbuf_append_tt(templ, CCN_DTAG_Scope, CCN_DTAG);
			ccn_charbuf_append_tt(templ, 1, CCN_UDATA);
			ccn_charbuf_append(templ, "2", 1);
			ccn_charbuf_append_closer(templ); /* </Scope> */

			appendLifetime(templ,nlsr->interest_resend_time);

			ccn_charbuf_append_closer(templ); /* </Interest> */


			/* Adding Exclusion filter done */

			res=ccn_express_interest(nlsr->ccn,name,&(nlsr->in_content),templ);

			if ( res >= 0 )
			{
				printf("Interest sending Successfull .... \n");	
				update_adjacent_last_lsdb_requested_to_adl(nbr->name,get_current_time_sec());

			}
			ccn_charbuf_destroy(&c);
			ccn_charbuf_destroy(&templ);
		}
		ccn_charbuf_destroy(&name);
	}	
	set_is_lsdb_send_interest_scheduled_to_zero(nbr->name);
}

void 
send_interest_for_name_lsa(struct name_prefix *nbr, char *orig_router, char *ls_type, char *ls_id)
{
	printf("send_interest_for_name_lsa called\n");

	int res;
	char lsa_str[5];
	char nlsr_str[5];

	memset(&nlsr_str,0,5);
	sprintf(nlsr_str,"nlsr");
	memset(&lsa_str,0,5);
	sprintf(lsa_str,"lsa");

	char *int_name=(char *)malloc(nbr->length + strlen(ls_type)+strlen(orig_router)+strlen(nlsr_str)+strlen(lsa_str)+3);
	memset(int_name,0,nbr->length +strlen(ls_type)+ strlen(orig_router)+strlen(nlsr_str)+strlen(lsa_str)+3);

	memcpy(int_name+strlen(int_name),nbr->name,nbr->length);
	memcpy(int_name+strlen(int_name),"/",1);
	memcpy(int_name+strlen(int_name),nlsr_str,strlen(nlsr_str));
	memcpy(int_name+strlen(int_name),"/",1);
	memcpy(int_name+strlen(int_name),lsa_str,strlen(lsa_str));
	memcpy(int_name+strlen(int_name),"/",1);
	memcpy(int_name+strlen(int_name),ls_type,strlen(ls_type));
	memcpy(int_name+strlen(int_name),orig_router,strlen(orig_router));


	struct ccn_charbuf *name;	
	name=ccn_charbuf_create();


	res=ccn_name_from_uri(name,int_name);
	ccn_name_append_str(name,ls_type);
	ccn_name_append_str(name,ls_id);


	/* adding InterestLifeTime and InterestScope filter */

	struct ccn_charbuf *templ;
	templ = ccn_charbuf_create();

	ccn_charbuf_append_tt(templ, CCN_DTAG_Interest, CCN_DTAG);
	ccn_charbuf_append_tt(templ, CCN_DTAG_Name, CCN_DTAG);
	ccn_charbuf_append_closer(templ); /* </Name> */
	//ccnb_tagged_putf(templ, CCN_DTAG_Scope, "%d", scope);
	ccn_charbuf_append_tt(templ, CCN_DTAG_Scope, CCN_DTAG);
	ccn_charbuf_append_tt(templ, 1, CCN_UDATA);
	ccn_charbuf_append(templ, "2", 1); //scope of interest: 2 (not further than next host)
	ccn_charbuf_append_closer(templ); /* </Scope> */

	appendLifetime(templ,nlsr->interest_resend_time);
	ccn_charbuf_append_closer(templ); /* </Interest> */
	/* Adding InterestLifeTime and InterestScope filter done */

	printf("Sending NAME LSA interest on name prefix : %s/%s/%s\n",int_name,ls_type,ls_id);

	res=ccn_express_interest(nlsr->ccn,name,&(nlsr->in_content),templ);

	if ( res >= 0 )
		printf("NAME LSA interest sending Successfull .... \n");	
	
	ccn_charbuf_destroy(&templ);
	ccn_charbuf_destroy(&name);
	free(int_name);


}

void 
send_interest_for_adj_lsa(struct name_prefix *nbr, char *orig_router, char *ls_type)
{
	printf("send_interest_for_adj_lsa called\n");

	int res;
	char lsa_str[5];
	char nlsr_str[5];

	memset(&nlsr_str,0,5);
	sprintf(nlsr_str,"nlsr");
	memset(&lsa_str,0,5);
	sprintf(lsa_str,"lsa");

	char *int_name=(char *)malloc(nbr->length + strlen(ls_type)+strlen(orig_router)+strlen(nlsr_str)+strlen(lsa_str)+3+strlen(ls_type)+1);
	memset(int_name,0,nbr->length +strlen(ls_type)+ strlen(orig_router)+strlen(nlsr_str)+strlen(lsa_str)+3+strlen(ls_type)+1);

	memcpy(int_name+strlen(int_name),nbr->name,nbr->length);
	memcpy(int_name+strlen(int_name),"/",1);
	memcpy(int_name+strlen(int_name),nlsr_str,strlen(nlsr_str));
	memcpy(int_name+strlen(int_name),"/",1);
	memcpy(int_name+strlen(int_name),lsa_str,strlen(lsa_str));
	memcpy(int_name+strlen(int_name),"/",1);
	memcpy(int_name+strlen(int_name),ls_type,strlen(ls_type));
	memcpy(int_name+strlen(int_name),orig_router,strlen(orig_router));
	memcpy(int_name+strlen(int_name),"/",1);
	memcpy(int_name+strlen(int_name),ls_type,strlen(ls_type));

	struct ccn_charbuf *name;	
	name=ccn_charbuf_create();


	ccn_name_from_uri(name,int_name);
	
	/* adding InterestLifeTime and InterestScope filter */

	struct ccn_charbuf *templ;
	templ = ccn_charbuf_create();

	ccn_charbuf_append_tt(templ, CCN_DTAG_Interest, CCN_DTAG);
	ccn_charbuf_append_tt(templ, CCN_DTAG_Name, CCN_DTAG);
	ccn_charbuf_append_closer(templ); /* </Name> */
	ccn_charbuf_append_tt(templ, CCN_DTAG_Scope, CCN_DTAG);
	ccn_charbuf_append_tt(templ, 1, CCN_UDATA);
	ccn_charbuf_append(templ, "2", 1); //scope of interest: 2 (not further than next host)
	ccn_charbuf_append_closer(templ); /* </Scope> */

	appendLifetime(templ,nlsr->interest_resend_time);
	ccn_charbuf_append_closer(templ); /* </Interest> */
	/* Adding InterestLifeTime and InterestScope filter done */

	printf("Sending ADJ LSA interest on name prefix : %s\n",int_name);

	res=ccn_express_interest(nlsr->ccn,name,&(nlsr->in_content),templ);

	if ( res >= 0 )
	{
		printf("ADJ LSA interest sending Successfull .... \n");	
	}
	
	ccn_charbuf_destroy(&templ);
	ccn_charbuf_destroy(&name);
	free(int_name);
}
