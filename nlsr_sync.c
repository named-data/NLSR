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


	char *
hex_string(unsigned char *s, size_t l)
{
	const char *hex_digits = "0123456789abcdef";
	char *r;
	int i;
	r = calloc(1, 1 + 2 * l);
	for (i = 0; i < l; i++) {
		r[2*i] = hex_digits[(s[i]>>4) & 0xf];
		r[1+2*i] = hex_digits[s[i] & 0xf];
	}
	return(r);
}

	int
sync_cb(struct ccns_name_closure *nc,
		struct ccn_charbuf *lhash,
		struct ccn_charbuf *rhash,
		struct ccn_charbuf *name)
{
	int res;
	struct ccn_charbuf *content_name; 
	struct ccn_indexbuf *content_comps;
	struct ccn_indexbuf *name_comps;
	
	content_comps = ccn_indexbuf_create();
	res = ccn_name_split(name, content_comps);
	if ( res < 0 )
		return 0;
	
	if (content_comps->n < 2)
		return 0;

	content_name = ccn_charbuf_create();
	ccn_name_init(content_name);
	
	res = ccn_name_append_components( content_name,	name->buf,
			content_comps->buf[0], content_comps->buf[content_comps->n - 1]);

	if ( res < 0)
		return 0;

	// for debugging
	struct ccn_charbuf *temp=ccn_charbuf_create();
	ccn_uri_append(temp, content_name->buf, content_name->length, 0);
	if ( nlsr->debugging )
		printf("Name before chopping: %s \n",ccn_charbuf_as_string(temp));
	ccn_charbuf_destroy(&temp);

	name_comps = ccn_indexbuf_create();
	res=ccn_name_split (content_name, name_comps);
	if (res < 0)
		return 0;		

	if ( nlsr->debugging )
	{
		printf("Number of components in name = %d \n",res);
		printf("Number of components in name as indexbuf->n = %d \n",
				(int)name_comps->n);
	}

	ccn_name_chop(content_name, name_comps, -3);
	if ( nlsr->debugging )
		printf("Number of components in name as indexbuf->n after chopping= %d \n"
				, (int)name_comps->n);	

	//for debugging 
	struct ccn_charbuf *temp1=ccn_charbuf_create();
	ccn_uri_append(temp1, content_name->buf, content_name->length, 0);
	if ( nlsr->debugging )
		printf("Name after chopping: %s \n",ccn_charbuf_as_string(temp1));
	ccn_charbuf_destroy(&temp1);

	//main method that processes contents from the sync
	process_content_from_sync(content_name, name_comps);
	
	ccn_charbuf_destroy(&content_name);
	ccn_indexbuf_destroy(&content_comps);
	ccn_indexbuf_destroy(&name_comps);

	return(0);
}



	void
get_name_part(struct name_prefix *name_part,struct ccn_charbuf * interest_ccnb, 
		struct ccn_indexbuf *interest_comps, int offset)
{
	//int i;
	int lsa_position=0;
	//int len=0;

	struct ccn_indexbuf cid={0};
	struct ccn_indexbuf *components=&cid;
	struct ccn_charbuf *name=ccn_charbuf_create();
	ccn_name_from_uri(name,nlsr->slice_prefix);
	ccn_name_split (name, components);
	lsa_position=components->n-2;
	ccn_charbuf_destroy(&name);

	struct ccn_charbuf *temp=ccn_charbuf_create();
	ccn_name_init(temp);	
	ccn_name_append_components( temp,	interest_ccnb->buf,
			interest_comps->buf[lsa_position+1], interest_comps->buf[interest_comps->n - 1]);

	struct ccn_charbuf *temp1=ccn_charbuf_create();
	ccn_uri_append(temp1, temp->buf, temp->length, 0);

	name_part->name=(char *)calloc(strlen(ccn_charbuf_as_string(temp1))+1,sizeof(char));
	memcpy(name_part->name,ccn_charbuf_as_string(temp1),strlen(ccn_charbuf_as_string(temp1)));
	name_part->name[strlen(ccn_charbuf_as_string(temp1))]='\0';
	name_part->length=strlen(ccn_charbuf_as_string(temp1))+1;

