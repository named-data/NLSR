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



#include "nlsr_face.h"

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

static void 
ccn_fib_fatal(int lineno, const char *format, ...)
{
	struct timeval t;
	va_list ap;
	va_start(ap, format);
	gettimeofday(&t, NULL);
	fprintf(stderr, "%d.%06d ccn_fib[%d]:%d: ", (int)t.tv_sec, (unsigned)t.tv_usec, (int)getpid(), lineno);
	vfprintf(stderr, format, ap);
	va_end(ap);
	exit(1);
}

#define ON_ERROR_EXIT(resval, msg) on_error_exit((resval), __LINE__, msg)

static void 
on_error_exit(int res, int lineno, const char *msg)
{
	if (res >= 0)
		return;
	ccn_fib_fatal(lineno, "fatal error, res = %d, %s\n", res, msg);
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


/**
 * 
 * Bind a prefix to a face
 *
 */
static int 
register_unregister_prefix(struct ccn *h, struct ccn_charbuf *local_scope_template,
        struct ccn_charbuf *no_name, struct ccn_charbuf *name_prefix,
        struct ccn_face_instance *face_instance, int operation)
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
	forwarding_entry->ccnd_id = face_instance->ccnd_id;
	forwarding_entry->ccnd_id_size = face_instance->ccnd_id_size;
	forwarding_entry->faceid = face_instance->faceid;
	forwarding_entry->flags = -1;
	forwarding_entry->lifetime = (~0U) >> 1;

	prefixreg = ccn_charbuf_create();
	ccnb_append_forwarding_entry(prefixreg, forwarding_entry);
	temp = ccn_charbuf_create();
	res = ccn_sign_content(h, temp, no_name, NULL, prefixreg->buf, prefixreg->length);
	resultbuf = ccn_charbuf_create();

	/* construct Interest containing prefixreg request */
	name = ccn_charbuf_create();
	ccn_name_init(name);
	ccn_name_append_str(name, "ccnx");
	ccn_name_append(name, face_instance->ccnd_id, face_instance->ccnd_id_size);
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

/**
 *
 * Create new face by sending out a request Interest
 * The actual new face instance is returned
 * 
 */
static 
struct ccn_face_instance *create_face(struct ccn *h, struct ccn_charbuf *local_scope_template,
        struct ccn_charbuf *no_name, struct ccn_face_instance *face_instance)
{
	struct ccn_charbuf *newface = NULL;
	struct ccn_charbuf *signed_info = NULL;
	struct ccn_charbuf *temp = NULL;
	struct ccn_charbuf *name = NULL;
	struct ccn_charbuf *resultbuf = NULL;
	struct ccn_parsed_ContentObject pcobuf = {0};
	struct ccn_face_instance *new_face_instance = NULL;
	const unsigned char *ptr = NULL;
	size_t length = 0;
	int res = 0;

	/* Encode the given face instance */
	newface = ccn_charbuf_create();
	ccnb_append_face_instance(newface, face_instance);

	temp = ccn_charbuf_create();
	res = ccn_sign_content(h, temp, no_name, NULL, newface->buf, newface->length);
	resultbuf = ccn_charbuf_create();

	/* Construct the Interest name that will create the face */
	name = ccn_charbuf_create();
	ccn_name_init(name);
	ccn_name_append_str(name, "ccnx");
	ccn_name_append(name, face_instance->ccnd_id, face_instance->ccnd_id_size);
	ccn_name_append_str(name, face_instance->action);
	ccn_name_append(name, temp->buf, temp->length);

	/* send Interest to retrieve Data that contains the newly created face */
	res = ccn_get(h, name, local_scope_template, 1000, resultbuf, &pcobuf, NULL, 0);
	ON_ERROR_CLEANUP(res);

	/* decode Data to get the actual face instance */
	res = ccn_content_get_value(resultbuf->buf, resultbuf->length, &pcobuf, &ptr, &length);
	ON_ERROR_CLEANUP(res);

	new_face_instance = ccn_face_instance_parse(ptr, length);

	ccn_charbuf_destroy(&newface);
	ccn_charbuf_destroy(&signed_info);
	ccn_charbuf_destroy(&temp);
	ccn_charbuf_destroy(&resultbuf);
	ccn_charbuf_destroy(&name);

	return new_face_instance;

	cleanup:
		ccn_charbuf_destroy(&newface);
		ccn_charbuf_destroy(&signed_info);
		ccn_charbuf_destroy(&temp);
		ccn_charbuf_destroy(&resultbuf);
		ccn_charbuf_destroy(&name);

	return NULL;
}

/**
 *
 * Get ccnd id
 *
 */
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
	ON_ERROR_EXIT(res, "Unable to parse service locator URI for ccnd key\n");

	/* get Data */
	res = ccn_get(h, name, local_scope_template, 4500, resultbuf, &pcobuf, NULL, 0);
	ON_ERROR_EXIT(res, "Unable to get key from ccnd\n");

	/* extract from Data */
	res = ccn_ref_tagged_BLOB(CCN_DTAG_PublisherPublicKeyDigest,
            resultbuf->buf,
            pcobuf.offset[CCN_PCO_B_PublisherPublicKeyDigest],
            pcobuf.offset[CCN_PCO_E_PublisherPublicKeyDigest],
            &ccndid_result, &ccndid_result_size);
	ON_ERROR_EXIT(res, "Unable to parse ccnd response for ccnd id\n");

	memcpy((void *)ccndid, ccndid_result, ccndid_result_size);

	ccn_charbuf_destroy(&name);
	ccn_charbuf_destroy(&resultbuf);

	return (ccndid_result_size);
}

