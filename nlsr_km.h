#ifndef _NLSR_KM_H_
#define _NLSR_KM_H_

enum key_type{
	ROOT_KEY, //0
	SITE_KEY, //1
	OPERATOR_KEY,//2
	ROUTING_KEY,//3
	NLSR_KEY,//4
	UNKNOWN_KEY//5
};


struct nlsr_key{
	char *key_name;
};


int
sign_content_with_user_defined_keystore(struct ccn_charbuf *content_name,
										struct ccn_charbuf *resultbuf,
										const void *data,
										size_t data_size,
										char *keystore_path,
										char *keystore_passphrase,
										char *key_repo_name,
										char *site_name,
										char *router_name);

int	contain_key_name(const unsigned char *ccnb, 
					struct ccn_parsed_ContentObject *pco);
struct ccn_charbuf * get_key_name(const unsigned char *ccnb, 
					struct ccn_parsed_ContentObject *pco);

int verify_key(const unsigned char *ccnb, 
				struct ccn_parsed_ContentObject *pco,
				int content_type);

void add_key(char *keyname);
int does_key_exist(char *keyname);

void destroy_keys(void);

#endif
