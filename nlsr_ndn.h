#ifndef _NLSR_NDN_H_
#define _NLSR_NDN_H_

struct upcalldata {
     int magic; /* 856372 */
     long *counter;
     unsigned warn;
     int flags;
     int n_excl;
     struct ccn_charbuf **excl; /* Array of n_excl items */
};

int send_lsdb_interest(struct ccn_schedule *sched, void *clienth, struct ccn_scheduled_event *ev, int flags);

enum ccn_upcall_res incoming_interest(struct ccn_closure *selfp, enum ccn_upcall_kind kind, struct ccn_upcall_info *info);
void process_incoming_interest(struct ccn_closure *selfp, struct ccn_upcall_info *info);
void process_incoming_interest_lsdb(struct ccn_closure *selfp, struct ccn_upcall_info *info);

void process_incoming_timed_out_interest(struct ccn_closure* selfp, struct ccn_upcall_info* info);
void process_incoming_timed_out_interest_lsdb(struct ccn_closure* selfp, struct ccn_upcall_info* info);


enum ccn_upcall_res incoming_content(struct ccn_closure* selfp, enum ccn_upcall_kind kind, struct ccn_upcall_info* info);
void process_incoming_content(struct ccn_closure* selfp, struct ccn_upcall_info* info);
void process_incoming_content_lsdb(struct ccn_closure* selfp, struct ccn_upcall_info* info);

#endif

