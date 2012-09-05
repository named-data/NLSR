#ifndef _UTILITY_H_
#define _UTILITY_H_

char * getLocalTimeStamp(void);
char * getGmTimeStamp(void);
char * nth_named_component(const char *name_prefix, int n);
long int get_current_time_sec(void);
long int get_current_time_microsec(void);

char * get_current_timestamp_micro(void);

#endif
