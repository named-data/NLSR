#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>
#include <time.h>
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

#include "utility.h"


char * getLocalTimeStamp(void)
{
	char *timestamp = (char *)malloc(sizeof(char) * 16);
	time_t ltime;
	ltime=time(NULL);
	struct tm *tm;
	tm=localtime(&ltime);
  
	sprintf(timestamp, "%04d%02d%02d%02d%02d%02d", tm->tm_year+1900, tm->tm_mon+1, 
		tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	return timestamp;
}

char * getGmTimeStamp(void)
{
	char *timestamp = (char *)malloc(sizeof(char) * 16);
	time_t gtime;
	gtime=time(NULL);
	struct tm *tm;
	tm=gmtime(&gtime);
  
	sprintf(timestamp, "%04d%02d%02d%02d%02d%02d", tm->tm_year+1900, tm->tm_mon+1, 
		tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	return timestamp;
}




long int 
get_current_time_sec(void)
{
	struct timeval now;
	gettimeofday(&now,NULL);
	return now.tv_sec;
}


void
get_current_timestamp_micro(char * microSec)
{
	struct timeval now; 
	gettimeofday(&now, NULL);
	sprintf(microSec,"%ld%06ld",now.tv_sec,(long int)now.tv_usec);
}


long int
get_time_diff(char *time1, char *time2)
{
	long int diff_secs;

	long int time1_in_sec,	time2_in_sec;

	char *time1_sec=(char *)malloc(strlen(time1)-6+1);
	memset(time1_sec,0,strlen(time1)-6+1);
	memcpy(time1_sec,time1,strlen(time1)-6);

	char *time2_sec=(char *)malloc(strlen(time2)-6+1);
	memset(time2_sec,0,strlen(time2)-6+1);
	memcpy(time2_sec,time2,strlen(time2)-6);

	time1_in_sec=strtol(time1_sec,NULL,10);
	time2_in_sec=strtol(time2_sec,NULL,10);

	diff_secs=time1_in_sec-time2_in_sec;

	free(time1_sec);
	free(time2_sec);

	return diff_secs;
}

