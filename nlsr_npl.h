#ifndef _NLSR_NPL_H_
#define _NLSR_NPL_H_

struct name_prefix_list_entry
{
	struct name_prefix *np;
	long int name_lsa_id;
};

void add_name_to_npl(struct name_prefix *np);
void print_name_prefix_from_npl(void);
int does_name_exist_in_npl(struct name_prefix *np);
void update_nlsa_id_for_name_in_npl(struct name_prefix *np, long int nlsa_id);
long int get_lsa_id_from_npl(struct name_prefix *np);
void destroy_npl(void);
void destroy_npl_entry_component(struct name_prefix_list_entry *npe);
#endif
