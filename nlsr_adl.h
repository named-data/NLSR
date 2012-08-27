#ifndef _NLSR_ADL_H_
#define _NLSR_ADL_H_

void add_adjacent_to_adl(struct ndn_neighbor *nbr);
void print_adjacent_from_adl(void);
void update_adjacent_status_to_adl(struct ccn_charbuf *nbr, int status);
void update_adjacent_lsdb_version_to_adl(struct ccn_charbuf *nbr, long int version);

int get_timed_out_number(struct ccn_charbuf *nbr);
void update_adjacent_timed_out_to_adl(struct ccn_charbuf *nbr, int increment);
void update_adjacent_timed_out_zero_to_adl(struct ccn_charbuf *nbr);

#endif
