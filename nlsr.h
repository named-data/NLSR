#ifndef _NLSR_H_
#define _NLSR_H_


#define LSDB_SYNCH_INTERVAL 300
#define INTEREST_RETRY 3
#define INTEREST_RESEND_TIME 15
#define NLSR_LOCKED 1
#define NLSR_UNLOCKED 0

struct name_prefix
{
	char *name;
	int length;
};

struct linkStateDatabase
{
	struct hashtb *name_lsdb;
	struct hashtb *adj_lsdb;
	char *lsdb_version;
};

struct pneding_interest
{
	char *int_name;
	int timed_out;
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
	struct ccn_scheduled_event *event_calculate_route;

	struct hashtb *adl;
	struct hashtb *npl;
	struct hashtb *pit_alsa;
	struct hashtb *map;
	struct hashtb *rev_map;
	struct hashtb *npt;
	struct hashtb *routing_table;

	struct linkStateDatabase *lsdb;

	struct ccn *ccn;
	char *router_name;

	

	int is_synch_init;
	long int nlsa_id;
	int adj_build_flag;
	long int adj_build_count;
	int is_build_adj_lsa_sheduled;
	int is_send_lsdb_interest_scheduled;
	int is_route_calculation_scheduled;

	long int lsdb_synch_interval;
	int interest_retry;
	long int interest_resend_time;

	int semaphor;
	
};

struct nlsr *nlsr;

void process_command_ccnneighbor(char *command);
void process_command_ccnname(char *command);
void process_command_lsdb_synch_interval(char *command);
void process_command_interest_retry(char *command);
void process_command_interest_resend_time(char *command);
void process_conf_command(char *command);
int readConfigFile(const char *filename);

void nlsr_lock(void);
void nlsr_unlock(void);

int init_nlsr(void);
void nlsr_destroy( void );
void nlsr_stop_signal_handler(int sig);

#endif
