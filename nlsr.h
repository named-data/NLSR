#ifndef _NLSR_H_
#define _NLSR_H_

#define LSA_ADJ_TYPE 1
#define LSA_NAME_TYPE 2

struct name_prefix
{
	char *name;
	int length;
};

struct ndn_neighbor
{
	struct name_prefix *neighbor;
	int face;
	int status;
	int last_lsdb_version;
	struct hashtb *lsa_update_queue;
};

struct nlsr
{

	struct ccn_closure in_interest;
	struct ccn_closure in_content;
	struct ccn_schedule *sched;
    	struct ccn_scheduled_event *event;

	struct hashtb *adl;
	struct hashtb *npl;

	struct ccn *ccn;
	char *router_name;

	int is_synch_init;
};

struct nlsr *nlsr;


void ndn_rtr_gettime(const struct ccn_gettime *self, struct ccn_timeval *result);
void process_command_router_name(char *command);
void process_command_ccnname(char *command);
void process_command_ccnneighbor(char *command);
void process_conf_command(char *command);
int readConfigFile(const char *filename);

void add_name_prefix_to_npl(struct name_prefix *name_prefix);
void print_name_prefix_from_npl(void);


void add_adjacent_to_adl(struct ndn_neighbor *nbr);
void print_adjacent_from_adl(void);


void nlsr_destroy( void );

#endif
