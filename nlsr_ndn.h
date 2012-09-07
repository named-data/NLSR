#ifndef _NLSR_NDN_H_
#define _NLSR_NDN_H_

int appendLifetime(struct ccn_charbuf *cb, int lifetime);
void get_nbr(struct name_prefix *nbr,struct ccn_closure *selfp, struct ccn_upcall_info *info);
void get_lsa_identifier(struct name_prefix *lsaId,struct ccn_closure *selfp, struct ccn_upcall_info *info,int offset);
void get_lsdb_version(char *lsdb_version,struct ccn_closure *selfp, struct ccn_upcall_info *info );
int get_ls_type(struct ccn_closure *selfp, struct ccn_upcall_info *info);

enum ccn_upcall_res incoming_interest(struct ccn_closure *selfp, enum ccn_upcall_kind kind, struct ccn_upcall_info *info);
 
void process_incoming_interest(struct ccn_closure *selfp, struct ccn_upcall_info *info);
void process_incoming_interest_info(struct ccn_closure *selfp, struct ccn_upcall_info *info);
void process_incoming_interest_lsdb(struct ccn_closure *selfp, struct ccn_upcall_info *info);
void process_incoming_interest_lsa(struct ccn_closure *selfp, struct ccn_upcall_info *info);


enum ccn_upcall_res incoming_content(struct ccn_closure* selfp, enum ccn_upcall_kind kind, struct ccn_upcall_info* info);
void process_incoming_content(struct ccn_closure *selfp, struct ccn_upcall_info* info);
void process_incoming_content_info(struct ccn_closure *selfp, struct ccn_upcall_info* info);
void process_incoming_content_lsdb(struct ccn_closure *selfp, struct ccn_upcall_info* info);
void process_incoming_content_lsa(struct ccn_closure *selfp, struct ccn_upcall_info* info);

void process_incoming_timed_out_interest(struct ccn_closure* selfp, struct ccn_upcall_info* info);
void process_incoming_timed_out_interest_info(struct ccn_closure* selfp, struct ccn_upcall_info* info);
void process_incoming_timed_out_interest_lsdb(struct ccn_closure* selfp, struct ccn_upcall_info* info);
void process_incoming_timed_out_interest_lsa(struct ccn_closure* selfp, struct ccn_upcall_info* info);

int send_info_interest(struct ccn_schedule *sched, void *clienth, struct ccn_scheduled_event *ev, int flags);
void send_info_interest_to_neighbor(struct name_prefix *nbr);

int send_lsdb_interest(struct ccn_schedule *sched, void *clienth, struct ccn_scheduled_event *ev, int flags);
void send_lsdb_interest_to_nbr(struct name_prefix *nbr);
void send_interest_for_name_lsa(struct name_prefix *nbr, char *orig_router, char *ls_type, char *ls_id);
void send_interest_for_adj_lsa(struct name_prefix *nbr, char *orig_router, char *ls_type);

#endif
