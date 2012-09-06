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
#include "nlsr_npl.h"


void 
add_name_to_npl(struct name_prefix *np)
{
	struct name_prefix *hnp=(struct name_prefix *)malloc(sizeof(struct name_prefix )); //free

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee; 	
    	int res;

   	hashtb_start(nlsr->npl, e);
    	res = hashtb_seek(e, np->name, np->length, 0);

	if(res == HT_NEW_ENTRY)
	{   

		hnp = e->data;
		hnp->length=np->length;
		hnp->name=(char *)malloc(np->length); //free
		memcpy(hnp->name,np->name,np->length);
	}
    	
	hashtb_end(e);

}


void
print_name_prefix_from_npl(void)
{
	printf("print_name_prefix_from_npl called \n");	
	int i, npl_element;
	struct name_prefix *np;

	struct hashtb_enumerator ee;
    	struct hashtb_enumerator *e = &ee;
    	
    	hashtb_start(nlsr->npl, e);
	npl_element=hashtb_n(nlsr->npl);

	for(i=0;i<npl_element;i++)
	{
		np=e->data;
		printf("Name Prefix: %s and Length: %d \n",np->name,np->length);	
		hashtb_next(e);		
	}

	hashtb_end(e);

	printf("\n");
}

