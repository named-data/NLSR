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


char * 
get_orig_router_from_lsa_name(struct ccn_charbuf * content_name)
{
	int start=0;

	size_t comp_size;
	const unsigned char *second_last_comp;
	char *second_comp_type;
	char *sep=".";
	char *rem;

	struct ccn_indexbuf *components=ccn_indexbuf_create();
	struct ccn_charbuf *name=ccn_charbuf_create();
	ccn_name_from_uri(name,nlsr->slice_prefix);
	ccn_name_split (name, components);
	start=components->n-2;
	ccn_charbuf_destroy(&name);
	ccn_indexbuf_destroy(&components);

	struct ccn_indexbuf *comps=ccn_indexbuf_create();
	ccn_name_split (content_name, comps);
	ccn_name_comp_get( content_name->buf, comps, 
					  comps->n-1-2, &second_last_comp, &comp_size);

	second_comp_type=strtok_r((char *)second_last_comp, sep, &rem);
	if ( strcmp( second_comp_type, "lsId" ) == 0 ){
		ccn_name_chop(content_name,comps,-3);
	}
	else{
		ccn_name_chop(content_name,comps,-2);
	}
	

	struct ccn_charbuf *temp=ccn_charbuf_create();
	ccn_name_init(temp);	
	ccn_name_append_components( temp,	content_name->buf,
								comps->buf[start+1], 
								comps->buf[comps->n - 1]);

	struct ccn_charbuf *temp1=ccn_charbuf_create();
	ccn_uri_append(temp1, temp->buf, temp->length, 0);

	char *orig_router=(char *)calloc(strlen(ccn_charbuf_as_string(temp1))+1,
																sizeof(char));
	memcpy(orig_router,ccn_charbuf_as_string(temp1),
										strlen(ccn_charbuf_as_string(temp1)));
	orig_router[strlen(orig_router)]='\0';
	
	ccn_charbuf_destroy(&temp);
	ccn_charbuf_destroy(&temp1);
	ccn_indexbuf_destroy(&comps);
	return orig_router;
	

}


char * 
get_orig_router_from_info_content_name(struct ccn_charbuf * content_name)
{
	int start,end;

	start=0;

	struct ccn_indexbuf *comps=ccn_indexbuf_create();
	ccn_name_split (content_name, comps);

	end=check_for_name_component_in_name(content_name,comps,"nlsr");


	struct ccn_charbuf *temp=ccn_charbuf_create();
	ccn_name_init(temp);	
	ccn_name_append_components( temp,	content_name->buf,
								comps->buf[start], 
								comps->buf[end]);

	struct ccn_charbuf *temp1=ccn_charbuf_create();
	ccn_uri_append(temp1, temp->buf, temp->length, 0);

	char *orig_router=(char *)calloc(strlen(ccn_charbuf_as_string(temp1))+1,
																sizeof(char));
	memcpy(orig_router,ccn_charbuf_as_string(temp1),
										strlen(ccn_charbuf_as_string(temp1)));
	orig_router[strlen(orig_router)]='\0';
	
	ccn_charbuf_destroy(&temp);
	ccn_charbuf_destroy(&temp1);
	ccn_indexbuf_destroy(&comps);
	return orig_router;
	

}


