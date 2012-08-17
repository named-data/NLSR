#ifndef _NLSR_LSDB_H_
#define _NLSR_LSDB_H_

struct link
{
	struct charbuf *nbr;
	int face;
	int metric;
};

struct adj_lsa
{

	unsigned char ls_type;
	char orig_time[15];
	struct charbuf *orig_router;
	int no_link;

	struct link *links;
};

struct name_lsa
{
	unsigned char ls_type;
	char orig_time[15];
	long ls_id;
	struct charbuf *orig_router;
	unsigned char isValid;

	struct charbuf *name_prefix;
};

#endif
