#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <assert.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <ccn/ccn.h>
#include <ccn/uri.h>
#include <ccn/keystore.h>
#include <ccn/signing.h>
#include <ccn/schedule.h>
#include <ccn/hashtb.h>
#include <ccn/sync.h>
#include <ccn/seqwriter.h>

#include "nlsr.h"
#include "nlsr_sync.h"
#include "nlsr_lsdb.h"
#include "utility.h"

int 
get_lsa_position(struct ccn_charbuf * ccnb, struct ccn_indexbuf *comps)
{

	
	
	int res,i;
	int lsa_position=0;
	int name_comps=(int)comps->n;

	for(i=0;i<name_comps;i++)
	{
		res=ccn_name_comp_strcmp(ccnb->buf,comps,i,"LSA");
		if( res == 0)
		{
			lsa_position=i;
			break;
		}	
	}

	return lsa_position;

}

void 
get_name_part(struct name_prefix *name_part,struct ccn_charbuf * interest_ccnb, struct ccn_indexbuf *interest_comps, int offset)
{

	
	
	int res,i;
	int lsa_position=0;
	int len=0;

	lsa_position=get_lsa_position(interest_ccnb,interest_comps);

	const unsigned char *comp_ptr1;
	size_t comp_size;
	for(i=lsa_position+1+offset;i<interest_comps->n-1;i++)
	{
		res=ccn_name_comp_get(interest_ccnb->buf, interest_comps,i,&comp_ptr1, &comp_size);
		len+=1;
		len+=(int)comp_size;	
	}
	len++;

	char *neighbor=(char *)malloc(len);
	memset(neighbor,0,len);

	for(i=lsa_position+1+offset; i<interest_comps->n-1;i++)
	{
		res=ccn_name_comp_get(interest_ccnb->buf, interest_comps,i,&comp_ptr1, &comp_size);
		memcpy(neighbor+strlen(neighbor),"/",1);
		memcpy(neighbor+strlen(neighbor),(char *)comp_ptr1,strlen((char *)comp_ptr1));

	}

	name_part->name=(char *)malloc(strlen(neighbor)+1);
	memset(name_part->name,0,strlen(neighbor)+1);
	memcpy(name_part->name,neighbor,strlen(neighbor)+1);
	name_part->length=strlen(neighbor)+1;

	
}


char *
get_content_by_content_name(char *content_name)
{

    struct ccn_charbuf *name = NULL;
    struct ccn_charbuf *templ = NULL;
    struct ccn_charbuf *resultbuf = NULL;
    struct ccn_parsed_ContentObject pcobuf = { 0 };
    int res;
    int allow_stale = 0;
    int content_only = 1;
    int scope = -1;
    const unsigned char *ptr;
    size_t length;
    int resolve_version = CCN_V_HIGHEST;
    int timeout_ms = 3000;
    const unsigned lifetime_default = CCN_INTEREST_LIFETIME_SEC << 12;
    unsigned lifetime_l12 = lifetime_default;
    int get_flags = 0;
    

    name = ccn_charbuf_create();
    res = ccn_name_from_uri(name,content_name);
    if (res < 0) {
        fprintf(stderr, "Bad ccn URI: %s\n", content_name);
        exit(1);
    }
  
   
	if (allow_stale || lifetime_l12 != lifetime_default || scope != -1) {
        templ = ccn_charbuf_create();
        ccn_charbuf_append_tt(templ, CCN_DTAG_Interest, CCN_DTAG);
        ccn_charbuf_append_tt(templ, CCN_DTAG_Name, CCN_DTAG);
        ccn_charbuf_append_closer(templ); /* </Name> */
		if (allow_stale) {
			ccn_charbuf_append_tt(templ, CCN_DTAG_AnswerOriginKind, CCN_DTAG);
			ccnb_append_number(templ,
							   CCN_AOK_DEFAULT | CCN_AOK_STALE);
			ccn_charbuf_append_closer(templ); /* </AnswerOriginKind> */
		}
        if (scope != -1) {
            ccnb_tagged_putf(templ, CCN_DTAG_Scope, "%d", scope);
        }
		if (lifetime_l12 != lifetime_default) {
			/*
			 * Choose the interest lifetime so there are at least 3
			 * expressions (in the unsatisfied case).
			 */
			unsigned char buf[3] = { 0 };
			int i;
			for (i = sizeof(buf) - 1; i >= 0; i--, lifetime_l12 >>= 8)
				buf[i] = lifetime_l12 & 0xff;
			ccnb_append_tagged_blob(templ, CCN_DTAG_InterestLifetime, buf, sizeof(buf));
		}
        ccn_charbuf_append_closer(templ); /* </Interest> */
    }
    resultbuf = ccn_charbuf_create();
    if (resolve_version != 0) {
        res = ccn_resolve_version(nlsr->ccn, name, resolve_version, 500);
        if (res >= 0) {
            ccn_uri_append(resultbuf, name->buf, name->length, 1);
            //fprintf(stderr, "== %s\n",ccn_charbuf_as_string(resultbuf));
            resultbuf->length = 0;
        }
    }
    res = ccn_get(nlsr->ccn, name, templ, timeout_ms, resultbuf, &pcobuf, NULL, get_flags);
    if (res >= 0) {
        ptr = resultbuf->buf;
        length = resultbuf->length;
        if (content_only)
            ccn_content_get_value(ptr, length, &pcobuf, &ptr, &length);
    }
    ccn_charbuf_destroy(&resultbuf);
    ccn_charbuf_destroy(&templ);
    ccn_charbuf_destroy(&name);   
	return (char *)ptr;
}

