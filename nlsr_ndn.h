#ifndef _NLSR_NDN_H_
#define _NLSR_NDN_H_

int appendLifetime(struct ccn_charbuf *cb, int lifetime);

int send_lsdb_interest(struct ccn_schedule *sched, void *clienth, struct ccn_scheduled_event *ev, int flags);

enum ccn_upcall_res incoming_interest(struct ccn_closure *selfp, enum ccn_upcall_kind kind, struct ccn_upcall_info *info);
void process_incoming_interest(struct ccn_closure *selfp, struct ccn_upcall_info *info);
void process_incoming_interest_lsdb(struct ccn_closure *selfp, struct ccn_upcall_info *info);
void process_incoming_interest_info(struct ccn_closure *selfp, struct ccn_upcall_info *info);

void process_incoming_timed_out_interest(struct ccn_closure* selfp, struct ccn_upcall_info* info);
void process_incoming_timed_out_interest_lsdb(struct ccn_closure* selfp, struct ccn_upcall_info* info);
void process_incoming_timed_out_interest_info(struct ccn_closure* selfp, struct ccn_upcall_info* info);


enum ccn_upcall_res incoming_content(struct ccn_closure* selfp, enum ccn_upcall_kind kind, struct ccn_upcall_info* info);
void process_incoming_content(struct ccn_closure* selfp, struct ccn_upcall_info* info);
void process_incoming_content_lsdb(struct ccn_closure* selfp, struct ccn_upcall_info* info);
void process_incoming_content_info(struct ccn_closure* selfp, struct ccn_upcall_info* info);


int send_info_interest(struct ccn_schedule *sched, void *clienth, struct ccn_scheduled_event *ev, int flags);
void send_info_interest_to_neighbor(struct ccn_charbuf *nbr);

#endif

