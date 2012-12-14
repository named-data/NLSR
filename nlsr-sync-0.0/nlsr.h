#ifndef _NLSR_H_
#define _NLSR_H_


#define LSDB_SYNCH_INTERVAL 300
#define INTEREST_RETRY 3
#define INTEREST_RESEND_TIME 15
#define NLSR_LOCKED 1
#define NLSR_UNLOCKED 0
#define LSA_REFRESH_TIME 1800
#define ROUTER_DEAD_INTERVAL 3600
#define MULTI_PATH_FACE_NUM 0

#define LINK_METRIC 10

#define NAME_LSA_VALID 1
#define NAME_LSA_INVALID 0

#define API_PORT 9696


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

struct pending_interest
{
	char *int_name;
	int timed_out;
};

struct nlsr
{

	struct ccn_closure in_interest;
	struct ccn_closure in_content;

	struct ccns_name_closure *closure;

	struct ccns_slice *slice;
    	struct ccns_handle *ccns;

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
	long int lsa_refresh_time;
	long int router_dead_interval;
	long int multi_path_face_num;
	char *logDir;
	int detailed_logging;
	int debugging;

	int semaphor;

	int nlsr_api_server_sock_fd;
	fd_set readfds;
	int api_port;

	char *topo_prefix;
	char *slice_prefix;

	int is_hyperbolic_calc;
	
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
