#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <unistd.h>
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
#include "nlsr_km_util.h"
#include "nlsr_km.h"

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
	res |= ccnb_append_tagged_blob(cb, CCN_DTAG_InterestLifetime, buf+pos, 
															sizeof(buf)-pos);
	return res;
}

int
contain_key_name(const unsigned char *ccnb, struct ccn_parsed_ContentObject *pco) 
{
	if (pco->offset[CCN_PCO_B_KeyLocator] == pco->offset[CCN_PCO_E_KeyLocator])
		return -1;

	struct ccn_buf_decoder decoder;
	struct ccn_buf_decoder *d;
	d = ccn_buf_decoder_start(&decoder, ccnb + 
		pco->offset[CCN_PCO_B_Key_Certificate_KeyName], 
		pco->offset[CCN_PCO_E_Key_Certificate_KeyName] - 
		pco->offset[CCN_PCO_B_Key_Certificate_KeyName]);
	if (ccn_buf_match_dtag(d, CCN_DTAG_KeyName))
		return 1;

	return -1;
}

struct ccn_charbuf *
get_key_name(const unsigned char *ccnb, struct ccn_parsed_ContentObject *pco) 
{
	struct ccn_charbuf *key_name = ccn_charbuf_create();
	ccn_charbuf_append(key_name, ccnb + pco->offset[CCN_PCO_B_KeyName_Name], 
	pco->offset[CCN_PCO_E_KeyName_Name] - pco->offset[CCN_PCO_B_KeyName_Name]);

	return key_name;
}


int 
get_orig_router_from_key_name(struct ccn_charbuf *orig_router ,struct ccn_charbuf *name) 
{
	int res;	
	struct ccn_indexbuf *name_comps;
	
	name_comps = ccn_indexbuf_create();
	res = ccn_name_split(name, name_comps);
	if ( res < 0 ){
		ccn_indexbuf_destroy(&name_comps);
		return res;
	}
	else{
		res=ccn_name_chop(name, name_comps, -3);
		if ( res < 0 ){
			ccn_indexbuf_destroy(&name_comps);
			return res;
		}
		else{
			res=check_for_tag_component_in_name(name,name_comps,"R.N.Start");
			if ( res > 0 ){
				ccn_name_init(orig_router);	
				ccn_name_append_components(orig_router,name->buf,
										name_comps->buf[res+1], 
										name_comps->buf[name_comps->n - 1]);
			}
			else{
				ccn_indexbuf_destroy(&name_comps);
				return -1;
			}
		}
	}	

	ccn_indexbuf_destroy(&name_comps);
	return 0;
}

int
check_for_name_component_in_name(const struct ccn_charbuf *name, 
								const struct ccn_indexbuf *indx,
								const char *component){
	
	int res,i;
	int result_position=0;
	int name_comps=(int)indx->n;

	for(i=0;i<name_comps;i++){
		res=ccn_name_comp_strcmp(name->buf,indx,i,component);
		if( res == 0){
			
				result_position=i;
				break;
		}	
	}

	return result_position;
}


int
check_for_tag_component_in_name(const struct ccn_charbuf *name, 
								const struct ccn_indexbuf *indx,
								const char *component){
	
	int res,i;
	int result_position=0;
	int name_comps=(int)indx->n;

	for(i=0;i<name_comps;i++){
		const unsigned char *comp_ptr;
		size_t comp_size;
		res=ccn_name_comp_get(name->buf, indx,i,&comp_ptr, &comp_size);
		if( res == 0){
			if ( strstr((char *)comp_ptr,component) != NULL ){
				result_position=i;
				break;
			}
		}	
	}

	return result_position;
}

enum key_type
get_key_type_from_key_name(struct ccn_charbuf *keyname)
{
	printf("get_key_type_from_key_name called\n");

	int res;
	int return_key=UNKNOWN_KEY;	

	struct ccn_indexbuf *indx=ccn_indexbuf_create();
	if ( indx == NULL ){
		printf("Error in creating index for key name \n");
		return UNKNOWN_KEY;
	}

	res=ccn_name_split(keyname,indx);
	if ( res < 0 ){
		printf("Error in parsing key name \n");
		ccn_indexbuf_destroy(&indx);
		return UNKNOWN_KEY;
	}
	else if ( res == 3){
		int chk_ndn=check_for_name_component_in_name(keyname,indx,"ndn");
		int chk_key=check_for_name_component_in_name(keyname,indx,"keys");
		if ( chk_ndn == 0 && chk_key == 1)
			return_key=ROOT_KEY;
	}
	else{
		int check_op,check_rt;
		check_op=check_for_tag_component_in_name(keyname,indx,
													 "O.N.Start");
		check_rt=check_for_tag_component_in_name(keyname,indx,
													 "R.N.Start");
		if ( check_op > 0){
			return_key=OPERATOR_KEY;
		}
		else if(check_rt >0){
			int check_nlsr;
			check_nlsr=check_for_name_component_in_name(keyname,indx,
													 "nlsr");
			if ( check_rt > 0 ){
				if ( check_nlsr > 0){
					return_key=NLSR_KEY;
				}
				else{
					return_key=ROUTING_KEY;
				}
			}
		}
		else if ( check_rt == 0 && check_op == 0 && res > 3){
			return_key=SITE_KEY;
		}
	}

	ccn_indexbuf_destroy(&indx);
	return return_key;
}