void 
process_incoming_sync_content_lsa(char *content_data)
{


	if ( nlsr->debugging )
		printf("process_incoming_sync_content_lsa called \n");
	//if ( nlsr->detailed_logging )
		//writeLogg(__FILE__,__FUNCTION__,__LINE__,"process_incoming_content_lsa called \n");	

	char *sep="|";
	char *rem;
	char *orig_router;
	char *orl;
	int orig_router_length;
	char *lst;
	int ls_type;
	char *lsid;
	long int ls_id;
	char *isvld;
	int isValid;
	char *num_link;
	int no_link;
	char *np;
	char *np_length;
	int name_length;
	char *data;
	char *orig_time;

	
	if ( nlsr->debugging )
		printf("LSA Data \n");
	//if ( nlsr->detailed_logging )
	//	writeLogg(__FILE__,__FUNCTION__,__LINE__,"LSA Data\n");	

	if( strlen(content_data ) > 0 )
	{

		orig_router=strtok_r((char *)content_data,sep,&rem);
		orl=strtok_r(NULL,sep,&rem);
		orig_router_length=atoi(orl);

		if ( nlsr->debugging )
		{
			printf("	Orig Router Name  : %s\n",orig_router);
			printf("	Orig Router Length: %d\n",orig_router_length);
		}

		lst=strtok_r(NULL,sep,&rem);		
		ls_type=atoi(lst);

		if ( nlsr->debugging )
			printf("	LS Type  : %d\n",ls_type);

		if ( ls_type == LS_TYPE_NAME )
		{
			lsid=strtok_r(NULL,sep,&rem);
			ls_id=atoi(lsid);
			orig_time=strtok_r(NULL,sep,&rem);
			isvld=strtok_r(NULL,sep,&rem);
			isValid=atoi(isvld);
			np=strtok_r(NULL,sep,&rem);
			np_length=strtok_r(NULL,sep,&rem);
			name_length=atoi(np_length);
			if ( nlsr->debugging )
			{
				printf("	LS ID  : %ld\n",ls_id);
				printf("	isValid  : %d\n",isValid);
				printf("	Name Prefix : %s\n",np);
				printf("	Orig Time   : %s\n",orig_time);
				printf("	Name Prefix length: %d\n",name_length);
			}

			build_and_install_others_name_lsa(orig_router,ls_type,ls_id,orig_time,isValid,np);

			print_name_lsdb();

		}
		else if ( ls_type == LS_TYPE_ADJ )
		{
			orig_time=strtok_r(NULL,sep,&rem);
			num_link=strtok_r(NULL,sep,&rem);
			no_link=atoi(num_link);
			data=rem;

			if ( nlsr->debugging )
			{
				printf("	No Link  : %d\n",no_link);
				printf("	Data  : %s\n",data);
			}
			build_and_install_others_adj_lsa(orig_router,ls_type,orig_time,no_link,data);
		}
	}
}

