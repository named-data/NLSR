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
#include "nlsr_km.h"
#include "nlsr_km_util.h"

int
sign_content_with_user_defined_keystore(struct ccn_charbuf *content_name,
										struct ccn_charbuf *resultbuf,
										const void *data,
										size_t data_size,
										char *keystore_path,
										char *keystore_passphrase,
										char *key_repo_name,
										char *site_name,
										char *router_name){
	
	if ( nlsr->debugging )
		printf("sign_content_with_user_defined_keystore called\n");

	
	int res;


	struct ccn_charbuf * pubid_out=ccn_charbuf_create();
	struct ccn_charbuf * keyname;

	
	struct ccn_keystore *keystore = NULL;
	keystore=ccn_keystore_create();
	res=ccn_keystore_init(keystore, keystore_path,keystore_passphrase );
	if ( res < 0 ){
		if ( nlsr->debugging )
			printf("Error in initiating keystore :(\n");
		ccn_keystore_destroy(&keystore);
		return -1;
	}
	

	res=ccn_load_private_key	(nlsr->ccn,
							keystore_path,
							keystore_passphrase,
							pubid_out);

	if(res < 0 ){
		if ( nlsr->debugging )
			printf("Error in loading keystore :( \n");
		ccn_charbuf_destroy(&pubid_out);
		return -1;
	}

	char *baseuri=(char *)calloc(strlen(key_repo_name)+strlen(site_name)+
				  strlen(router_name)+strlen("/%C1.R.N.Start")+5,sizeof(char));
	memcpy(baseuri,key_repo_name,strlen(key_repo_name)+1);
	if ( site_name[0] != '/')
		memcpy(baseuri+strlen(baseuri),"/",1);
	memcpy(baseuri+strlen(baseuri),site_name,strlen(site_name)+1);
	memcpy(baseuri+strlen(baseuri),"/%C1.R.N.Start",strlen("/%C1.R.N.Start"));
	memcpy(baseuri+strlen(baseuri),router_name,strlen(router_name)+1);
	baseuri[strlen(baseuri)]='\0';
	

	keyname=ccn_charbuf_create();
	if(keyname == NULL ){
		ccn_charbuf_destroy(&pubid_out);
		free(baseuri);
		return -1;
	}
	ccn_name_from_uri(keyname,baseuri);
	if ( res < 0 ){
		if ( nlsr->debugging )
			printf("Bad URI format: %s\n",baseuri);
		ccn_charbuf_destroy(&pubid_out);
		ccn_charbuf_destroy(&keyname);
		free(baseuri);
		return -1;		
	}
	
	ccn_name_append_str(keyname,"routing");
	ccn_name_append_str(keyname,"nlsr");
	struct ccn_charbuf *keyid = ccn_charbuf_create();
	ccn_charbuf_append_value(keyid, CCN_MARKER_CONTROL, 1);
	ccn_charbuf_append_string(keyid, ".M.K");
	ccn_charbuf_append_value(keyid, 0, 1);
	ccn_charbuf_append_charbuf(keyid, pubid_out);
	ccn_name_append(keyname, keyid->buf, keyid->length);
	
	

	struct ccn_charbuf *uri = ccn_charbuf_create();
	ccn_uri_append(uri, keyname->buf, keyname->length, 0);
	if ( nlsr->debugging )
		printf("Key Name Included when processing content: %s\n", ccn_charbuf_as_string(uri));
	ccn_charbuf_destroy(&uri);	

	struct ccn_signing_params sp = CCN_SIGNING_PARAMS_INIT;
	sp.type = CCN_CONTENT_DATA;
 	sp.template_ccnb = ccn_charbuf_create();
  	ccn_charbuf_append_tt(sp.template_ccnb, CCN_DTAG_SignedInfo, CCN_DTAG);
	ccn_charbuf_append_tt(sp.template_ccnb, CCN_DTAG_KeyLocator, CCN_DTAG);
  	ccn_charbuf_append_tt(sp.template_ccnb, CCN_DTAG_KeyName, CCN_DTAG);
  	ccn_charbuf_append(sp.template_ccnb, keyname->buf, keyname->length); 
	ccn_charbuf_append_closer(sp.template_ccnb); // KeyName closer
  	ccn_charbuf_append_closer(sp.template_ccnb); // KeyLocator closer
  	ccn_charbuf_append_closer(sp.template_ccnb); // SignedInfo closer
	
	sp.sp_flags |= CCN_SP_TEMPL_KEY_LOCATOR;
	sp.sp_flags |= CCN_SP_FINAL_BLOCK;
	sp.freshness = 60;


	if (pubid_out->length != sizeof(sp.pubid)){
		if ( nlsr->debugging )
			printf("Size of pubid and sp.pubid is not equal");
		ccn_charbuf_destroy(&keyname);
		ccn_charbuf_destroy(&pubid_out);
		free(baseuri);
		return -1;
	}
	
	memcpy(sp.pubid, pubid_out->buf, pubid_out->length);
	


	res=ccn_sign_content(nlsr->ccn,resultbuf,content_name,&sp,data,data_size);
	if( res < 0 ){
		if ( nlsr->debugging )
			printf("Content signing error \n");
		ccn_charbuf_destroy(&sp.template_ccnb);
		ccn_charbuf_destroy(&keyid);
		ccn_charbuf_destroy(&keyname);
		ccn_charbuf_destroy(&pubid_out);
		free(baseuri);
 		return -1;
	}

	ccn_charbuf_destroy(&sp.template_ccnb);
	ccn_charbuf_destroy(&keyid);
	ccn_charbuf_destroy(&keyname);
	ccn_charbuf_destroy(&pubid_out);
	free(baseuri);
 	return 0;
}