/**
 * Construct a new face instance based on the given address and port
 * This face instance is only used to send new face request
 */
static struct 
ccn_face_instance *construct_face(const unsigned char *ccndid, size_t ccndid_size,
        const char *address, const char *port, unsigned int tunnel_proto)
{
	struct ccn_face_instance *fi = calloc(1, sizeof(*fi));
	char rhostnamebuf[NI_MAXHOST];
	char rhostportbuf[NI_MAXSERV];
	struct addrinfo hints = {.ai_family = AF_UNSPEC, .ai_flags = (AI_ADDRCONFIG),
        .ai_socktype = SOCK_DGRAM};
	struct addrinfo *raddrinfo = NULL;
	struct ccn_charbuf *store = ccn_charbuf_create();
	int host_off = -1;
	int port_off = -1;
	int res;

	res = getaddrinfo(address, port, &hints, &raddrinfo);
	if (res != 0 || raddrinfo == NULL) 
	{
		fprintf(stderr, "Error: getaddrinfo\n");
		return NULL;
	}

	res = getnameinfo(raddrinfo->ai_addr, raddrinfo->ai_addrlen,
            rhostnamebuf, sizeof(rhostnamebuf),
            rhostportbuf, sizeof(rhostportbuf),
            NI_NUMERICHOST | NI_NUMERICSERV);
	freeaddrinfo(raddrinfo);	
	if (res != 0) 
	{
		fprintf(stderr, "Error: getnameinfo\n");
		return NULL;
	}

	fi->store = store;
	fi->descr.ipproto = tunnel_proto;
	fi->descr.mcast_ttl = CCN_FIB_MCASTTTL;
	fi->lifetime = CCN_FIB_LIFETIME;

	ccn_charbuf_append(store, "newface", strlen("newface") + 1);
	host_off = store->length;
	ccn_charbuf_append(store, rhostnamebuf, strlen(rhostnamebuf) + 1);
	port_off = store->length;
	ccn_charbuf_append(store, rhostportbuf, strlen(rhostportbuf) + 1);

	char *b = (char *)store->buf;
	fi->action = b;
	fi->descr.address = b + host_off;
	fi->descr.port = b + port_off;
	fi->descr.source_address = NULL;
	fi->ccnd_id = ccndid;
	fi->ccnd_id_size = ccndid_size;

	return fi;
}

/**
 * initialize local data
 */
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

static int 
add_delete_ccn_face(struct ccn *h, const char *uri, const char *address, const unsigned int p, int operation,unsigned int tunnel_proto)
{
	struct ccn_charbuf *prefix;
	char port[6];
	struct ccn_charbuf *local_scope_template = ccn_charbuf_create();
	struct ccn_charbuf *no_name = ccn_charbuf_create();
	unsigned char ccndid_storage[32] = {0};
	unsigned char *ccndid = ccndid_storage;
	size_t ccndid_size = 0;
	struct ccn_face_instance *fi;
	struct ccn_face_instance *nfi;
	int res;

	prefix = ccn_charbuf_create();
	res = ccn_name_from_uri(prefix, uri);
	ON_ERROR_CLEANUP(res);
	memset(port, 0, 6);
	sprintf(port, "%d", p);

	init_data(local_scope_template, no_name);

	ccndid_size = get_ccndid(h, local_scope_template, ccndid);
	if (ccndid_size != sizeof(ccndid_storage))
 	{
		fprintf(stderr, "Incorrect size for ccnd id in response\n");
		ON_ERROR_CLEANUP(-1);
	}

	/* construct a face instance for new face request */
	fi = construct_face(ccndid, ccndid_size, address, port,tunnel_proto);
	ON_NULL_CLEANUP(fi);

	/* send new face request to actually create a new face */
	nfi = create_face(h, local_scope_template, no_name, fi);
	ON_NULL_CLEANUP(nfi);


	res = register_unregister_prefix(h, local_scope_template, no_name, prefix, nfi, operation);
	ON_ERROR_CLEANUP(res);

	int faceid=nfi->faceid;

	ccn_charbuf_destroy(&local_scope_template);
	ccn_charbuf_destroy(&no_name);
	ccn_face_instance_destroy(&fi);
	ccn_face_instance_destroy(&nfi);
	ccn_charbuf_destroy(&prefix);

	
	return faceid;

	cleanup:
		ccn_charbuf_destroy(&prefix);
		ccn_charbuf_destroy(&local_scope_template);
		ccn_charbuf_destroy(&no_name);
		ccn_face_instance_destroy(&fi);
		ccn_face_instance_destroy(&nfi);

	return -1;
}


int 
add_ccn_face(struct ccn *h, const char *uri, const char *address, const unsigned int port, unsigned int tunnel_proto)
{
	return add_delete_ccn_face(h, uri, address, port, OP_REG,tunnel_proto);
}


int 
delete_ccn_face(struct ccn *h, const char *uri, const char *address, const unsigned int port,unsigned int tunnel_proto)
{
	return add_delete_ccn_face(h, uri, address, port, OP_UNREG,tunnel_proto);
}

