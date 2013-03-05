#ifndef _UTILITY_H_
#define _UTILITY_H_

char * getLocalTimeStamp(void);
char * getGmTimeStamp(void);
long int get_current_time_sec(void);
void get_current_timestamp_micro(char * microSec);
long int get_time_diff(const char *time1, const char *time2);

void startLogging(char *loggingDir);
void writeLogg(const char *source_file, const char *function, const int line, const char *format, ...);
struct sockaddr_in * get_ip_from_hostname(char *hostname);
int get_ip_from_hostname_02(char * hostname , char* ip);
char * get_current_timestamp_micro_v2();

int add_ccn_uri_name(struct ccn_charbuf *res_name, struct ccn_charbuf *add);

#endif