void
process_content_from_sync(struct ccn_charbuf *content_name, struct ccn_indexbuf *components)
{
	int lsa_position;
	int res;
	size_t comp_size;
	const unsigned char *lst;
	const unsigned char *lsid;
	const unsigned char *origtime;

	int ls_type;
	long int ls_id=0;

	char *time_stamp=(char *)malloc(20);
	memset(time_stamp,0,20);
	get_current_timestamp_micro(time_stamp);

	struct ccn_charbuf *uri = ccn_charbuf_create();
	ccn_uri_append(uri, content_name->buf, content_name->length, 0);	

	struct name_prefix *orig_router=(struct name_prefix *)malloc(sizeof(struct name_prefix));
	
	lsa_position=get_lsa_position(content_name, components);

	res=ccn_name_comp_get(content_name->buf, components,lsa_position+1,&lst, &comp_size);
	
	//printf("Ls Type: %s\n",lst);
	ls_type=atoi((char *)lst);
	if(ls_type == LS_TYPE_NAME)
	{
		
		res=ccn_name_comp_get(content_name->buf, components,lsa_position+2,&lsid, &comp_size);
		ls_id=atoi((char *)lsid);
		res=ccn_name_comp_get(content_name->buf, components,lsa_position+3,&origtime, &comp_size);
		get_name_part(orig_router,content_name,components,3);

		int lsa_life_time=get_time_diff(time_stamp,(char *)origtime);

		//printf("Ls ID: %s\nOrig Time: %s\nOrig Router: %s\n",lsid,origtime,orig_router->name);

		if ( (strcmp((char *)orig_router,nlsr->router_name) == 0 && lsa_life_time < nlsr->lsa_refresh_time) || (strcmp((char *)orig_router,nlsr->router_name) != 0 && lsa_life_time < nlsr->router_dead_interval) )
		{
			int is_new_name_lsa=check_is_new_name_lsa(orig_router->name,(char *)lst,(char *)lsid,(char *)origtime);
			if ( is_new_name_lsa == 1 )
			{
				printf("New NAME LSA.....\n");	
				char *content_data=get_content_by_content_name(ccn_charbuf_as_string(uri));
				printf("Content Data: %s \n",content_data);
				process_incoming_sync_content_lsa(content_data);
			}
			else 
			{
				printf("Name LSA / Newer Name LSA already xists in LSDB\n");
				char *content_data=get_content_by_content_name(ccn_charbuf_as_string(uri));
				printf("Content Data: %s \n",content_data);
			}
		}
		else 
		{
			printf("Lsa is older than Router LSA refresh time/ Dead Interval\n");
		}
	}
	else if(ls_type == LS_TYPE_ADJ)
	{
		res=ccn_name_comp_get(content_name->buf, components,lsa_position+2,&origtime, &comp_size);
		get_name_part(orig_router,content_name,components,2);
		printf("Orig Time: %s\nOrig Router: %s\n",origtime,orig_router->name);

		int lsa_life_time=get_time_diff(time_stamp,(char *)origtime);

		//printf("Ls ID: %s\nOrig Time: %s\nOrig Router: %s\n",lsid,origtime,orig_router->name);

		if ( (strcmp((char *)orig_router,nlsr->router_name) == 0 && lsa_life_time < nlsr->lsa_refresh_time) || (strcmp((char *)orig_router,nlsr->router_name) != 0 && lsa_life_time < nlsr->router_dead_interval) )	
		{
			int is_new_adj_lsa=check_is_new_adj_lsa(orig_router->name,(char *)lst,(char *)origtime);
			if ( is_new_adj_lsa == 1 )
			{
				printf("New Adj LSA.....\n");	
				char *content_data=get_content_by_content_name(ccn_charbuf_as_string(uri));
				printf("Content Data: %s \n",content_data);
				process_incoming_sync_content_lsa(content_data);			
			}
			else
			{

				printf("Adj LSA / Newer Adj LSA already exists in LSDB\n");
				char *content_data=get_content_by_content_name(ccn_charbuf_as_string(uri));
				printf("Content Data: %s \n",content_data);
			}
		}
		else 
		{
			printf("Lsa is older than Router LSA refresh time/ Dead Interval\n");
		}
	}

	ccn_charbuf_destroy(&uri);

}

int
sync_callback(struct ccns_name_closure *nc,
        struct ccn_charbuf *lhash,
        struct ccn_charbuf *rhash,
        struct ccn_charbuf *name)
{


    	struct ccn_indexbuf cid={0};

    	struct ccn_indexbuf *components=&cid;
    	ccn_name_split (name, components);
    	ccn_name_chop(name,components,-3);

	process_content_from_sync(name,components);

    	return(0);
}


void
sync_monitor(char *topo_prefix, char *slice_prefix)
{

    static struct ccns_name_closure nc={0};	

    nlsr->closure = &nc;
    struct ccn_charbuf *prefix = ccn_charbuf_create();
    struct ccn_charbuf *roothash = NULL;
    struct ccn_charbuf *topo = ccn_charbuf_create(); 
    nlsr->slice = ccns_slice_create();
    ccn_charbuf_reset(prefix);
    ccn_charbuf_reset(topo);
 
    ccn_charbuf_reset(prefix);
    ccn_name_from_uri(prefix, slice_prefix);
            
    ccn_charbuf_reset(topo);
    ccn_name_from_uri(topo, topo_prefix);
    

    ccns_slice_set_topo_prefix(nlsr->slice, topo, prefix);
    nlsr->closure->callback = &sync_callback;
    nlsr->ccns = ccns_open(nlsr->ccn, nlsr->slice, nlsr->closure, roothash, NULL);

}

