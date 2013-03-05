#ifndef _NLSR_KM_UTIL_H_
#define _NLSR_KM_UTIL_H_

int
contain_key_name(const unsigned char *ccnb, 
				struct ccn_parsed_ContentObject *pco);

struct ccn_charbuf *
get_key_name(const unsigned char *ccnb, 
				struct ccn_parsed_ContentObject *pco);

int
check_for_name_component_in_name(const struct ccn_charbuf *name, 
								const struct ccn_indexbuf *indx,
								const char *component);
int
check_for_tag_component_in_name(const struct ccn_charbuf *name, 
								const struct ccn_indexbuf *indx,
								const char *component);
enum key_type
get_key_type_from_key_name(struct ccn_charbuf *keyname); 

int
get_orig_router_from_key_name(struct ccn_charbuf *orig_router,
									 struct ccn_charbuf *name);


int
appendLifetime(struct ccn_charbuf *cb, int lifetime);

#endif
