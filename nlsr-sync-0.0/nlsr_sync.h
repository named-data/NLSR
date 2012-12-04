#ifndef _NLSR_SYNC_H_
#define _NLSR_SYNC_H_

void sync_monitor(char *topo_prefix, char *slice_prefix);
void write_data_to_repo(char *data,char *name_prefix);
int create_sync_slice(char *topo_prefix, char *slice_prefix);

#endif
