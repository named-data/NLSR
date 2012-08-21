#ifndef _NLSR_ADL_H_
#define _NLSR_ADL_H_

void add_adjacent_to_adl(struct ndn_neighbor *nbr);
void print_adjacent_from_adl(void);
void update_adjacent_status_to_adl(struct ccn_charbuf *nbr, int status);
void update_adjacent_lsdb_version_to_adl(struct ccn_charbuf *nbr, char *version);

#endif
