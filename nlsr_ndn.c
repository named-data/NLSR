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
#include <ccn/bloom.h>

#include "nlsr.h"
#include "nlsr_ndn.h"
#include "utility.h"

enum ccn_upcall_res 
incoming_interest(struct ccn_closure *selfp,
        enum ccn_upcall_kind kind, struct ccn_upcall_info *info)
{
    
    switch (kind) {
        case CCN_UPCALL_FINAL:
            break;
        case CCN_UPCALL_INTEREST:
		// printing the name prefix for which it received interest
            	printf("Interest Received for name: "); 
	    	struct ccn_charbuf*c;
		c=ccn_charbuf_create();
		ccn_uri_append(c,info->interest_ccnb,info->pi->offset[CCN_PI_E_Name]-info->pi->offset[CCN_PI_B_Name],0);
		//ccn_name_chop(c,NULL,-1);
		printf("%s\n",ccn_charbuf_as_string(c));
		ccn_charbuf_destroy(&c);

		process_incoming_interest(selfp, info);
		
		/* 
	    	struct ccn_charbuf *data=ccn_charbuf_create();
	    	struct ccn_charbuf *name=ccn_charbuf_create();
	    	struct ccn_signing_params sp=CCN_SIGNING_PARAMS_INIT;
	   
		 ccn_charbuf_append(name, info->interest_ccnb + info->pi->offset[CCN_PI_B_Name],
            info->pi->offset[CCN_PI_E_Name] - info->pi->offset[CCN_PI_B_Name]); 

		sp.template_ccnb=ccn_charbuf_create();
		ccn_charbuf_append_tt(sp.template_ccnb,CCN_DTAG_SignedInfo, CCN_DTAG);
		ccnb_tagged_putf(sp.template_ccnb, CCN_DTAG_FreshnessSeconds, "%ld", 1010);
                sp.sp_flags |= CCN_SP_TEMPL_FRESHNESS;
        	ccn_charbuf_append_closer(sp.template_ccnb);		   

		res= ccn_sign_content(ospfndn->ccn, data, name, &sp, "hello", strlen("hello")); 
	    	res=ccn_put(ospfndn->ccn,data->buf,data->length);
            	ccn_charbuf_destroy(&data);  

		*/
		break;

        default:
            break;
    }

    return CCN_UPCALL_RESULT_OK;
}


enum ccn_upcall_res incoming_content(struct ccn_closure* selfp,
        enum ccn_upcall_kind kind, struct ccn_upcall_info* info)
{


    switch(kind) {
        case CCN_UPCALL_FINAL:
            break;
        case CCN_UPCALL_CONTENT:
            	printf("Content Received for name: ");  
		struct ccn_charbuf*c;
		c=ccn_charbuf_create();
		ccn_uri_append(c,info->interest_ccnb,info->pi->offset[CCN_PI_E],0);
		printf("%s\n",ccn_charbuf_as_string(c));
		ccn_charbuf_destroy(&c);

		//process_incoming_content(selfp, info);
	    break;
        case CCN_UPCALL_INTEREST_TIMED_OUT:
          /*  printf("Interest timed out \n"); 

		const unsigned char *comp_ptr;
		size_t comp_size;
		int res;
		
		res=ccn_name_comp_get(info->interest_ccnb, info->interest_comps,2,&comp_ptr, &comp_size);

		printf("Parsed Interest: %s size: %d Size of name prefix: %d\n",comp_ptr,(int)comp_size,(int)info->interest_comps->n);
	   */
	    
		//process_timed_out_interest(selfp,info);
	    break;
        default:
            fprintf(stderr, "Unexpected response of kind %d\n", kind);
            return CCN_UPCALL_RESULT_ERR;
    }

    return CCN_UPCALL_RESULT_OK;
}

void 
process_incoming_interest(struct ccn_closure *selfp, struct ccn_upcall_info *info)
{
	printf("process_incoming_interest called \n");


	struct ccn_charbuf*c;
	c=ccn_charbuf_create();
	ccn_uri_append(c,info->interest_ccnb,info->pi->offset[CCN_PI_E_Name]-info->pi->offset[CCN_PI_B_Name],0);
	printf("%s\n",ccn_charbuf_as_string(c));
	ccn_charbuf_destroy(&c);

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

	if(!strcmp((char *)comp_ptr1,"lsdb"))
	{
		process_incoming_interest_lsdb(selfp,info);
	}


}


void 
process_incoming_interest_lsdb(struct ccn_closure *selfp, struct ccn_upcall_info *info)
{
	printf("process_incoming_interest_lsdb called \n");
	
	int l;
	const unsigned char *exclbase;
	size_t size;
	struct ccn_buf_decoder decoder;
	struct ccn_buf_decoder *d;
	const unsigned char *comp;	


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
		if (d->decoder.state < 0)
			printf("Parse Failed\n");
		if (comp != NULL)
			printf("No Number in Exclusion Filter\n");
			
		/* Now comp points to the start of your potential number, and size is its length */
	}

	
	
}

int
send_lsdb_interest(struct ccn_schedule *sched, void *clienth,
        struct ccn_scheduled_event *ev, int flags)
{

	struct ccn_charbuf *name;
	long int rnum;
	char rnumstr[20];
	char lsdb_str[5];
	char nlsr_str[5];

	int res,i;
	int adl_element;

	rnum=random();
	memset(&rnumstr,0,20);
	sprintf(rnumstr,"%ld",rnum);
	memset(&nlsr_str,0,5);
	sprintf(nlsr_str,"nlsr");
	memset(&lsdb_str,0,5);
	sprintf(lsdb_str,"lsdb");
	

	struct ndn_neighbor *nbr;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->adl, e);
	adl_element=hashtb_n(nlsr->adl);
	int mynumber=15;

	for(i=0;i<adl_element;i++)
	{
		nbr=e->data;
		printf("Sending interest for name prefix:%s/%s/%s/%s\n",nbr->neighbor->name,nlsr_str,lsdb_str,rnumstr);	
		name=ccn_charbuf_create();
		res=ccn_name_from_uri(name,nbr->neighbor->name);
		ccn_name_append_str(name,nlsr_str);
		ccn_name_append_str(name,lsdb_str);
		ccn_name_append_str(name,rnumstr);

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
		ccn_charbuf_putf(c, "%u", (unsigned)mynumber);
		ccnb_append_tagged_blob(templ, CCN_DTAG_Component, c->buf, c->length);
		ccn_charbuf_append_closer(templ); /* </Exclude> */

		ccn_charbuf_append_closer(templ); /* </Interest> */
	
		/* Adding Exclusion filter done */
				
		res=ccn_express_interest(nlsr->ccn,name,&(nlsr->in_content),templ);
			
		if ( res >= 0 )
			printf("Interest sending Successfull .... \n");	
		//ccn_charbuf_destroy(&c);
		ccn_charbuf_destroy(&templ);
		ccn_charbuf_destroy(&name);
	
		hashtb_next(e);		
	}

	hashtb_end(e);

	nlsr->event_send_lsdb_interest = ccn_schedule_event(nlsr->sched, 60000000, &send_lsdb_interest, NULL, 0);

	return 0;

}