struct ccn_charbuf *
make_template(int scope)
{
    struct ccn_charbuf *templ = NULL;
    templ = ccn_charbuf_create();
    ccn_charbuf_append_tt(templ, CCN_DTAG_Interest, CCN_DTAG);
    ccn_charbuf_append_tt(templ, CCN_DTAG_Name, CCN_DTAG);
    ccn_charbuf_append_closer(templ); /* </Name> */
    if (0 <= scope && scope <= 2)
        ccnb_tagged_putf(templ, CCN_DTAG_Scope, "%d", scope);
    ccn_charbuf_append_closer(templ); /* </Interest> */
    return(templ);
}

void
write_data_to_repo(char *data, char *name_prefix)
{

    struct ccn_charbuf *name = NULL;
    struct ccn_seqwriter *w = NULL;
    int blocksize = 1024;
    int freshness = -1;
    int torepo = 1;
    int scope = 1;
    int res;
    size_t blockread;
    struct ccn_charbuf *templ;
    
    name = ccn_charbuf_create();
    res = ccn_name_from_uri(name, name_prefix);
    if (res < 0) {
        fprintf(stderr, "bad CCN URI: %s\n",name_prefix);
        exit(1);
    }
  
    
    w = ccn_seqw_create(nlsr->ccn, name);
    if (w == NULL) {
        fprintf(stderr, "ccn_seqw_create failed\n");
        exit(1);
    }
    ccn_seqw_set_block_limits(w, blocksize, blocksize);
    if (freshness > -1)
        ccn_seqw_set_freshness(w, freshness);
    if (torepo) {
        struct ccn_charbuf *name_v = ccn_charbuf_create();
        ccn_seqw_get_name(w, name_v);
        ccn_name_from_uri(name_v, "%C1.R.sw");
        ccn_name_append_nonce(name_v);
        templ = make_template(scope);
        res = ccn_get(nlsr->ccn, name_v, templ, 60000, NULL, NULL, NULL, 0);
        ccn_charbuf_destroy(&templ);
        ccn_charbuf_destroy(&name_v);
        if (res < 0) {
            fprintf(stderr, "No response from repository\n");
            exit(1);
        }
    }




	blockread = 0;
	//struct ccn_charbuf *buf=ccn_charbuf_create();
	//get_name_lsdb_summary(buf);	
	//blockread=buf->length;

	blockread=strlen(data);

	if (blockread > 0) {
        	//res = ccn_seqw_write(w, ccn_charbuf_as_string(buf), blockread);
		res = ccn_seqw_write(w, data, blockread);	
        while (res == -1) {
            ccn_run(nlsr->ccn, 100);
            	//res = ccn_seqw_write(w, ccn_charbuf_as_string(buf), blockread);
	       res = ccn_seqw_write(w, data, blockread);
           }
    	}

	
	ccn_run(nlsr->ccn, 1);
   
    ccn_seqw_close(w);
    //ccn_charbuf_destroy(&buf);
    ccn_charbuf_destroy(&name);
}


int
create_sync_slice(char *topo_prefix, char *slice_prefix)
{
    int res;
    struct ccns_slice *slice;
    struct ccn_charbuf *prefix = ccn_charbuf_create();
    struct ccn_charbuf *topo = ccn_charbuf_create();
    struct ccn_charbuf *clause = ccn_charbuf_create();
    struct ccn_charbuf *slice_name = ccn_charbuf_create();
    struct ccn_charbuf *slice_uri = ccn_charbuf_create();
    
    if (prefix == NULL || topo == NULL || clause == NULL ||
        slice_name == NULL || slice_uri == NULL) {
        fprintf(stderr, "Unable to allocate required memory.\n");
        exit(1);
    }
    
    
    slice = ccns_slice_create();
    
    ccn_charbuf_reset(topo);
    ccn_name_from_uri(topo, topo_prefix);
    ccn_charbuf_reset(prefix);
    ccn_name_from_uri(prefix,slice_prefix );
    ccns_slice_set_topo_prefix(slice, topo, prefix);
 
  
    res = ccns_write_slice(nlsr->ccn, slice, slice_name);
  
    ccns_slice_destroy(&slice);
    ccn_charbuf_destroy(&prefix);
    ccn_charbuf_destroy(&topo);
    ccn_charbuf_destroy(&clause);
    ccn_charbuf_destroy(&slice_name);
    ccn_charbuf_destroy(&slice_uri);
	
    return 0;
}