	ccn_charbuf_destroy(&temp1);
	ccn_charbuf_destroy(&temp);

	if ( nlsr->debugging )
		printf("Name Part: %s \n",name_part->name);
	

	/*
	const unsigned char *comp_ptr1;
	size_t comp_size;
	for(i=lsa_position+1+offset;i<interest_comps->n-1;i++)
	{
		ccn_name_comp_get(interest_ccnb->buf, interest_comps,i,&comp_ptr1, 
				&comp_size);
		len+=1;
		len+=(int)comp_size;	
	}
	len++;

	char *neighbor=(char *)malloc(len);
	memset(neighbor,0,len);

	for(i=lsa_position+1+offset; i<interest_comps->n-1;i++)
	{
		ccn_name_comp_get(interest_ccnb->buf, interest_comps,i,&comp_ptr1, 
				&comp_size);
		memcpy(neighbor+strlen(neighbor),"/",1);
		memcpy(neighbor+strlen(neighbor),(char *)comp_ptr1,
				strlen((char *)comp_ptr1));

	}

	name_part->name=(char *)malloc(strlen(neighbor)+1);
	memset(name_part->name,0,strlen(neighbor)+1);
	memcpy(name_part->name,neighbor,strlen(neighbor)+1);
	name_part->length=strlen(neighbor)+1;

	// Add 01/31/2013
	free(neighbor);
	*/
}





	void 
get_content_by_content_name(char *content_name, unsigned char **content_data)
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
			ccnb_append_tagged_blob(templ, CCN_DTAG_InterestLifetime, buf, 
					sizeof(buf));
		}
		ccn_charbuf_append_closer(templ); /* </Interest> */
	}
	resultbuf = ccn_charbuf_create();
	if (resolve_version != 0) {
		res = ccn_resolve_version(nlsr->ccn, name, resolve_version, 500);
		if (res >= 0) {
			ccn_uri_append(resultbuf, name->buf, name->length, 1);
			resultbuf->length = 0;
		}
	}
	res = ccn_get(nlsr->ccn, name, templ, timeout_ms, resultbuf, &pcobuf, NULL, 
			get_flags);
	if (res >= 0) {
		ptr = resultbuf->buf;
		length = resultbuf->length;
		if (content_only){
			ccn_content_get_value(ptr, length, &pcobuf, &ptr, &length);
			*content_data = (unsigned char *) calloc(length, sizeof(char *));
			memcpy (*content_data, ptr, length);
		}
	}
	ccn_charbuf_destroy(&resultbuf);
	ccn_charbuf_destroy(&templ);
	ccn_charbuf_destroy(&name);   
}

	void 
process_incoming_sync_content_lsa( unsigned char *content_data)
{


	if ( nlsr->debugging )
		printf("process_incoming_sync_content_lsa called \n");	

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

	if( strlen((char *)content_data ) > 0 )
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
				printf("	Orig Time   : %s\n",orig_time);
				printf("	No Link  : %d\n",no_link);
				printf("	Data  : %s\n",data);
			}
			build_and_install_others_adj_lsa(orig_router,ls_type,orig_time,no_link,data);
		}
		else if ( ls_type == LS_TYPE_COR )
		{
			orig_time=strtok_r(NULL,sep,&rem);
			char *cor_r=strtok_r(NULL,sep,&rem);
			char *cor_theta=strtok_r(NULL,sep,&rem);

			double r, theta;
			r=strtod(cor_r,NULL);
			theta=strtod(cor_theta,NULL);

			if ( nlsr->debugging )
			{
				printf("	Orig Time   : %s\n",orig_time);
				printf("	Cor R	    : %f\n",r);
				printf("	Cor Theta   : %f\n",theta);
			}
			build_and_install_others_cor_lsa(orig_router,ls_type,orig_time, (double)r, (double)theta);	
		}

	}
}

	void
process_content_from_sync (struct ccn_charbuf *content_name, 
								struct ccn_indexbuf *components)
{
	if (nlsr->debugging)
		printf("process_content_from_sync called \n");
	size_t comp_size;
	char *lst;
	char *lsid;
	const unsigned char *second_last_comp;
	const unsigned char *third_last_comp;
	const unsigned char *origtime;
	char *sep=".";
	char *rem;
	char *second_comp_type;

