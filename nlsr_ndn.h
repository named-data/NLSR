#ifndef _NLSR_NDN_H_
#define _NLSR_NDN_H_

enum ccn_upcall_res incoming_interest(struct ccn_closure *selfp, enum ccn_upcall_kind kind, struct ccn_upcall_info *info);


enum ccn_upcall_res incoming_content(struct ccn_closure* selfp, enum ccn_upcall_kind kind, struct ccn_upcall_info* info);

#endif

