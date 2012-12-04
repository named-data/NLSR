#ifndef _NLSR_FIB_H_
#define _NLSR_FIB_H_

#define CCN_FIB_LIFETIME ((~0U) >> 1)
#define CCN_FIB_MCASTTTL (-1)
#define OP_REG  0
#define OP_UNREG 1

int add_delete_ccn_face_by_face_id(struct ccn *h, const char *uri, int operation, int faceid);

#endif
