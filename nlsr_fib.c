#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>

#include <ccn/ccn.h>
#include <ccn/uri.h>
#include <ccn/face_mgmt.h>
#include <ccn/reg_mgmt.h>
#include <ccn/charbuf.h>

#include "nlsr_fib.h"
#include "nlsr.h"


static void 
ccn_fib_warn(int lineno, const char *format, ...)
{
	struct timeval t;
	va_list ap;
	va_start(ap, format);
	gettimeofday(&t, NULL);
	fprintf(stderr, "%d.%06d ccn_fib[%d]:%d: ", (int)t.tv_sec, (unsigned)t.tv_usec, (int)getpid(), lineno);
	vfprintf(stderr, format, ap);
	va_end(ap);
}

#define ON_ERROR_CLEANUP(resval) \
{           \
    if ((resval) < 0) { \
        ccn_fib_warn (__LINE__, "OnError cleanup\n"); \
        goto cleanup; \
    } \
}

#define ON_NULL_CLEANUP(resval) \
{           \
    if ((resval) == NULL) { \
        ccn_fib_warn(__LINE__, "OnNull cleanup\n"); \
        goto cleanup; \
    } \
}



static int 
register_unregister_prefix(struct ccn *h, struct ccn_charbuf *local_scope_template,
        struct ccn_charbuf *no_name, struct ccn_charbuf *name_prefix,const unsigned char *ccndid, size_t ccnd_id_size, int faceid, int operation)
{
	struct ccn_charbuf *temp = NULL;
	struct ccn_charbuf *resultbuf = NULL;
	struct ccn_charbuf *signed_info = NULL;
	struct ccn_charbuf *name = NULL;
	struct ccn_charbuf *prefixreg = NULL;
	struct ccn_parsed_ContentObject pcobuf = {0};
	struct ccn_forwarding_entry forwarding_entry_storage = {0};
	struct ccn_forwarding_entry *forwarding_entry = &forwarding_entry_storage;
	struct ccn_forwarding_entry *new_forwarding_entry;
	const unsigned char *ptr = NULL;
	size_t length = 0;
	int res;

	/* Register or unregister the prefix */
	forwarding_entry->action = (operation == OP_REG) ? "prefixreg" : "unreg";
	forwarding_entry->name_prefix = name_prefix;
	forwarding_entry->ccnd_id = ccndid;
	forwarding_entry->ccnd_id_size =ccnd_id_size;
	forwarding_entry->faceid = faceid;
	forwarding_entry->flags = -1;
	forwarding_entry->lifetime = 2100;

	prefixreg = ccn_charbuf_create();
	ccnb_append_forwarding_entry(prefixreg, forwarding_entry);
	temp = ccn_charbuf_create();
	res = ccn_sign_content(h, temp, no_name, NULL, prefixreg->buf, prefixreg->length);
	resultbuf = ccn_charbuf_create();

	/* construct Interest containing prefixreg request */
	name = ccn_charbuf_create();
	ccn_name_init(name);
	ccn_name_append_str(name, "ccnx");
	ccn_name_append(name, ccndid, ccnd_id_size);
	ccn_name_append_str(name, (operation == OP_REG) ? "prefixreg" : "unreg");
	ccn_name_append(name, temp->buf, temp->length);

	/* send Interest, get Data */
	res = ccn_get(h, name, local_scope_template, 1000, resultbuf, &pcobuf, NULL, 0);
	ON_ERROR_CLEANUP(res);

	res = ccn_content_get_value(resultbuf->buf, resultbuf->length, &pcobuf, &ptr, &length);
	ON_ERROR_CLEANUP(res);
    
	/* extract new forwarding entry from Data */
	new_forwarding_entry = ccn_forwarding_entry_parse(ptr, length);
	ON_NULL_CLEANUP(new_forwarding_entry);

	res = new_forwarding_entry->faceid;

	ccn_forwarding_entry_destroy(&new_forwarding_entry);
	ccn_charbuf_destroy(&signed_info);
	ccn_charbuf_destroy(&temp);
	ccn_charbuf_destroy(&resultbuf);
	ccn_charbuf_destroy(&name);
	ccn_charbuf_destroy(&prefixreg);

	return res;