	int ls_type;
	long int ls_id=0;

	unsigned char *content_data = NULL;

	char *time_stamp=get_current_timestamp_micro_v2();

	struct ccn_charbuf *uri = ccn_charbuf_create();
	ccn_uri_append(uri, content_name->buf, content_name->length, 0);	

	struct name_prefix *orig_router=(struct name_prefix *)
								calloc( 1, sizeof(struct name_prefix));

	ccn_name_comp_get( content_name->buf, components, 
					  components->n-1-2, &second_last_comp, &comp_size);
	
	if (nlsr->debugging)
		printf("2nd Last Component: %s \n", second_last_comp);

	second_comp_type=strtok_r((char *)second_last_comp, sep, &rem);
	if (second_comp_type == NULL || rem == NULL)
	{
		printf ("Error: unable to tokenize the string: %s, calling exit()\n",
													(char *)second_last_comp); 
		exit(0);
	}

	if ( strcmp( second_comp_type, "lsId" ) == 0 )
	{	
		lsid=rem;
		ls_id=atoi(rem);
		ccn_name_comp_get(content_name->buf, components,components->n-2-2,&third_last_comp, &comp_size);
		lst=strtok_r((char *)third_last_comp,sep,&rem);
		lst=rem;
		ls_type=atoi(lst);
		ccn_name_comp_get(content_name->buf, components,components->n-2,&origtime, &comp_size);
		ccn_name_chop(content_name,components,-3);
		get_name_part(orig_router,content_name,components,0);

		if ( nlsr->debugging )
			printf("Orig Router: %s Ls Type: %d Ls id: %ld Orig Time: %s\n",orig_router->name,ls_type,ls_id,origtime);

		int lsa_life_time=get_time_diff(time_stamp,(char *)origtime);
		if ( nlsr->debugging )
			printf("LSA Life time: %d\n",lsa_life_time);

		if ( (strcmp(orig_router->name,nlsr->router_name) == 0 && lsa_life_time < nlsr->lsa_refresh_time) 
				|| (strcmp(orig_router->name,nlsr->router_name) != 0 && lsa_life_time < nlsr->router_dead_interval) )
		{
			int is_new_name_lsa=check_is_new_name_lsa(orig_router->name,(char *)lst,(char *)lsid,(char *)origtime);
			if ( is_new_name_lsa == 1 )
			{
				if ( nlsr->debugging )
					printf("New NAME LSA.....\n");	
				get_content_by_content_name(ccn_charbuf_as_string(uri), &content_data);
				if ( nlsr->debugging )
					printf("Content Data: %s \n",content_data);
				process_incoming_sync_content_lsa(content_data);
			}
			else 
			{
				if ( nlsr->debugging )
					printf("Name LSA / Newer Name LSA already xists in LSDB\n");
				get_content_by_content_name(ccn_charbuf_as_string(uri), &content_data);

				if ( nlsr->debugging )
					printf("Content Data: %s \n",content_data);
			}
		}
		else 
		{
			if ( nlsr->debugging )
				printf("Lsa is older than Router LSA refresh time/ Dead Interval\n");
		}
	}
	else
	{
		ls_type=atoi(rem);
		lst=rem;
		if(ls_type == LS_TYPE_ADJ)
		{
			ccn_name_comp_get(content_name->buf, components,components->n-2, 
														&origtime, &comp_size);
			ccn_name_chop(content_name,components,-2);
			get_name_part(orig_router,content_name,components,0);

			if ( nlsr->debugging )
				printf("Orig Router: %s Ls Type: %d Orig Time: %s\n",
											orig_router->name,ls_type,origtime);

			int lsa_life_time=get_time_diff(time_stamp,(char *)origtime);
			if ( nlsr->debugging )
				printf("LSA Life time: %d\n",lsa_life_time);

			if ( (strcmp(orig_router->name,nlsr->router_name) == 0 && 
							lsa_life_time < nlsr->lsa_refresh_time) || 
							(strcmp(orig_router->name,nlsr->router_name) != 0 && 
							 lsa_life_time < nlsr->router_dead_interval) )	
			{
				int is_new_adj_lsa = check_is_new_adj_lsa( orig_router->name, 
													(char *)lst, (char *)origtime);
				if ( is_new_adj_lsa == 1 )
				{
					if ( nlsr->debugging )
						printf("New Adj LSA.....\n");	
					get_content_by_content_name(ccn_charbuf_as_string(uri), &content_data);

					if ( nlsr->debugging )
						printf("Content Data: %s \n",content_data);
					process_incoming_sync_content_lsa(content_data);			
				}
				else
				{
					if ( nlsr->debugging )
						printf("Adj LSA / Newer Adj LSA already exists in LSDB\n");
					get_content_by_content_name(ccn_charbuf_as_string(uri), &content_data);
					if ( nlsr->debugging )
						printf("Content Data: %s \n",content_data);
				}
			}
			else 
			{
				if ( nlsr->debugging )
					printf("Lsa is older than Router LSA refresh time/ Dead Interval\n");
			}
		}
		else if(ls_type == LS_TYPE_COR)
		{
			ccn_name_comp_get(content_name->buf, components, components->n-2, 
														&origtime, &comp_size);
			ccn_name_chop(content_name,components,-2);
			get_name_part(orig_router,content_name,components,0);

			if ( nlsr->debugging )
				printf("Orig Router: %s Ls Type: %d Orig Time: %s\n", 
											orig_router->name,ls_type,origtime);

			int lsa_life_time=get_time_diff(time_stamp,(char *)origtime);
			if ( nlsr->debugging )
				printf("LSA Life time: %d\n",lsa_life_time);

			if ( (strcmp(orig_router->name,nlsr->router_name) == 0 && 
							lsa_life_time < nlsr->lsa_refresh_time) || 
							(strcmp(orig_router->name,nlsr->router_name) != 0 && 
							 lsa_life_time < nlsr->router_dead_interval) )	
			{
				int is_new_cor_lsa=check_is_new_cor_lsa( orig_router->name, 
												(char *)lst,(char *) origtime);
				if ( is_new_cor_lsa == 1 )
				{
					if ( nlsr->debugging )
						printf("New Cor LSA.....\n");	
					get_content_by_content_name(ccn_charbuf_as_string(uri), 
																&content_data);

					if ( nlsr->debugging )
						printf("Content Data: %s \n",content_data);
					process_incoming_sync_content_lsa(content_data);			
				}
				else
				{
					if ( nlsr->debugging )
						printf("Cor LSA / Newer Cor LSA already exists in LSDB\n");
					get_content_by_content_name(ccn_charbuf_as_string(uri), &content_data);
					if ( nlsr->debugging )
						printf("Content Data: %s \n",content_data);
				}
			}
			else 
			{	
				if ( nlsr->debugging )
					printf("Lsa is older than Router LSA refresh time/ Dead Interval\n");
			}

		}
	}

