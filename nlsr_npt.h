#ifndef _NLSR_NPT_H_
#define _NLSR_NPT_H_

#define NO_FACE -12345

struct npt_entry
{
	char *name_prefix;
	struct hashtb *next_hop_table;
};

struct next_hop_entry
{
	char *orig_router;
	int next_hop_face;
};


int add_npt_entry(char *orig_router, char *name_prefix, int face);
void print_npt(void);

#endif
