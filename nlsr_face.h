#ifndef _NLSR_FACE_H_
#define _NLSR_FACE_H_

#define CCN_FIB_LIFETIME ((~0U) >> 1)
#define CCN_FIB_MCASTTTL (-1)
#define OP_REG  0
#define OP_UNREG 1

int add_ccn_face(struct ccn *h, const char *uri, const char *address, const unsigned int port,unsigned int tunnel_proto);
int delete_ccn_face(struct ccn *h, const char *uri, const char *address, const unsigned int port,unsigned int tunnel_proto);

#endif
