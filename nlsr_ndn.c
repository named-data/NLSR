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

		//process_incoming_interest(selfp, info);
		
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