/*
int
process_incoming_content(struct ccn_closure* selfp, 
												struct ccn_upcall_info* info){

	printf("process_incoming_content called\n");

	int res=verify_key(info->content_ccnb,info->pco->offset[CCN_PCO_E],info->pco);

	if ( res != 0 ){
		printf("Error in verfiying keys !! :( \n");
	}
	else{
		printf("Key verification is successful :)\n");
	}
	return 0;
}
*/

int 
verify_key(const unsigned char *ccnb, 
										struct ccn_parsed_ContentObject *pco){
	if ( nlsr->debugging )
		printf("verify key called\n");
	int ret=-1;

	if ( contain_key_name(ccnb, pco) == 1){
		
		struct ccn_charbuf *key_name=get_key_name(ccnb, pco);
		struct ccn_charbuf *key_uri = ccn_charbuf_create();
		ccn_uri_append(key_uri, key_name->buf, key_name->length, 0);
		if ( nlsr->debugging )
			printf("Key Name from Incoming Content: %s\n",ccn_charbuf_as_string(key_uri));
		int res=get_key_type_from_key_name(key_name);
		if ( nlsr->debugging )		
			printf("Key Type: %d \n",res);

		struct ccn_charbuf *result = ccn_charbuf_create();
		struct ccn_parsed_ContentObject temp_pco = {0};
		int get_flags = 0;
		get_flags |= CCN_GET_NOKEYWAIT;
		int counter = 0;
		while(ccn_get(nlsr->ccn, key_name, NULL, 500, result, &temp_pco, NULL, 
										get_flags) < 0 && counter < 3) counter++;

		int chk_verify=ccn_verify_content(nlsr->ccn,ccnb,pco);		

		if ( chk_verify == 0 ){
			if ( nlsr->debugging )
				printf("Verification Successful :)\n");

			if ( counter == 3){
				if ( nlsr->debugging )
					printf("Could not retrieve key by name !!!\n");
			}
			else{
				if ( res == ROOT_KEY ){
					ret=0;
				}
				else{
					ret=verify_key(result->buf,&temp_pco);
				}
			}
		}
		ccn_charbuf_destroy(&result);
		ccn_charbuf_destroy(&key_uri);
		ccn_charbuf_destroy(&key_name);
		return ret;
	}

	return ret;
}

