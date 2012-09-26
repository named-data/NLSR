#ifndef _UTILITY_H_
#define _UTILITY_H_

char * getLocalTimeStamp(void);
char * getGmTimeStamp(void);
long int get_current_time_sec(void);
void get_current_timestamp_micro(char * microSec);
long int get_time_diff(const char *time1, const char *time2);

void startLogging(char *loggingDir);
void writeLogg(const char *source_file, const char *function, const int line, const char *format, ...);
#endif
