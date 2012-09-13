#ifndef _UTILITY_H_
#define _UTILITY_H_

char * getLocalTimeStamp(void);
char * getGmTimeStamp(void);
long int get_current_time_sec(void);
void get_current_timestamp_micro(char * microSec);
long int get_time_diff(char *time1, char *time2);

#endif
