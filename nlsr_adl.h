#ifndef _NLSR_ADL_H_
#define _NLSR_ADL_H_

#define NBR_DOWN 0
#define NBR_ACTIVE 1

struct ndn_neighbor
{
	struct name_prefix *neighbor;
	int face;
	int status;
	char * last_lsdb_version;
	char * last_info_version;
	char *ip_address;
	int info_interest_timed_out;
	int lsdb_interest_timed_out;
	int lsdb_random_time_component;
	long int lsdb_synch_interval;
	long int last_lsdb_requested;
	int is_lsdb_send_interest_scheduled;
	int metric;
};

void add_nbr_to_adl(struct name_prefix *new_nbr,int face,char *ip);
void delete_nbr_from_adl(struct name_prefix *nbr);
void print_adjacent(struct ndn_neighbor *nbr);
void print_adjacent_from_adl(void);
int get_adjacent_status(struct name_prefix *nbr);
int get_timed_out_number(struct name_prefix *nbr);
int get_lsdb_interest_timed_out_number(struct name_prefix *nbr);
void update_adjacent_timed_out_to_adl(struct name_prefix *nbr, int increment);
void update_adjacent_timed_out_zero_to_adl(struct name_prefix *nbr);
void update_lsdb_interest_timed_out_to_adl(struct name_prefix *nbr, int increment);
void update_lsdb_interest_timed_out_zero_to_adl(struct name_prefix *nbr);
void update_adjacent_status_to_adl(struct name_prefix *nbr, int status);
void update_lsdb_synch_interval_to_adl(struct name_prefix *nbr, long int interval);
int no_active_nbr(void);
int is_adj_lsa_build(void);
void get_active_nbr_adj_data(struct ccn_charbuf *c);
long int get_nbr_time_diff_lsdb_req(char *nbr);
long int get_nbr_last_lsdb_requested(char *nbr);
long int get_nbr_last_lsdb_requested(char *nbr);
long int get_lsdb_synch_interval(char *nbr);
int get_nbr_random_time_component(char *nbr);
char * get_nbr_lsdb_version(char *nbr);
void update_adjacent_last_lsdb_requested_to_adl(char *nbr, long int timestamp);
void set_is_lsdb_send_interest_scheduled_to_zero(char *nbr);
void update_adjacent_lsdb_version_to_adl(struct name_prefix *nbr, char *version);
void adjust_adjacent_last_lsdb_requested_to_adl(char *nbr, long int sec);
int get_next_hop_face_from_adl(char *nbr);
int is_neighbor(char *nbr);
int is_active_neighbor(char *nbr);
void update_face_to_adl_for_nbr(char *nbr, int face);

void get_host_name_from_command_string(struct name_prefix *name_part,char *nbr_name_uri, int offset);
#endif