int 
check_key_name_hierarchy(const unsigned char *ccnb, 
										struct ccn_parsed_ContentObject *pco,
										int key_type, int content_type){
	printf("check_key_name_hierarchy called\n");	
	if (key_type == UNKNOWN_KEY ){
		return 1;
	}
	int res;
	struct ccn_charbuf *key_name=get_key_name(ccnb, pco);

	struct ccn_charbuf *key_uri = ccn_charbuf_create();
	ccn_uri_append(key_uri, key_name->buf, key_name->length, 0);
	printf("Key Name: %s\n",ccn_charbuf_as_string(key_uri));
	ccn_charbuf_destroy(&key_uri);

	struct ccn_charbuf *content_name=ccn_charbuf_create();
	res=ccn_charbuf_append(content_name, ccnb + pco->offset[CCN_PCO_B_Name],
			pco->offset[CCN_PCO_E_Name] - pco->offset[CCN_PCO_B_Name]);

	struct ccn_charbuf *content_uri = ccn_charbuf_create();
	ccn_uri_append(content_uri, content_name->buf, content_name->length, 0);
	printf("Content Name: %s\n",ccn_charbuf_as_string(content_uri));
	ccn_charbuf_destroy(&content_uri);
	
	if ( key_type == NLSR_KEY){
		char *orig_router_key_name=get_orig_router_from_key_name(key_name,0,0);
		char *orig_router_content_name;
		if ( content_type == 1 ){
			orig_router_content_name=get_orig_router_from_lsa_name(content_name);
		}
		else if ( content_type == 0 ){
			orig_router_content_name=get_orig_router_from_info_content_name(content_name);
		}
		printf("Orig Router (Key Name):%s\n",orig_router_key_name);
		printf("Orig Router (Content Name):%s\n",orig_router_content_name);

		if (strcmp(orig_router_key_name,orig_router_content_name) == 0 ){
			free(orig_router_key_name);
			free(orig_router_content_name);
			ccn_charbuf_destroy(&key_name);
			ccn_charbuf_destroy(&content_name);
			return 1;
		}
	}

	if ( key_type == ROUTING_KEY){
		char *orig_router_key_name=get_orig_router_from_key_name(key_name,1,0);
		char *orig_router_content_name=get_orig_router_from_key_name(content_name,1,1);
		printf("Orig Router (Key Name):%s\n",orig_router_key_name);
		printf("Orig Router (Content Name):%s\n",orig_router_content_name);
		
		if (strcmp(orig_router_key_name,orig_router_content_name) == 0 ){
			free(orig_router_key_name);
			free(orig_router_content_name);
			ccn_charbuf_destroy(&key_name);
			ccn_charbuf_destroy(&content_name);
			return 1;
		}
	}
	if ( key_type == OPERATOR_KEY){
		struct ccn_indexbuf *key_name_comps;
		key_name_comps = ccn_indexbuf_create();
		res = ccn_name_split(key_name, key_name_comps);
		int last_indx=check_for_tag_component_in_name(key_name,key_name_comps,"O.N.Start");
		char *site_key_prefix_key=get_name_segments_from_name(key_name,0,last_indx);
		printf("Site key prefix(key Name):%s\n",site_key_prefix_key);
		ccn_indexbuf_destroy(&key_name_comps);

		struct ccn_indexbuf *content_name_comps;
		content_name_comps = ccn_indexbuf_create();
		res = ccn_name_split(content_name, content_name_comps);
		int last_indx_rtr=check_for_tag_component_in_name(content_name,content_name_comps,"R.N.Start");
		char *site_key_prefix_content=get_name_segments_from_name(key_name,0,last_indx_rtr);
		printf("Site key prefix(Content Name):%s\n",site_key_prefix_content);
		ccn_indexbuf_destroy(&content_name_comps);

		if( strcmp(site_key_prefix_key,site_key_prefix_content) == 0 ){
			free(site_key_prefix_key);
			free(site_key_prefix_content);
			ccn_charbuf_destroy(&key_name);
			ccn_charbuf_destroy(&content_name);
			return 1;
		}

	}

	if ( key_type == SITE_KEY){
		struct ccn_indexbuf *key_name_comps;
		key_name_comps = ccn_indexbuf_create();
		res = ccn_name_split(key_name, key_name_comps);
		int last_indx=check_for_tag_component_in_name(key_name,key_name_comps,"M.K");
		char *site_key_prefix_key=get_name_segments_from_name(key_name,0,last_indx);
		printf("Site key prefix(key Name):%s\n",site_key_prefix_key);
		ccn_indexbuf_destroy(&key_name_comps);

		struct ccn_indexbuf *content_name_comps;
		content_name_comps = ccn_indexbuf_create();
		res = ccn_name_split(content_name, content_name_comps);
		int last_indx_rtr=check_for_tag_component_in_name(content_name,content_name_comps,"O.N.Start");
		char *site_key_prefix_content=get_name_segments_from_name(key_name,0,last_indx_rtr);
		printf("Site key prefix(Content Name):%s\n",site_key_prefix_content);
		ccn_indexbuf_destroy(&content_name_comps);

		if( strcmp(site_key_prefix_key,site_key_prefix_content) == 0 ){
			free(site_key_prefix_key);
			free(site_key_prefix_content);
			ccn_charbuf_destroy(&key_name);
			ccn_charbuf_destroy(&content_name);
			return 1;
		}

	}
	
	if ( key_type == ROOT_KEY){
		ccn_charbuf_destroy(&key_name);
		ccn_charbuf_destroy(&content_name);
		return 1;
	}

	ccn_charbuf_destroy(&key_name);
	ccn_charbuf_destroy(&content_name);
	return 0;
}

int 
verify_key(const unsigned char *ccnb, 
		struct ccn_parsed_ContentObject *pco,
		int content_type){
	if ( nlsr->debugging )
		printf("verify key called\n");
	int ret=-1;
	//int res;
	
	if ( contain_key_name(ccnb, pco) == 1){
		
		struct ccn_charbuf *key_name=get_key_name(ccnb, pco);
		struct ccn_charbuf *key_uri = ccn_charbuf_create();
		ccn_uri_append(key_uri, key_name->buf, key_name->length, 0);
		if ( nlsr->debugging )
			printf("Key Name from Incoming Content: %s\n",ccn_charbuf_as_string(key_uri));
		int key_type=get_key_type_from_key_name(key_name);
		if ( nlsr->debugging )		
			printf("Key Type: %d \n",key_type);

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
				printf("Content verification Successful :)\n");

			if ( counter == 3){
				if ( nlsr->debugging )
					printf("Could not retrieve key by name !!!\n");
			}
			else{
				if ( key_type == ROOT_KEY ){
					ret=0;
				}
				else{
					if ( nlsr->isStrictHierchicalKeyCheck ){
						int key_name_test=check_key_name_hierarchy(ccnb,
																	pco,
																	key_type,
																	content_type);
						if ( key_name_test == 1){
							ret=verify_key(result->buf,&temp_pco,content_type);
						}
					}
					else{ 
						ret=verify_key(result->buf,&temp_pco,content_type);
					}
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