	cleanup:
		ccn_forwarding_entry_destroy(&new_forwarding_entry);
		ccn_charbuf_destroy(&signed_info);
		ccn_charbuf_destroy(&temp);
		ccn_charbuf_destroy(&resultbuf);
		ccn_charbuf_destroy(&name);
		ccn_charbuf_destroy(&prefixreg);

	return -1;
}


static int 
get_ccndid(struct ccn *h, struct ccn_charbuf *local_scope_template,
        unsigned char *ccndid)
{
	struct ccn_charbuf *name = NULL;
	struct ccn_charbuf *resultbuf = NULL;
	struct ccn_parsed_ContentObject pcobuf = {0};
	char ccndid_uri[] = "ccnx:/%C1.M.S.localhost/%C1.M.SRV/ccnd/KEY";
	const unsigned char *ccndid_result;
	static size_t ccndid_result_size;
	int res;

	name = ccn_charbuf_create();
	resultbuf = ccn_charbuf_create();

	res = ccn_name_from_uri(name, ccndid_uri);
	ON_ERROR_CLEANUP(res);

	/* get Data */
	res = ccn_get(h, name, local_scope_template, 4500, resultbuf, &pcobuf, NULL, 0);
	ON_ERROR_CLEANUP(res);

	/* extract from Data */
	res = ccn_ref_tagged_BLOB(CCN_DTAG_PublisherPublicKeyDigest,
            resultbuf->buf,
            pcobuf.offset[CCN_PCO_B_PublisherPublicKeyDigest],
            pcobuf.offset[CCN_PCO_E_PublisherPublicKeyDigest],
            &ccndid_result, &ccndid_result_size);
	ON_ERROR_CLEANUP(res);

	memcpy((void *)ccndid, ccndid_result, ccndid_result_size);

	ccn_charbuf_destroy(&name);
	ccn_charbuf_destroy(&resultbuf);

	return (ccndid_result_size);

	cleanup:
		ccn_charbuf_destroy(&name);
		ccn_charbuf_destroy(&resultbuf);

	return -1;
}

static void 
init_data(struct ccn_charbuf *local_scope_template,
        struct ccn_charbuf *no_name)
{
	ccn_charbuf_append_tt(local_scope_template, CCN_DTAG_Interest, CCN_DTAG);
	ccn_charbuf_append_tt(local_scope_template, CCN_DTAG_Name, CCN_DTAG);
	ccn_charbuf_append_closer(local_scope_template);    /* </Name> */
	ccnb_tagged_putf(local_scope_template, CCN_DTAG_Scope, "1");
	ccn_charbuf_append_closer(local_scope_template);    /* </Interest> */

	ccn_name_init(no_name);
}

int 
add_delete_ccn_face_by_face_id(struct ccn *h, const char *uri, int operation, int faceid)
{
	if ( nlsr->debugging )
	{
		printf("add_delete_ccn_face_by_face_id called\n");
		printf("Uri: %s  Face: %d Operation: %s \n",(char *)uri , faceid, operation == OP_REG ? "Registration" : "Unregistration");
	}

	struct ccn_charbuf *prefix;
	struct ccn_charbuf *local_scope_template = ccn_charbuf_create();
	struct ccn_charbuf *no_name = ccn_charbuf_create();
	unsigned char ccndid_storage[32] = {0};
	unsigned char *ccndid = ccndid_storage;
	size_t ccndid_size = 0;
	int res;

	prefix = ccn_charbuf_create();
	res = ccn_name_from_uri(prefix, uri);
	ON_ERROR_CLEANUP(res);


	init_data(local_scope_template, no_name);

	ccndid_size = get_ccndid(h, local_scope_template, ccndid);
	if (ccndid_size != sizeof(ccndid_storage))
 	{
		fprintf(stderr, "Incorrect size for ccnd id in response\n");
		ON_ERROR_CLEANUP(-1);
	}

	res = register_unregister_prefix(h, local_scope_template, no_name, prefix,ccndid, ccndid_size,faceid, operation);
	
	ON_ERROR_CLEANUP(res);

	ccn_charbuf_destroy(&local_scope_template);
	ccn_charbuf_destroy(&no_name);
	ccn_charbuf_destroy(&prefix);

	return 0;

	cleanup:
		ccn_charbuf_destroy(&prefix);
		ccn_charbuf_destroy(&local_scope_template);
		ccn_charbuf_destroy(&no_name);

	return -1;
}