	if (orig_router->name)
		free(orig_router->name);
	if (orig_router)
		free(orig_router);
	ccn_charbuf_destroy(&uri);
	//01/31/2013	
	free(time_stamp);
}

	int
sync_monitor(char *topo_prefix, char *slice_prefix)
{

	struct ccn_charbuf *prefix = ccn_charbuf_create();
	struct ccn_charbuf *topo = ccn_charbuf_create(); 
	
	nlsr->closure=(struct ccns_name_closure *) 
						calloc(1,sizeof(struct ccns_name_closure)); // leak

	nlsr->slice = ccns_slice_create();

	ccn_charbuf_reset(prefix);
	ccn_name_from_uri(prefix, slice_prefix);

	ccn_charbuf_reset(topo);
	ccn_name_from_uri(topo, topo_prefix);

	ccns_slice_set_topo_prefix(nlsr->slice, topo, prefix);
	nlsr->closure->callback = &sync_cb;
	nlsr->ccns = ccns_open(nlsr->ccn, nlsr->slice, nlsr->closure, NULL, NULL);

	ccn_charbuf_destroy(&prefix);
	ccn_charbuf_destroy(&topo);
	return 0;
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

	int
write_data_to_repo(char *data, char *name_prefix)
{
	if ( nlsr->debugging )
	{
		printf("write_data_to_repo called\n");
		printf("Content Name: %s \n",name_prefix);
		printf("Content Data: %s \n",data);
	}

	struct ccn *temp_ccn;
	temp_ccn=ccn_create();
	int ccn_fd=ccn_connect(temp_ccn, NULL);
	if(ccn_fd == -1)
	{
		fprintf(stderr,"Could not connect to ccnd for Data Writing\n");
		writeLogg(__FILE__,__FUNCTION__,__LINE__, 
						"Could not connect to ccnd for Data Writing\n");
		return -1;
	}

	struct ccn_charbuf *name = NULL;
	struct ccn_seqwriter *w = NULL;
	int blocksize = 4096;
	int freshness = -1;
	int scope = 1;
	int res;
	size_t blockread;
	struct ccn_charbuf *templ;

	name = ccn_charbuf_create();
	res = ccn_name_from_uri(name, name_prefix);
	if (res < 0) {
		fprintf(stderr, "bad CCN URI: %s\n",name_prefix);
		return -1;
	}

	w = ccn_seqw_create(temp_ccn, name);
	if (w == NULL) {
		fprintf(stderr, "ccn_seqw_create failed\n");
		return -1;
	}
	ccn_seqw_set_block_limits(w, blocksize, blocksize);
	if (freshness > -1)
		ccn_seqw_set_freshness(w, freshness);

	struct ccn_charbuf *name_v = ccn_charbuf_create();
	ccn_seqw_get_name(w, name_v);
	ccn_name_from_uri(name_v, "%C1.R.sw");
	ccn_name_append_nonce(name_v);
	templ = make_template(scope);
	res = ccn_get(temp_ccn, name_v, templ, 60000, NULL, NULL, NULL, 0);
	ccn_charbuf_destroy(&templ);
	ccn_charbuf_destroy(&name_v);
	if (res < 0) {
		fprintf(stderr, "No response from repository\n");
		return -1;
	}

	blockread = 0;

	blockread=strlen(data)+1;

	if (blockread > 0) {
		res = ccn_seqw_write(w, data, blockread);	
		while (res == -1) {
			ccn_run(temp_ccn,100);
			res = ccn_seqw_write(w, data, blockread);
		}
	}

	ccn_seqw_close(w);
	ccn_run(temp_ccn, 100);
	ccn_charbuf_destroy(&name);
	ccn_destroy(&temp_ccn);

	return 0;
}
	int
	create_sync_slice(char *topo_prefix, char *slice_prefix)
{
	int res;
	struct ccn *handle; //obaid: probably we don't need it use the same handle i.e. nlsr->ccn
	struct ccns_slice *slice;
	struct ccn_charbuf *prefix = ccn_charbuf_create();
	struct ccn_charbuf *topo = ccn_charbuf_create();
	struct ccn_charbuf *clause = ccn_charbuf_create();
	struct ccn_charbuf *slice_name = ccn_charbuf_create();
	struct ccn_charbuf *slice_uri = ccn_charbuf_create();

	if (prefix == NULL || topo == NULL || clause == NULL ||
			slice_name == NULL || slice_uri == NULL) {
		fprintf(stderr, "Unable to allocate required memory.\n");
		return -1;
	}

	handle = ccn_create();
	res = ccn_connect(handle, NULL);
	if (0 > res) {
		fprintf(stderr, "Unable to connect to ccnd.\n");
		return -1;
	}    

	slice = ccns_slice_create();

	ccn_charbuf_reset(topo);
	ccn_name_from_uri(topo, topo_prefix);
	ccn_charbuf_reset(prefix);
	ccn_name_from_uri(prefix,slice_prefix );
	ccns_slice_set_topo_prefix(slice, topo, prefix);


	res = ccns_write_slice(handle, slice, slice_name);
	//res = ccns_write_slice(nlsr->ccn, slice, slice_name);

	//01/31/2013
	ccns_slice_destroy(&slice);
	ccn_destroy(&handle);
	ccn_charbuf_destroy(&prefix);
	ccn_charbuf_destroy(&topo);
	ccn_charbuf_destroy(&clause);
	ccn_charbuf_destroy(&slice_name);
	ccn_charbuf_destroy(&slice_uri);

	return 0;
}


