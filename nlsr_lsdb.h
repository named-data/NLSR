#ifndef _NLSR_LSDB_H_
#define _NLSR_LSDB_H_


#define LS_TYPE_ADJ 1
#define LS_TYPE_NAME 2

struct link
{
	struct name_prefix *nbr;
	int face;
	int metric;
};


struct alsa_header 
{
	unsigned int ls_type;
	char *orig_time;
	struct name_prefix *orig_router;
	
};

struct alsa
{
	struct alsa_header *header;	
	int no_link;
	char *body;
};

struct nlsa_header
{
	unsigned int ls_type;
	char *orig_time;
	long int ls_id;
	struct name_prefix *orig_router;
	unsigned int isValid;

};

struct nlsa
{
	struct nlsa_header *header;
	struct name_prefix *name_prefix;
};

void set_new_lsdb_version(void);

void build_and_install_name_lsas(void);
void build_and_install_others_name_lsa(char *orig_router,int ls_type,long int ls_id,int isValid,char *np);
void build_name_lsa(struct nlsa *name_lsa, struct name_prefix *np);
void build_others_name_lsa(struct nlsa *name_lsa, char *orig_router,int ls_type,long int ls_id,int isValid,char *np);
void print_name_lsa(struct nlsa *name_lsa);
void install_name_lsa(struct nlsa *name_lsa);
char * make_name_lsa_key(char *orig_router, int ls_type, long int ls_id);
void print_name_lsdb(void);

int build_and_install_adj_lsa(struct ccn_schedule *sched, void *clienth, struct ccn_scheduled_event *ev, int flags);
void build_and_install_others_adj_lsa(char *orig_router,int ls_type,char *orig_time, int no_link,char *data);
void build_adj_lsa(struct alsa * adj_lsa);
void build_others_adj_lsa(struct alsa *adj_lsa,char *orig_router,int ls_type,char *orig_time,int no_link,char *data);
void install_adj_lsa(struct alsa * adj_lsa);
void make_adj_lsa_key(char *key,struct alsa *adj_lsa);
void print_adj_lsa_body(const char *body, int no_link);
void print_adj_lsa(struct alsa * adj_lsa);
void print_adj_lsdb(void);

char * get_name_lsdb_summary(void);
char * get_adj_lsdb_summary(void);
void get_name_lsa_data(struct ccn_charbuf *lsa_data,struct name_prefix *lsaId);
void get_adj_lsa_data(struct ccn_charbuf *lsa_data,struct name_prefix *lsaId);

long int get_name_lsdb_num_element(void);
long int get_adj_lsdb_num_element(void);

#endif
