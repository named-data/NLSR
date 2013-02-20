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
	if ( nlsr->debugging )
		printf("get_nbr called\n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"get_nbr called\n");

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

	if ( nlsr->debugging )
		printf("Neighbor: %s Length: %d\n",nbr->name,nbr->length);
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"Neighbor: %s Length: %d\n",nbr->name,nbr->length);

	

}

void 
get_lsa_identifier(struct name_prefix *lsaId,struct ccn_closure *selfp, struct ccn_upcall_info *info, int offset)
{

	//printf("get_lsa_identifier called\n");

	if ( nlsr->debugging )
		printf("get_lsa_identifier called\n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"get_lsa_identifier called\n");
	
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

	if ( nlsr->debugging )
		printf("LSA Identifier: %s Length: %d\n",lsaId->name,lsaId->length-1);
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"LSA Identifier: %s Length: %d\n",lsaId->name,lsaId->length-1);

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
		if ( nlsr->debugging )
            		printf("Interest Received for name: "); 
		if ( nlsr->detailed_logging )
            		writeLogg(__FILE__,__FUNCTION__,__LINE__,"Interest Received for name: ");
		
	    	struct ccn_charbuf*c;
		c=ccn_charbuf_create();
		ccn_uri_append(c,info->interest_ccnb,info->pi->offset[CCN_PI_E_Name],0);

		if ( nlsr->debugging )
			printf("%s\n",ccn_charbuf_as_string(c));
		if ( nlsr->detailed_logging )
			writeLogg(__FILE__,__FUNCTION__,__LINE__,"%s\n",ccn_charbuf_as_string(c));

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
	if ( nlsr->debugging )
		printf("process_incoming_interest called \n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"process_incoming_interest called \n");
	
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


	if(!strcmp((char *)comp_ptr1,"info"))
	{
		process_incoming_interest_info(selfp,info);
	}

}

void 
process_incoming_interest_info(struct ccn_closure *selfp, struct ccn_upcall_info *info)
{
	if ( nlsr->debugging )
	{
		printf("process_incoming_interest_info called \n");
		printf("Sending Info Content back.....\n");
	}
	if ( nlsr->detailed_logging )
	{
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"process_incoming_interest_info called \n");
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"Sending Info Content back.....\n");
	}
	

	int res;
	struct ccn_charbuf *data=ccn_charbuf_create();
    	struct ccn_charbuf *name=ccn_charbuf_create();
    	struct ccn_signing_params sp=CCN_SIGNING_PARAMS_INIT;

	res=ccn_charbuf_append(name, info->interest_ccnb + info->pi->offset[CCN_PI_B_Name],info->pi->offset[CCN_PI_E_Name] - info->pi->offset[CCN_PI_B_Name]);
	if (res >= 0)
	{
		sp.template_ccnb=ccn_charbuf_create();

		struct ccn_charbuf *pubid = ccn_charbuf_create();
		struct ccn_charbuf *pubkey = ccn_charbuf_create();

		int res1;
		res1 = ccn_get_public_key(nlsr->ccn, NULL, pubid, pubkey);		
		ccn_charbuf_append_tt(sp.template_ccnb, CCN_DTAG_SignedInfo, CCN_DTAG);
		ccn_charbuf_append_tt(sp.template_ccnb, CCN_DTAG_KeyLocator, CCN_DTAG);
		ccn_charbuf_append_tt(sp.template_ccnb, CCN_DTAG_KeyName, CCN_DTAG);
		ccn_charbuf_append_charbuf(sp.template_ccnb, pubid);
		ccn_charbuf_append_closer(sp.template_ccnb);
		ccn_charbuf_append_closer(sp.template_ccnb);

		ccnb_tagged_putf(sp.template_ccnb, CCN_DTAG_FreshnessSeconds, "%ld", 10);
       	 	sp.sp_flags |= CCN_SP_TEMPL_FRESHNESS;		

		ccn_charbuf_append_closer(sp.template_ccnb);
		
		sp.sp_flags |= CCN_SP_TEMPL_KEY_LOCATOR;
		sp.sp_flags |= CCN_SP_FINAL_BLOCK;
		sp.type = CCN_CONTENT_KEY;
		
		//ccn_charbuf_append_tt(sp.template_ccnb,CCN_DTAG_SignedInfo, CCN_DTAG);
		//ccnb_tagged_putf(sp.template_ccnb, CCN_DTAG_FreshnessSeconds, "%ld", 10);
       	 	//sp.sp_flags |= CCN_SP_TEMPL_FRESHNESS;
		//ccn_charbuf_append_closer(sp.template_ccnb);


		char *raw_data=(char *)malloc(20);
		memset(raw_data,0,20);
		sprintf(raw_data,"%s", nlsr->lsdb->lsdb_version);	

		res= ccn_sign_content(nlsr->ccn, data, name, &sp, pubkey->buf,pubkey->length); 
		if(res >= 0)
		{
			if ( nlsr->debugging )
				printf("Signing info Content is successful \n");
			if ( nlsr->detailed_logging )
				writeLogg(__FILE__,__FUNCTION__,__LINE__,"Signing info Content is successful \n");

		}
    		res=ccn_put(nlsr->ccn,data->buf,data->length);		
		if(res >= 0)
		{
			if ( nlsr->debugging )
				printf("Sending Info Content is successful \n");
			if ( nlsr->detailed_logging )
				writeLogg(__FILE__,__FUNCTION__,__LINE__,"Sending info Content is successful \n");
		}
		


		struct name_prefix *nbr=(struct name_prefix * )malloc(sizeof(struct name_prefix *));
		get_lsa_identifier(nbr,selfp,info,-1);

		if ( nlsr->debugging )
			printf("Neighbor : %s Length : %d Status : %d\n",nbr->name,nbr->length,get_adjacent_status(nbr));
		if ( nlsr->detailed_logging )
			writeLogg(__FILE__,__FUNCTION__,__LINE__,"Neighbor : %s Length : %d Status : %d\n",nbr->name,nbr->length,get_adjacent_status(nbr));		

		//printf("Neighbor : %s Length : %d Status : %d\n",nbr->name,nbr->length,get_adjacent_status(nbr));


		if( get_adjacent_status(nbr) == 0 && get_timed_out_number(nbr)>=nlsr->interest_retry )
		{
			update_adjacent_timed_out_zero_to_adl(nbr);
			send_info_interest_to_neighbor(nbr);
		}

		free(nbr);
		free(raw_data);
		ccn_charbuf_destroy(&sp.template_ccnb);
		ccn_charbuf_destroy(&pubid);
		ccn_charbuf_destroy(&pubkey);
	}

	ccn_charbuf_destroy(&data);
	ccn_charbuf_destroy(&name);

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
		if ( nlsr->debugging )
			printf("Content Received for Name: "); 
		if ( nlsr->detailed_logging )
			writeLogg(__FILE__,__FUNCTION__,__LINE__,"Content Received for Name: ");
    
		struct ccn_charbuf*c;
		c=ccn_charbuf_create();
		ccn_uri_append(c,info->interest_ccnb,info->pi->offset[CCN_PI_E],0);
		if ( nlsr->debugging )
			printf("%s\n",ccn_charbuf_as_string(c));
		if ( nlsr->detailed_logging )
			writeLogg(__FILE__,__FUNCTION__,__LINE__,"%s\n",ccn_charbuf_as_string(c));

		ccn_charbuf_destroy(&c);

		process_incoming_content(selfp,info);

	    break;
        case CCN_UPCALL_INTEREST_TIMED_OUT:
		//printf("Interest Timed Out Received for Name: ");
		if ( nlsr->debugging )
			printf("Interest Timed Out Received for Name:  "); 
		if ( nlsr->detailed_logging )
			writeLogg(__FILE__,__FUNCTION__,__LINE__,"Interest Timed Out Received for Name: ");
		
		struct ccn_charbuf*ito;
		ito=ccn_charbuf_create();
		ccn_uri_append(ito,info->interest_ccnb,info->pi->offset[CCN_PI_E],0);
		if ( nlsr->debugging )
			printf("%s\n",ccn_charbuf_as_string(ito));
		if ( nlsr->detailed_logging )
			writeLogg(__FILE__,__FUNCTION__,__LINE__,"%s\n",ccn_charbuf_as_string(ito));
		ccn_charbuf_destroy(&ito);

		

		process_incoming_timed_out_interest(selfp,info);

	    break;
        default:
            fprintf(stderr, "Unexpected response of kind %d\n", kind);
	    if ( nlsr->debugging )
		printf("Unexpected response of kind %d\n", kind);
	    if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"Unexpected response of kind %d\n", kind);
	    break;
    }
    
     nlsr_unlock();

    return CCN_UPCALL_RESULT_OK;
}


