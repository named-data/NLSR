#ifndef _NLSR_SYNC_H_
#define _NLSR_SYNC_H_

int sync_monitor(char *topo_prefix, char *slice_prefix);
int write_data_to_repo(char *data,char *name_prefix);
int create_sync_slice(char *topo_prefix, char *slice_prefix);
void get_host_name_from_command_string(struct name_prefix *name_part,char *nbr_name_uri, int offset);
void process_content_from_sync(struct ccn_charbuf *content_name, struct ccn_indexbuf *components);

#endif
