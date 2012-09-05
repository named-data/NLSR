#ifndef _NLSR_ADL_H_
#define _NLSR_ADL_H_

#define NBR_ACTIVE 1
#define NBR_DOWN 0

void add_adjacent_to_adl(struct name_prefix *np, int face);
void print_adjacent(struct ndn_neighbor *nbr);
void print_adjacent_from_adl(void);
void update_adjacent_status_to_adl(struct name_prefix *nbr, int status);
int get_adjacent_status(struct name_prefix *nbr);
void update_adjacent_lsdb_version_to_adl(struct name_prefix *nbr, char * version);
void update_lsdb_synch_interval_to_adl(struct name_prefix *nbr, long int interval);

int get_timed_out_number(struct name_prefix *nbr);
void update_adjacent_timed_out_to_adl(struct name_prefix *nbr, int increment);
void update_adjacent_timed_out_zero_to_adl(struct name_prefix *nbr);

int is_adj_lsa_build(void);
int no_active_nbr(void);
long int len_active_nbr_data(void);
void get_active_nbr_adj_data(struct ccn_charbuf *c);
char * get_nbr_lsdb_version(char *nbr);
void update_adjacent_last_lsdb_requested_to_adl(char *nbr, long int timestamp);
long int get_nbr_last_lsdb_requested(char *nbr);
long int get_nbr_time_diff_lsdb_req(char *nbr);
long int get_lsdb_synch_interval(char *nbr);
void set_is_lsdb_send_interest_scheduled_to_zero(char *nbr);

void adjust_adjacent_last_lsdb_requested_to_adl(char *nbr, long int sec);
//long int get_lsdb_synch_interval(char *nbr);


#endif
