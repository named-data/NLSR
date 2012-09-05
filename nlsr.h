#ifndef _NLSR_H_
#define _NLSR_H_

#define LSA_ADJ_TYPE 1
#define LSA_NAME_TYPE 2
#define LSDB_SYNCH_INTERVAL 300
#define INTEREST_RETRY 3
#define INTEREST_RESEND_TIME 15

#define NBR_DOWN 0
#define NBR_ACTIVE 1

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
	char * last_lsdb_version;
	int info_interest_timed_out;
	long int lsdb_synch_interval;
	long int last_lsdb_requested;
	int is_lsdb_send_interest_scheduled;
	int metric;
};

struct linkStateDatabase
{
	char  * version;
	struct hashtb *adj_lsdb;
	struct hashtb *name_lsdb;
};

struct nlsr
{

	struct ccn_closure in_interest;
	struct ccn_closure in_content;
	struct ccn_schedule *sched;
    	struct ccn_scheduled_event *event;
	struct ccn_scheduled_event *event_send_lsdb_interest;
	struct ccn_scheduled_event *event_send_info_interest;
	struct ccn_scheduled_event *event_build_name_lsa;
	struct ccn_scheduled_event *event_build_adj_lsa;

	struct hashtb *adl;
	struct hashtb *npl;

	struct ccn *ccn;
	struct name_prefix *router_name;

	struct linkStateDatabase *lsdb;

	int is_synch_init;
	long int nlsa_id;
	int adj_build_flag;
	long int adj_build_count;
	int is_build_adj_lsa_sheduled;
	int is_send_lsdb_interest_scheduled;

	long int lsdb_synch_interval;
	int interest_retry;
	long int interest_resend_time;

	int semaphor;
	
};

struct nlsr *nlsr;

void init_nlsr(void);

void ndn_rtr_gettime(const struct ccn_gettime *self, struct ccn_timeval *result);
void process_command_router_name(char *command);
void process_command_ccnname(char *command);
void process_command_ccnneighbor(char *command);
void process_command_lsdb_synch_interval(char *command);
void process_command_interest_retry(char *command);
void process_command_interest_resend_time(char *command);
void process_conf_command(char *command);
int readConfigFile(const char *filename);

void add_name_prefix_to_npl(struct name_prefix *np);
void print_name_prefix_from_npl(void);

void my_lock(void);
void my_unlock(void);



void nlsr_destroy( void );

#endif