void 
process_incoming_content(struct ccn_closure *selfp, struct ccn_upcall_info* info)
{
	if ( nlsr->debugging )
		printf("process_incoming_content called \n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"process_incoming_content called \n");

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


	if(!strcmp((char *)comp_ptr1,"info"))
	{
		process_incoming_content_info(selfp,info);
	}

}


void 
process_incoming_content_info(struct ccn_closure *selfp, struct ccn_upcall_info* info)
{
	if ( nlsr->debugging )
		printf("process_incoming_content_info called \n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"process_incoming_content_info called \n");

	struct name_prefix *nbr=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
	get_nbr(nbr,selfp,info);

	if ( nlsr->debugging )
		printf("Info Content Received For Neighbor: %s Length:%d\n",nbr->name,nbr->length);
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"Info Content Received For Neighbor: %s Length:%d\n",nbr->name,nbr->length);


	if ( contain_key_name(info->content_ccnb, info->pco) == 1)
	{
		struct ccn_charbuf *key_name=get_key_name(info->content_ccnb, info->pco);
		if(nlsr->debugging)
			printf("Key Name: %s\n",ccn_charbuf_as_string(key_name));
	}

	update_adjacent_timed_out_zero_to_adl(nbr);	
	update_adjacent_status_to_adl(nbr,NBR_ACTIVE);
	print_adjacent_from_adl();



	if(!nlsr->is_build_adj_lsa_sheduled)
	{
		if ( nlsr->debugging )
			printf("Scheduling Build and Install Adj LSA...\n");
		if ( nlsr->detailed_logging )
			writeLogg(__FILE__,__FUNCTION__,__LINE__,"Scheduling Build and Install Adj LSA...\n");
		nlsr->event_build_adj_lsa = ccn_schedule_event(nlsr->sched, 100000, &build_and_install_adj_lsa, NULL, 0);
		nlsr->is_build_adj_lsa_sheduled=1;		
	}
	else
	{
		if ( nlsr->debugging )
			printf("Build and Install Adj LSA already scheduled\n");
		if ( nlsr->detailed_logging )
			writeLogg(__FILE__,__FUNCTION__,__LINE__,"Build and Install Adj LSA already scheduled\n");
	}


	free(nbr);


}




void
process_incoming_timed_out_interest(struct ccn_closure* selfp, struct ccn_upcall_info* info)
{
	

	if ( nlsr->debugging )
		printf("process_incoming_timed_out_interest called \n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"process_incoming_timed_out_interest called \n");

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
}

void
process_incoming_timed_out_interest_info(struct ccn_closure* selfp, struct ccn_upcall_info* info)
{
	
	if ( nlsr->debugging )
		printf("process_incoming_timed_out_interest_info called \n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"process_incoming_timed_out_interest_info called \n");

	struct name_prefix *nbr=(struct name_prefix *)malloc(sizeof(struct name_prefix ));
	get_nbr(nbr,selfp,info);

	if ( nlsr->debugging )
		printf("Info Interest Timed Out for for Neighbor: %s Length:%d\n",nbr->name,nbr->length);
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"Info Interest Timed Out for for Neighbor: %s Length:%d\n",nbr->name,nbr->length);
	


	update_adjacent_timed_out_to_adl(nbr,1);
	print_adjacent_from_adl();	
	int timed_out=get_timed_out_number(nbr);

	if ( nlsr->debugging )
		printf("Neighbor: %s Info Interest Timed Out: %d times\n",nbr->name,timed_out);
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"Neighbor: %s Info Interest Timed Out: %d times\n",nbr->name,timed_out);


	if(timed_out<nlsr->interest_retry && timed_out>0) // use configured variables 
	{
		send_info_interest_to_neighbor(nbr);
	}
	else
	{		
		update_adjacent_status_to_adl(nbr,NBR_DOWN);
		if(!nlsr->is_build_adj_lsa_sheduled)
		{
			nlsr->event_build_adj_lsa = ccn_schedule_event(nlsr->sched, 1000, &build_and_install_adj_lsa, NULL, 0);
			nlsr->is_build_adj_lsa_sheduled=1;		
		}
	}

	free(nbr);


}


int
send_info_interest(struct ccn_schedule *sched, void *clienth, struct ccn_scheduled_event *ev, int flags)
{
	if(flags == CCN_SCHEDULE_CANCEL)
	{
 	 	return -1;
	}

         nlsr_lock();

	if ( nlsr->debugging )
		printf("send_info_interest called \n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"send_info_interest called \n");

	if ( nlsr->debugging )
		printf("\n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"\n");

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

	nlsr->event = ccn_schedule_event(nlsr->sched, 60000000, &send_info_interest, NULL, 0);

	return 0;
}

void 
send_info_interest_to_neighbor(struct name_prefix *nbr)
{

	if ( nlsr->debugging )
		printf("send_info_interest_to_neighbor called \n");
	if ( nlsr->detailed_logging )
		writeLogg(__FILE__,__FUNCTION__,__LINE__,"send_info_interest_to_neighbor called \n");


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
		/* Adding InterestLifeTime and InterestScope filter done */		
		ccn_charbuf_append(templ, "2", 1); //scope of interest: 2 (not further than next host)
		ccn_charbuf_append_closer(templ); /* </Scope> */

		appendLifetime(templ,nlsr->interest_resend_time);
		unsigned int face_id=get_next_hop_face_from_adl(nbr->name);
		ccnb_tagged_putf(templ, CCN_DTAG_FaceID, "%u", face_id);
		ccn_charbuf_append_closer(templ); /* </Interest> */
		
	
		if ( nlsr->debugging )
			printf("Sending info interest on name prefix : %s through Face:%u\n",int_name,face_id);
		if ( nlsr->detailed_logging )
			writeLogg(__FILE__,__FUNCTION__,__LINE__,"Sending info interest on name prefix : %s through Face:%u\n",int_name,face_id);

		res=ccn_express_interest(nlsr->ccn,name,&(nlsr->in_content),templ);

		if ( res >= 0 )
		{
			if ( nlsr->debugging )
				printf("Info interest sending Successfull .... \n");
			if ( nlsr->detailed_logging )
				writeLogg(__FILE__,__FUNCTION__,__LINE__,"Info interest sending Successfull .... \n");
		}	
		ccn_charbuf_destroy(&templ);
	}
	ccn_charbuf_destroy(&name);
	free(int_name);
}

int
contain_key_name(const unsigned char *ccnb, struct ccn_parsed_ContentObject *pco) 
{
	if (pco->offset[CCN_PCO_B_KeyLocator] == pco->offset[CCN_PCO_E_KeyLocator])
		return -1;

	struct ccn_buf_decoder decoder;
	struct ccn_buf_decoder *d;
	d = ccn_buf_decoder_start(&decoder, ccnb + pco->offset[CCN_PCO_B_Key_Certificate_KeyName], pco->offset[CCN_PCO_E_Key_Certificate_KeyName] - pco->offset[CCN_PCO_B_Key_Certificate_KeyName]);
	if (ccn_buf_match_dtag(d, CCN_DTAG_KeyName))
		return 1;

	return -1;
}

struct ccn_charbuf *
get_key_name(const unsigned char *ccnb, struct ccn_parsed_ContentObject *pco) 
{
	struct ccn_charbuf *key_name = ccn_charbuf_create();
	ccn_charbuf_append(key_name, ccnb + pco->offset[CCN_PCO_B_KeyName_Name], pco->offset[CCN_PCO_E_KeyName_Name] - pco->offset[CCN_PCO_B_KeyName_Name]);

	return key_name;
}
