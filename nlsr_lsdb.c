#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>
#include <assert.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <ccn/ccn.h>
#include <ccn/uri.h>
#include <ccn/keystore.h>
#include <ccn/signing.h>
#include <ccn/schedule.h>
#include <ccn/hashtb.h>

#include "nlsr.h"
#include "nlsr_lsdb.h"
#include "utility.h"

void
make_name_lsa_key(struct ccn_charbuf *key, struct ccn_charbuf *orig_router, unsigned int ls_type, long int nlsa_id, long int orig_time)
{
	printf("make_name_lsa_key called \n");	
	ccn_charbuf_append_string(key,ccn_charbuf_as_string(orig_router));

	struct ccn_charbuf *c=ccn_charbuf_create();
	ccn_charbuf_reset(c);
	ccn_charbuf_putf(c, "%d", (unsigned)ls_type);
	ccn_charbuf_append_string(key,ccn_charbuf_as_string(c));
	
	ccn_charbuf_reset(c);
	ccn_charbuf_putf(c, "%d", (unsigned)nlsa_id);
	ccn_charbuf_append_string(key,ccn_charbuf_as_string(c));
	
	ccn_charbuf_reset(c);
	ccn_charbuf_putf(c, "%ld", (unsigned)orig_time);
	ccn_charbuf_append_string(key,ccn_charbuf_as_string(c));

	ccn_charbuf_destroy(&c);

	printf("Key: %s length: %d\n",ccn_charbuf_as_string(key),(int)key->length);

}

void 
install_name_lsa(struct nlsa *new_name_lsa)
{
	printf("install_name_lsa called \n");
	struct ccn_charbuf *key=ccn_charbuf_create();
	make_name_lsa_key(key,new_name_lsa->header->orig_router,new_name_lsa->header->ls_type,new_name_lsa->header->ls_id,new_name_lsa->header->orig_time);

	struct nlsa *name_lsa=(struct nlsa*)malloc(sizeof(struct nlsa *));
	
	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res;

   	hashtb_start(nlsr->lsdb->name_lsdb, e);
    	res = hashtb_seek(e, key->buf , key->length, 0);

	if(res == HT_NEW_ENTRY )
	{
   		printf("New Name LSA. Added...\n");
		name_lsa = e->data;

		name_lsa->header=(struct nlsa_header *)malloc(sizeof(struct nlsa_header *));
	
	
		name_lsa->header->ls_type=new_name_lsa->header->ls_type;
		name_lsa->header->orig_time=new_name_lsa->header->orig_time;
		name_lsa->header->ls_id=new_name_lsa->header->ls_id;
		
		name_lsa->header->orig_router=ccn_charbuf_create();
		ccn_charbuf_append_string(name_lsa->header->orig_router,ccn_charbuf_as_string(new_name_lsa->header->orig_router));	
		name_lsa->header->isValid=new_name_lsa->header->isValid;

		name_lsa->name_prefix=ccn_charbuf_create();	
		ccn_charbuf_append_string(name_lsa->name_prefix,ccn_charbuf_as_string(new_name_lsa->name_prefix));
		//ccn_charbuf_append_charbuf(name_lsa->name_prefix,new_name_lsa->name_prefix);
	}
	else if(res == HT_OLD_ENTRY)
	{
		printf("Duplicate Name LSA. Discarded...\n");
	
	}
	
    	hashtb_end(e);

	ccn_charbuf_destroy(&key);
	
}

struct nlsa *
build_name_lsa(struct ccn_charbuf *name_prefix)
{
	printf("build_name_lsa called \n");
	struct nlsa *name_lsa=(struct nlsa*)malloc(sizeof(struct nlsa *));

	name_lsa->header=(struct nlsa_header *)malloc(sizeof(struct nlsa_header *));
	name_lsa->header->orig_router=ccn_charbuf_create();
	
	name_lsa->header->ls_type=LS_TYPE_NAME;
	name_lsa->header->orig_time=get_current_time_sec();
	name_lsa->header->ls_id=++nlsr->nlsa_id;
	ccn_charbuf_append_string(name_lsa->header->orig_router,nlsr->router_name);	
	name_lsa->header->isValid=1;

	name_lsa->name_prefix=ccn_charbuf_create();	
	ccn_charbuf_append_string(name_lsa->name_prefix,ccn_charbuf_as_string(name_prefix));
	//ccn_charbuf_append_charbuf(name_lsa->name_prefix,name_prefix);

	return name_lsa;
}

int 
initial_build_name_lsa(struct ccn_schedule *sched, void *clienth, struct ccn_scheduled_event *ev, int flags)
{
	printf("initial_build_name_lsa called \n");	
	int i, npl_element;
	struct name_prefix *np;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->npl, e);
	npl_element=hashtb_n(nlsr->npl);

	for(i=0;i<npl_element;i++)
	{
		np=e->data;

		struct nlsa *name_lsa;
		struct ccn_charbuf *name;
		name=ccn_charbuf_create();
		ccn_charbuf_append_string(name,np->name);
		name_lsa=build_name_lsa(name);
		install_name_lsa(name_lsa);

		ccn_charbuf_destroy(&name_lsa->header->orig_router);
		ccn_charbuf_destroy(&name_lsa->name_prefix);
		free(name_lsa->header);
		free(name_lsa);
		ccn_charbuf_destroy(&name);
	
		hashtb_next(e);		
	}

	hashtb_end(e);

	print_name_lsdb();

	return 0;
}

void 
print_name_lsa(struct nlsa *name_lsa)
{
	printf("print_name_lsa called \n");
	printf("-----------Name LSA---------------\n");
	printf("	Origination Router       :	%s\n",ccn_charbuf_as_string(name_lsa->header->orig_router));
	printf("	Origination Router Length:	%d\n",(int)name_lsa->header->orig_router->length);
	printf("	LS Type			 :	%d\n",name_lsa->header->ls_type);
	printf("	LS Id			 :	%ld\n",name_lsa->header->ls_id);
	printf("	Origination Time	 :	%ld\n",name_lsa->header->orig_time);
	printf("	Is Valid 		 :	%u\n",name_lsa->header->isValid);

	printf("	LSA Data			\n");
	printf("		Name Prefix:	 	:	%s\n",ccn_charbuf_as_string(name_lsa->name_prefix));
	printf("		Name Prefix Length	:	%d\n",(int)name_lsa->name_prefix->length);

	printf("\n");

}

void
print_name_lsdb(void)
{
	printf("print_name_lsdb called \n");	
	int i, lsdb_element;
	struct nlsa *name_lsa;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->lsdb->name_lsdb, e);
	lsdb_element=hashtb_n(nlsr->lsdb->name_lsdb);

	for(i=0;i<lsdb_element;i++)
	{
		name_lsa=e->data;
		print_name_lsa(name_lsa);		
		hashtb_next(e);		
	}

	hashtb_end(e);

}
