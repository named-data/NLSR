#ifndef _NLSR_LSDB_H_
#define _NLSR_LSDB_H_


#define LS_TYPE_ADJ 1
#define LS_TYPE_NAME 2

struct link
{
	struct ccn_charbuf *nbr;
	int face;
	int metric;
};


struct alsa_header 
{
	unsigned int ls_type;
	long int orig_time;
	struct ccn_charbuf *orig_router;
};

struct alsa
{
	struct alsa_header *header;	
	int no_link;
	struct link *links;
};

struct nlsa_header
{
	unsigned int ls_type;
	long int orig_time;
	long int ls_id;
	struct ccn_charbuf *orig_router;
	unsigned int isValid;

};

struct nlsa
{
	struct nlsa_header *header;
	struct ccn_charbuf *name_prefix;
};


int initial_build_name_lsa(struct ccn_schedule *sched, void *clienth, struct ccn_scheduled_event *ev, int flags);
struct nlsa * build_name_lsa(struct ccn_charbuf *name_prefix);
void install_name_lsa(struct nlsa *new_name_lsa);
void make_name_lsa_key(struct ccn_charbuf *key, struct ccn_charbuf *orig_router, unsigned int ls_type, long int nlsa_id, long int orig_time);
void print_name_lsdb(void);
void print_name_lsa(struct nlsa *name_lsa);

#endif
